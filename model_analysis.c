/* $Id: model_analysis.c,v 1.33 2002/04/22 09:21:39 dsanta Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   Submitted to ICME 2002, available on http://mesh.epfl.ch
 *
 */

#include <model_analysis.h>

#include <assert.h>
#include <xalloc.h>

#ifdef INLINE
# error Name clash with INLINE macro
#endif

#if defined (_MSC_VER)
# define INLINE __inline
#elif defined (__INTEL_COMPILER) && (__INTEL_COMPILER >= 500)
# define INLINE __inline
#elif defined (__GNUC__)
# define INLINE __inline__
#elif defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define INLINE inline
#else 
# define INLINE
#endif

/* Give hints for more optimization */
#if defined(__GNUC__) && (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define BMAP_CALLOC_ATTR __attribute__ ((__malloc__))
#else
#define BMAP_CALLOC_ATTR
#endif

/* Type for bitmaps */
typedef unsigned int bmap_t;

/* A stack for arbitrary objects */
struct stack {
  char *data;       /* the buffer to store the data */
  char *end;        /* the position just past the end of the data buffer */
  char *next;       /* the position where to store the next element */
  size_t elem_sz;   /* the element size, in bytes */
};

/* A list of vertices */
struct vtx_list {
  int *vtcs;    /* The list of vertex indices */
  int n_elems;  /* The number of elements in the list */
};

/* Structure to hold topology info */
struct topology {
  int manifold; /* is manifold */
  int closed;   /* is closed */
};

/* Structure to hold the list of extra faces adjacent to a face */
struct adj_faces_x {
  int *faces_on_edge[3];  /* list of adjacent faces for each edge */
  int n_faces_on_edge[3]; /* number of elements in above lists. */
};

/* Structure to hold the list of faces adjacent to a face */
struct adj_faces {
  int face_on_edge[3];       /* index of the face adjacent on the f0-f1, f1-f2
                              * and f2-f0 edges. -1 if none. */
  struct adj_faces_x *extra; /* The additional adjacent faces for each of the
                              * above edges, if any. NULL if none. Only
                              * non-manifold models have more than one
                              * adjacent face at any edge. */
};

/* --------------------------------------------------------------------------*
 *                                  Macros                                   *
 * --------------------------------------------------------------------------*/

/* We need a power of two bits in char (well, that would a very weird machine
 * but better to check that to see the program fail with no messages). */
#if (CHAR_BIT != 8 && CHAR_BIT != 16 && CHAR_BIT != 32 && CHAR_BIT != 64)
#error CHAR_BIT is not a power of two
#endif
/* The number of bits in bitmap_t */
#define BMAP_T_BITS (sizeof(bmap_t)*CHAR_BIT)
/* The bitmask to obtain the bit position within bmap_t */
#define BMAP_T_MASK (BMAP_T_BITS-1)
/* Gets the value (0 or 1) of the nth bit in the bmap bitmap */
#define BMAP_ISSET(bmap,n) (((bmap)[n/BMAP_T_BITS] >> ((n)&BMAP_T_MASK)) & 1)
/* Sets (to 1) the nth bit in the bmap bitmap */
#define BMAP_SET(bmap,n) ((bmap)[n/BMAP_T_BITS] |= 1 << ((n)&BMAP_T_MASK))

/* --------------------------------------------------------------------------*
 *                            Utility functions                              *
 * --------------------------------------------------------------------------*/

/* Allocates a bitmap of size sz bits, initialized to zero (i.e. all bits
 * cleared). The storage can be freed by calling free on the returned
 * pointer. */
static bmap_t * bmap_calloc(size_t sz) BMAP_CALLOC_ATTR;
static bmap_t * bmap_calloc(size_t sz)
{
  return xa_calloc((sz+BMAP_T_BITS-1)/BMAP_T_BITS,sizeof(bmap_t));
}

/* Initializes the stack s for storing elements of elem_sz bytes. */
static void stack_init(struct stack *s, size_t elem_sz)
{
  size_t buf_sz;
  assert(elem_sz > 0);
  s->elem_sz = elem_sz;
  buf_sz = 16350*elem_sz;
  s->data = xa_malloc(buf_sz);
  s->end = s->data+buf_sz;
  s->next = s->data;
}

/* Frees allocated storage for stack s */
static void stack_fini(struct stack *s)
{
  free(s->data);
  s->data = NULL;
  s->end = NULL;
  s->next = NULL;
}

/* Grows the stack buffer. Currently grows by 16384 elements. */
static void stack_grow_buf(struct stack *s)
{
  size_t buf_sz;
  ptrdiff_t next_off;

  buf_sz = (s->end-s->data)+16384*s->elem_sz;
  next_off = s->next-s->data;
  s->data = xa_realloc(s->data,buf_sz);
  s->end = s->data+buf_sz;
  s->next = s->data+next_off;
}

/* Pushes the element pointed by *e and of the size specifed at stack
 * initialization into the stack s. */
static INLINE void stack_push(struct stack *s, void *e)
{
  assert(s->next <= s->end);
  if (s->next == s->end) stack_grow_buf(s);
  memcpy(s->next,e,s->elem_sz);
  s->next += s->elem_sz;
  assert(s->next <= s->end);
}

/* If the stack is not empty, it pops the last element of the stack and stores
 * it the location pointed by e and of the size specified at stack
 * initialization, and returns one. If the stack is empty it returns zero. */
static INLINE int stack_pop(struct stack *s, void *e)
{
  assert(s->next <= s->end);
  if (s->next == s->data) {
    return 0; /* stack empty */
  } else {
    s->next -= s->elem_sz;
    assert(s->next <= s->end);
    memcpy(e,s->next,s->elem_sz);
    return 1;
  }
}

/* --------------------------------------------------------------------------*
 *                         Model analysis functions                          *
 * --------------------------------------------------------------------------*/

/* Given the face orientation map face_rev_orientation, orients the model
 * m. The map face_rev_orientation indicates which faces should have their
 * orientation reversed. */
static void orient_model(struct model *m, const bmap_t *face_rev_orientation)
{
  int i,imax;
  int tmpi;

  for (i=0, imax=m->num_faces; i<imax; i++) {
    if (BMAP_ISSET(face_rev_orientation,i)) { /* revert orientation */
      tmpi = m->faces[i].f0;
      m->faces[i].f0 = m->faces[i].f1;
      m->faces[i].f1 = tmpi;
    }
  }
}

/* Grows the storage space for list->vtcs and adjusts *sz accordingly. */
static void vtx_list_grow(struct vtx_list *list, int *sz)
{
  *sz += 2;
  list->vtcs = xa_realloc(list->vtcs,*sz*sizeof(*(list->vtcs)));
}

/* Searches for vertex index v in list. If not found v is added to
 * list. Returns zero if not-found and non-zero otherwise. list->vtcs storage
 * must be large enough for v to be added. The current size of the list buffer
 * is given in *sz and adjusted if necessary. */
static INLINE int vtx_in_list_or_add(struct vtx_list *list, int v, int *sz)
{
  int i;
  i = 0;
  while (i < list->n_elems) {
    if (list->vtcs[i++] == v) return 1; /* already in list */
  }
  /* not in list */
  if (*sz == list->n_elems) vtx_list_grow(list,sz);
  list->vtcs[list->n_elems++] = v;
  return 0;
}

/* Returns the indices of the vertices if face, that are not center_vidx, in
 * *start_vidx and *edge_vidx. The order is so that center_vidx, *start_vidx
 * and *edge_vidx are in the order f0,f1,f2, upto a cyclic permuation. Note
 * that *face must be incident on the center_vidx vertex. This is used to
 * obtain the starting vertices to do an oriented walk of the vertices around
 * center_vidx. */
static INLINE void get_ordered_vtcs(const face_t *face, int center_vidx,
                                    int *start_vidx, int *edge_vidx)
{
  if (center_vidx == face->f0) {
    *start_vidx = face->f1;
    *edge_vidx = face->f2;
  } else if (center_vidx == face->f1) {
    *start_vidx = face->f2;
    *edge_vidx = face->f0;
  } else { /* vidx == face->f2 */
    assert(center_vidx == face->f2);
    *start_vidx = face->f0;
    *edge_vidx = face->f1;
  }
}

/* Given the list, vfaces, of faces incident on the vertex center_vidx and not
 * yet visited, it finds the first face that has the edge from center_vidx to
 * *outer_vidx. The length of the vfaces list is *vfaces_len. The return value
 * is the index of the face found, or -1 if there is none. The entry in vfaces
 * corresponding to the face found is removed from *vfaces and *vfaces_len is
 * adjusted accordingly. The vertex of the found face that is not on the
 * specifed edge is returned in *outer_vidx. The faces of the model are given
 * in the mfaces array. */
static INLINE int find_face_with_edge(const face_t *mfaces, int *vfaces,
                                      int *vfaces_len, int center_vidx,
                                      int *outer_vidx)
{
  int i,maxi,fidx;
  int out_vidx;
  const face_t *face;
  
  /* when we find one at i we exchange i with last and remove last */
  out_vidx = *outer_vidx;
  for (i=0, maxi=*vfaces_len; i<maxi; i++) {
    fidx = vfaces[i];
    face = &mfaces[fidx];
    if (out_vidx == face->f0) {
      *outer_vidx = (center_vidx == face->f2) ? face->f1 : face->f2;
      vfaces[i] = vfaces[--(*vfaces_len)];
      return fidx;
    } else if (out_vidx == face->f1) {
      *outer_vidx = (center_vidx == face->f0) ? face->f2 : face->f0;
      vfaces[i] = vfaces[--(*vfaces_len)];
      return fidx;
    } else if (out_vidx == face->f2) {
      *outer_vidx = (center_vidx == face->f1) ? face->f0 : face->f1;
      vfaces[i] = vfaces[--(*vfaces_len)];
      return fidx;
    }
    /* not an adjacent face => continue searching */
  }
  return -1; /* no adjacent face found */
}

/* Reverses the order of the elements of list, of length len. */
static INLINE void reverse_list(int *list, int len)
{
  int j,l2,tmpi;

  l2 = len/2;
  for (j=0; j<l2; j++) {
    tmpi = list[j];
    list[j] = list[len-1-j];
    list[len-1-j] = tmpi;
  }
}

/* Performs local analysis of the faces incident on vertex vidx. The faces of
 * the model are given in the mfaces array. The list of faces incident on vidx
 * is given by *flist. The local topology information is returned in
 * *ltop. The list of incident faces in *flist is reordered, so that for
 * manfifold vertices two consecutive entries are adjacent faces (in addition
 * the last anf first ones are adjacent if the vertex is closed). For
 * non-manifold vertices no special guarantees can be made on the resulting
 * order. In addition it constructs the list of vertices, different from vidx
 * and without repetition, that belong to the faces incident on vidx in
 * *vlist. */
static void get_vertex_topology(const face_t *mfaces, int vidx,
                                const struct face_list *flist,
                                struct topology *ltop,
                                struct vtx_list *vlist)
{
  int nf;          /* number of faces incident on current vertex */
  int fidx;        /* current face index */
  int v2;          /* vertex on which next face should be incident */
  int vstart;      /* starting vertex, to check for closed surface */
  int rev_orient;  /* reversed processing orientation flag */
  int *vfaces;     /* list of faces incident on current vertex */
  int n_vfaces;    /* number of faces in vfaces */
  int v2_was_in_list; /* flag: v2 already visited when last encountered */
  int vstart_was_in_list; /* same as above but for vstart */
  int vtx_buf_sz;  /* size of the vertex list buffer */

  /* Initialize */
  ltop->manifold = 1;
  ltop->closed = 1;
  vlist->n_elems = 0;
  nf = flist->n_faces;
  if (nf == 0) { /* isolated vertex => nothing to be done */
    vlist->vtcs = NULL;
    return;
  }
  n_vfaces = nf-1;
  fidx = flist->face[n_vfaces];
  vfaces = xa_malloc(sizeof(*(vfaces))*n_vfaces);
  memcpy(vfaces,flist->face,sizeof(*(vfaces))*n_vfaces);
  /* do typical size allocation */
  vtx_buf_sz = nf;
  vlist->vtcs = xa_realloc(vlist->vtcs,sizeof(*(vlist->vtcs))*vtx_buf_sz);
  /* Get first face vertices */
  assert(fidx>=0);
  get_ordered_vtcs(&mfaces[fidx],vidx,&vstart,&v2);
  /* Check the other faces in order */
  rev_orient = 0;
  vstart_was_in_list = 0; /* list empty, so vstart always added */
  vtx_in_list_or_add(vlist,vstart,&vtx_buf_sz);
  v2_was_in_list = vtx_in_list_or_add(vlist,v2,&vtx_buf_sz);
  while (n_vfaces > 0) { /* process the remaining faces */
    fidx = find_face_with_edge(mfaces,vfaces,&n_vfaces,vidx,&v2);
    if (fidx >= 0) { /* found an adjacent face */
      flist->face[n_vfaces] = fidx;
      v2_was_in_list = vtx_in_list_or_add(vlist,v2,&vtx_buf_sz);
      if (v2_was_in_list) {
        if (v2 == vstart) vstart_was_in_list = 1; /* handle duplicate */
        /* v2 appearing twice is only addmissible in a manifold if it is the
         * last face and it closes a disk (and of course we didn't already
         * detect as non-manifold) */
        if (!(n_vfaces == 0 && v2 == vstart)) ltop->manifold = 0;
      }
    } else { /* no of the not yet visited faces share's v2! */
      /* vertex vidx can be at a border or there is really a non-manifold */
      if (rev_orient || v2 == vstart) {
        /* already reversed orientation or completed a disk => non-manifold */
        ltop->manifold = 0;
        /* closedness is not affected if we just closed a cycle or if vstart
         * and v2 were already visited the last time they were tested. */
        if (!(v2 == vstart || (vstart_was_in_list && v2_was_in_list))) {
          ltop->closed = 0;
        }
        /* Restart with first not yet counted triangle (always one) */
        assert(n_vfaces > 0);
        fidx = vfaces[--n_vfaces];
        flist->face[n_vfaces] = fidx;
        assert(fidx >= 0);
        rev_orient = 0; /* restore original orientation */
        /* Get new first face vertices */
        get_ordered_vtcs(&mfaces[fidx],vidx,&vstart,&v2);
        vstart_was_in_list = vtx_in_list_or_add(vlist,vstart,&vtx_buf_sz);
        v2_was_in_list = vtx_in_list_or_add(vlist,v2,&vtx_buf_sz);
      } else {
        /* we reverse scanning orientation and continue from the other side */
        int tmpi;
        rev_orient = 1;
        if (ltop->manifold) { /* if non-manifold order becomes useless */
          reverse_list(flist->face+n_vfaces,flist->n_faces-n_vfaces);
        }
        tmpi = v2;
        v2 = vstart;
        vstart = tmpi;
        tmpi = v2_was_in_list;
        v2_was_in_list = vstart_was_in_list;
        vstart_was_in_list = tmpi;
      }
    }
  }
  /* closedness is not affected if we just closed a cycle or if vstart and v2
   * were already visited the last time they were tested. */
  if (!(v2 == vstart || (vstart_was_in_list && v2_was_in_list))) {
    ltop->closed = 0;
  }
  free(vfaces);
}

/* Adds the face_idx to the list of extra adjacent faces *af, as adjacent on
 * the edge edge_idx. */
static void add_adj_face_extra(struct adj_faces *af, int edge_idx, int face_idx)
{
  int j;

  if (af->extra == NULL) { /* no extra lists for no edge yet */
    af->extra = xa_calloc(1,sizeof(*(af->extra)));
  }
  j = (af->extra->n_faces_on_edge[edge_idx])++;
  af->extra->faces_on_edge[edge_idx] =
    xa_realloc(af->extra->faces_on_edge[edge_idx],
               sizeof(*(af->extra->faces_on_edge[0]))*(j+1));
  af->extra->faces_on_edge[edge_idx][j] = face_idx;
  
}

/* Adds the face_idx to the list of adjacent faces *af, as adjacent on the
 * edge edge_idx. */
static INLINE void add_adj_face(struct adj_faces *af, int edge_idx,
                                int face_idx)
{
  if (af->face_on_edge[edge_idx] < 0) {
    af->face_on_edge[edge_idx] = face_idx;
  } else {
    add_adj_face_extra(af,edge_idx,face_idx);
  }
}

/* Frees all the storage array of lists of adjacent faces, of length n_faces. */
static void free_adj_face_list(struct adj_faces *aflist, int n_faces)
{
  int i;

  for (i=0; i<n_faces; i++) {
    if (aflist[i].extra != NULL) {
      free(aflist[i].extra->faces_on_edge[0]);
      free(aflist[i].extra->faces_on_edge[1]);
      free(aflist[i].extra->faces_on_edge[2]);
      free(aflist[i].extra);
    }
  }
  free(aflist);
}

/* Builds and returns the list of adjacent faces for each face in the
 * model. n_faces is the number of faces, mfaces the array of faces of the
 * model, and flist the list of faces incident on each vertex. manifold_vtcs
 * flags, for each vertex, if it is manifold or not. Degenerate faces are
 * skipped. The returned array is of length n_faces. */
static struct adj_faces * find_adjacent_faces(const face_t *mfaces, int n_faces,
                                              const struct face_list *flist,
                                              const bmap_t *manifold_vtcs)
{
  struct adj_faces *aflist;
  int k,i,imax;
  int f0,f1,f2;
  int adj_fidx;
  const struct face_list *faces_at_vtx;

  aflist = xa_malloc(n_faces*sizeof(*aflist));
  for (k=0; k<n_faces; k++) {
    aflist[k].face_on_edge[0] = -1;
    aflist[k].face_on_edge[1] = -1;
    aflist[k].face_on_edge[2] = -1;
    aflist[k].extra = NULL;

    f0 = mfaces[k].f0;
    f1 = mfaces[k].f1;
    f2 = mfaces[k].f2;

    if (f0 == f1 || f1 == f2 || f2 == f0) continue; /* ignore degenerates */

    /* Find faces adjacent on the f0-f1 edge */
    faces_at_vtx = &flist[f0];
    if (BMAP_ISSET(manifold_vtcs,f0)) {
      /* manifold => faces_at_vtx is ordered (i.e. adjacent faces are either
       * just before or just after in faces_at_vtx) and there is at most one
       * adjacent face on the edge f0-f1. */
      i = imax = faces_at_vtx->n_faces-1;
      if (imax > 0) { /* k is not the only face on this vertex */
        while (faces_at_vtx->face[i] != k) {
          i--;
          assert(i>=0); /* there is always a match */
        }
        adj_fidx = faces_at_vtx->face[(i > 0) ? i-1 : imax];
        assert(k != adj_fidx);
        if (mfaces[adj_fidx].f0 == f1 || mfaces[adj_fidx].f1 == f1 ||
            mfaces[adj_fidx].f2 == f1) { /* adjacent on f0-f1 */
          add_adj_face(&aflist[k],0,adj_fidx);
        } else { /* there can only be one adjacent */
          adj_fidx = faces_at_vtx->face[(i < imax) ? i+1 : 0];
          assert(k != adj_fidx);
          if (mfaces[adj_fidx].f0 == f1 || mfaces[adj_fidx].f1 == f1 ||
              mfaces[adj_fidx].f2 == f1) { /* adjacent on f0-f1 */
            add_adj_face(&aflist[k],0,adj_fidx);
          }
        }
      }
    } else { /* unordered faces_at_vtx => full scan */
      for (i=faces_at_vtx->n_faces-1; i>=0; i--) {
        adj_fidx = faces_at_vtx->face[i];
        if (adj_fidx == k) continue; /* don't include ourselves */
        if (mfaces[adj_fidx].f0 == f1 || mfaces[adj_fidx].f1 == f1 ||
            mfaces[adj_fidx].f2 == f1) { /* adjacent on f0-f1 */
          add_adj_face(&aflist[k],0,adj_fidx);
        }
      }
    }

    /* Find faces adjacent on the f1-f2 edge */
    faces_at_vtx = &flist[f1];
    if (BMAP_ISSET(manifold_vtcs,f1)) {
      /* manifold => faces_at_vtx is ordered (i.e. adjacent faces are either
       * just before or just after in faces_at_vtx) and there is at most one
       * adjacent face on the edge f1-f2. */
      i = imax = faces_at_vtx->n_faces-1;
      if (imax > 0) { /* k is not the only face on this vertex */
        while (faces_at_vtx->face[i] != k) {
          i--;
          assert(i>=0); /* there is always a match */
        }
        adj_fidx = faces_at_vtx->face[(i > 0) ? i-1 : imax];
        assert(k != adj_fidx);
        if (mfaces[adj_fidx].f0 == f2 || mfaces[adj_fidx].f1 == f2 ||
            mfaces[adj_fidx].f2 == f2) { /* adjacent on f1-f2 */
          add_adj_face(&aflist[k],1,adj_fidx);
        } else { /* there can only be one adjacent */
          adj_fidx = faces_at_vtx->face[(i < imax) ? i+1 : 0];
          assert(k != adj_fidx);
          if (mfaces[adj_fidx].f0 == f2 || mfaces[adj_fidx].f1 == f2 ||
              mfaces[adj_fidx].f2 == f2) { /* adjacent on f1-f2 */
            add_adj_face(&aflist[k],1,adj_fidx);
          }
        }
      }
    } else { /* unordered faces_at_vtx => full scan */
      for (i=faces_at_vtx->n_faces-1; i>=0; i--) {
        adj_fidx = faces_at_vtx->face[i];
        if (adj_fidx == k) continue; /* don't include ourselves */
        if (mfaces[adj_fidx].f0 == f2 || mfaces[adj_fidx].f1 == f2 ||
            mfaces[adj_fidx].f2 == f2) { /* adjacent on f1-f2 */
          add_adj_face(&aflist[k],1,adj_fidx);
        }
      }
    }

    /* Find faces adjacent on the f2-f0 edge */
    faces_at_vtx = &flist[f2];
    if (BMAP_ISSET(manifold_vtcs,f2)) {
      /* manifold => faces_at_vtx is ordered (i.e. adjacent faces are either
       * just before or just after in faces_at_vtx) and there is at most one
       * adjacent face on the edge f2-f0. */
      i = imax = faces_at_vtx->n_faces-1;
      if (imax > 0) { /* k is not the only face on this vertex */
        while (faces_at_vtx->face[i] != k) {
          i--;
          assert(i>=0); /* there is always a match */
        }
        adj_fidx = faces_at_vtx->face[(i > 0) ? i-1 : imax];
        assert(k != adj_fidx);
        if (mfaces[adj_fidx].f0 == f0 || mfaces[adj_fidx].f1 == f0 ||
            mfaces[adj_fidx].f2 == f0) { /* adjacent on f2-f0 */
          add_adj_face(&aflist[k],2,adj_fidx);
        } else { /* there can only be one adjacent */
          adj_fidx = faces_at_vtx->face[(i < imax) ? i+1 : 0];
          assert(k != adj_fidx);
          if (mfaces[adj_fidx].f0 == f0 || mfaces[adj_fidx].f1 == f0 ||
              mfaces[adj_fidx].f2 == f0) { /* adjacent on f2-f0 */
            add_adj_face(&aflist[k],2,adj_fidx);
          }
        }
      }
    } else { /* unordered faces_at_vtx => full scan */
      for (i=faces_at_vtx->n_faces-1; i>=0; i--) {
        adj_fidx = faces_at_vtx->face[i];
        if (adj_fidx == k) continue; /* don't include ourselves */
        if (mfaces[adj_fidx].f0 == f0 || mfaces[adj_fidx].f1 == f0 ||
            mfaces[adj_fidx].f2 == f0) { /* adjacent on f2-f0 */
          add_adj_face(&aflist[k],2,adj_fidx);
        }
      }
    }
  }
  return aflist;
}

/* Returns the next face in the list of adjacent faces aflist[fidx]. The
 * search starts at edge *edge_idx and at position *edge_pos within that
 * edge. The values edge and position for the next search are returned in
 * *edge_idx and *edge_pos. The index of the edge of fidx on which the
 * adjacent face is found is returned in *join_eidx. If there are no more
 * adjacent faces -1 is returned. Otherwise fidx is removed from
 * aflist[next_fidx], where next_fidx is the returned value (i.e. the
 * symmetric face adjacency is removed). */
static INLINE int get_next_adj_face_rm(struct adj_faces *aflist, int fidx,
                                       int *edge_idx, int *edge_pos,
                                       int *join_eidx)
{
  int next_fidx;
  int eidx,epos,jeidx;
  struct adj_faces *af;

  eidx = *edge_idx;
  epos = *edge_pos;

  assert(eidx >= 0 && eidx < 3);
  af = &aflist[fidx];
  do {
    if (epos == 0) { /* take from main adjacent faces */
      next_fidx = af->face_on_edge[eidx];
      jeidx = eidx;
      if (af->extra != NULL && epos < af->extra->n_faces_on_edge[eidx]) {
        epos++;
      } else {
        eidx++;
      }
    } else { /* take from extra adjacent faces */
      next_fidx = af->extra->faces_on_edge[eidx][epos-1];
      jeidx = eidx;
      if (epos < af->extra->n_faces_on_edge[eidx]) {
        epos++;
      } else {
        eidx++;
        epos = 0;
      }
    }
    /* get indices of next adjacent face */
  } while (next_fidx < 0 && eidx < 3);
  if (next_fidx < 0) return -1; /* none found */
  /* return values */
  *edge_idx = eidx;
  *edge_pos = epos;
  *join_eidx = jeidx;
  /* remove symmetric adjacency from next_fidx */
  af = &aflist[next_fidx];
  for (eidx=0; eidx<3; eidx++) { /* remove from main adjacent faces */
    if (af->face_on_edge[eidx] == fidx) {
      af->face_on_edge[eidx] = -1;
      return next_fidx; /* done */
    }
  }
  if (af->extra != NULL) { /* not in main adjacent faces => remove from extra */
    for (eidx=0; eidx<3; eidx++) {
      for (epos=0; epos < af->extra->n_faces_on_edge[eidx]; epos++) {
        if (af->extra->faces_on_edge[eidx][epos] == fidx) {
          af->extra->faces_on_edge[eidx][epos] = -1;
          return next_fidx; /* done */
        }
      }
    }
  }
  return next_fidx;
}

/* Compare the orientation of *face1 and *face2, where f2_join_eidx is the
 * index of *face2 that is shared with *face1. Returns zero if both faces have
 * the same orientation and one otherwise. */
static INLINE int get_orientation(const face_t *face1, const face_t *face2,
                                  int f2_join_eidx)
{
  int v0,v1; /* ordered vertices of the joining edge */

  switch (f2_join_eidx) {
  case 0:
    v0 = face2->f0;
    v1 = face2->f1;
    break;
  case 1:
    v0 = face2->f1;
    v1 = face2->f2;
    break;
  default: /* must always be 2 */
    assert(f2_join_eidx == 2);
    v0 = face2->f2;
    v1 = face2->f0;
  }

  if (face1->f0 == v0) {
    return (face1->f2 != v1);
  } else if (face1->f1 == v0) {
    return (face1->f0 != v1);
  } else { /* face1->f2 == v0 */
    assert(face1->f2 == v0);
    return (face1->f1 != v1);
  }
}

/* Evaluates the orientation of the model given by the faces in mfaces, the
 * number of faces n_faces and the list of incident faces for each vertex
 * flist. manifold_vtcs flags, for each vertex, if it is manifold or not. The
 * results is returned in the following fields of minfo: oriented and
 * orientable. The orientation of each face is returned as a malloc'ed bitmap
 * array of length n_faces (in bits). If the corresponding bit is set the
 * orientation of the corresponding face needs to be reversed to obtain an
 * oriented model (if orientable). If the model is not orientable, the model
 * would be mostly oriented if the returned orientation map is applied. */
static bmap_t * model_orientation(const face_t *mfaces, int n_faces,
                                  const struct face_list *flist,
                                  const bmap_t *manifold_vtcs,
                                  struct model_info *minfo)
{
  struct adj_faces *aflist; /* for each face, the list of adjacent faces */
  bmap_t *face_oriented;    /* flag for each face: already tested orient. */
  bmap_t *face_revo;        /* flag for each face: orient. should be reversed */
  int n_oriented_faces;     /* number of faces whose orient. has been tested */
  int next_fidx;            /* index of next face to test for orientation */
  unsigned int next_rev_of_fidx; /* flag: orientation of next_rev_of_fidx is
                                  * the reverse of that of fidx. */
  int join_eidx;            /* shared edge index between adjacent faces */
  struct stack stack;       /* stack to walk face tree */
  struct {
    int fidx;  /* current face index in aflist */
    int eidx;  /* current edge index in within face */
    int epos;  /* current position index within edge */
  } cur;                    /* the indices to the current entry in aflist */

  /* Initialize */
  aflist = find_adjacent_faces(mfaces,n_faces,flist,manifold_vtcs);
  face_oriented = bmap_calloc(n_faces);
  face_revo = bmap_calloc(n_faces);
  stack_init(&stack,sizeof(cur));
  minfo->orientable = 1;
  minfo->oriented = 1;
  n_oriented_faces = 0;
  cur.fidx = 0;
  while (n_oriented_faces < n_faces) {
    /* find next not yet oriented face. Use fast skip by testing BMAP_T_BITS
     * entries in the bitmap at once, and then find the individual entry. */
    for (cur.fidx &= (~BMAP_T_MASK); cur.fidx<n_faces; cur.fidx+=BMAP_T_BITS) {
      if (face_oriented[cur.fidx/BMAP_T_BITS] != ~0U) break;
    }
    for (; cur.fidx<n_faces; cur.fidx++) {
      if (!BMAP_ISSET(face_oriented,cur.fidx)) break;
    }
    assert(cur.fidx < n_faces);
    assert(!BMAP_ISSET(face_revo,cur.fidx));
    BMAP_SET(face_oriented,cur.fidx);
    n_oriented_faces++;
    cur.eidx = 0;
    cur.epos = 0;
    /* do a walk on all connected faces, checking orientation */
    do {
      /* get next face that is adjacent to current one */
      if (cur.eidx < 3 &&
          (next_fidx = get_next_adj_face_rm(aflist,cur.fidx,&cur.eidx,
                                            &cur.epos,&join_eidx)) >= 0) {
        /* still an adjacent face */
        assert(BMAP_ISSET(face_oriented,cur.fidx));
        assert(next_fidx != cur.fidx);
        next_rev_of_fidx = get_orientation(&mfaces[next_fidx],
                                           &mfaces[cur.fidx],join_eidx);
        if (next_rev_of_fidx) minfo->oriented = 0;
        if (!BMAP_ISSET(face_oriented,next_fidx)) {
          /* not yet oriented face => orient and continue from new */
          if ((next_rev_of_fidx^BMAP_ISSET(face_revo,cur.fidx)) != 0) {
            BMAP_SET(face_revo,next_fidx);
          }
          BMAP_SET(face_oriented,next_fidx);
          n_oriented_faces++;
          stack_push(&stack,&cur);
          cur.fidx = next_fidx;
          cur.eidx = 0;
          cur.epos = 0;
        } else { /* face already oriented, check consistency */
          if ((next_rev_of_fidx^BMAP_ISSET(face_revo,cur.fidx)) !=
              BMAP_ISSET(face_revo,next_fidx)) {
            minfo->orientable = 0;
          }
        }
      } else { /* no more adjacent faces for cur.fidx => walk back */
        if (!stack_pop(&stack,&cur)) {
          break; /* stack is empty => we are done */
        }
      }
    } while (1);
  }
  free_adj_face_list(aflist,n_faces);
  free(face_oriented);
  stack_fini(&stack);
  return face_revo;
}

/* Evaluates the topology of the model given by the faces in mfaces, the list
 * of incident faces for each vertex flist and the number of vertices
 * n_vtcs. The result is returned in the following fields of minfo: manifold,
 * closed and n_disjoint_parts. It returns a malloc'ed bitmap array (of length
 * n_vtcs bits) indicating which vertices are manifold. The entries for each
 * vertex in flist are reordered (as explained in get_vertex_topology()), but
 * the contents are the same. */
static bmap_t * model_topology(int n_vtcs, const face_t *mfaces,
                               const struct face_list *flist,
                               struct model_info *minfo)
{
  struct vtx_list *vlist; /* for each vertex, the list of vertices sharing an
                           * edge with them. */
  struct stack stack;     /* stack to walk vertex tree */
  bmap_t *visited_vtcs;   /* array to mark visited vertices */
  bmap_t *manifold_vtcs;  /* array flagging manifold vertices */
  int n_visited_vertices; /* number of already visited vertices */
  int next_vidx;          /* the index of the next vertex to visit */
  struct topology vtx_top;/* local vertex toppology */
  struct {
    int vidx; /* index of entry in vlist */
    int lpos; /* current position in the list at entry vidx in vlist */
  } cur;                  /* the indices to the current vertex entry in vlist */

  /* Initialize */
  minfo->manifold = 1;
  minfo->closed = 1;
  minfo->n_disjoint_parts = 0;
  stack_init(&stack,sizeof(cur));
  vlist = xa_calloc(n_vtcs,sizeof(*vlist));
  visited_vtcs = bmap_calloc(n_vtcs);
  manifold_vtcs = bmap_calloc(n_vtcs);
  next_vidx = 0; /* keep compiler happy */

  n_visited_vertices = 0;
  cur.vidx = 0;
  while (n_visited_vertices < n_vtcs) {
    /* Find next unvisited vertex (always one). Use fast skip by testing
     * BMAP_T_BITS entries in the bitmap at once, and then find the individual
     * entry. */
    for (cur.vidx &= (~BMAP_T_MASK); cur.vidx<n_vtcs; cur.vidx+=BMAP_T_BITS) {
      if (visited_vtcs[cur.vidx/BMAP_T_BITS] != ~0U) break;
    }
    for (; cur.vidx<n_vtcs; cur.vidx++) {
      if (!BMAP_ISSET(visited_vtcs,cur.vidx)) break;
    }
    assert(cur.vidx<n_vtcs);
    /* mark all the connected vertices as visited and count one part */
    assert(!BMAP_ISSET(visited_vtcs,cur.vidx));
    BMAP_SET(visited_vtcs,cur.vidx);
    n_visited_vertices++;
    get_vertex_topology(mfaces,cur.vidx,&flist[cur.vidx],
                        &vtx_top,&vlist[cur.vidx]);
    minfo->manifold = minfo->manifold && vtx_top.manifold;
    minfo->closed = minfo->closed && vtx_top.closed;
    if (vtx_top.manifold) BMAP_SET(manifold_vtcs,cur.vidx);
    if (vlist[cur.vidx].n_elems != 0) { /* vertex is not alone */
      minfo->n_disjoint_parts++;
      cur.lpos = 0;
      do {
        for (; cur.lpos < vlist[cur.vidx].n_elems; cur.lpos++) {
          next_vidx = vlist[cur.vidx].vtcs[cur.lpos];
          if (!BMAP_ISSET(visited_vtcs,next_vidx)) break;
        }
        if (cur.lpos < vlist[cur.vidx].n_elems) {
          /* found connected and not yet visited vertex => continue from new */
          assert(!BMAP_ISSET(visited_vtcs,next_vidx));
          BMAP_SET(visited_vtcs,next_vidx);
          n_visited_vertices++;
          get_vertex_topology(mfaces,next_vidx,&flist[next_vidx],
                              &vtx_top,&vlist[next_vidx]);
          minfo->manifold = minfo->manifold && vtx_top.manifold;
          minfo->closed = minfo->closed && vtx_top.closed;
          if (vtx_top.manifold) BMAP_SET(manifold_vtcs,next_vidx);
          cur.lpos++;
          stack_push(&stack,&cur);
          cur.vidx = next_vidx;
          cur.lpos = 0;
        } else {
          /* no connected and not yet visited vertices => walk back */
          free(vlist[cur.vidx].vtcs);
          vlist[cur.vidx].vtcs = NULL;
          if (!stack_pop(&stack,&cur)) {
            break; /* stack is empty => we are done */
          }
        }
      } while (1);
    }
  }

  stack_fini(&stack);
  free(visited_vtcs);
  free(vlist);
  return manifold_vtcs;
}

/* Prints a list of non-manifold vertices to out. manifold_vtcs flags, for
 * each vertex, if it is manifold or not. n_vtcs is the number of vertices of
 * the model. First a title is printed with name and the number of
 * non-manifold vertices. Then the list of non-manifold vertices is
 * printed. */
static void print_manifold_vertices(struct outbuf *out, const char *name,
                                    const bmap_t *manifold_vtcs, int n_vtcs)
{
  int i,j,jmax,k;
  int n_non_man;

  for (n_non_man=0, i=0; i<n_vtcs; i+=BMAP_T_BITS) {
    if (manifold_vtcs[i/BMAP_T_BITS] != ~0U) {
      jmax = (i+(int)BMAP_T_BITS <= n_vtcs) ? (i+(int)BMAP_T_BITS) : n_vtcs;
      for (j=i; j<jmax; j++) {
        if (!BMAP_ISSET(manifold_vtcs,j)) n_non_man++;
      }
    }
  }
  outbuf_printf(out,"%s has %i non-manfifold vertices (of %i):\n",
                name,n_non_man,n_vtcs);
  for (i=0,k=0; i<n_vtcs; i+=BMAP_T_BITS) {
    if (manifold_vtcs[i/BMAP_T_BITS] != ~0U) {
      jmax = (i+(int)BMAP_T_BITS <= n_vtcs) ? (i+(int)BMAP_T_BITS) : n_vtcs;
      for (j=i; j<jmax; j++) {
        if (!BMAP_ISSET(manifold_vtcs,j)) {
          if (++k%10 == 0) {
            outbuf_printf(out," %i\n",j);
          } else {
            outbuf_printf(out," %i",j);
          }
        }
      }
    }
  }
  if (k%10 != 0) outbuf_printf(out,"\n");
  outbuf_flush(out);
}

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See model_analysis.h */
void analyze_model(struct model *m, struct model_info *info, int do_orient,
                   int verbose, struct outbuf *out, const char *name)
{
  struct face_list *flist;       /* list of faces incident on each vertex */
  bmap_t *face_revo;             /* flag for each face: if its orientation
                                  * should be reversed. */
  bmap_t *manifold_vtcs;         /* array flagging manifold vertices */

  /* Initialize */
  memset(info,0,sizeof(*info));
  flist = faces_of_vertex(m,&(info->n_degenerate));

  /* Make topology and orientation analysis */
  manifold_vtcs = model_topology(m->num_vert,m->faces,flist,info);
  face_revo = model_orientation(m->faces,m->num_faces,flist,manifold_vtcs,info);

  /* Save original oriented state */
  info->orig_oriented = info->oriented;

  /* Orient model if requested and  possible */
  if (((do_orient && info->orientable) || (do_orient > 1)) &&
      !info->oriented) {
    orient_model(m,face_revo);
  }

  if (verbose && !info->manifold) {
    print_manifold_vertices(out,name,manifold_vtcs,m->num_vert);
  }

  /* Free memory */
  free(face_revo);
  free(manifold_vtcs);
  free_face_lists(flist,m->num_vert);
}

/* See model_analysis.h */
struct face_list *faces_of_vertex(const struct model *m, int *n_degenerate)
{
  int j,jmax;           /* indices and loop limits */
  int v0,v1,v2;         /* current triangle's vertex indices */
  struct face_list *fl; /* the face list to return */

  /* NOTE: we do a two scan allocation, first gather the required sizes and
   * then allocate storage. It is much faster than a single scan, since the
   * number of calls to memory allocation routines is greatly reduced. In
   * addition the obtained memory arrangement is more compact. */

  fl = xa_calloc(m->num_vert,sizeof(*fl));
  /* First scan: count number of incident faces per vertex */
  for (*n_degenerate=0, j=0, jmax=m->num_faces; j<jmax; j++) {
    v0 = m->faces[j].f0;
    v1 = m->faces[j].f1;
    v2 = m->faces[j].f2;
    /* degenerate faces not included */
    if (v0 == v1 || v0 == v2 || v1 == v2) {
      (*n_degenerate)++;
      continue;
    }
    fl[v0].n_faces++;
    fl[v1].n_faces++;
    fl[v2].n_faces++;
  }
  /* Allocate storage for each vertex */
  for (j=0, jmax=m->num_vert; j<jmax; j++) {
    fl[j].face = xa_malloc(fl[j].n_faces*sizeof(*(fl->face)));
    fl[j].n_faces = 0;
  }
  /* Second scan: fill list of incident faces */
  for (j=0, jmax=m->num_faces; j<jmax; j++) {
    v0 = m->faces[j].f0;
    v1 = m->faces[j].f1;
    v2 = m->faces[j].f2;
    /* degenerate faces not included */
    if (v0 == v1 || v0 == v2 || v1 == v2) continue;
    fl[v0].face[fl[v0].n_faces++] = j;
    fl[v1].face[fl[v1].n_faces++] = j;
    fl[v2].face[fl[v2].n_faces++] = j;
  }
  return fl;
}

/* See model_analysis.h */
void free_face_lists(struct face_list *fl, int n)
{
  int i;
  if (fl == NULL) return;
  for (i=0; i<n; i++) {
    free(fl[i].face);
  }
  free(fl);
}

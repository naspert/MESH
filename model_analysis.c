/* $Id: model_analysis.c,v 1.21 2002/03/28 17:34:29 dsanta Exp $ */


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
#endif


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
 *                            Utility functions                              *
 * --------------------------------------------------------------------------*/

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

/* Returns one if face is degenerate and zero if not.*/
static INLINE int is_degenerate(face_t face)
{
  return (face.f0 == face.f1 || face.f1 == face.f2 || face.f2 == face.f0);
}

/* --------------------------------------------------------------------------*
 *                         Model analysis functions                          *
 * --------------------------------------------------------------------------*/

/* Given the face orientation map face_orientation, orients the model m. */
static void orient_model(struct model *m, signed char *face_orientation)
{
  int i,imax;
  int tmpi;

  for (i=0, imax=m->num_faces; i<imax; i++) {
    if (face_orientation[i] < 0) { /* revert orientation */
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

/* Performs local analysis of the faces incident on vertex vidx. The faces of
 * the model are given in the mfaces array. The list of faces incident on vidx
 * is given by flist. The local topology information is returned in *ltop. In
 * addition it constructs the list of vertices, different from vidx and
 * without repetition, that belong to the faces incident on vidx in *vlist. */
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
      v2_was_in_list = vtx_in_list_or_add(vlist,v2,&vtx_buf_sz);
      if (v2_was_in_list) {
        if (v2 == vstart) vstart_was_in_list = 1;
        /* v2 appearing more than once is only admissible in a manifold if the
         * triangle is degenerate or if it is the last triangle and it closes
         * a cycle */
        if (!(n_vfaces == 0 && v2 == vstart) && !is_degenerate(mfaces[fidx])) {
          ltop->manifold = 0;
        }
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
    xa_realloc(af->extra->faces_on_edge[edge_idx],j+1);
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
                                              const char *manifold_vtcs)
{
  struct adj_faces *aflist;
  int k,i;
  int f0,f1,f2;
  int adj_fidx;
  const struct face_list *faces_at_vtx;

  aflist = xa_malloc(n_faces*sizeof(*aflist));
  for (k=0; k<n_faces; k++) {
    aflist[k].face_on_edge[0] = -1;
    aflist[k].face_on_edge[1] = -1;
    aflist[k].face_on_edge[2] = -1;
    aflist[k].extra = NULL;
    if (is_degenerate(mfaces[k])) continue; /* degenerates are irrelevant */
    f0 = mfaces[k].f0;
    f1 = mfaces[k].f1;
    f2 = mfaces[k].f2;
    /* Find faces adjacent on the f0-f1 edge */
    faces_at_vtx = &flist[f0];
    for (i=0; i<faces_at_vtx->n_faces; i++) {
      adj_fidx = faces_at_vtx->face[i];
      if (adj_fidx == k) continue; /* don't include ourselves */
      if (mfaces[adj_fidx].f0 == f1 || mfaces[adj_fidx].f1 == f1 ||
          mfaces[adj_fidx].f2 == f1) { /* adjacent on f0-f1 */
        add_adj_face(&aflist[k],0,adj_fidx);
        if (manifold_vtcs[f0]) break; /* can have only one adjacent face */
      }
    }
    /* Find faces adjacent on the f1-f2 edge */
    faces_at_vtx = &flist[f1];
    for (i=0; i<faces_at_vtx->n_faces; i++) {
      adj_fidx = faces_at_vtx->face[i];
      if (adj_fidx == k) continue; /* don't include ourselves */
      if (mfaces[adj_fidx].f0 == f2 || mfaces[adj_fidx].f1 == f2 ||
          mfaces[adj_fidx].f2 == f2) { /* adjacent on f1-f2 */
        add_adj_face(&aflist[k],1,adj_fidx);
        if (manifold_vtcs[f1]) break; /* can have only one adjacent face */
      }
    }
    /* Find faces adjacent on the f2-f0 edge */
    faces_at_vtx = &flist[f2];
    for (i=0; i<faces_at_vtx->n_faces; i++) {
      adj_fidx = faces_at_vtx->face[i];
      if (adj_fidx == k) continue; /* don't include ourselves */
      if (mfaces[adj_fidx].f0 == f0 || mfaces[adj_fidx].f1 == f0 ||
          mfaces[adj_fidx].f2 == f0) { /* adjacent on f2-f0 */
        add_adj_face(&aflist[k],2,adj_fidx);
        if (manifold_vtcs[f2]) break; /* can have only one adjacent face */
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

/* Returns the orientation (1 or -1) of *new_face that is compatible with that
 * of the already oriented face and adjacent face *oriented_face. The
 * orientation of the oriented face is orientation. The index of the edge of
 * *oriented_face where *new_face is adjacent is of join_eidx. */
static INLINE signed char get_orientation(const face_t *new_face,
                                          const face_t *oriented_face,
                                          signed char orientation,
                                          int of_join_eidx)
{
  int v0,v1; /* ordered vertices of the joining edge */

  switch (of_join_eidx) {
  case 0:
    v0 = oriented_face->f0;
    v1 = oriented_face->f1;
    break;
  case 1:
    v0 = oriented_face->f1;
    v1 = oriented_face->f2;
    break;
  default: /* must always be 2 */
    assert(of_join_eidx == 2);
    v0 = oriented_face->f2;
    v1 = oriented_face->f0;
  }

  if (new_face->f0 == v0) {
    return (new_face->f2 == v1) ? orientation : -orientation;
  } else if (new_face->f1 == v0) {
    return (new_face->f0 == v1) ? orientation : -orientation;
  } else { /* new_face->f2 == v0 */
    assert(new_face->f2 == v0);
    return (new_face->f1 == v1) ? orientation : -orientation;
  }
}

/* Evaluates the orientation of the model given by the faces in mfaces, the
 * number of faces n_faces and the list of incident faces for each vertex
 * flist. manifold_vtcs flags, for each vertex, if it is manifold or not. The
 * results is returned in the following fields of minfo: oriented and
 * orientable. The orientation of each face is returned as a malloc'ed array
 * of length n_faces. A negative entry means that the orientation of the
 * corresponding face needs to be reversed. */
static signed char * model_orientation(const face_t *mfaces, int n_faces,
                                       const struct face_list *flist,
                                       const char *manifold_vtcs,
                                       struct model_info *minfo)
{
  struct adj_faces *aflist;
  signed char *face_orientation;
  int n_oriented_faces;
  int next_fidx;
  signed char next_orientation;
  int join_eidx;
  struct stack stack;
  struct {
    int fidx;
    int eidx;
    int epos;
  } cur;

  /* Initialize */
  aflist = find_adjacent_faces(mfaces,n_faces,flist,manifold_vtcs);
  face_orientation = xa_calloc(n_faces,sizeof(*face_orientation));
  stack_init(&stack,sizeof(cur));
  minfo->orientable = 1;
  minfo->oriented = 1;
  n_oriented_faces = 0;
  cur.fidx = 0;
  while (n_oriented_faces < n_faces) {
    /* find next not yet oriented face */
    for (; cur.fidx<n_faces; cur.fidx++) {
      if (face_orientation[cur.fidx] == 0) break;
    }
    assert(cur.fidx < n_faces);
    face_orientation[cur.fidx] = 1;
    n_oriented_faces++;
    if (is_degenerate(mfaces[cur.fidx])) continue;
    cur.eidx = 0;
    cur.epos = 0;
    /* do a walk on all connected faces, checking orientation */
    do {
      /* get next face that is adjacent to current one */
      if (cur.eidx < 3 &&
          (next_fidx = get_next_adj_face_rm(aflist,cur.fidx,&cur.eidx,
                                            &cur.epos,&join_eidx)) >= 0) {
        /* still an adjacent face */
        assert(face_orientation[cur.fidx] != 0);
        next_orientation =
          get_orientation(&mfaces[next_fidx],&mfaces[cur.fidx],
                          face_orientation[cur.fidx],join_eidx);
        if (next_orientation != face_orientation[cur.fidx]) {
          minfo->oriented = 0;
        }
        if (face_orientation[next_fidx] == 0) {
          /* not yet oriented face => orient and continue from new */
          face_orientation[next_fidx] = next_orientation;
          n_oriented_faces++;
          stack_push(&stack,&cur);
          cur.fidx = next_fidx;
          cur.eidx = 0;
          cur.epos = 0;
        } else { /* face already oriented, check consistency */
          if (face_orientation[next_fidx] != next_orientation) {
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
  stack_fini(&stack);
  return face_orientation;
}

/* Evaluates the topology of the model given by the faces in mfaces, the list
 * of incident faces for each vertex flist and the number of vertices
 * n_vtcs. The result is returned in the following fields of minfo: manifold,
 * closed and n_disjoint_parts. It returns a malloc'ed array indicating which
 * vertices are manifold. */
static char * model_topology(int n_vtcs, const face_t *mfaces,
                             const struct face_list *flist,
                             struct model_info *minfo)
{
  struct vtx_list *vlist; /* for each vertex, the list of vertices sharing an
                           * edge with them. */
  struct stack stack;     /* stack to walk vertex tree */
  char *visited_vtcs;     /* array to mark visited vertices */
  char *manifold_vtcs;    /* array flagging manifold vertices */
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
  visited_vtcs = xa_calloc(n_vtcs,sizeof(*visited_vtcs));
  manifold_vtcs = xa_malloc(n_vtcs*sizeof(*manifold_vtcs));
  next_vidx = 0; /* keep compiler happy */

  n_visited_vertices = 0;
  cur.vidx = 0;
  while (n_visited_vertices < n_vtcs) {
    /* Find next unvisited vertex (always one) */
    for (; cur.vidx<n_vtcs; cur.vidx++) {
      if (!visited_vtcs[cur.vidx]) break;
    }
    assert(cur.vidx<n_vtcs);
    /* mark all the connected vertices as visited and count one part */
    assert(visited_vtcs[cur.vidx] == 0);
    visited_vtcs[cur.vidx] = 1;
    n_visited_vertices++;
    get_vertex_topology(mfaces,cur.vidx,&flist[cur.vidx],
                        &vtx_top,&vlist[cur.vidx]);
    minfo->manifold = minfo->manifold && vtx_top.manifold;
    minfo->closed = minfo->closed && vtx_top.closed;
    manifold_vtcs[cur.vidx] = minfo->manifold;
    if (vlist[cur.vidx].n_elems != 0) { /* vertex is not alone */
      minfo->n_disjoint_parts++;
      cur.lpos = 0;
      do {
        for (; cur.lpos < vlist[cur.vidx].n_elems; cur.lpos++) {
          next_vidx = vlist[cur.vidx].vtcs[cur.lpos];
          if (!visited_vtcs[next_vidx]) break;
        }
        if (cur.lpos < vlist[cur.vidx].n_elems) {
          /* found connected and not yet visited vertex => continue from new */
          visited_vtcs[next_vidx] = 1;
          n_visited_vertices++;
          get_vertex_topology(mfaces,next_vidx,&flist[next_vidx],
                              &vtx_top,&vlist[next_vidx]);
          minfo->manifold = minfo->manifold && vtx_top.manifold;
          minfo->closed = minfo->closed && vtx_top.closed;
          manifold_vtcs[next_vidx] = minfo->manifold;
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

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See model_analysis.h */
void analyze_model(struct model *m, const struct face_list *flist,
                   struct model_info *info, int do_orient)
{
  signed char *face_orientation; /* the face orientation map */
  struct face_list *flist_local; /* the locally generated flist, if any */
  char *manifold_vtcs;           /* array flagging manifold vertices */

  /* Initialize */
  memset(info,0,sizeof(*info));
  if (flist == NULL) {
    flist_local = faces_of_vertex(m);
    flist = flist_local;
  } else {
    flist_local = NULL;
  }

  /* Make topology and orientation analysis */
  manifold_vtcs = model_topology(m->num_vert,m->faces,flist,info);
  face_orientation = model_orientation(m->faces,m->num_faces,flist,
                                       manifold_vtcs,info);

  /* Save original oriented state */
  info->orig_oriented = info->oriented;

  /* Orient model if requested and  possible */
  if (do_orient && info->orientable && !info->oriented) {
    orient_model(m,face_orientation);
  }

  /* Free memory */
  free(face_orientation);
  free(manifold_vtcs);
  free_face_lists(flist_local,m->num_vert);
}

/* See model_analysis.h */
struct face_list *faces_of_vertex(const struct model *m)
{
  int j,jmax;           /* indices and loop limits */
  int v0,v1,v2;         /* current triangle's vertex indices */
  struct face_list *fl; /* the face list to return */

  fl = xa_calloc(m->num_vert,sizeof(*fl));
  for (j=0, jmax=m->num_faces; j<jmax; j++) {
    v0 = m->faces[j].f0;
    v1 = m->faces[j].f1;
    v2 = m->faces[j].f2;
    fl[v0].face =
      xa_realloc(fl[v0].face,(fl[v0].n_faces+1)*sizeof(*(fl->face)));
    fl[v0].face[fl[v0].n_faces++] = j;
    if (v1 != v0) { /* avoid double inclusion in degenerate cases */
      fl[v1].face =
        xa_realloc(fl[v1].face,(fl[v1].n_faces+1)*sizeof(*(fl->face)));
      fl[v1].face[fl[v1].n_faces++] = j;
    }
    if (v2 != v0 && v2 != v1) { /* avoid double inclusion in degenerate cases */
      fl[v2].face =
        xa_realloc(fl[v2].face,(fl[v2].n_faces+1)*sizeof(*(fl->face)));
      fl[v2].face[fl[v2].n_faces++] = j;
    }
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


/* $Id: model_analysis.c,v 1.16 2002/03/27 08:23:15 dsanta Exp $ */


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

/* The state for recursively walking the vertex tree */
struct vtx_walk_state {
  char *visited_vertex;          /* array to mark the already visited
                                  * vertices (visited_vertex[i] == 1 if vertex
                                  * i has been visited). */
  int n_visited_vertices;        /* the number of alredy visited vertices */
  struct face_list *flist;       /* the list of faces incident on each vertex */
  struct model_info minfo;       /* the model information */
  signed char *face_orientation; /* the orientation for each face. If
                                  * face_orientation[i] is 0 it has not yet
                                  * been oriented, if positive its orientation
                                  * is correct (for an orientable model), if
                                  * negative its orientation should be
                                  * inversed to obtain an oriented model (if
                                  * the model is orientable). */
};

/* A list of vertices */
struct vtx_list {
  int *vtcs;    /* The list of vertex indices */
  int n_elems;  /* The number of elements in the list */
};

/* --------------------------------------------------------------------------*
 *                            Local functions                                *
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

/* Searches for vertex index v in list. If not found v is added to
 * list. Returns zero if not-found and non-zero otherwise. list->vtcs storage
 * must be large enough for v to be added.  */
static int vtx_in_list_or_add(struct vtx_list *list, int v)
{
  int i;
  i = 0;
  while (i < list->n_elems && list->vtcs[i] != v) {
    i++;
  }
  if (i == list->n_elems) {
    list->vtcs[list->n_elems++] = v;
    return 0;
  } else {
    return 1;
  }
}

/* Returns one if face is degenerate and zero if not.*/
static int is_degenerate(const face_t *face)
{
  return (face->f0 == face->f1 || face->f1 == face->f2 || face->f2 == face->f0);
}

/* Returns the indices of the vertices if face, that are not center_vidx, in
 * *start_vidx and *edge_vidx. The order is so that center_vidx, *start_vidx
 * and *edge_vidx are in the order f0,f1,f2, upto a cyclic permuation. Note
 * that *face must be incident on the center_vidx vertex. This is used to
 * obtain the starting vertices to do an oriented walk of the vertices around
 * center_vidx. */
static void get_ordered_vtcs(const face_t *face, int center_vidx,
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

/* Tests if *new_face is adjacent to the previous face at the edge defined by
 * the vertices with index center_vidx and edge_vidx. Returns zero if
 * non-adjacent and one if adjacent. The current correct face orientation (1
 * or -1) is given by face_orientation. The flag rev_orient indicates if the
 * orientation order has been reversed. If *new_face is non-adjacent no
 * argument are modified. Otherwise, the non-adjacent vertex index of
 * *new_face is returned in *edge_vidx. The orientation of the new face is
 * returned in *newf_orientation. */
static int is_adjacent_and_update(const face_t *new_face, int center_vidx,
                                  int *edge_vidx, signed char face_orientation,
                                  int rev_orient,
                                  signed char *newf_orientation)
{
  int e_vidx;

  /* Try to find adjacent edge and check orientation */
  e_vidx = *edge_vidx;
  if (e_vidx == new_face->f0) {
    if (center_vidx == new_face->f2) { /* same orientation as first face */
      *edge_vidx = new_face->f1;
      *newf_orientation = (!rev_orient) ? face_orientation : -face_orientation;
    } else { /* opposite orientation */
      *edge_vidx = new_face->f2; /* do as if face orient was opposite */
      *newf_orientation = (rev_orient) ? face_orientation : -face_orientation;
    }
  } else if (e_vidx == new_face->f1) {
    if (center_vidx == new_face->f0) { /* same orientation as first face */
      *edge_vidx = new_face->f2; /* do as if face orient was opposite */
      *newf_orientation = (!rev_orient) ? face_orientation : -face_orientation;
    } else { /* opposite orientation */
      *edge_vidx = new_face->f0; /* do as if face orient was opposite */
      *newf_orientation = (rev_orient) ? face_orientation : -face_orientation;
    }
  } else if (e_vidx == new_face->f2) {
    if (center_vidx == new_face->f1) { /* same orientation as first face */
      *edge_vidx = new_face->f0;
      *newf_orientation = (!rev_orient) ? face_orientation : -face_orientation;
    } else { /* opposite orientation */
      *edge_vidx = new_face->f1; /* do as if face orient was opposite */
      *newf_orientation = (rev_orient) ? face_orientation : -face_orientation;
    }
  } else {
    return 0; /* not an adjacent face */
  }
  return 1; /* we did find the adjacent face */
}

/* Recursively analyze the faces incident on the on any of the vertices
 * connected to vertex vidx, and updates the information in st
 * accordingly. The face analysis starts at face pfidx, which should be
 * incident on vidx and already oriented (st->face_orientation[ofidx] !=
 * 0). The vertex of each face is given by mfaces. The analysis updates the
 * oriented, orientable, manifold and closed properties of the model. The
 * orientation of each face to obtain an oriented model is recorded in
 * st->face_orientation. The vertex vidx should already be marked as visited
 * (st->visited_vertex[vidx] != 0). */
static void analyze_faces_rec(const face_t *mfaces, int vidx, int pfidx,
                              struct vtx_walk_state *st)
{
  int j;           /* loop counter */
  int nf;          /* number of faces incident on current vertex */
  int nf_left;     /* number of not yet processed faces in vfaces */
  int fidx;        /* current face index */
  int v2;          /* vertex on which next face should be incident */
  int vstart;      /* starting vertex, to check for closed surface */
  char rev_orient; /* reversed processing orientation flag */
  int tmpi;        /* temporary integer */
  signed char fface_orient;/* orientation of first face */
  signed char cface_orient;/* orientation of current face */
  int *vfaces;     /* list of faces incident on current vertex */
  int ffidx;       /* first face index */
  struct vtx_list vlist; /* list of found vertices  */
  char cur_degen;  /* flag for degenerate current triangle */
  char new_degen;  /* flag for degenerate new triangle */
  char v2_was_in_list; /* flag: v2 already visited when last encountered */
  char vstart_was_in_list; /* same as above but for vstart */

  /* Initialize */
  nf = st->flist[vidx].n_faces;
  vlist.vtcs = xa_malloc(sizeof(*(vlist.vtcs))*nf*2); /* worst case size */
  vlist.n_elems = 0;
  assert(st->visited_vertex[vidx] != 0);
  assert(nf != 0); /* An isolated vertex should never get here  */
  vfaces = st->flist[vidx].face;
  /* Locate the face from which we are coming, so that it is the first face */
  for (j=0; j<nf; j++) {
    if (vfaces[j] == pfidx) break;
  }
  assert(j<nf);
  /* Get first face vertices */
  fidx = pfidx;
  get_ordered_vtcs(&mfaces[fidx],vidx,&vstart,&v2);
  /* Get orientation of first face (which is already oriented) */
  fface_orient = st->face_orientation[fidx];
  assert(fface_orient != 0);
  /* Recursively analyze from the non-center vertices of first face */
  if (!st->visited_vertex[vstart]) {
    st->visited_vertex[vstart] = 1;
    (st->n_visited_vertices)++;
    analyze_faces_rec(mfaces,vstart,fidx,st);
  }
  if (!st->visited_vertex[v2]) {
    st->visited_vertex[v2] = 1;
    (st->n_visited_vertices)++;
    analyze_faces_rec(mfaces,v2,fidx,st);
  }
  /* Check the other faces in an oriented order */
  nf_left = nf-1;
  vfaces[j] = -1;
  rev_orient = 0;
  ffidx = fidx;
  cur_degen = is_degenerate(&mfaces[fidx]);
  vstart_was_in_list = 0;
  vtx_in_list_or_add(&vlist,vstart); /* list empty, so always added */
  v2_was_in_list = vtx_in_list_or_add(&vlist,v2);
  while (nf_left > 0) { /* process the remaining faces */
    for (j=0; j<nf; j++) { /* search for face that shares v2 */
      fidx = vfaces[j];
      if (fidx == -1) continue; /* face already counted */
      if (!is_adjacent_and_update(&mfaces[fidx],vidx,&v2,fface_orient,
                                  rev_orient,&cface_orient)) {
        continue; /* non-adjacent face => test next */
      }
      /* Update oriented state */
      new_degen = is_degenerate(&mfaces[fidx]);
      if (cface_orient != fface_orient &&
          !(new_degen || cur_degen)) st->minfo.oriented = 0;
      /* Check that we can orient face in a consistent manner */
      if (st->face_orientation[fidx] == 0) { /* not yet oriented */
        st->face_orientation[fidx] = cface_orient;
      } else if (st->face_orientation[fidx] != cface_orient) {
        /* can not get consistent orientation */
        if (!(new_degen || cur_degen)) st->minfo.orientable = 0;
        fface_orient = cface_orient; /* take new orientation */
      }
      /* Recursively analyze from new vertex */
      if (!st->visited_vertex[v2]) {
        st->visited_vertex[v2] = 1;
        (st->n_visited_vertices)++;
        analyze_faces_rec(mfaces,v2,fidx,st);
      }
      vfaces[j] = -1; /* mark face as counted */
      nf_left--;  /* goto search for face that shares new v2 */
      v2_was_in_list = vtx_in_list_or_add(&vlist,v2);
      if (v2_was_in_list) {
        if (v2 == vstart) vstart_was_in_list = 1;
        /* v2 appearing more than once is only admissible in a manifold if the
         * triangle is degenerate or if it is the last triangle and it closes
         * a cycle */
        if (!new_degen && !(nf_left == 0 && v2 == vstart)) {
          st->minfo.manifold = 0;
          /* The vidx-v2 edge is shared by more than two non-degenerate
           * triangles => cannot be orientable nor oriented */
          st->minfo.oriented = 0;
          st->minfo.orientable = 0;
        }
      }
      cur_degen = new_degen;
      break;
    }
    if (j == nf) { /* none of unvisited triangles share's v2! */
      /* vertex vidx can be at a border or there is really a non-manifold */
      if (rev_orient || v2 == vstart) {
        /* already reversed orientation or completed a disk => non-manifold */
        st->minfo.manifold = 0;
        /* closedness is not affected if we just closed a cycle or if vstart
         * and v2 were already visited the last time they were tested. */
        if (!(v2 == vstart || (vstart_was_in_list && v2_was_in_list))) {
          st->minfo.closed = 0;
        }
        /* Restart with first not yet counted triangle (always one) */
        for (j=0; j<nf; j++) {
          fidx = vfaces[j];
          if (fidx != -1) break;
        }
        assert(fidx >= 0);
        rev_orient = 0; /* restore original orientation */
        /* Get new first face vertices */
        get_ordered_vtcs(&mfaces[fidx],vidx,&vstart,&v2);
        /* Get and mark orientation of new first face */
        fface_orient = st->face_orientation[fidx];
        if (fface_orient == 0) {
          st->face_orientation[fidx] = 1;
          fface_orient = 1;
        }
        /* Recursively analyze from the non-center vertices of new first face */
        if (!st->visited_vertex[vstart]) {
          st->visited_vertex[vstart] = 1;
          (st->n_visited_vertices)++;
          analyze_faces_rec(mfaces,vstart,fidx,st);
        }
        if (!st->visited_vertex[v2]) {
          st->visited_vertex[v2] = 1;
          (st->n_visited_vertices)++;
          analyze_faces_rec(mfaces,v2,fidx,st);
        }
        ffidx = fidx;
        vfaces[j] = -1;
        nf_left--;
        cur_degen = (vidx == vstart || vidx == v2 || vstart == v2);
        vstart_was_in_list = vtx_in_list_or_add(&vlist,vstart);
        v2_was_in_list = vtx_in_list_or_add(&vlist,v2);
        if (!cur_degen && (vstart_was_in_list || v2_was_in_list)) {
          /* The vidx-vstart or vidx-v2 edge is shared by more than two
           * non-degenerate triangles => can not be orientable nor oriented */
          st->minfo.oriented = 0;
          st->minfo.orientable = 0;
        }
      } else {
        /* we reverse scanning orientation and continue from the other side */
        rev_orient = 1;
        tmpi = v2;
        v2 = vstart;
        vstart = tmpi;
        tmpi = v2_was_in_list;
        v2_was_in_list = vstart_was_in_list;
        vstart_was_in_list = tmpi;
        /* re-take original orientation */
        fface_orient = st->face_orientation[ffidx];
      }
    }
  }
  if (!(v2 == vstart || (vstart_was_in_list && v2_was_in_list))) {
    st->minfo.closed = 0;
  }
  free(vlist.vtcs);
}

/* Walks the vertex tree rooted at the vertex with index vidx. The vertex tree
 * is made of all vertices that are connected (directly or indirectly) to vidx
 * trough some face. The vertex on each face are given by the mface array. For
 * each face incident on a visited vertex, the model properties (orientable,
 * oriented, manifold, etc.), and other state information, are updated in
 * st. If vidx is an isolated vertex, it is marked and counted and visited but
 * nothing else is done. The counter for the number of disjoint parts is
 * incremented by one (if the vertex is not isolated). */
static void walk_vertex_tree(const face_t *mfaces, int vidx,
                             struct vtx_walk_state *st) {
  /* Mark vidx vertex as visited */
  st->visited_vertex[vidx] = 1;
  (st->n_visited_vertices)++;
  /* Ignore isolated vertices */
  if (st->flist[vidx].n_faces == 0) return;
  /* Orient the first face at the root of the tree (orientation is
   * arbitrary) */
  assert(st->face_orientation[st->flist[vidx].face[0]] == 0);
  st->face_orientation[st->flist[vidx].face[0]] = 1;
  /* Analyze the tree rooted at vidx */
  analyze_faces_rec(mfaces,vidx,st->flist[vidx].face[0],st);
  st->minfo.n_disjoint_parts++;
}

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See model_analysis.h */
void analyze_model(struct model *m, const struct face_list *flist,
                   struct model_info *info, int do_orient)
{
  struct vtx_walk_state st; /* walk state storage */
  int start_idx;            /* index where to start next vertex walk */
  int i,imax;

  /* Initialize */
  memset(&st,0,sizeof(st));
  if (flist != NULL) { /* make local copy of flist */
    st.flist = xa_malloc(m->num_vert*sizeof(*flist));
    for (i=0, imax=m->num_vert; i<imax; i++) {
      st.flist[i].n_faces = flist[i].n_faces;
      st.flist[i].face = xa_malloc(sizeof(*(flist[i].face))*flist[i].n_faces);
      memcpy(st.flist[i].face,flist[i].face,
             sizeof(*(flist[i].face))*flist[i].n_faces);
    }
  } else { /* locally get flist */
    st.flist = faces_of_vertex(m);
  }

  /* Start assuming the model is manifold, oriented, etc. */
  st.minfo.orientable = 1;
  st.minfo.oriented = 1;
  st.minfo.manifold = 1;
  st.minfo.closed = 1;
  
  /* Analyze model for each disjoint element */
  start_idx = 0;
  st.visited_vertex = xa_calloc(m->num_vert,sizeof(*(st.visited_vertex)));
  st.face_orientation = xa_calloc(m->num_faces,sizeof(*(st.face_orientation)));
  st.n_visited_vertices = 0;
  while (st.n_visited_vertices != m->num_vert) {
    while (st.visited_vertex[start_idx]) { /* Search for root of next element */
      start_idx++;
    }
    /* Walk all the vertices connected to root */
    walk_vertex_tree(m->faces,start_idx,&st);
  }

  /* Save original oriented state */
  st.minfo.orig_oriented = st.minfo.oriented;

  /* Orient model if requested and  possible */
  if (do_orient && st.minfo.orientable && !st.minfo.oriented) {
    orient_model(m,st.face_orientation);
  }

  /* Free memory */
  free(st.visited_vertex);
  free(st.face_orientation);
  free_face_lists(st.flist,m->num_vert);

  /* Return model analysis info */
  *info = st.minfo;
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


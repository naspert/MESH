/* $Id: model_analysis.c,v 1.6 2002/02/04 15:51:56 dsanta Exp $ */

#include <model_analysis.h>

#include <assert.h>
#include <xalloc.h>

/* The state for recursively walking the vertex tree */
struct vtx_walk_state {
  char *visited_vertex;          /* array to mark the already visited
                                  * vertices (visited_vertex[i] == 1 if vertex
                                  * i has been visited). */
  int n_visited_vertices;        /* the number of laredy visited vertices */
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

/* Recursively analyze the faces incident on the on any of the vertices
 * connected to vertex vidx, and updates the information in st
 * accordingly. The face analysis starts at face ofidx, which should be
 * incident on vidx and already oriented (st->face_orientation[ofidx] !=
 * 0). The vertex of each face is given by mfaces. The analysis updates the
 * oriented, orientable, manifold and closed properties of the model. The
 * orientation of each face to obtain an oriented model is recorder in
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
  int rev_orient;  /* reversed processing orientation flag */
  int tmpi;        /* temporary integer */
  int fface_orient;/* orientation of first face */
  int cface_orient;/* orientation of current face */
  int *vfaces;     /* list of faces incident on current vertex */
  int ffidx;       /* first face index */

  nf = st->flist[vidx].n_faces;
  assert(nf != 0); /* An isolated vertex should never get here  */
  vfaces = st->flist[vidx].face;
  /* Locate the face from which we are coming, so that it is the first face */
  for (j=0; j<nf; j++) {
    if (vfaces[j] == pfidx) break;
  }
  /* Get first face vertices */
  fidx = vfaces[j];
  if (vidx == mfaces[fidx].f0) {
    vstart = mfaces[fidx].f1;
    v2 = mfaces[fidx].f2;
  } else if (vidx == mfaces[fidx].f1) {
    vstart = mfaces[fidx].f2;
    v2 = mfaces[fidx].f0;
  } else { /* vidx == mfaces[fidx].f2 */
    vstart = mfaces[fidx].f0;
    v2 = mfaces[fidx].f1;
  }
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
  while (nf_left > 0) { /* process the remaining faces */
    for (j=0; j<nf; j++) { /* search for face that shares v2 */
      fidx = vfaces[j];
      if (fidx == -1) continue; /* face already counted */
      if (v2 == mfaces[fidx].f0) {
        if (vidx == mfaces[fidx].f2) { /* same orientation as first face */
          v2 = mfaces[fidx].f1;
          if (rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        } else { /* opposite orientation */
          v2 = mfaces[fidx].f2; /* do as if face orient was opposite */
          if (!rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        }
      } else if (v2 == mfaces[fidx].f1) {
        if (vidx == mfaces[fidx].f0) { /* same orientation as first face */
          v2 = mfaces[fidx].f2;
          if (rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        } else { /* opposite orientation */
          v2 = mfaces[fidx].f0; /* do as if face orient was opposite */
          if (!rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        }
      } else if (v2 == mfaces[fidx].f2) {
        if (vidx == mfaces[fidx].f1) { /* same orientation as first face */
          v2 = mfaces[fidx].f0;
          if (rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        } else { /* opposite orientation */
          v2 = mfaces[fidx].f1; /* do as if face orient was opposite */
          if (!rev_orient) {
            cface_orient = -fface_orient;
            st->minfo.oriented = 0;
          } else {
            cface_orient = fface_orient;
          }
        }
      } else { /* non-adjacent triangle */
        continue; /* test next triangle */
      }
      /* Check that we can orient face in a consistent manner */
      if (st->face_orientation[fidx] == 0) { /* not yet oriented */
        st->face_orientation[fidx] = cface_orient;
      } else if (st->face_orientation[fidx] != cface_orient) {
        st->minfo.orientable = 0; /* can not get consistent orientation */
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
      break;
    }
    if (j == nf) { /* there is no triangle that share's v2 ! */
      /* vertex vidx can be at a border or there is really a non-manifold */
      if (rev_orient) { /* we already reversed orientation => non-manifold */
        st->minfo.manifold = 0;
        st->minfo.closed = 0;
        /* Restart with first not yet counted triangle (always one) */
        for (j=1; j<nf; j++) {
          fidx = vfaces[j];
          if (fidx != -1) break;
        }
        assert(fidx >= 0);
        vfaces[j] = -1;
        nf_left--;
        /* Get new first face vertices */
        if (vidx == mfaces[fidx].f0) { /* Orientation matters here ? */
          if (!rev_orient) {
            vstart = mfaces[fidx].f1;
            v2 = mfaces[fidx].f2;
          } else {
            vstart = mfaces[fidx].f2;
            v2 = mfaces[fidx].f1;
          }
        } else if (vidx == mfaces[fidx].f1) {
          if (!rev_orient) {
            vstart = mfaces[fidx].f2;
            v2 = mfaces[fidx].f0;
          } else {
            vstart = mfaces[fidx].f0;
            v2 = mfaces[fidx].f2;
          }
        } else { /* vidx == mfaces[fidx].f2 */
          if (!rev_orient) {
            vstart = mfaces[fidx].f0;
            v2 = mfaces[fidx].f1;
          } else {
            vstart = mfaces[fidx].f1;
            v2 = mfaces[fidx].f0;
          }
        }
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
      } else {
        /* we reverse scanning orientation and continue from the other side */
        rev_orient = 1;
        tmpi = v2;
        v2 = vstart;
        vstart = tmpi;
        /* re-take original orientation */
        fface_orient = st->face_orientation[ffidx];
      }
    }
  }
  if (v2 != vstart) { /* last vertex is not the same as first one */
    st->minfo.closed = 0;
  }
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
    fl[v1].face =
      xa_realloc(fl[v1].face,(fl[v1].n_faces+1)*sizeof(*(fl->face)));
    fl[v1].face[fl[v1].n_faces++] = j;
    fl[v2].face =
      xa_realloc(fl[v2].face,(fl[v2].n_faces+1)*sizeof(*(fl->face)));
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


/* $Id: model_analysis.c,v 1.1 2001/08/16 13:05:54 dsanta Exp $ */

#include <model_analysis.h>

#include <mutils.h>

/* --------------------------------------------------------------------------*
 *                            Local functions                                *
 * --------------------------------------------------------------------------*/

/* Recursively visits all vertices connected to vertex vidx which have not
 * already been visited, marking them visited in the visited_vertex array and
 * counting them in *n_visited_vertices. The list of faces incident on each
 * vertex is given by flist, and the list of vertices in each face by
 * mfaces. The vidx vertex must already be marked as visited in
 * visited_vertex. */
static void rec_walk_vtcs(const face *mfaces, const struct face_list *flist,
                          int vidx, char *visited_vertex,
                          int *n_visited_vertices)
{
  int i,nf,fidx,new_vidx;
  int *vfaces;
  
  nf = flist[vidx].n_faces;
  vfaces = flist[vidx].face;
  for (i=0; i<nf; i++) {
    fidx = vfaces[i];
    /* Vertex 0 */
    new_vidx = mfaces[fidx].f0;
    if (!visited_vertex[new_vidx]) {
      visited_vertex[new_vidx] = 1;
      (*n_visited_vertices)++;
      rec_walk_vtcs(mfaces,flist,new_vidx,visited_vertex,n_visited_vertices);
    }
    /* Vertex 1 */
    new_vidx = mfaces[fidx].f1;
    if (!visited_vertex[new_vidx]) {
      visited_vertex[new_vidx] = 1;
      (*n_visited_vertices)++;
      rec_walk_vtcs(mfaces,flist,new_vidx,visited_vertex,n_visited_vertices);
    }
    /* Vertex 2 */
    new_vidx = mfaces[fidx].f2;
    if (!visited_vertex[new_vidx]) {
      visited_vertex[new_vidx] = 1;
      (*n_visited_vertices)++;
      rec_walk_vtcs(mfaces,flist,new_vidx,visited_vertex,n_visited_vertices);
    }
  }
}

/* Visits all the vertices of the faces in the mfaces array that are connected
 * to the vertex with index vidx. Each visited vertex, with index i, is marked
 * with a non-zero value in visited_vertex[i] and the counter
 * *n_visited_vertices is incremented. If any vertices are connected to the
 * vidx vertex (i.e. the vertex belongs to some face) the
 * info->n_disjoint_parts counter is incremented by one. The list of faces
 * incident on each vertex is given by flist. */
static void walk_vertices(const face *mfaces, const struct face_list *flist,
                          int vidx, char *visited_vertex,
                          int *n_visited_vertices, struct model_info *info) {
  /* Mark vidx vertex as visited */
  visited_vertex[vidx] = 1;
  (*n_visited_vertices)++;
  /* Ignore isolated vertices */
  if (flist[vidx].n_faces == 0) return;
  /* Recursively walk all vertices connected to vidx */
  rec_walk_vtcs(mfaces,flist,vidx,visited_vertex,n_visited_vertices);
  info->n_disjoint_parts++;
}

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See model_analysis.h */
void analyze_model(const model *m, const struct face_list *flist,
                   struct model_info *info)
{
  int i,j,imax;             /* counters and loop limits */
  int *vfaces;              /* list of faces incident on current vertex */
  int vfaces_sz;            /* size of the vfaces buffer */
  int nf;                   /* number of faces incident on current vertex */
  int nf_left;              /* number of not yet processed faces in vfaces */
  int fidx;                 /* current face index */
  int v2;                   /* vertex on which next face should be incident */
  int vstart;               /* starting vertex, to check for closed surface */
  int rev_orient;           /* reversed processing orientation flag */
  int tmpi;                 /* temporary integer */
  int start_idx;            /* index where to start next vertex walk */
  char *visited_vertex;     /* array to mark visited vertices in walk */
  int n_visited_vertices;   /* number of already visited vertices in walk */
  int local_flist;          /* flag indicating locally allocated flist */

  /* Initialize */
  vfaces = NULL;
  vfaces_sz = 0;
  if (flist == NULL) {
    local_flist = 1;
    flist = faces_of_vertex(m);
  } else {
    local_flist = 0;
  }

  /* Start assuming the model is manifold, oriented, etc. */
  info->oriented = 1;
  info->manifold = 1;
  info->closed = 1;
  
  /* Perform vertex neighbourhood analysis, for each one */
  for (i=0, imax = m->num_vert; i<imax; i++) {
    /* Get faces on vertex in a modifiable buffer */
    nf = flist[i].n_faces;
    if (nf == 0) continue; /* An isolated vertex (no face uses it): ignore */
    if (vfaces_sz < nf) {
      vfaces = xrealloc(vfaces,sizeof(*vfaces)*nf);
      vfaces_sz = nf;
    }
    memcpy(vfaces,flist[i].face,sizeof(*vfaces)*nf);
    /* Get first face vertices */
    fidx = vfaces[0];
    if (i == m->faces[fidx].f0) {
      vstart = m->faces[fidx].f1;
      v2 = m->faces[fidx].f2;
    } else if (i == m->faces[fidx].f1) {
      vstart = m->faces[fidx].f2;
      v2 = m->faces[fidx].f0;
    } else { /* i == m->faces[fidx].f2 */
      vstart = m->faces[fidx].f0;
      v2 = m->faces[fidx].f1;
    }
    /* Check the other faces in an oriented order */
    nf_left = nf-1;
    vfaces[0] = -1;
    rev_orient = 0;
    while (nf_left > 0) { /* process the remaining faces */
      for (j=1; j<nf; j++) { /* search for face that shares v2 */
        fidx = vfaces[j];
        if (fidx == -1) continue; /* face already counted */
        if (v2 == m->faces[fidx].f0) {
          if (i == m->faces[fidx].f2) { /* same orientation */
            v2 = m->faces[fidx].f1;
            if (rev_orient) info->oriented = 0;
          } else { /* opposite orientation */
            v2 = m->faces[fidx].f2; /* do as if face orient was opposite */
            if (!rev_orient) info->oriented = 0;
          }
        } else if (v2 == m->faces[fidx].f1) {
          if (i == m->faces[fidx].f0) { /* same orientation */
            v2 = m->faces[fidx].f2;
            if (rev_orient) info->oriented = 0;
          } else { /* opposite orientation */
            if (!rev_orient) info->oriented = 0;
            v2 = m->faces[fidx].f0; /* do as if face orient was opposite */
          }
        } else if (v2 == m->faces[fidx].f2) {
          if (i == m->faces[fidx].f1) { /* same orientation */
            v2 = m->faces[fidx].f0;
            if (rev_orient) info->oriented = 0;
          } else { /* opposite orientation */
            if (!rev_orient) info->oriented = 0;
            v2 = m->faces[fidx].f1; /* do as if face orient was opposite */
          }
        } else { /* non-adjacent triangle */
          continue; /* test next triangle */
        }
        vfaces[j] = -1; /* mark face as counted */
        nf_left--;  /* goto search for face that shares new v2 */
        break;
      }
      if (j == nf) { /* there is no triangle that share's v2 ! */
        /* vertex i can be at a border or there is really a non-manifold here */
        if (rev_orient) { /* we already reversed orientation => non-manifold */
          info->manifold = 0;
          info->closed = 0;
          /* Restart with first not yet counted triangle (always one) */
          for (j=1; j<nf; j++) {
            fidx = vfaces[j];
            if (fidx != -1) break;
          }
          vfaces[j] = -1;
          nf_left--;
          if (i == m->faces[fidx].f0) { /* Orientation matters here ? */
            v2 = rev_orient ? m->faces[fidx].f2 : m->faces[fidx].f1;
          } else if (i == m->faces[fidx].f1) {
            v2 = rev_orient ? m->faces[fidx].f0 : m->faces[fidx].f2;
          } else { /* i == m->faces[fidx].f2 */
            v2 = rev_orient ? m->faces[fidx].f1 : m->faces[fidx].f0;
          }
        } else { /* we reverse scanning orientation and continue */
          rev_orient = 1;
          tmpi = v2;
          v2 = vstart;
          vstart = tmpi;
        }
      }
    }
    if (v2 != vstart) { /* last vertex is not the same as first one */
      info->closed = 0;
    }
  }
  free(vfaces);
  vfaces = NULL;

  /* Search number of disjoint elements */
  info->n_disjoint_parts = 0;
  start_idx = 0;
  visited_vertex = xcalloc(m->num_vert,sizeof(*visited_vertex));
  n_visited_vertices = 0;
  do {
    while (visited_vertex[start_idx]) { /* Search for root of next element */
      start_idx++;
    }
    /* Walk all the vertices connected to root */
    walk_vertices(m->faces,flist,start_idx,visited_vertex,
                  &n_visited_vertices,info);
  } while (n_visited_vertices != m->num_vert);
  free(visited_vertex);
  visited_vertex = NULL;

  if (local_flist) {
    /* Since flist is locally created, we can make it non-const */
    free_face_lists((struct face_list*)flist,m->num_vert);
  }
}

/* See model_analysis.h */
struct face_list *faces_of_vertex(const model *m)
{
  int j,jmax;           /* indices and loop limits */
  int v0,v1,v2;         /* current triangle's vertex indices */
  struct face_list *fl; /* the face list to return */

  fl = xcalloc(m->num_vert,sizeof(*fl));
  for (j=0, jmax=m->num_faces; j<jmax; j++) {
    v0 = m->faces[j].f0;
    v1 = m->faces[j].f1;
    v2 = m->faces[j].f2;
    fl[v0].face = xrealloc(fl[v0].face,(fl[v0].n_faces+1)*sizeof(*(fl->face)));
    fl[v0].face[fl[v0].n_faces++] = j;
    fl[v1].face = xrealloc(fl[v1].face,(fl[v1].n_faces+1)*sizeof(*(fl->face)));
    fl[v1].face[fl[v1].n_faces++] = j;
    fl[v2].face = xrealloc(fl[v2].face,(fl[v2].n_faces+1)*sizeof(*(fl->face)));
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


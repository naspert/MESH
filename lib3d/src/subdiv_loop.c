/* $Id: subdiv_loop.c,v 1.5 2002/11/06 07:58:41 aspert Exp $ */
#include <3dmodel.h>
#include <normals.h>
#include <geomutils.h>
#include <subdiv_methods.h>
#include <assert.h>

void compute_midpoint_loop(const struct ring_info *rings, const int center, 
                           const int v1, 
			   const struct model *raw_model, vertex_t *vout) 
{

  vertex_t np, tmp;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  int n = rings[center].size;
  int p0, p1;

  __add_v(raw_model->vertices[center], raw_model->vertices[center2], np);
  __prod_v(0.375, np, np);

  p0 = ring.ord_vert[(v1+1)%n];
  if (v1 > 0)
    p1 = ring.ord_vert[v1-1];
  else
    p1 = ring.ord_vert[n-1];
 
  __add_v(raw_model->vertices[p0], raw_model->vertices[p1], tmp);

  __add_prod_v(0.125, tmp, np, np);

  *vout = np;

}


void compute_midpoint_loop_crease(const struct ring_info *rings, 
                                  const int center,  
				  const int v1, const struct model *raw_model, 
				  vertex_t *vout) {
  int n = rings[center].size, m;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int v2 = 0; /* index of center vertex_t in opp. ring */
  vertex_t p;

  while (ring_op.ord_vert[v2] != center)
    v2++;
  m = ring_op.size;

  if (ring.type == 1 && ring_op.type == 1) { /* we have an boundary here */
    __add_v(raw_model->vertices[center], raw_model->vertices[center2], p);
    prod_v(0.5, &p, vout);
    return;
  } else if (ring.type == 1) { /* only one-half-boundary vertex */
    if (n < 7) {
      compute_midpoint_loop(rings, center, v1, raw_model, &p);
      *vout = p;
      return;
    } else {
      __prod_v(0.25 + 0.25*cos(2*M_PI/(n-1)), raw_model->vertices[center], p);
      __add_prod_v(0.5 - 0.25*cos(2*M_PI/(n-1)), 
                   raw_model->vertices[center2], p, p);
      __add_prod_v(0.125, raw_model->vertices[ring.ord_vert[0]], p, p);
      __add_prod_v(0.125, raw_model->vertices[ring.ord_vert[n-1]], p, p);
      *vout = p;
      return;
     } 
  } else {/* ring_op.type == 1 */
    if (m < 7) {
      compute_midpoint_loop(rings, center2, v2, raw_model, &p);
      *vout = p;
      return;
    } else {
      __prod_v(0.25 + 0.25*cos(2*M_PI/(m-1)), raw_model->vertices[center2], p);
      __add_prod_v(0.5 - 0.25*cos(2*M_PI/(m-1)), 
                   raw_model->vertices[center], p, p);
      __add_prod_v(0.125, raw_model->vertices[ring_op.ord_vert[0]], p, p);
      __add_prod_v(0.125, raw_model->vertices[ring_op.ord_vert[m-1]], p, p);
      *vout = p;
      return;
    } 
  }
}

void update_vertices_loop(const struct model *or_model, 
                          struct model *subdiv_model, 
                          const struct ring_info *rings) {
  int i, j, v, n;
  float beta;
  vertex_t tmp;

  for (i=0; i<or_model->num_vert; i++) {
    n = rings[i].size;
    if (rings[i].type == 0) {
      if (n == 3)
	beta = 0.1875; /* 3/16 */
      else
	beta = 0.375/n;
      
      __prod_v(1.0-n*beta, or_model->vertices[i], tmp);
      
      for (j=0; j<n; j++) {
	v = rings[i].ord_vert[j];
	__add_prod_v(beta, or_model->vertices[v], tmp, tmp);
      }

    } else {
      __add_v(or_model->vertices[rings[i].ord_vert[0]], 
              or_model->vertices[rings[i].ord_vert[n-1]], tmp);
      __prod_v(0.125, tmp, tmp);
      __add_prod_v(0.75, or_model->vertices[i], tmp, tmp);
      
    }
    subdiv_model->vertices[i] = tmp;
  }

}


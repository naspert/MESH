/* $Id: subdiv_loop.c,v 1.3 2002/11/01 10:06:14 aspert Exp $ */
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
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int v2 = 0; /* index of center vertex_t in opp. ring */
  vertex_t p;

  while (ring_op.ord_vert[v2] != center)
    v2++;

  if (ring.type == 1 && ring_op.type == 1) { /* we have an boundary here */
    add_v(&(raw_model->vertices[center]), 
	  &(raw_model->vertices[center2]), &p);
    prod_v(0.5, &p, vout);
    return;
  } else if (ring.type == 1) { /* only one-half-boundary vertex */
    if (n < 7) {
      compute_midpoint_loop(rings, center, v1, raw_model, &p);
      *vout = p;
      return;
    } else {
      prod_v(0.5, &(raw_model->vertices[center]), &p);
      add_prod_v(0.25, &(raw_model->vertices[center2]), &p, &p);
      add_prod_v(0.125, &(raw_model->vertices[ring.ord_vert[0]]), &p, &p);
      add_prod_v(0.125, &(raw_model->vertices[ring.ord_vert[n-1]]), &p, &p);
      *vout = p;
      return;
    }
  } else {/* ring_op.type == 1 */
    if (n < 7) {
      compute_midpoint_loop(rings, center2, v2, raw_model, &p);
      *vout = p;
      return;
    } else {
      prod_v(0.5, &(raw_model->vertices[center2]), &p);
      add_prod_v(0.25, &(raw_model->vertices[center]), &p, &p);
      add_prod_v(0.125, &(raw_model->vertices[ring_op.ord_vert[0]]), &p, &p);
      add_prod_v(0.125, &(raw_model->vertices[ring_op.ord_vert[n-1]]), &p, &p);
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
	beta = 3.0/16.0;
      else
	beta = 3.0/(8.0*n);
      
      prod_v(1.0-n*beta, &(or_model->vertices[i]), &tmp);
      
      for (j=0; j<n; j++) {
	v = rings[i].ord_vert[j];
	add_prod_v(beta, &(or_model->vertices[v]), &tmp, &tmp);
      }

    } else {
      add_v(&(or_model->vertices[rings[i].ord_vert[0]]), 
	    &(or_model->vertices[rings[i].ord_vert[n-1]]), &tmp);
      prod_v(0.125, &tmp, &tmp);
      add_prod_v(0.75, &(or_model->vertices[i]), &tmp, &tmp);
      
    }
    subdiv_model->vertices[i] = tmp;
  }

}


/* $Id$ */

#include <3dmodel.h>
#include <normals.h>
#include <geomutils.h>
#include <subdiv_methods.h>
#if defined(DEBUG) || defined(BOUNDARY_SUBDIV_DEBUG) || defined(SUBDIV_BUTTERFLY_DEBUG)
# include <debug_print.h>
#endif

/* These are parameters for Butterfly subdivision */
# define _2W_    0.0           /* w = 0.0 gives better results
                                * ... 1.0/16.0 could be another
                                * possible choice for w */
# define _M1_12 -0.0833333333333 /* -1.0/12.0 */
# define _5_12   0.4166666666666 /* 5.0/12.0 */

/* Precomputed regular stencil */
static const float reg_sten[6] = {0.25 - _2W_,  
                                  0.125 + _2W_, -0.125 - _2W_, 
                                  _2W_, -0.125 - _2W_, 0.125 + _2W_};
static const float sten_4[4] = {0.375, 0.0, -0.125, 0.0};
static const float sten_3[3] = {_5_12, _M1_12, _M1_12};

/* 
 * Builds a Butterfly subd. mask of size n for irregular vertices.
 * The array must be pre-malloc'ed !
 */
static void make_sub_mask(float *mask, int n) {
  int j;
  float inv_n;

  switch(n) {
  case 3:
    memcpy(mask, sten_3, 3*sizeof(float));
    break;
  case 4:
    memcpy(mask, sten_4, 4*sizeof(float));
    break;
  default: /* n = 5, 7, 8, ... */
    inv_n = 1.0/(float)n;
    for (j=0; j<n; j++) 
      mask[j] = (0.25 + cos(2*M_PI*j*inv_n) +
                 0.5*cos(4*M_PI*j*inv_n))*inv_n;
    break;
  }
}

void compute_midpoint_butterfly(const struct ring_info *rings, 
                                const int center, const int v1, 
				const struct model *raw_model, 
				float (*sph_h_func)(const float),
                                vertex_t *vout) 
{
  float *s=NULL, *t=NULL;
  int j;
  vertex_t p={0.0, 0.0, 0.0}, r={0.0, 0.0, 0.0};
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size; /* size of opp. ring */
  int v2 = 0; /* index of center vertex_t in opp. ring */



#ifdef BOUNDARY_SUBDIV_DEBUG
  DEBUG_PRINT("Subdiv edge %d %d\n", center, center2);
  DEBUG_PRINT("n=%d m=%d\n", n, m);
#endif

#ifdef SUBDIV_BUTTERFLY_DEBUG
  DEBUG_PRINT("Subdiv edge %d %d\n", center, center2);
  DEBUG_PRINT("n=%d m=%d\n", n, m);
  DEBUG_PRINT("center %d = %f %f %f\n", center,  
              raw_model->vertices[center].x, 
              raw_model->vertices[center].y, 
              raw_model->vertices[center].z);
  DEBUG_PRINT("center2 %d = %f %f %f\n", center2, 
              raw_model->vertices[center2].x, 
              raw_model->vertices[center2].y, 
              raw_model->vertices[center2].z);
#endif

  if (n != 6 && m != 6) {/* double irreg */    
    while (ring_op.ord_vert[v2] != center)
      v2++;
    
    s = (float*)malloc(n*sizeof(float));
    t = (float*)malloc(m*sizeof(float));

    /* Compute values of stencil for end-vertex */
    make_sub_mask(t, m);


    /* Compute values of stencil for center vertex */
    make_sub_mask(s, n);


    /* Apply stencil to 1st vertex */
    for (j=0; j<n; j++) {
      __add_prod_v(s[j], raw_model->vertices[ring.ord_vert[(v1+j)%n]], p, 
                   p);

#ifdef SUBDIV_BUTTERFLY_DEBUG
      DEBUG_PRINT("s[%d]=%f\n",j, s[j]);
      DEBUG_PRINT("v = %f %f %f\n", 
                  raw_model->vertices[ring.ord_vert[(v1+j)%n]].x,
                  raw_model->vertices[ring.ord_vert[(v1+j)%n]].y,
                  raw_model->vertices[ring.ord_vert[(v1+j)%n]].z);
      DEBUG_PRINT("idx = %d\n", (v1+j)%n);
      DEBUG_PRINT("%d: p = %f %f %f\n", j,p.x, p.y, p.z);
#endif
    }



    __add_prod_v(0.75, (raw_model->vertices[center]), p, p);


    
    /* Apply stencil to end vertex */
    for (j=0; j<m; j++) {
      __add_prod_v(t[j], raw_model->vertices[ring_op.ord_vert[(v2+j)%m]], r, 
                   r);
    }


    __add_prod_v(0.75, raw_model->vertices[center2], r, r);

    __add_v(p, r, p);
    __prod_v(0.5, p, p);
    
    free(s);
    free(t);
  }
  else if (n == 6 && m == 6) {/* regular */
    /* apply the 10 point stencil */


    while (ring_op.ord_vert[v2] != center)
      v2++;

    /* Apply stencil to 1st vertex_t */
    for (j=0; j<6; j++) {
      __add_prod_v(reg_sten[j], 
                   raw_model->vertices[ring.ord_vert[(v1+j)%6]], p, p);
    }

    __add_prod_v(0.75, raw_model->vertices[center], p, p);


    /* Apply stencil to end vertex_t */
    for (j=0; j<6; j++) {
      __add_prod_v(reg_sten[j], 
                   raw_model->vertices[ring_op.ord_vert[(v2+j)%6]], p, p);
    }

    __add_prod_v(0.75, raw_model->vertices[center2], p, p);


    __prod_v(0.5, p, p);

  }
  else if (n!=6 && m==6){ /* only one irreg. vertex_t */
    s = (float*)malloc(n*sizeof(float));
    make_sub_mask(s, n);

    for (j=0; j<n; j++) {
      __add_prod_v(s[j], raw_model->vertices[ring.ord_vert[(v1+j)%n]], p, 
                   p);
    }

    __add_prod_v(0.75, raw_model->vertices[center], p, p);

    free(s);
  } else if (n==6 && m!=6) {
    t = (float*)malloc(m*sizeof(float));
    make_sub_mask(t, m);

    while (ring_op.ord_vert[v2] != center)
      v2++;

    for (j=0; j<m; j++) {
      __add_prod_v(t[j], raw_model->vertices[ring_op.ord_vert[(v2+j)%m]], p,
                   p);
    }

    __add_prod_v(0.75, raw_model->vertices[center2], p, p);

    free(t);
  } 
  
  *vout = p;
}


void compute_midpoint_butterfly_crease(const struct ring_info *rings, 
                                       const int center,  
				       const int v1, 
                                       const struct model *raw_model, 
				       float (*sph_h_func)(const float),
				       vertex_t *vout) 
{
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size;
  int v2 = 0; /* index of center vertex_t in opp. ring */
  int v3 = -1, v4 = -1;
  vertex_t p, q, np, r;

  p = raw_model->vertices[center];
  q = raw_model->vertices[center2];
  
  while (ring_op.ord_vert[v2] != center)
    v2++;


  if (ring.type == 1 && ring_op.type == 1) {
    
    __add_v(p, q, np);
    
    if (ring.ord_vert[0] == center2)
      v3 = ring.ord_vert[n - 1];
    else if (ring.ord_vert[n - 1] == center2)
      v3 = ring.ord_vert[0];
    else { /* we have a non-boundary -> midpoint */
      prod_v(0.5, &np, vout);
      return;
    }

    if (ring_op.ord_vert[0] == center)
      v4 = ring_op.ord_vert[m-1];
    else if (ring_op.ord_vert[m-1] == center)
      v4 = ring_op.ord_vert[0];
    else {
      prod_v(0.5, &np, vout);
      return;
    }

    /* If we are here, we found a true boundary */
    __prod_v(0.5625, np, np);
    __add_v(raw_model->vertices[v3], raw_model->vertices[v4], r);
    __prod_v(-0.0625, r, r);
    add_v(&np, &r, vout);
  } else if (ring.type == 1) 
    compute_midpoint_butterfly(rings, center2, v2, 
                               raw_model, sph_h_func, vout);
  else
    compute_midpoint_butterfly(rings, center, v1, 
                               raw_model, sph_h_func, vout);
}

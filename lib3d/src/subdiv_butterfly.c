/* $Id: subdiv_butterfly.c,v 1.3 2002/06/04 08:46:04 aspert Exp $ */

#include <3dmodel.h>
#include <normals.h>
#include <geomutils.h>
#include <subdiv_methods.h>
#include <assert.h>

/* These are parameters for Butterfly subdivision */
# define _Q_     0.75
# define _2W_    0.0           /* w = 0.0 gives better results
                                * ... 1.0/16.0 could be another
                                * possible choice for w */
# define _M1_12 -0.0833333333333 /* -1.0/12.0 */
# define _5_12   0.4166666666666 /* 5.0/12.0 */

/* Precomputed regular stencil */
const static float reg_sten[6] = {0.25 - _2W_,  
                                  0.125 + _2W_, -0.125 - _2W_, 
                                  _2W_, -0.125 - _2W_, 0.125 + _2W_};
const static float sten_4[4] = {0.375, 0.0, -0.125, 0.0};
const static float sten_3[3] = {_5_12, _M1_12, _M1_12};

/* 
 * Builds a Butterfly subd. mask of size n for irregular vertices.
 * The array must be pre-malloc'ed !
 */
void make_sub_mask(float *mask, int n) {
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

void compute_midpoint_butterfly(struct ring_info *rings, 
                                int center, int v1, 
				struct model *raw_model, vertex_t *vout) {
  float *s=NULL, *t=NULL;
  int j;
  vertex_t p={0.0, 0.0, 0.0}, r={0.0, 0.0, 0.0};
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size; /* size of opp. ring */
  int v2 = 0; /* index of center vertex_t in opp. ring */



#ifdef __BOUNDARY_SUBDIV_DEBUG
  printf("Subdiv edge %d %d\n", center, center2);
  printf("n=%d m=%d\n", n, m);
#endif

#ifdef __SUBDIV_BUTTERFLY_DEBUG
  printf("Subdiv edge %d %d\n", center, center2);
  printf("n=%d m=%d\n", n, m);
  printf("center %d = %f %f %f\n", center,  raw_model->vertices[center].x, 
	 raw_model->vertices[center].y, 
	 raw_model->vertices[center].z);
  printf("center2 %d = %f %f %f\n", center2, raw_model->vertices[center2].x, 
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

#ifdef __SUBDIV_BUTTERFLY_DEBUG
      printf("s[%d]=%f\n",j, s[j]);
      printf("v = %f %f %f\n", raw_model->vertices[ring.ord_vert[(v1+j)%n]].x,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].y,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].z);
      printf("idx = %d\n", (v1+j)%n);
      printf("%d: p = %f %f %f\n", j,p.x, p.y, p.z);
#endif
    }



    __add_prod_v(_Q_, (raw_model->vertices[center]), p, p);


    
    /* Apply stencil to end vertex */
    for (j=0; j<m; j++) {
      __add_prod_v(t[j], raw_model->vertices[ring_op.ord_vert[(v2+j)%m]], r, 
                   r);
    }


    __add_prod_v(_Q_, raw_model->vertices[center2], r, r);

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

    __add_prod_v(_Q_, raw_model->vertices[center], p, p);


    /* Apply stencil to end vertex_t */
    for (j=0; j<6; j++) {
      __add_prod_v(reg_sten[j], 
                   raw_model->vertices[ring_op.ord_vert[(v2+j)%6]], p, p);
    }

    __add_prod_v(_Q_, raw_model->vertices[center2], p, p);


    __prod_v(0.5, p, p);

  }
  else if (n!=6 && m==6){ /* only one irreg. vertex_t */
    s = (float*)malloc(n*sizeof(float));
    make_sub_mask(s, n);

    for (j=0; j<n; j++) {
      __add_prod_v(s[j], raw_model->vertices[ring.ord_vert[(v1+j)%n]], p, 
                   p);
    }

    __add_prod_v(_Q_, raw_model->vertices[center], p, p);

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

    __add_prod_v(_Q_, raw_model->vertices[center2], p, p);

    free(t);
  } 
  
  *vout = p;
}


void compute_midpoint_butterfly_crease(struct ring_info *rings, int center,  
				       int v1, struct model *raw_model, 
				       vertex_t *vout) {
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size;
  int v2 = 0; /* index of center vertex_t in opp. ring */
  int i, tmp;
  vertex_t p={0.0, 0.0, 0.0}, r={0.0, 0.0, 0.0};
  float *s, *t, thk, q;

  while (ring_op.ord_vert[v2] != center)
    v2++;

  printf("ring.type = %d\tring_op.type = %d\n", ring.type, ring_op.type);
  printf("n = %d\tm = %d\n", n, m);
  if (ring.type == 1 && ring_op.type == 1) {

    add_v(&raw_model->vertices[center], &raw_model->vertices[center2], &p);

    if ((v1 != 0 && v1 != n-1) ||
        (v2 != 0 && v2 != m-1)) { /* not real crease -> bail out in a
                                     graceful way by taking the true
                                     midpoint of the edge */
      prod_v(0.5, &p, vout);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "crease-crease rule : midpoint %d %d\n", center, 
              center2);
#endif
      return;
    }

    /* otherwise we have a 'classical' boundary edge */
    prod_v(0.5625, &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
    fprintf(stderr, "9/16 * (v[%d] + v[%d])\n", center, center2);
#endif

    assert(v1 == 0 || v1 == n-1);
    if (v1 == 0) {
      tmp = n-1;
      add_prod_v(-0.0625, &raw_model->vertices[ring.ord_vert[n-1]], &p, &p);
    }
    else {
      tmp = 0;
      add_prod_v(-0.0625, &raw_model->vertices[ring.ord_vert[0]], &p, &p);
    }

#ifdef __BUTTERFLY_CREASE_DEBUG
    fprintf(stderr, "-1/16 * v[%d]\n", ring.ord_vert[tmp;
#endif

    assert(v2 == 0 || v2 == m-1);
    if (v2 == 0) {
      tmp = 0;
      add_prod_v(-0.0625, &raw_model->vertices[ring_op.ord_vert[m-1]], &p, 
                 vout);
    }
    else {
      tmp = m-1;
      add_prod_v(-0.0625, &raw_model->vertices[ring_op.ord_vert[0]], &p, 
                 vout);
    }

#ifdef __BUTTERFLY_CREASE_DEBUG
    fprintf(stderr, "-1/16 * v[%d]\n", ring_op.ord_vert[tmp]);
#endif

    return;
  
  } else if (ring.type == 1) {
    if (ring.size == 4 && ring_op.size == 6) {/* regular crease-int. */

    } else if (ring.size != 4 && ring_op.size != 6) { /* extr. crease
                                                       * - extr. int */
      s = (float*)malloc(n*sizeof(float));
      thk = M_PI/(float)(n-2);
      q = 1.0 - sin(thk)*sin(v1*thk)/((n-2)*(1 - cos(thk)));
      prod_v(q, &(raw_model->vertices[center]), &p);
      
      s[0] = 0.25*(cos(v1*thk) - sin(2.0*thk)*sin(2.0*v1*thk)/
		   ((n-2)*(cos(thk) - cos(2.0*thk))));
      s[n-1] = s[0];
      
      add_prod_v(s[0], &(raw_model->vertices[ring.ord_vert[0]]), &p, &p);

      for (i=1; i<n-1; i++) {
	s[i] = (sin(v1*thk)*sin(i*thk) + 0.5*sin(2*v1*thk)*sin(2*i*thk))/
	  (float)(n-1);
	add_prod_v(s[i], &(raw_model->vertices[ring.ord_vert[i]]), &p, &p);
      }
      add_prod_v(s[n-1], &(raw_model->vertices[ring.ord_vert[n-1]]), 
		 &p, &p);
      free(s);

      t = (float*)malloc(m*sizeof(float));
      make_sub_mask(t, m);
      for (i=0; i<m; i++) {
        add_prod_v(t[i], &raw_model->vertices[ring_op.ord_vert[(v2+i)%m]], &r, 
                   &r);

      }
      free(t);

      add_v(&p, &r, vout);
      prod_v(0.5, vout, vout);
      return;
    } else 
      abort();
  } else { /* ring_op.type == 1 */
    if (ring_op.size == 4 && ring.size == 6) {/* regular crease-int. */

    } else if (ring_op.size != 4 && ring.size != 6) { /* extr. crease */
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "extr. crease %d\n", center2);
#endif
      s = (float*)malloc(m*sizeof(float));
      thk = M_PI/(float)(m-2);
      q = 1.0 - sin(thk)*sin(v2*thk)/((m-2)*(1 - cos(thk)));
      prod_v(q, &(raw_model->vertices[center2]), &p);
      
      s[0] = 0.25*(cos(v2*thk) - sin(2.0*thk)*sin(2.0*v2*thk)/
		   ((m-2)*(cos(thk) - cos(2.0*thk))));
      s[m-1] = s[0];

      add_prod_v(s[0], &(raw_model->vertices[ring_op.ord_vert[0]]), 
		 &p, &p);
      for (i=1; i<m-1; i++) {
	s[i] = (sin(v2*thk)*sin(i*thk) + 0.5*sin(2*v2*thk)*sin(2*i*thk))/
	  (float)(m-1);
	add_prod_v(s[i], &(raw_model->vertices[ring_op.ord_vert[i]]), 
		   &p, &p);
      }
      add_prod_v(s[m-1], &(raw_model->vertices[ring_op.ord_vert[m-1]]), &p, 
		 &p);
      free(s);
      t = (float*)malloc(n*sizeof(float));
      make_sub_mask(t, n);
      for (i=0; i<n; i++) {
        add_prod_v(t[i], &raw_model->vertices[ring.ord_vert[(v1+i)%n]], &r, 
                   &r);

      }
      free(t);

      add_v(&p, &r, vout);
      prod_v(0.5, vout, vout);
      return;
    } else 
      abort();
  }
}

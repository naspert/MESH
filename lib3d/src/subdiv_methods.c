/* $Id: subdiv_methods.c,v 1.7 2002/02/13 10:25:57 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>
#include <subdiv_methods.h>

void compute_midpoint_sph(struct ring_info *rings, int center, int v1, 
			  struct model *raw_model, vertex_t *vout) {

  int center2 = rings[center].ord_vert[v1];
  struct ring_info ring_op = rings[center2];
  int v2 = 0;
  vertex_t n,p, vj, dir, m, u, v, np1, np2, np;
  float r, ph, lambda, pl_off, nr, nph, dz, rp, g;


  n = raw_model->normals[center];
  p = raw_model->vertices[center];
  vj = raw_model->vertices[rings[center].ord_vert[v1]];
  
  pl_off = -scalprod_v(&p, &n);

  substract_v(&vj, &p, &dir);
  
  r = norm_v(&dir);
  
  lambda = -(pl_off + scalprod_v(&vj, &n));
  
  prod_v(lambda, &n, &m);

  add_v(&vj, &m, &u);

  substract_v(&u, &p, &v);


  if (lambda >= 0.0)
    ph = -atan(norm_v(&m)/norm_v(&v));
  else
    ph = atan(norm_v(&m)/norm_v(&v));



#ifdef __SUBDIV_SPH_DEBUG
  printf("p[%d] = %f %f %f\n", center, p.x, p.y, p.z);
  printf("n[%d] = %f %f %f\n", center, n.x, n.y, n.z);
  printf("v[%d] = %f %f %f\n", rings[center].ord_vert[v1], vj.x, vj.y, vj.z);
  printf("pl_off = %f\n", pl_off);
  printf("r = %f\n", r);
  printf("phi = %f\n", ph*180.0/M_PI);
  printf("u.n + d = %f\n", scalprod(u,n)+pl_off);
  printf("test %f %f %f\n", norm(v), r*cos(ph), norm(v)-r*cos(ph));
#endif

  /* Compute the new position */
  nr = 0.5*r;
  if (ph <= -M_PI_2 || ph >= M_PI_2)
    nph = ph;
  else if (ph < -M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 + 1.0)*(ph/M_PI_4 + 1.0));
    nph = g*ph;
  } else if (ph > M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 - 1.0)*(ph/M_PI_4 - 1.0));
    nph = g*ph;
  } else {
    nph = 0.5*ph; 
  }

  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize_v(&v);
  
  prod_v(rp, &v, &np1);
  
 
  np1.x += dz*n.x + p.x;
  np1.y += dz*n.y + p.y;
  np1.z += dz*n.z + p.z;

  while (ring_op.ord_vert[v2] != center)
      v2++;
  
  n = raw_model->normals[center2];
  p = raw_model->vertices[center2];
  vj = raw_model->vertices[ring_op.ord_vert[v2]];
  
  pl_off = -scalprod_v(&p, &n);
  
  substract_v(&vj, &p, &dir);
  

  
  r = norm_v(&dir);
  
  lambda = -(pl_off + scalprod_v(&vj, &n));
  
  prod_v(lambda, &n, &m);

  add_v(&vj, &m, &u);

  substract_v(&u, &p, &v);


  if (lambda >= 0.0)
    ph = -atan(norm_v(&m)/norm_v(&v));
  else
    ph = atan(norm_v(&m)/norm_v(&v));


#ifdef __SUBDIV_SPH_DEBUG
  printf("p[%d] = %f %f %f\n", center, p.x, p.y, p.z);
  printf("n[%d] = %f %f %f\n", center, n.x, n.y, n.z);
  printf("v[%d] = %f %f %f\n", rings[center].ord_vert[v1], vj.x, vj.y, vj.z);
  printf("pl_off = %f\n", pl_off);
  printf("r = %f\n", r);
  printf("phi = %f\n", ph*180.0/M_PI);
  printf("u.n + d = %f\n", scalprod(u,n)+pl_off);
  printf("test %f %f %f\n", norm(v), r*cos(ph), norm(v)-r*cos(ph));
#endif

  nr = 0.5*r;
  if (ph <= -M_PI_2 || ph >= M_PI_2)
    nph = ph;
  else if (ph < -M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 + 1.0)*(ph/M_PI_4 + 1.0));
    nph = g*ph;
  } else if (ph > M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 - 1.0)*(ph/M_PI_4 - 1.0));
    nph = g*ph;
  } else {
    nph = 0.5*ph; 
  } 



  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize_v(&v);
  prod_v(rp, &v, &np2);

  np2.x += dz*n.x + p.x;
  np2.y += dz*n.y + p.y;
  np2.z += dz*n.z + p.z;


  add_v(&np1, &np2, &np);
  prod_v(0.5, &np, &np);


  *vout = np;
}


void compute_midpoint_butterfly(struct ring_info *rings, int center,  int v1, 
				struct model *raw_model, vertex_t *vout) {
  float *s, *t;
  int j;
  vertex_t p, r;
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
    
    s = (float*)calloc(n, sizeof(float));
    t = (float*)calloc(m, sizeof(float));

    /* Compute values of stencil for end-vertex_t */
    if (m > 4) {
      for (j=0; j<m; j++) {
 	t[j] = (0.25 + cos(2*M_PI*j/(float)m) + 
		0.5*cos(4*M_PI*j/(float)m))/(float)m;
      }
      
    } else if (m == 4) {
      t[0] = 0.375;
      t[2] = -0.125;
    } else if (m == 3) {
      t[0] = 5.0/12.0;
      t[1] = -1.0/12.0;
      t[2] = t[1];
    }


    /* Compute values of stencil for center vertex_t */
    if (n > 4) {
      for (j=0; j<n; j++) {
	s[j] = (0.25 + cos(2*M_PI*j/(float)n) + 
		0.5*cos(4*M_PI*j/(float)n))/(float)n;
      }

      
    } else if (n == 4) {
      s[0] = .375;
      s[2] = -0.125;
    } else if (n == 3) {
      s[0] = 5.0/12.0;
      s[1] = -1.0/12.0;
      s[2] = s[1];
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    r.x = 0.0;
    r.y = 0.0;
    r.z = 0.0;

    /* Apply stencil to 1st vertex_t */
    for (j=0; j<n; j++) {
      add_prod_v(s[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%n]]), &p, 
		 &p);

#ifdef __SUBDIV_BUTTERFLY_DEBUG
      printf("s[%d]=%f\n",j, s[j]);
      printf("v = %f %f %f\n", raw_model->vertices[ring.ord_vert[(v1+j)%n]].x,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].y,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].z);
      printf("idx = %d\n", (v1+j)%n);
      printf("%d: p = %f %f %f\n", j,p.x, p.y, p.z);
#endif
    }

    add_prod_v(__QS, &(raw_model->vertices[center]), &p, &p);


    
    /* Apply stencil to end vertex_t */
    for (j=0; j<m; j++) 
      add_prod_v(t[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%m]]), 
		 &r, &r);

    add_prod_v(__QT, &(raw_model->vertices[center2]), &r, &r); 


    prod_v(0.5, &r, &r);
    prod_v(0.5, &p, &p);
    add_v(&p, &r, &p);

    
    free(s);
    free(t);
  }
  else if (n == 6 && m == 6) {/* regular */
    /* apply the 10 point stencil */


    while (ring_op.ord_vert[v2] != center)
      v2++;

    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    r.x = 0.0;
    r.y = 0.0;
    r.z = 0.0;

    /* Apply stencil to 1st vertex_t */
    for (j=0; j<6; j++) 
      add_prod_v(r_sten[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%6]]), 
                 &p, &p);

    add_prod_v(__QS, &(raw_model->vertices[center]), &p, &p);


    /* Apply stencil to end vertex_t */
    for (j=0; j<6; j++) 
      add_prod_v(r_sten[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%6]]),
                 &p, &p);

    add_prod_v(__QS, &(raw_model->vertices[center2]), &p, &p);


    prod_v(0.5, &p, &p);

  }
  else if (n!=6 && m==6){ /* only one irreg. vertex_t */
    s = (float*)calloc(n, sizeof(float));
    if (n > 4) {
      for (j=0; j<n; j++) {
	s[j] = (0.25 + cos(2*M_PI*j/(float)n) + 
		0.5*cos(4*M_PI*j/(float)n))/(float)n;
      }
      
    } else if (n == 4) {
      s[0] = .375;
      s[2] = -0.125;
    } else if (n == 3) {
      s[0] = 5.0/12.0;
      s[1] = -1.0/12.0;
      s[2] = s[1];
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    for (j=0; j<n; j++) 
      add_prod_v(s[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%n]]), &p, 
		 &p);

    add_prod_v(__QS, &(raw_model->vertices[center]), &p, &p);


  } else if (n==6 && m!=6) {
    t = (float*)calloc(m, sizeof(float));

    while (ring_op.ord_vert[v2] != center)
      v2++;

    if (m > 4) {
      for (j=0; j<m; j++) {
	t[j] = (0.25 + cos(2*M_PI*j/(float)m) + 
		0.5*cos(4*M_PI*j/(float)m))/(float)m;
      }

      
    } else if (m == 4) {
      t[0] = 0.375;
      t[2] = -0.125;
    } else if (m == 3) {
      t[0] = 5.0/12.0;
      t[1] = -1.0/12.0;
      t[2] = t[1];
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    for (j=0; j<m; j++) 
      add_prod_v(t[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%m]]), &p,
		 &p);


    add_prod_v(__QT, &(raw_model->vertices[center2]), &p, &p);

    free(t);
  } 
  
  *vout = p;
}

void compute_midpoint_loop(struct ring_info *rings, int center, int v1, 
			   struct model *raw_model, vertex_t *vout) {

  vertex_t np, tmp;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  int n = rings[center].size;
  int p0, p1;

  add_v(&(raw_model->vertices[center]), &(raw_model->vertices[center2]), 
	&np);
  prod_v(0.375, &np, &np);

  p0 = ring.ord_vert[(v1+1)%n];
  if (v1 > 0)
    p1 = ring.ord_vert[v1-1];
  else
    p1 = ring.ord_vert[n-1];
 
  add_v(&(raw_model->vertices[p0]), &(raw_model->vertices[p1]), 
	&tmp);

  add_prod_v(0.125, &tmp, &np, &np);

  *vout = np;

}

void update_vertices_loop(struct model *or_model, 
			  struct model *subdiv_model, 
			  struct ring_info *rings) {
  int i, j, v, n;
  float beta;
  vertex_t tmp;
  for (i=0; i<or_model->num_vert; i++) {
    n = rings[i].size;
    
    if (n == 3)
      beta = 3.0/16.0;
    else
      beta = 3.0/(8.0*n);

    prod_v(1.0-n*beta, &(or_model->vertices[i]), &tmp);

    for (j=0; j<n; j++) {
      v = rings[i].ord_vert[j];
      add_prod_v(beta, &(or_model->vertices[v]), &tmp, &tmp);
    }
    subdiv_model->vertices[i] = tmp;
  }

}


void compute_midpoint_butterfly_crease(struct ring_info *rings, int center,  
				       int v1, struct model *raw_model, 
				       vertex_t *vout) {
  int n = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2]; /* center of opp ring */
  int m=ring_op.size;
  int v2 = 0; /* index of center vertex_t in opp. ring */
  int i;
  vertex_t p;
  float *s, thk, q;

  while (ring_op.ord_vert[v2] != center)
    v2++;

  if (ring.type == 1 && ring_op.type == 1) {
#ifdef __BUTTERFLY_CREASE_DEBUG
    fprintf(stderr, "Regular edge %d %d\n", center, center2);
#endif
    add_v(&(raw_model->vertices[center]), &(raw_model->vertices[center2]), &p);
    prod_v(0.5625, &p, &p);

#ifdef __BUTTERFLY_CREASE_DEBUG
    fprintf(stderr, "9/16 * (vert[%d] + vert[%d])\n", center, center2);
#endif

    if (v2 == 0) {
      add_prod_v(-0.0625, &(raw_model->vertices[ring_op.ord_vert[m-1]]), 
		 &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "-1/16 * vert[%d]\n", ring_op.ord_vert[m-1]);
#endif
    }
    else if (v2 == m-1) {
      add_prod_v(-0.0625, &(raw_model->vertices[ring_op.ord_vert[0]]), 
		 &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "-1/16 * vert[%d]\n", ring_op.ord_vert[0]);
#endif
    }
    else {
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "Strange boundary v2=%d/%d\n", v2, m-1);
#endif
      add_v(&(raw_model->vertices[center]), 
	    &(raw_model->vertices[center2]), vout);
      prod_v(0.5, vout, vout);
      return;
    }

    if (v1 == 0) {
      add_prod_v(-0.0625, &(raw_model->vertices[ring.ord_vert[n-1]]), &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "-1/16 * vert[%d]\n", ring.ord_vert[n-1]);
#endif
    } else if (v1 == n-1) {
      add_prod_v(-0.0625, &(raw_model->vertices[ring.ord_vert[0]]), &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "-1/16 * vert[%d]\n", ring.ord_vert[0]);
#endif
    } else {
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "Strange boundary v1=%d/%d\n", v1, n-1);
#endif
      add_v(&(raw_model->vertices[center]), 
	    &(raw_model->vertices[center2]), vout);
      prod_v(0.5, vout, vout);
      return;
    }

    *vout = p;  
    return;
  } else if (ring.type == 1) {
    if (ring.size == 4 && ring_op.size == 6) {/* regular crease-int. */
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "reg. int-crease %d %d\n", center2, center);
#endif
      s = (float*)malloc(4*sizeof(float));
      s[0] = -0.0625;
      s[3] = s[0];
      if (v1 == 1) {
	s[1] = -0.125;
	s[2] = 0.0625;
      } else if (v1 == 2) { 
	s[1] = 0.0625;
	s[2] = -0.125;
      } else
	fprintf(stderr, "Strange v1 = %d\n", v1);
      prod_v(0.125, &(raw_model->vertices[center]), vout);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "1/8 * vert[%d]\n", center);
#endif
      for (i=0; i<4; i++) {
	add_prod_v(s[i], &(raw_model->vertices[ring.ord_vert[i]]), vout, vout);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "%f * vert[%d]\n", s[i], ring.ord_vert[i]);
#endif
      }
      free(s);
      s = (float*)malloc(6*sizeof(float));
      s[v2] = 0.25;
      s[(v2+1)%6] = 0.125;
      s[(v2+2)%6] = -0.0625;
      s[(v2+3)%6] = 0.0;
      s[(v2+4)%6] = -0.125;
      s[(v2+5)%6] = 0.125;
      prod_v(0.75, &(raw_model->vertices[center2]), &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "3/4 * vert[%d]\n", center2);
#endif
      for (i=0; i<6; i++) {
	add_prod_v(s[i], &(raw_model->vertices[ring_op.ord_vert[i]]), 
		   &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "%f * vert[%d]\n", s[i], ring_op.ord_vert[i]);
#endif
      }
      add_v(&p, vout, vout);
      free(s);
      return;
    } else { /* extr. crease */
      s = (float*)malloc(n*sizeof(float));
      thk = M_PI/(float)(n-2);
      q = 1.0 - sin(thk)*sin(v1*thk)/((n-2)*(1 - cos(thk)));
      prod_v(q, &(raw_model->vertices[center]), vout);
      
      s[0] = 0.25*(cos(v1*thk) - sin(2.0*thk)*sin(2.0*v1*thk)/
		   ((n-2)*(cos(thk) - cos(2.0*thk))));
      s[n-1] = s[0];
      add_prod_v(s[0], &(raw_model->vertices[ring.ord_vert[0]]), vout, vout);
      add_prod_v(s[n-1], &(raw_model->vertices[ring.ord_vert[n-1]]), 
		 vout, vout);
      for (i=1; i<n-1; i++) {
	s[i] = (sin(v1*thk)*sin(i*thk) + 0.5*sin(2*v1*thk)*sin(2*i*thk))/
	  (float)(n-1);
	add_prod_v(s[i], &(raw_model->vertices[ring.ord_vert[i]]), vout, vout);
      }
      free(s);
      return;
    }
  } else { /* ring_op.type == 1 */
    if (ring_op.size == 4 && ring.size == 6) {/* regular crease-int. */
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "reg. int-crease %d %d\n", center, center2);
#endif
      s = (float*)malloc(4*sizeof(float));
      s[0] = -0.0625;
      s[3] = s[0];
      if (v2 == 1) {
	s[1] = -0.125;
	s[2] = 0.0625;
      } else if (v2 == 2) { 
	s[1] = 0.0625;
	s[2] = -0.125;
      } else 
	fprintf(stderr, "Strange v2 = %d\n", v2);

      prod_v(0.125, &(raw_model->vertices[center2]), vout);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "1/8 * vert[%d]\n", center2);
#endif
      for (i=0; i<4; i++) {
	add_prod_v(s[i], &(raw_model->vertices[ring_op.ord_vert[i]]), 
		   vout, vout);
#ifdef __BUTTERFLY_CREASE_DEBUG
	fprintf(stderr, "%f * vert[%d]\n", s[i], ring_op.ord_vert[i]);
#endif
      }
      free(s);
      s = (float*)malloc(6*sizeof(float));
      s[v1] = 0.25;
      s[(v1+1)%6] = 0.125;
      s[(v1+2)%6] = -0.0625;
      s[(v1+3)%6] = 0.0;
      s[(v1+4)%6] = -0.125;
      s[(v1+5)%6] = 0.125;
      prod_v(0.75, &(raw_model->vertices[center]), &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "3/4 * vert[%d]\n", center);
#endif
      for (i=0; i<6; i++) {
	add_prod_v(s[i], &(raw_model->vertices[ring.ord_vert[i]]), 
		   &p, &p);
#ifdef __BUTTERFLY_CREASE_DEBUG
	fprintf(stderr, "%f * vert[%d]\n", s[i], ring.ord_vert[i]);
#endif
      }
      add_v(&p, vout, vout);
      free(s);
      return;
    } else { /* extr. crease */
#ifdef __BUTTERFLY_CREASE_DEBUG
      fprintf(stderr, "extr. crease %d\n", center2);
#endif
      s = (float*)malloc(m*sizeof(float));
      thk = M_PI/(float)(m-2);
      q = 1.0 - sin(thk)*sin(v2*thk)/((m-2)*(1 - cos(thk)));
      prod_v(q, &(raw_model->vertices[center2]), vout);
      
      s[0] = 0.25*(cos(v2*thk) - sin(2.0*thk)*sin(2.0*v2*thk)/
		   ((m-2)*(cos(thk) - cos(2.0*thk))));
      s[m-1] = s[0];
      add_prod_v(s[0], &(raw_model->vertices[ring_op.ord_vert[0]]), 
		 vout, vout);
      add_prod_v(s[m-1], &(raw_model->vertices[ring_op.ord_vert[m-1]]), vout, 
		 vout);
      for (i=1; i<m-1; i++) {
	s[i] = (sin(v2*thk)*sin(i*thk) + 0.5*sin(2*v2*thk)*sin(2*i*thk))/
	  (float)(m-1);
	add_prod_v(s[i], &(raw_model->vertices[ring_op.ord_vert[i]]), 
		   vout, vout);
      }
      free(s);
      return;
    }
  }
}

void compute_midpoint_loop_crease(struct ring_info *rings, int center,  
				  int v1, struct model *raw_model, 
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

void update_vertices_loop_crease(struct model *or_model, 
				 struct model *subdiv_model, 
				 struct ring_info *rings) {
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


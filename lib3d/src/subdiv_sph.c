/* $Id: subdiv_sph.c,v 1.8 2002/09/17 08:36:54 aspert Exp $ */
#include <3dmodel.h>
#include <normals.h>
#include <geomutils.h>
#include <subdiv_methods.h>
#include <assert.h>

/* ph -> h(ph) */
static float h(float x) {
  float tmp, res;
  if (x <= -M_PI_2 || x >= M_PI_2)
    res = x;
  else if (x < -M_PI_4) {
    tmp = x + M_PI_4;
    res = -M_1_PI*tmp*tmp*(24.0*M_1_PI*tmp + 10) + 0.5*x;
  }
  else if (x <= M_PI_4)
    res = 0.5*x;
  else {
    tmp = x - M_PI_4;
    res = M_1_PI*tmp*tmp*(-24.0*M_1_PI*tmp + 10) + 0.5*x;
  }
  return res;
}


void compute_midpoint_sph(struct ring_info *rings, int center, int v1, 
			  struct model *raw_model, vertex_t *vout) {

  int center2 = rings[center].ord_vert[v1];
  struct ring_info ring_op = rings[center2];
  int v2 = 0;
  vertex_t n,p, vj, dir, m, u, v, np1, np2, np;
  float r, ph, lambda, pl_off, nr, nph, dz, rp;


  n = raw_model->normals[center];
  p = raw_model->vertices[center];
  vj = raw_model->vertices[rings[center].ord_vert[v1]];
  
  pl_off = -__scalprod_v(p, n);

  __substract_v(vj, p, dir);
  
  r = __norm_v(dir);
  
  lambda = -(pl_off + __scalprod_v(vj, n));
  
  __prod_v(lambda, n, m);

  __add_v(vj, m, u);

  __substract_v(u, p, v);


  if (lambda >= 0.0)
    ph = -atan(__norm_v(m)/__norm_v(v));
  else
    ph = atan(__norm_v(m)/__norm_v(v));



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
  nph = h(ph);

  dz = nr*sin(nph);
  rp = nr*cos(nph);

  __normalize_v(v);
  
  __prod_v(rp, v, np1);

  /* np1 += dz*n + p */
  __add_prod_v(dz, n, np1, np1);
  __add_v(np1, p, np1);

  while (ring_op.ord_vert[v2] != center)
      v2++;
  
  n = raw_model->normals[center2];
  p = raw_model->vertices[center2];
  vj = raw_model->vertices[ring_op.ord_vert[v2]];
  
  pl_off = -__scalprod_v(p, n);
  
  __substract_v(vj, p, dir);
  

  
  r = __norm_v(dir);
  
  lambda = -(pl_off + __scalprod_v(vj, n));
  
  __prod_v(lambda, n, m);

  __add_v(vj, m, u);

  __substract_v(u, p, v);


  if (lambda >= 0.0)
    ph = -atan(__norm_v(m)/__norm_v(v));
  else
    ph = atan(__norm_v(m)/__norm_v(v));


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
  nph = h(ph);



  dz = nr*sin(nph);
  rp = nr*cos(nph);

  __normalize_v(v);
  __prod_v(rp, v, np2);

  /* np2 += dz*n + p */
  __add_prod_v(dz, n, np2, np2);
  __add_v(np2, p, np2);


  __add_v(np1, np2, np);
  __prod_v(0.5, np, np);


  *vout = np;
}










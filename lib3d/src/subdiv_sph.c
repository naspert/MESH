/* $Id: subdiv_sph.c,v 1.12 2002/11/13 12:18:25 aspert Exp $ */
#include <3dutils.h>
#include <3dmodel.h>
#include <normals.h>
#include <geomutils.h>
#include <subdiv_methods.h>
#include <assert.h>
#if defined(SUBDIV_SPH_DEBUG) || defined(DEBUG)
# include <debug_print.h>
#endif

#define EPS 1e-10
#define DEG(x) ((x)*180.0/M_PI)



/* ph -> h(ph) */
static float h(const float x) 
{
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


static void half_sph(const vertex_t *p, 
                     const vertex_t *n, 
                     const vertex_t *q, vertex_t *vout)
{
  float r, dz, th, nth, rp, lambda, pl_off, nr;
  vertex_t dir, m, u, v, np;

  pl_off = -scalprod_v(p, n);
  substract_v(q, p, &dir);

  r = __norm_v(dir);

  lambda = -(pl_off + scalprod_v(q, n));
  prod_v(lambda, n, &m);

  /* u = projection of vj into the plane defined by \vec{n} */
  add_v(q, &m, &u);
  /* v = vector from u to p (i.e. in the plane defined by \vec{n}) */ 
  substract_v(&u, p, &v);
  
  /* Sanity check for curve subdivision mostly */
  /* some cases can happen where ||v||=0 */
  /* It sucks. Let's take the midpoint of the edge */
  if (__norm_v(v) < EPS) {
    add_v(p, q, &np);
    prod_v(0.5, &np, vout);
#ifdef SUBDIV_SPH_DEBUG
    DEBUG_PRINT("Ouch");
#endif
    return;
  }

  if (lambda >= 0.0)
    th = -atan(__norm_v(m)/__norm_v(v));
  else
    th = atan(__norm_v(m)/__norm_v(v));

  nr = 0.5*r;
  nth = h(th);

  dz = nr*sin(nth);
  rp = nr*cos(nth);

  __normalize_v(v);
  prod_v(rp, &v, &np);

  /* np += dz*n + p */
  add_prod_v(dz, n, &np, &np);
  add_v(&np, p, vout);

#ifdef SUBDIV_SPH_DEBUG
  printf("p = %f %f %f\n", p->x, p->y, p->z);
  printf("n = %f %f %f\n", n->x, n->y, n->z);
  printf("q = %f %f %f\n", q->x, q->y, q->z);
  printf("pl_off = %f\n", pl_off);
  printf("r = %f\n", r);
  printf("th = %f\n", DEG(th));
  printf("nth = %f\n", DEG(nth));
  printf("u.n + d = %f\n", scalprod_v(&u,n)+pl_off);
  printf("test %f %f %f\n", norm_v(&v), r*cos(th), norm_v(&v)-r*cos(th));
#endif



}   

void compute_midpoint_sph(const struct ring_info *rings, const int center, 
                          const int v1, 
			  const struct model *raw_model, vertex_t *vout) 
{

  int center2 = rings[center].ord_vert[v1];
  struct ring_info ring_op = rings[center2];
  int v2 = 0;
  vertex_t n, p, vj, np1, np2, np;


  n = raw_model->normals[center];
  p = raw_model->vertices[center];
  vj = raw_model->vertices[center2];
  
#ifdef SUBDIV_SPH_DEBUG
  DEBUG_PRINT("Edge %d %d\n", center, center2);
#endif
  half_sph(&p, &n, &vj, &np1);


  while (ring_op.ord_vert[v2] != center)
      v2++;
  
  n = raw_model->normals[center2];
  p = raw_model->vertices[center2];
  vj = raw_model->vertices[center];

#ifdef SUBDIV_SPH_DEBUG
  DEBUG_PRINT("Edge %d %d\n", center2, center);
#endif
  half_sph(&p, &n, &vj, &np2);

  __add_v(np1, np2, np);
  prod_v(0.5, &np, vout);


}



void compute_midpoint_sph_crease(const struct ring_info *rings, 
                                 const int center, const int v1, 
                                 const struct model *raw_model, 
                                 vertex_t *vout)
{
  int n_r = rings[center].size;
  struct ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  struct ring_info ring_op = rings[center2];
  int nrop;
  int v3 = -1;
  int v2 = 0;
  vertex_t p, q, np, a, b, ns1, ns2, n, np1, np2;
  float r1, r2;

  p = raw_model->vertices[center];
  q = raw_model->vertices[center2];

  nrop = ring_op.size;
  while (ring_op.ord_vert[v2] != center)
    v2++;

  if (ring.type == 1 && ring_op.type == 1) { /* boundary here */
    /* go backward */
    if (ring.ord_vert[0] == center2)
      v3 = ring.ord_vert[n_r-1];
    else if (ring.ord_vert[n_r-1] == center2)
      v3 = ring.ord_vert[0];
    else { /* we have a non-boundary -> midpoint */
      add_v(&p, &q, &np);
      prod_v(0.5, &np, vout);
      return;
    }

    __substract_v(raw_model->vertices[v3], p, a);
    r1 = __norm_v(a);
    __substract_v(q, p, b);
    r2 = __norm_v(b);

    __crossprod_v(a, b, np);
    __normalize_v(np);

    /* get side normals */
    __crossprod_v(a, np, ns1);
    __normalize_v(ns1);
    __crossprod_v(np, b, ns2);
    __normalize_v(ns2);

    /* Now get the normal est. at vertex 'center' */
    __prod_v(r1, ns1, n);
    __add_prod_v(r2, ns2, n, n);
    __normalize_v(n);


    /* Now proceed through a usual spherical subdivision */
    half_sph(&p, &n, &q, &np1);

    /* go forward */
    if (ring_op.ord_vert[0] == center)
      v3 = ring_op.ord_vert[nrop-1];
    else if (ring_op.ord_vert[nrop-1] == center)
      v3 = ring_op.ord_vert[0];
    else {
      __add_v(p, q, np);
      prod_v(0.5, &np, vout);
      return;
    }

    /* Get the normal to the plane (center, center2, v3) */
    __substract_v(p, q, a);
    r1 = r2;
    __substract_v(raw_model->vertices[v3], q, b);
    r2 = __norm_v(b);
    __crossprod_v(a, b, np);
    __normalize_v(np);

    /* get side normals */
    __crossprod_v(a, np, ns1);
    __normalize_v(ns1);
    __crossprod_v(np, b, ns2);
    __normalize_v(ns2);

    /* Now get the normal est. at vertex 'center2' */
    __prod_v(r1, ns1, n);
    __add_prod_v(r2, ns2, n, n);
    __normalize_v(n);


    /* Perform sph. subdivision */
    half_sph(&q, &n, &p, &np2);

    /* gather those new points */
    __add_v(np1, np2, np);
    prod_v(0.5, &np, vout);

  } else if (ring.type == 1)  /* && ring_op.type == 0 */
    compute_midpoint_sph(rings, center2, v2, raw_model, vout);
  else  /* ring_op.type == 1 && ring.type == 0 */
    compute_midpoint_sph(rings, center, v1, raw_model, vout);


}






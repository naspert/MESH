/* $Id: geomutils.h,v 1.12 2001/09/20 15:21:46 dsanta Exp $ */
#include <3dmodel.h>

#ifndef _GEOMUTILS_PROTO_
#define _GEOMUTILS_PROTO_

#ifdef __cplusplus
extern "C" {
#endif

/* a few useful macros */
#ifndef min
#define min(__X, __Y) ((__X)<(__Y)?(__X):(__Y))
#endif
#ifndef max
#define max(__X, __Y) ((__X)>(__Y)?(__X):(__Y))
#endif
#ifndef max3
#define max3(__X,__Y,__Z) max((__X), max((__Y), (__Z)))
#endif
#ifndef min3
#define min3(__X,__Y,__Z) min((__X), min((__Y),(__Z)))
#endif

  /* exported functions */
  double dist(vertex, vertex);
  vertex ncrossp(vertex, vertex, vertex);
  vertex crossprod(vertex, vertex);
  double cross_product2d(vertex, vertex, vertex);
  double scalprod(vertex, vertex);
  double norm(vertex);
  void normalize(vertex*);
  vertex rotate_3d(vertex, vertex, double);
  int inside(vertex, vertex, double);
  void compute_circle2d(vertex, vertex, vertex, double*, vertex*);
  void compute_circle3d(vertex, vertex, vertex, double*, vertex*);
  double tri_area(vertex, vertex, vertex);

  /* Diego's faster functions */
  double scalprod_v(const vertex*, const vertex*);
  double norm2_v(const vertex*);
  double norm_v(const vertex*);
  double dist_v(const vertex*, const vertex*);
  double dist2_v(const vertex *, const vertex *);
  void normalize_v(vertex*);
  void substract_v(const vertex*, const vertex*, vertex*);
  void add_v(const vertex*, const vertex*, vertex*);
  void prod_v(double, const vertex*, vertex*);
  void add_prod_v(double, const vertex*, const vertex*, vertex*);
  void crossprod_v(const vertex*, const vertex*, vertex*);
  void ncrossp_v(const vertex*, const vertex*, const vertex*, vertex*);
  double tri_area_v(const vertex*, const vertex*, const vertex*);

/* inline definitions */
#ifdef INLINE
# error Name clash with INLINE macro
#endif

#ifdef _GEOMUTILS_C_ /* we are including from the 'master' C file */
# if defined (_MSC_VER)
#  define INLINE __inline
# elif defined(__GNUC__)
#  define INLINE __inline__
# elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#  define INLINE extern inline
# else
#  define INLINE
# endif
#else /* including from other files */
# if defined (_MSC_VER)
#  define INLINE __inline
# elif defined(__GNUC__)
#  define INLINE extern __inline__
# elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#  define INLINE inline
# endif
#endif

#ifdef INLINE /* give definition of functions */

  /* returns the scalar product of 2 vectors */
  INLINE double scalprod_v(const vertex *v1, const vertex *v2) {
    return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
  }
  
  /* returns the squared norm of a vector */
  INLINE double norm2_v(const vertex *v) {
    return (v->x*v->x + v->y*v->y + v->z*v->z);
  }
  
  /* Returns the norm (i.e. length) of vector v */
  INLINE double norm_v(const vertex *v) {
    return (sqrt(v->x*v->x + v->y*v->y + v->z*v->z));
  }

  /* Returns the distance between vertices v1 and v2 */
  INLINE double dist_v(const vertex *v1, const vertex *v2) {
    vertex tmp;
  
    tmp.x = v1->x - v2->x;
    tmp.y = v1->y - v2->y;
    tmp.z = v1->z - v2->z;
    
    return sqrt(tmp.x*tmp.x+tmp.y*tmp.y+tmp.z*tmp.z);
  }

  /* Returns the squared distance between vertices v1 and v2 */
  INLINE double dist2_v(const vertex *v1, const vertex *v2) {
    vertex tmp;
  
    tmp.x = v1->x - v2->x;
    tmp.y = v1->y - v2->y;
    tmp.z = v1->z - v2->z;
    
    return tmp.x*tmp.x+tmp.y*tmp.y+tmp.z*tmp.z;
  }
  
  /* Normalizes the vector v to be of unit length */
  INLINE void normalize_v(vertex *v) {
    double inv_l;
    inv_l = 1/norm_v(v); /* multiplication is faster than division */
    v->x *= inv_l;
    v->y *= inv_l;
    v->z *= inv_l;
  }
  
  /* Substracts vector v2 from v1 (i.e. v1-v2) and puts the result in vout. It
   * is OK for vout to alias v1 and/or v2. */
  INLINE void substract_v(const vertex *v1, const vertex *v2, vertex *vout) {
  vout->x = v1->x - v2->x;
  vout->y = v1->y - v2->y;
  vout->z = v1->z - v2->z;
  }
  
  /* Adds vectors v1 and v2 and puts the result in vout. It is OK for vout to
   * alias v1 and/or v2. */
  INLINE void add_v(const vertex *v1, const vertex *v2, vertex *vout)
  {
    vout->x = v1->x + v2->x;
    vout->y = v1->y + v2->y;
    vout->z = v1->z + v2->z;
  }

  /* Multiplies vector v by scalar m and puts the result in vout. It is OK for
   * vout to alias v. */
  INLINE void prod_v(double m, const vertex *v, vertex *vout) {
    vout->x = m*v->x;
    vout->y = m*v->y;
    vout->z = m*v->z;
  }
  

  /* Multiplies v1 by scalar m, adds v2 to it and puts the result in vout */
  INLINE void add_prod_v(double m, const vertex *v1, const vertex *v2, 
			 vertex *vout) {
    vout->x = m*v1->x + v2->x;
    vout->y = m*v1->y + v2->y;
    vout->z = m*v1->z + v2->z;
  }

  /* Calculates the cross product of vectors v1 and v2 and places the result in
   * vout. It is OK for vout to alias v1 and/or v2. */
  INLINE void crossprod_v(const vertex *v1, const vertex *v2, vertex *vout) {
    vertex res; /* Use temporary to be safe if vout alias v1 and/or v2 */
    res.x = v1->y*v2->z - v1->z*v2->y;
    res.y = v1->z*v2->x - v1->x*v2->z;
    res.z = v1->x*v2->y - v1->y*v2->x;
    *vout = res;
  }
  

  INLINE void ncrossp_v(const vertex *v1, const vertex *v2, const vertex *v3, 
			vertex *vout) {
    vertex res; /* Use temporary to be safe if vout alias v1 and/or v2 */
    vertex u,v;
    double n;
    
    u.x = v2->x - v1->x;
    u.y = v2->y - v1->y;
    u.z = v2->z - v1->z;
    
    v.x = v3->x - v1->x;
    v.y = v3->y - v1->y;
    v.z = v3->z - v1->z;

    /* u^v */
    res.x = u.y*v.z - u.z*v.y;
    res.y = u.z*v.x - u.x*v.z;
    res.z = u.x*v.y - u.y*v.x;

    n = 1/norm_v(&res);
    res.x *= n;
    res.y *= n;
    res.z *= n;

    *vout = res;
  }


  INLINE double tri_area_v(const vertex *v1, const vertex *v2, 
			   const vertex*v3) {
    vertex u,v,h;
    double nu2,uv;
    double tmp;
    
    u.x = v1->x - v3->x;
    u.y = v1->y - v3->y;
    u.z = v1->z - v3->z;
    
    v.x = v2->x - v3->x;
    v.y = v2->y - v3->y;
    v.z = v2->z - v3->z;
    
    /* <u,v> */
    uv = u.x*v.x + u.y*v.y +u.z*v.z;

    /* ||u||^2 */
    nu2 = u.x*u.x + u.y*u.y + u.z*u.z;

    tmp = uv/nu2;
    h.x = v.x - u.x*tmp;
    h.y = v.y - u.y*tmp;
    h.z = v.z - u.z*tmp;



    return (norm_v(&h)*sqrt(nu2)*0.5);
  }

#undef INLINE
#endif /* INLINE */
#ifdef __cplusplus
}
#endif

 
#endif

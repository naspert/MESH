/* $Id: curvature.c,v 1.7 2002/11/14 16:43:50 aspert Exp $ */
#include <3dutils.h>
#include <ring.h>
#include <curvature.h>
#ifdef CURV_DEBUG
# include <debug_print.h>
#endif

/* Returns the angle lying at vertex v0 in the triangle v0v1v2 */
static double get_top_angle(const vertex_t *v0, const vertex_t *v1, 
                            const vertex_t *v2) 
{
  vertex_t u0, u1;
  double tmp;

  substract_v(v1, v0, &u0);
  substract_v(v2, v0, &u1);

  tmp = __scalprod_v(u0, u1);
  tmp /= __norm_v(u0)*__norm_v(u1);
  return acos(tmp);

}


/* This is an alternate method to compute the top angle. I don't
 * remember exactly what was the advantage of the other or the
 * drawbacks of this one. Anyway, let's keep it at hand just in
 * case.... */

/* static double get_top_angle2(const vertex_t *v0, const vertex_t *v1,  */
/*                              const vertex_t *v2) { */
/*   vertex_t u0, u1, v, h; */
/*   double tmp; */
/*   substract_v(v1, v0, &u0); */
/*   substract_v(v2, v0, &u1); */
/*   tmp = scalprod_v(&u0, &u1); */
/*   prod_v(tmp/norm_v(&u0), &u0, &v); */
/*   substract_v(&u1, &v, &h); */
/*   if (fabs(tmp)< 1e-9) */
/*     return M_PI_2; */
/*   else if (tmp > 0.0) */
/*     return atan2(norm_v(&h), norm_v(&v)); */
/*   else  */
/*     return M_PI - atan2(norm_v(&h), norm_v(&v)); */
/* } */

/* Test face f from raw_model to check if this is an obtuse triangle */
static int obtuse_triangle(const vertex_t *v0, const vertex_t *v1, 
                           const vertex_t *v2) 
{
  double th, sum=0.0;

  th = get_top_angle(v0, v1, v2);
  sum += th;
  if (th > M_PI_2)
    return 1;
  th = get_top_angle(v2, v0, v1);
  sum += th;
  if (th > M_PI_2)
    return 1;
  if (M_PI-sum > M_PI_2)
    return 1;
  
  return 0;
}

static void
compute_mean_curvature_normal(const struct model *raw_model,
                              int v0, const struct ring_info *rings, 
                              vertex_t *sum_vert, double *mixed_area, 
                              double *gauss_curv, double *mean_curv) 
{

  int v1, v1_idx, v2f, v2b, v2b_idx, v2, i;
  int n=rings[v0].size;
  vertex_t tmp;
  double alpha, beta, c, theta, kg, ma;
  face_t *cur_face;

  ma = 0.0;
  sum_vert->x = 0.0;
  sum_vert->y = 0.0;
  sum_vert->z = 0.0;


  for (v1_idx=0; v1_idx<n; v1_idx++) {
    v1=rings[v0].ord_vert[v1_idx];
    v2f = rings[v0].ord_vert[(v1_idx + 1)%n];
    v2b_idx = (v1_idx > 0)?((v1_idx - 1)%n):(n - 1);
    v2b = rings[v0].ord_vert[v2b_idx];
    
    substract_v(&(raw_model->vertices[v1]), &(raw_model->vertices[v0]), &tmp);
    alpha = get_top_angle(&(raw_model->vertices[v2b]), 
			   &(raw_model->vertices[v1]), 
			   &(raw_model->vertices[v0]));
    beta = get_top_angle(&(raw_model->vertices[v2f]), 
			  &(raw_model->vertices[v1]), 
			  &(raw_model->vertices[v0]));
    c = (cos(alpha)/sin(alpha) + cos(beta)/sin(beta));

    add_prod_v(c, &tmp, sum_vert, sum_vert); 

  }
  
  kg = 2.0*M_PI;

  for (i=0; i<rings[v0].n_faces; i++) {
    cur_face = &(raw_model->faces[rings[v0].ord_face[i]]);
    if (cur_face->f0 == v0) {
      v1 = cur_face->f1;
      v2 = cur_face->f2;
    } else if (cur_face->f1 == v0) {
      v1 = cur_face->f0;
      v2 = cur_face->f2;
    } else { /* cur_face.f2 == v0 */
      v1 = cur_face->f0;
      v2 = cur_face->f1;
    }
    
    theta = get_top_angle(&(raw_model->vertices[v0]), 
			   &(raw_model->vertices[v1]),
			   &(raw_model->vertices[v2]));
    kg -= theta; 

    if (!obtuse_triangle(&(raw_model->vertices[v0]), 
			 &(raw_model->vertices[v1]), 
			 &(raw_model->vertices[v2]))) {
      
      alpha = get_top_angle(&(raw_model->vertices[v1]), 
			     &(raw_model->vertices[v0]),
			     &(raw_model->vertices[v2]));
      beta =  get_top_angle(&(raw_model->vertices[v2]), 
			     &(raw_model->vertices[v1]),
			     &(raw_model->vertices[v0]));
      
      ma += 0.125*(dist2_v(&(raw_model->vertices[v0]), 
                           &(raw_model->vertices[v1]))*
                   cos(beta)/sin(beta) +
                   dist2_v(&(raw_model->vertices[v0]), 
                           &(raw_model->vertices[v2]))*
                   cos(alpha)/sin(alpha));

      
    }
    else {
      if (theta > M_PI_2) {
	ma += 0.5*raw_model->area[rings[v0].ord_face[i]];
      }
      else {
	ma += 0.25*raw_model->area[rings[v0].ord_face[i]];
      }
     
    }
  }
  
  prod_v(0.5/ma, sum_vert, sum_vert);
  *mixed_area = ma;
  *gauss_curv = kg/ma;
  *mean_curv = 0.5*norm_v(sum_vert);
  
}


int compute_curvature_with_rings(const struct model *raw_model, 
                                 struct info_vertex *info, 
                                 const struct ring_info *rings) 
{
  int i;
  double k, k2, delta;
  
  for (i=0; i<raw_model->num_vert; i++) {
    if (rings[i].type != 0) {
      info[i].gauss_curv = 0.0;
      info[i].mean_curv = 0.0;
      continue;
    }
    compute_mean_curvature_normal(raw_model, i, rings, 
				  &(info[i].mean_curv_normal), 
				  &(info[i].mixed_area), 
				  &(info[i].gauss_curv), 
				  &(info[i].mean_curv));

    k = info[i].mean_curv;
    k2 = k*k;
    delta = k2 - info[i].gauss_curv;
    if (delta <= 0.0) {
#ifdef CURV_DEBUG
      DEBUG_PRINT("Strange delta=%f at vertex %d\n", delta, i);
#endif
      delta = 0.0;
    }
    info[i].k1 = k + sqrt(delta);
    info[i].k2 = k - sqrt(delta);

#ifdef CURV_DEBUG
    DEBUG_PRINT("Vertex %d\n", i);
    DEBUG_PRINT("Mean curv. normal = %f %f %f\n", info[i].mean_curv_normal.x,
                info[i].mean_curv_normal.y, info[i].mean_curv_normal.z);
    DEBUG_PRINT("Mixed area = %f\n", info[i].mixed_area);
    DEBUG_PRINT("Vertex normal = %f %f %f\n", raw_model->normals[i].x, 
                raw_model->normals[i].y, raw_model->normals[i].z);
    DEBUG_PRINT("Gauss_k=%f k1=%f k2=%f\n\n", 
                info[i].gauss_curv,  
                info[i].k1, info[i].k2); 
#endif
  }
  return 0;
}


/* Calls the above function but do the rings computation before */
int compute_curvature(const struct model *raw_model, struct info_vertex *info) 
{
  struct ring_info* rings;
  int i, ret;

  rings = 
    (struct ring_info*)malloc(raw_model->num_vert*sizeof(struct ring_info));

  build_star_global(raw_model, rings);
  ret = compute_curvature_with_rings(raw_model, info, rings);
  for (i=0; i<raw_model->num_vert; i++) {
    if (rings[i].ord_vert != NULL)
      free(rings[i].ord_vert);
    if (rings[i].ord_face != NULL)
    free(rings[i].ord_face);
  }
  free(rings);

  return ret;
}

/* $Id: compute_curvature.c,v 1.1 2001/09/27 08:56:57 aspert Exp $ */
#include <3dutils.h>
#include <compute_error.h>
#include <compute_curvature.h>

/* Returns the angle lying at vertex v0 in the triangle v0v1v2 */
double get_top_angle(const vertex *v0, const vertex *v1, const vertex *v2) {
  vertex u0, u1;
  double tmp;

  substract_v(v1, v0, &u0);
  substract_v(v2, v0, &u1);

  tmp = scalprod_v(&u0, &u1);
  tmp /= norm_v(&u0)*norm_v(&u1);
  return acos(tmp);

}

double get_top_angle2(const vertex *v0, const vertex *v1, const vertex *v2) {
  vertex u0, u1, v, h;
  double tmp;

  substract_v(v1, v0, &u0);
  substract_v(v2, v0, &u1);

  tmp = scalprod_v(&u0, &u1);
  prod_v(tmp/norm_v(&u0), &u0, &v);
  substract_v(&u1, &v, &h);
  if (fabs(tmp)< 1e-9)
    return M_PI_2;
  else if (tmp > 0.0)
    return atan2(norm_v(&h), norm_v(&v));
  else 
    return M_PI - atan2(norm_v(&h), norm_v(&v));


}

/* Test face f from raw_model to check if this is an obtuse triangle */
int obtuse_triangle(const vertex *v0, const vertex *v1, const vertex *v2) {
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

void compute_mean_curvature_normal(const model *raw_model, info_vertex *info, 
				   int v0, const ring_info *rings, 
				   vertex *sum_vert, double *mixed_area, 
				   double *gauss_curv) {
  int v1, v1_idx, v2f, v2b, v2b_idx, v2, i;
  int n=rings[v0].size;
  vertex tmp;
  double alpha, beta, c, theta;
  face cur_face;

  *mixed_area = 0.0;
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
  
  *gauss_curv = 2.0*M_PI;

  for (i=0; i<info[v0].num_faces; i++) {
    cur_face = raw_model->faces[info[v0].list_face[i]];
    if (cur_face.f0 == v0) {
      v1 = cur_face.f1;
      v2 = cur_face.f2;
    } else if (cur_face.f1 == v0) {
      v1 = cur_face.f0;
      v2 = cur_face.f2;
    } else {
      v1 = cur_face.f0;
      v2 = cur_face.f1;
    }
    
    theta = get_top_angle(&(raw_model->vertices[v0]), 
			   &(raw_model->vertices[v1]),
			   &(raw_model->vertices[v2]));
    *gauss_curv -= theta; 

    if (!obtuse_triangle(&(raw_model->vertices[v0]), 
			 &(raw_model->vertices[v1]), 
			 &(raw_model->vertices[v2]))) {
      
      alpha = get_top_angle(&(raw_model->vertices[v1]), 
			     &(raw_model->vertices[v0]),
			     &(raw_model->vertices[v2]));
      beta =  get_top_angle(&(raw_model->vertices[v2]), 
			     &(raw_model->vertices[v1]),
			     &(raw_model->vertices[v0]));
      
      *mixed_area += 0.125*(dist2_v(&(raw_model->vertices[v0]), 
				    &(raw_model->vertices[v1]))*
			    cos(beta)/sin(beta) +
			    dist2_v(&(raw_model->vertices[v0]), 
				    &(raw_model->vertices[v2]))*
			    cos(alpha)/sin(alpha));

      
    }
    else {
      if (theta > M_PI_2) {
	*mixed_area += 0.5*tri_area_v(&(raw_model->vertices[cur_face.f0]), 
				      &(raw_model->vertices[cur_face.f1]), 
				      &(raw_model->vertices[cur_face.f2]));
      }
      else {
	*mixed_area += 0.25*tri_area_v(&(raw_model->vertices[cur_face.f0]), 
				       &(raw_model->vertices[cur_face.f1]), 
				       &(raw_model->vertices[cur_face.f2]));
      }
      
    }
  }
  
  prod_v(0.5/(*mixed_area), sum_vert, sum_vert);
  *gauss_curv /= *mixed_area;
  
  
}


void compute_curvature(const model *raw_model, info_vertex *info, 
		       const ring_info *rings) {
  int i;
  double k, k2, delta;

  for (i=0; i<raw_model->num_vert; i++) {
    if (rings[i].type != 0) {
      fprintf(stderr, "Unsupported vertex type %d found at %d\n", 
	      rings[i].type, i);
      return;
    }
    compute_mean_curvature_normal(raw_model, info, i, rings, 
				  &(info[i].mean_curv_normal), 
				  &(info[i].mixed_area), 
				  &(info[i].gauss_curv));
    k2 = 0.25*norm2_v(&(info[i].mean_curv_normal));
    k = 0.5*norm_v(&(info[i].mean_curv_normal));
    delta = k2 - info[i].gauss_curv;
    if (delta <= 0.0) {
#ifdef __CURV_DEBUG
      printf("Strange delta=%f at vertex %d\n", delta, i);
#endif
      delta = 0.0;
    }
    info[i].k1 = k + sqrt(delta);
    info[i].k2 = k - sqrt(delta);

#ifdef __CURV_DEBUG
    printf("Vertex %d\n", i);
    printf("Mean curv. normal = %f %f %f\n", info[i].mean_curv_normal.x,
	   info[i].mean_curv_normal.y, info[i].mean_curv_normal.z);
    printf("Mixed area = %f\n", info[i].mixed_area);
    printf("Vertex normal = %f %f %f\n", raw_model->normals[i].x, 
	   raw_model->normals[i].y, raw_model->normals[i].z);
    printf("Vertex %d :Gauss_k=%f k1=%f k2=%f\n\n", i, info[i].gauss_curv,  
 	   info[i].k1, info[i].k2); 
#endif
  }
}

/* 
   This is the main method that computes the distortion of the 
   curvature of two models. They are assumed to have the SAME 
   number of vertices (test before calling the method)
   model1 should be the 'modified' model, whereas model2 stands
   for the original model 
*/
void compute_curvature_error(struct model_error *model1, 
			     struct model_error *model2) {

  int i;
  info_vertex *info1, *info2;
  ring_info *rings1, *rings2;


  printf("Computing face normals...\n");
  info1 = (info_vertex*)malloc(model1->mesh->num_vert*sizeof(info_vertex));
  model1->mesh->face_normals = compute_face_normals(model1->mesh, info1);


  info2 = (info_vertex*)malloc(model2->mesh->num_vert*sizeof(info_vertex));
  model2->mesh->face_normals = compute_face_normals(model2->mesh, info2);


  printf("Computing vertex normals...\n");
  model1->mesh->area = (double*)malloc(model1->mesh->num_faces*sizeof(double));
  model2->mesh->area = (double*)malloc(model2->mesh->num_faces*sizeof(double));
  compute_vertex_normal(model1->mesh, info1, 
			model1->mesh->face_normals); 
  compute_vertex_normal(model2->mesh, info2, 
			model2->mesh->face_normals); 

  printf("Generating 1-rings of all vertices...");fflush(stdout);
  rings1 = (ring_info*)malloc(model1->mesh->num_vert*sizeof(ring_info));
  rings2 = (ring_info*)malloc(model2->mesh->num_vert*sizeof(ring_info));
  for (i=0; i<model1->mesh->num_vert; i++) {
    build_star(model1->mesh, i, &(rings1[i]));
    build_star(model2->mesh, i, &(rings2[i]));
  }
  printf("done\n");

  printf("Computing curvature of model 1.... ");fflush(stdout);
  compute_curvature(model1->mesh, info1, rings1);
  printf("done\n");  
  printf("Computing curvature of model 2.... ");fflush(stdout);
  compute_curvature(model2->mesh, info2, rings2);
  printf("done\n");  
  

  /* The model 1 displays the curvature error when RawWidget is constructed */
  model1->k1_error = (double*)malloc(model1->mesh->num_vert*sizeof(double));
  model1->k2_error = (double*)malloc(model1->mesh->num_vert*sizeof(double));
  model1->kg_error = (double*)malloc(model1->mesh->num_vert*sizeof(double));

  model1->min_k1_error = FLT_MAX;
  model1->max_k1_error = -FLT_MAX;
  model1->mean_k1_error = 0.0;

  model1->min_k2_error = FLT_MAX;
  model1->max_k2_error = -FLT_MAX;
  model1->mean_k2_error = 0.0;

  model1->min_kg_error = FLT_MAX;
  model1->max_kg_error = -FLT_MAX;
  model1->mean_kg_error = 0.0;
  
  for (i=0; i<model1->mesh->num_vert; i++) {
    model1->k1_error[i] = fabs(info1[i].k1 - info2[i].k1);
    model1->k2_error[i] = fabs(info1[i].k2 - info2[i].k2);
    model1->kg_error[i] = fabs(info1[i].gauss_curv - info2[i].gauss_curv);
    
    if (model1->k1_error[i] > model1->max_k1_error)
      model1->max_k1_error = model1->k1_error[i];
    if (model1->k2_error[i] > model1->max_k2_error)
      model1->max_k2_error = model1->k2_error[i];
    if (model1->kg_error[i] > model1->max_kg_error)
      model1->max_kg_error = model1->kg_error[i];

    if (model1->k1_error[i] < model1->min_k1_error)
      model1->min_k1_error = model1->k1_error[i];
    if (model1->k2_error[i] < model1->min_k2_error)
      model1->min_k2_error = model1->k2_error[i];
    if (model1->kg_error[i] < model1->min_kg_error)
      model1->min_kg_error = model1->kg_error[i];
    
    model1->mean_k1_error += info1[i].mixed_area*model1->k1_error[i];
    model1->mean_k2_error += info1[i].mixed_area*model1->k2_error[i];
    model1->mean_kg_error += info1[i].mixed_area*model1->kg_error[i];
  }

  model1->mesh->total_area = 0.0;
  for (i=0; i<model1->mesh->num_faces; i++) 
    model1->mesh->total_area += model1->mesh->area[i];
  
  model1->mean_k1_error /= model1->mesh->total_area;
  model1->mean_k2_error /= model1->mesh->total_area;
  model1->mean_k2_error /= model1->mesh->total_area;
  
}



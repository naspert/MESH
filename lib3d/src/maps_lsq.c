#include <3dutils.h>
#include <gsl/gsl_multifit.h>





/* Computes the principal curvatures for each vertex */
/* using a least-squares fitting on the neighborhood */
void lsq_fit(struct model *raw_model, struct ring_info *rings, 
	     struct info_vertex *curv) {
  int i, j;
  vertex_t **t;
  vertex_t vi_j; /* vi - vj */
  vertex_t nvi;
  vertex_t t1l, t2l;
  vertex_t b0, b1; /* basis vectors of tangent plane*/ 
  vertex_t **d;
  double  *u,  k1, k2, delta;
  gsl_matrix *X, *cov;
  gsl_vector *y,  *c;
  gsl_multifit_linear_workspace *wspace;
  double chisq;

  t = (vertex_t**)malloc(raw_model->num_vert*sizeof(vertex_t*));
  d = (vertex_t**)malloc(raw_model->num_vert*sizeof(vertex_t*));

  

  for (i=0; i<raw_model->num_vert; i++) {
    t[i] = (vertex_t*)malloc(rings[i].size*sizeof(vertex_t));
    d[i] = (vertex_t*)malloc(rings[i].size*sizeof(vertex_t));


    nvi = raw_model->normals[i];

    for(j=0; j<rings[i].size; j++){

      /* Vj - Vi*/
      substract_v(&(raw_model->vertices[rings[i].ord_vert[j]]), 
		  &(raw_model->vertices[i]), &vi_j);

      /* signed distance */
      d[i][j].z = scalprod_v(&vi_j, &nvi);
      /* tangent vector */
      add_prod_v(-d[i][j].z, &nvi, &vi_j, &(t[i][j]));
    }/* End for j */

     /* Find basis of tangent plane */
    b0 = t[i][0];

    normalize_v(&b0);
    /* Gram-Schmidt */
    /* triangulated surface, so t[i][1] and t[i][0] are not colinear */    
    add_prod_v(-scalprod_v(&b0, &(t[i][1])), &b0, &(t[i][1]), &b1);
    normalize_v(&b1);

    /* Make the basis is direct */
    crossprod_v(&b0, &b1, &t1l);
    if (scalprod_v(&nvi, &t1l) < - 0.5) 
      prod_v(-1.0, &b1, &b1);

    crossprod_v(&b0, &b1, &t1l);
    substract_v(&t1l, &nvi, &t1l);
    printf("base = %f %f %f\n", t1l.x, t1l.y, t1l.z);
    u = (double*)malloc(3*rings[i].size*sizeof(double));    
    for (j=0; j<rings[i].size; j++) {      
      /* Coordinates of the projection of vj on the tangent plane */
      /* expressed in the basis (b0, b1, nvi) */
      d[i][j].x = scalprod_v(&(t[i][j]), &b0);
      d[i][j].y = scalprod_v(&(t[i][j]), &b1);
      t1l.x = t[i][j].x - (d[i][j].x*b0.x + d[i][j].y*b1.x);
      t1l.y = t[i][j].y - (d[i][j].x*b0.y + d[i][j].y*b1.y);
/*       printf("t1l = %f %f \n", t1l.x, t1l.y); */

      u[3*j] = 0.5*d[i][j].x * d[i][j].x;
      u[3*j+1] = d[i][j].x * d[i][j].y;      
      u[3*j+2] = 0.5*d[i][j].y * d[i][j].y;
      
    }

    
    X = gsl_matrix_alloc(rings[i].size, 3);
    y = gsl_vector_alloc(rings[i].size);

    c = gsl_vector_alloc(3);
    cov = gsl_matrix_alloc(3, 3);

    for (j=0; j<rings[i].size; j++) {
      gsl_matrix_set(X, j, 0, u[3*j]);
      gsl_matrix_set(X, j, 1, u[3*j+1]);
      gsl_matrix_set(X, j, 2, u[3*j+2]);

      gsl_vector_set(y, j, d[i][j].z);

    }

    wspace = gsl_multifit_linear_alloc(rings[i].size, 3);
    gsl_multifit_linear(X, y, c, cov, &chisq, wspace);
    gsl_multifit_linear_free(wspace);
    printf("Residual = %f\n", chisq);
    curv[i].c[0] = gsl_vector_get(c, 0);
    curv[i].c[1] = gsl_vector_get(c, 1);
    curv[i].c[2] = gsl_vector_get(c, 2);

    gsl_matrix_free(X);
    gsl_matrix_free(cov);
    gsl_vector_free(y);
    gsl_vector_free(c);
    /* Find the solutions of the systems */


    /* Compute the eigenvalues of the C matrix */
    /* This gives the principal curvatures */
    delta = (curv[i].c[0] - curv[i].c[2])*(curv[i].c[0] - curv[i].c[2]) +
      4.0*curv[i].c[1]*curv[i].c[1];
    k1 = 0.5*(curv[i].c[0] + curv[i].c[2] + sqrt(delta));
    k2 = 0.5*(curv[i].c[0] + curv[i].c[2] - sqrt(delta));

    if (fabs(k1)>fabs(k2)) {
      printf("Vertex %d : k1=%f k2=%f kg=%f\n",i,k1,k2, k1*k2); 
      curv[i].k1 = k1;
      curv[i].k2 = k2;
    }
    else {
      printf("Vertex %d : k1=%f k2=%f kg=%f\n",i,k2,k1, k1*k2); 
      curv[i].k1 = k2;
      curv[i].k2 = k1;
    }


 
    /* Compute the principal directions (if any) */
    /* They are in the plane (b0, b1) */
    t1l.z = 0.0;
    t2l.z = 0.0;
    if (fabs(delta)>1e-6) {/* if 2 != eigenvalues */
      if(fabs(curv[i].c[1])<1e-6) {/* II is diagonal */
	t1l.x = 1.0;
	t1l.y = 0.0;
	t2l.x = 0.0;
	t2l.y = 1.0;
      } else {
	t1l.x = 1.0;
	t1l.y = (curv[i].k1 - curv[i].c[0])/curv[i].c[1];
	t2l.x = 1.0;
	t2l.y = (curv[i].k2 - curv[i].c[0])/curv[i].c[1];
      }
    } else { /* 1 double eigenvalue */
      /* No principal direction */
      t1l.x = 0.0;
      t1l.y = 0.0;
      t2l.x = 0.0;
      t2l.y = 0.0;
    }
    
    curv[i].t1.x = t1l.x*b0.x + t1l.y*b1.x;
    curv[i].t1.y = t1l.y*b0.y + t1l.y*b1.y;
    curv[i].t1.z = 0.0;
    curv[i].t2.x = t2l.x*b0.x + t2l.y*b1.x;
    curv[i].t2.y = t2l.y*b0.y + t2l.y*b1.y;
    curv[i].t2.z = 0.0;
    normalize(&(curv[i].t1));
    normalize(&(curv[i].t2));
    printf("T1 = (%f, %f, 0.0)\t T2 = (%f, %f, 0.0)\n", curv[i].t1.x, 
	   curv[i].t1.y, curv[i].t2.x, curv[i].t2.y);
    printf("discr = %f\n",norm(crossprod(curv[i].t1,curv[i].t2)));
    free(u);
  }/* End for i */

  

  for (i=0; i<raw_model->num_vert; i++) {
    free(t[i]);
    free(d[i]);
  }
  free(t);
  free(d);
}




void compute_curvature(struct model *raw_model) {
  int i;
  struct info_vertex *curv;
  struct ring_info *rings;

  

  raw_model->area = (double*)malloc(raw_model->num_faces*sizeof(double));
  curv = (struct info_vertex*)
    malloc(raw_model->num_vert*sizeof(struct info_vertex));
  rings= (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
  /* Compute normals of each face of the model */
  raw_model->face_normals = compute_face_normals(raw_model, curv);

  
  /*Compute normals for each vertex */
  printf("Computing vertex normals ... ");fflush(stdout);
  compute_vertex_normal(raw_model, curv, raw_model->face_normals);
  printf("done\n");
  printf("Building 1-rings .... ");fflush(stdout);
  for (i=0; i<raw_model->num_vert; i++) {
    build_star(raw_model, i, &(rings[i]));
    prod_v(-1.0, &(raw_model->normals[i]), &(raw_model->normals[i]));
  }
  printf("done\n");
 
  

  
  
  lsq_fit(raw_model, rings, curv);

   for(i=0; i<raw_model->num_vert; i++) 
     free(curv[i].list_face); 
   
   free(curv); 

}


int main(int argc, char **argv) {
  
  char *basename;
  struct model *raw_model;
  

  if (argc != 2) {
    printf("maps model.raw \n"); 
    exit(0);
  }
  
  basename = argv[1];
  raw_model = read_raw_model(basename);
  printf("Model read\n");


  compute_curvature(raw_model); 
  
  
  free_raw_model(raw_model);
  return 0;
}

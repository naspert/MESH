/* $Id: compare_curv.c,v 1.11 2002/06/04 09:17:34 aspert Exp $ */
#include <3dutils.h>
#include <ring.h>



int main(int argc, char **argv) {
  struct model *raw_model1, *raw_model2;
  struct info_vertex *info1, *info2;
  struct ring_info *rings1, *rings2;
  int i;
  char *filename1, *filename2;
  double *deltak1, *deltak2, *deltakg;
  double maxk1=-FLT_MAX, maxk2=-FLT_MAX, maxkg=-FLT_MAX;
  double max_rel_k1=-FLT_MAX, max_rel_k2=-FLT_MAX, max_rel_kg=-FLT_MAX;
  double mean_dk1=0.0, mean_dk2=0.0, mean_dkg=0.0;
  float area=0.0;

  if (argc != 3) {
    fprintf(stderr, "Usage: compare_curv or_file.raw mod_file.raw\n");
    exit(-1);
  }
  filename1 = argv[1];
  filename2 = argv[2];
  
  raw_model1 = read_raw_model(filename1);
  raw_model2 = read_raw_model(filename2);

  if (raw_model1->num_vert != raw_model2->num_vert) {
    fprintf(stderr, "Unable to compare models w. different sizes\n");
    __free_raw_model(raw_model1);
    __free_raw_model(raw_model2);
    exit(-2);
  }

  printf("Computing face normals...\n");
  info1 = (struct info_vertex*)
    malloc(raw_model1->num_vert*sizeof(struct info_vertex));
  raw_model1->face_normals = compute_face_normals(raw_model1, info1);

  info2 = (struct info_vertex*)
    malloc(raw_model2->num_vert*sizeof(struct info_vertex));
  raw_model2->face_normals = compute_face_normals(raw_model2, info2);

  printf("Computing vertex normals...\n");
  raw_model1->area = (float*)malloc(raw_model1->num_faces*sizeof(float));
  raw_model2->area = (float*)malloc(raw_model2->num_faces*sizeof(float));
  compute_vertex_normal(raw_model1, info1, raw_model1->face_normals); 
  compute_vertex_normal(raw_model2, info2, raw_model2->face_normals); 

  printf("Generating 1-rings of all vertices...\n");
  rings1 = (struct ring_info*)
    malloc(raw_model1->num_vert*sizeof(struct ring_info));
  rings2 = (struct ring_info*)
    malloc(raw_model2->num_vert*sizeof(struct ring_info));

  build_star_global(raw_model1, rings1);
  build_star_global(raw_model2, rings2);

  
  printf("Computing curvature of model 1.... ");fflush(stdout);
  compute_curvature(raw_model1, info1, rings1);
  printf("done\n");  
  printf("Computing curvature of model 2.... ");fflush(stdout);
  compute_curvature(raw_model2, info2, rings2);
  printf("done\n");  

  deltak1 = (double*)malloc(raw_model2->num_vert*sizeof(double));
  deltak2 = (double*)malloc(raw_model2->num_vert*sizeof(double));
  deltakg = (double*)malloc(raw_model2->num_vert*sizeof(double));

  for (i=0; i<raw_model1->num_vert; i++) {
    if (info1[i].k1 > maxk1)
      maxk1 = info1[i].k1;

    deltak1[i] = fabs(info1[i].k1 - info2[i].k1);

    if (info1[i].k2 > maxk2)
      maxk2 = info1[i].k2;

    deltak2[i] = fabs(info1[i].k2 - info2[i].k2);

    if (info1[i].gauss_curv > maxkg)
      maxkg = info1[i].gauss_curv;

    deltakg[i] = fabs(info1[i].gauss_curv - info2[i].gauss_curv);
   
  }
  /* Compute relative error */
  for (i=0; i<raw_model1->num_vert; i++) {
    deltak1[i] /= maxk1;
    deltak2[i] /= maxk2;
    deltakg[i] /= maxkg;
    
    mean_dk1 += info2[i].mixed_area*deltak1[i];
    mean_dk2 += info2[i].mixed_area*deltak2[i];
    mean_dkg += info2[i].mixed_area*deltakg[i];


    if (deltak1[i] > max_rel_k1)
      max_rel_k1 = deltak1[i];
    if (deltak2[i] > max_rel_k2)
      max_rel_k2 = deltak2[i];
    if (deltakg[i] > max_rel_kg)
      max_rel_kg = deltakg[i];

  }

  for (i=0; i<raw_model2->num_faces; i++)
    area += raw_model2->area[i];

  mean_dk1 /= area;
  mean_dk2 /= area;
  mean_dkg /= area;
  
  printf("max_dk1 = %f\n", max_rel_k1);
  printf("max_dk2 = %f\n", max_rel_k2);
  printf("max_dkg = %f\n", max_rel_kg);
  printf("mean_dk1 = %f\n", mean_dk1);
  printf("mean_dk2 = %f\n", mean_dk2);
  printf("mean_dkg = %f\n", mean_dkg);

  for (i=0; i<raw_model1->num_vert; i++) {
    free(info1[i].list_face);
    free(rings1[i].ord_vert);
    free(info2[i].list_face);
    free(rings2[i].ord_vert);
  }
  free(info1);
  free(rings1);
  free(info2);
  free(rings2);
  free(deltak1);
  free(deltak2);
  free(deltakg);
  __free_raw_model(raw_model1);
  __free_raw_model(raw_model2);
  return 0;
}

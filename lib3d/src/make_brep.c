/* $Id: make_brep.c,v 1.1 2001/07/02 12:50:03 aspert Exp $ */
#include <3dutils.h>


int main(int argc, char **argv) {
  char *file1, *file2;
  char *out_file, *tmp=NULL;
  model *raw_model1, *raw_model2;
  vertex bb_min, bb_max;
  int gr_x, gr_y, gr_z, n;

  if (argc != 6) {
    fprintf(stderr, "Usage: make_brep model1 model2 grid_size_x grid_size_y grid_size_z\n");
    exit(1);
  }
  file1 = argv[1];
  file2 = argv[2];
  gr_x = atoi(argv[3]);
  gr_y = atoi(argv[4]);
  gr_z = atoi(argv[5]);

  raw_model1 = read_raw_model(file1);
  raw_model2 = read_raw_model(file2);

  bb_min.x = min(raw_model1->bBox[0].x, raw_model2->bBox[0].x);
  bb_min.y = min(raw_model1->bBox[0].y, raw_model2->bBox[0].y);
  bb_min.z = min(raw_model1->bBox[0].z, raw_model2->bBox[0].z);

  bb_max.x = max(raw_model1->bBox[1].x, raw_model2->bBox[1].x);
  bb_max.y = max(raw_model1->bBox[1].y, raw_model2->bBox[1].y);
  bb_max.z = max(raw_model1->bBox[1].z, raw_model2->bBox[1].z);


  tmp = strrchr(file1, '.');
  if (tmp != NULL) {
    n = tmp - file1;
    out_file = (char*)malloc((n+5)*sizeof(char));
    strncpy(out_file, file1, n*sizeof(char));
    out_file[n] = '\0';
    strcat(out_file, ".brep");
  } else {
    out_file = (char*)malloc((strlen(file1)+5)*sizeof(char));
    strcpy(out_file, file1);
    strcat(out_file, ".brep");
  }
  printf("Writing model 1 to %s\n", out_file);
  write_brep_file(raw_model1, out_file, gr_x, gr_y, gr_z, bb_min, bb_max);
  free(out_file);  

  tmp = strrchr(file2, '.');
  if (tmp != NULL) {
    n = tmp - file2;
    out_file = (char*)malloc((n+5)*sizeof(char));
    strncpy(out_file, file2, n*sizeof(char));
    out_file[n] = '\0';
    strcat(out_file, ".brep");
  } else {
    out_file = (char*)malloc((strlen(file2)+5)*sizeof(char));
    strcpy(out_file, file2);
    strcat(out_file, ".brep");
  }
  printf("Writing model 2 to %s\n", out_file);
  write_brep_file(raw_model2, out_file, gr_x, gr_y, gr_z, bb_min, bb_max);
  free(out_file);

  free(raw_model1->vertices);free(raw_model2->vertices);
  free(raw_model1->faces);  free(raw_model2->faces);
  free(raw_model1); free(raw_model2);
  return 0;
}

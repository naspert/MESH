/* $Id: torus.c,v 1.5 2001/10/30 09:26:18 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>
#include <3dmodel_io.h>


int main(int argc, char **argv) {
  int i,j,k;
  struct model *torus;
  float theta, phi, dth, dph;
  float r,h;
  double k1, k2;
  int nh, nr;
  char *filename;

  if (argc != 6) {
    fprintf(stderr,
	    "Usage : \n\ttorus r h n_points_th n_points_phi filename\n");
    exit(0);
  }

  r = atof(argv[1]);
  h = atof(argv[2]);
  nr = atoi(argv[3]);
  nh = atoi(argv[4]);
  filename = argv[5];

  if (r <= h) {
    fprintf(stderr,"We need r > h !!\n");
    exit(0);
  }

  if (nr == 0 || nh == 0) {
    fprintf(stderr,"We need nr!=0 and nh!=0 !!\n");
    exit(0);
  }
  
  torus = (struct model*)malloc(sizeof(struct model));
  torus->num_faces = nr*nh*2;
  torus->num_vert = nr*nh;
  torus->vertices = (vertex_t*)malloc(torus->num_vert*sizeof(vertex_t));
  torus->faces = (face_t*)malloc(torus->num_faces*sizeof(face_t));
  dth = 2.0*M_PI/(float)nr;
  dph = 2.0*M_PI/(float)nh;
  k2 = 1.0/h;


  theta = 0.0;
  phi = 0.0;

  for (i=0; i<nr; i++) {
    for (j=0; j<nh; j++) {
      torus->vertices[j+nh*i].x = (r + h*cos(phi))*cos(theta);
      torus->vertices[j+nh*i].y = (r + h*cos(phi))*sin(theta);
      torus->vertices[j+nh*i].z = h*sin(phi);
      k1 = k2*(1.0 - r/(r+h*cos(phi)));
      printf("Vertex %d : k1=%f\tk2=%f\tkg=%f\n", j+nh*i, k2, k1, k1*k2);
      phi += dph;
    }
    theta += dth;
    phi = 0.0;
  }
  
  k=0;
  for (i=0; i<nr-1; i++) {
    for (j=0; j<nh-1; j++) {
      torus->faces[k].f0 = nh*i + j;
      torus->faces[k].f1 = nh*i + j + 1;
      torus->faces[k].f2 = nh*(i+1) + j;
      k++;
      torus->faces[k].f0 = nh*i + j + 1;
      torus->faces[k].f1 = nh*(i+1) + j;
      torus->faces[k].f2 = nh*(i+1) + j + 1;
      k++;
    }
    j=nh-1;
    torus->faces[k].f0 = nh*i + j;
    torus->faces[k].f1 = nh*i;
    torus->faces[k].f2 = nh*(i+1) + j;
    k++;
    torus->faces[k].f0 = nh*i;
    torus->faces[k].f1 = nh*(i+1) + j;
    torus->faces[k].f2 = nh*(i+1);
    k++;
  }
  i=nr-1;
  for (j=0; j<nh-1; j++) {
    torus->faces[k].f0 = nh*i + j;
    torus->faces[k].f1 = nh*i + j + 1;
    torus->faces[k].f2 =  j;
    k++;
    torus->faces[k].f0 = nh*i + j + 1;
    torus->faces[k].f1 =   j;
    torus->faces[k].f2 = j + 1;
    k++;
  }
  j=nh-1;
  torus->faces[k].f0 = nh*i + j;
  torus->faces[k].f1 = nh*i;
  torus->faces[k].f2 =  j;
  k++;
  torus->faces[k].f0 = nh*i;
  torus->faces[k].f1 =  j;
  torus->faces[k].f2 = 0;
  k++;

  write_raw_model(torus,filename);
  free(torus->faces);
  free(torus->vertices);
  free(torus);
  return 0;

}

/* $Id: dirac.c,v 1.8 2003/01/14 11:37:38 aspert Exp $ */
#include <3dutils.h>

int main(int argc, char **argv) {
  int valence, i, fcount=0;
  float h, step, th=0.0;
  char *out_fname;
  struct model *dirac;

 if (argc != 4) {
    fprintf(stderr, "Usage: dirac height valence outfile\n");
    exit(-1);
  }
  
  h = atof(argv[1]);
  valence = atoi(argv[2]);
  out_fname = argv[3];
  

  if (valence < 3)
    valence =3;

  dirac = (struct model*)malloc(sizeof(struct model));
  memset(dirac, 0, sizeof(struct model));
  dirac->num_vert = valence + 1;
  dirac->num_faces = valence;
  dirac->vertices = (vertex_t*)malloc(dirac->num_vert*sizeof(vertex_t));
  dirac->faces = (face_t*)malloc(dirac->num_faces*sizeof(face_t));

  dirac->vertices[0].x = 0.0;
  dirac->vertices[0].x = 0.0;
  dirac->vertices[0].z = h;

  step = 2*M_PI/(float)valence;


  for (i=1; i<=valence; i++, th+=step) {
    dirac->vertices[i].x = cos(th);
    dirac->vertices[i].y = sin(th);
    dirac->vertices[i].z = 0.0;
    dirac->faces[fcount].f0 = i;
    dirac->faces[fcount].f1 = ((i+1)==valence+1)?1:i+1;
    dirac->faces[fcount++].f2 = 0;
  }
  

  write_raw_model(dirac, out_fname);

  __free_raw_model(dirac);
  return 0;

}

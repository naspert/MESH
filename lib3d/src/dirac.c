/* $Id: dirac.c,v 1.5 2002/02/26 14:46:46 aspert Exp $ */
#include <3dutils.h>

int main(int argc, char **argv) {
  int valence, i, j, fcount=0, nrings, offset;
  float h, step, r_step=1.5, th=0.0;
  char *out_fname;
  struct model *dirac;

 if (argc != 5) {
    fprintf(stderr, "Usage: dirac height valence nrings outfile\n");
    exit(-1);
  }
  
  h = atof(argv[1]);
  valence = atoi(argv[2]);
  nrings = atoi(argv[3]);
  out_fname = argv[4];
  
  if (nrings < 1)
    nrings = 1;
  if (valence < 3)
    valence =3;

  dirac = (struct model*)malloc(sizeof(struct model));
  memset(dirac, 0, sizeof(struct model));
  dirac->num_vert = (nrings + 1)*valence + 1;
  dirac->num_faces = (nrings*2 +1)*valence;
  dirac->vertices = (vertex_t*)malloc(dirac->num_vert*sizeof(vertex_t));
  dirac->faces = (face_t*)malloc(dirac->num_faces*sizeof(face_t));

  dirac->vertices[0].x = 0.0;
  dirac->vertices[0].x = 0.0;
  dirac->vertices[0].z = h;

  step = 2*M_PI/(float)valence;
  offset = valence + 1;

  for (i=1; i<=valence; i++, th+=step) {
    dirac->vertices[i].x = cos(th);
    dirac->vertices[i].y = sin(th);
    dirac->vertices[i].z = 0.0;
    dirac->faces[fcount].f0 = i;
    dirac->faces[fcount].f1 = ((i+1)==valence+1)?1:i+1;
    dirac->faces[fcount++].f2 = 0;
  }
  
  for (j=0; j<nrings; j++) {
    for (i=0; i<valence; i++) {
      dirac->vertices[offset + i + j*valence].x =  
	r_step*dirac->vertices[offset + i + (j-1)*valence].x;
      dirac->vertices[offset + i + j*valence].y =  
	r_step*dirac->vertices[offset + i + (j-1)*valence].y;
      dirac->vertices[offset + i + j*valence].z =  0.0;

      dirac->faces[fcount].f0 = offset + i + (j-1)*valence;
      dirac->faces[fcount].f1 = offset + i + j*valence;
      dirac->faces[fcount].f2 = 
	(offset + i + 1 +j*valence == offset + (j+1)*valence)?
	j*valence + offset:i + 1 + j*valence + offset;

#ifdef __DIRAC_DEBUG
      printf("1: %d %d %d\n", dirac->faces[fcount].f0, dirac->faces[fcount].f1,
	     dirac->faces[fcount].f2);
#endif
      fcount ++;

      dirac->faces[fcount].f0 = i + (j-1)*valence + offset ;
      dirac->faces[fcount].f1 = 
	(offset + i + 1 +j*valence == offset + (j+1)*valence)?
	j*valence  + offset:i + 1 + j*valence + offset;
      dirac->faces[fcount].f2 = 
	(i + 1 + (j-1)*valence + offset == offset + j*valence)?
	offset + (j-1)*valence :i + 1 + (j-1)*valence + offset;
      
#ifdef __DIRAC_DEBUG
      printf("2: %d %d %d\n", dirac->faces[fcount].f0, dirac->faces[fcount].f1,
	     dirac->faces[fcount].f2);
#endif 
      fcount ++;
    }
  }

  write_raw_model(dirac, out_fname);

  __free_raw_model(dirac);
  return 0;

}

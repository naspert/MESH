/* $Id: model_out.c,v 1.1 2002/05/13 08:38:50 aspert Exp $ */
#include <3dmodel.h>


void write_wrl_model(struct model *raw_model, char *filename) {
  FILE *pf;
  int i;

  pf = fopen(filename, "w");
  if (pf == NULL) {
    fprintf(stderr, "Unable to open %s for output\n", filename);
    exit(-1);
  }
  
  /* print header */
  fprintf(pf, "#VRML V2.0 utf8\n\n");
  
  fprintf(pf, "Transform { children [\n");
  fprintf(pf, "\tShape {\n");
  fprintf(pf, "\t\tappearance Appearance {\n");
  fprintf(pf, "\t\t\tmaterial Material {\n");
  fprintf(pf, "\t\t\t\tdiffuseColor 0.2 1.0 0.8\n");
  fprintf(pf, "\t\t\t\tspecularColor 0.5 1.0 0.5\n");
  fprintf(pf, "\t\t\t\tshininess 0.88\n");
  fprintf(pf, "\t\t\t} # Material\n");
  fprintf(pf, "\t\t} # Appearance\n");
  fprintf(pf, "\t\tgeometry IndexedFaceSet {\n");

  /* print all vertices */
  fprintf(pf, "\t\t\tcoord Coordinate { point [\n");
  for (i=0; i<raw_model->num_vert; i++) 
    fprintf(pf, "\t\t\t\t%f\t%f\t%f\n", raw_model->vertices[i].x, 
            raw_model->vertices[i].y, raw_model->vertices[i].z);
  fprintf(pf, "\t\t\t] } # point, Coordinate\n");
  
  /* print list of triangles */
  fprintf(pf, "\t\t\tcoordIndex [\n");
  for (i=0; i<raw_model->num_faces; i++)
    fprintf(pf, "\t\t\t\t%d\t%d\t%d\t-1\n", raw_model->faces[i].f0, 
            raw_model->faces[i].f1, raw_model->faces[i].f2);

  fprintf(pf, "\t\t\t] # coordIndex\n");
  fprintf(pf, "\t\t} # IndexedFaceSet\n");
  fprintf(pf, "\t} # Shape\n");
  fprintf(pf, "] } # children, Transform\n");
  fclose(pf);
  
  
}

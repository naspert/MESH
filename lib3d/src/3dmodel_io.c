/* $Id: 3dmodel_io.c,v 1.22 2002/02/26 13:18:17 aspert Exp $ */
#include <3dmodel.h>
#include <3dmodel_io.h>
#include <normals.h>

/* int nitems[4] = {nvert, nfaces, nvnorms, nfnorms}; */
int read_header(FILE *pf, int *nitems) {
  char buffer[300];
  int fcount=-1;

  /* set everything to 0 */
  memset(nitems, 0, 4*sizeof(int));

  /* Scan 1st line */
  fgets(buffer, 300, pf);
  
  /* Extract tokens */
  fcount = sscanf(buffer, "%d %d %d %d", &(nitems[0]), &(nitems[1]), 
		  &(nitems[2]), &(nitems[3]));

  /* check consistency of extracted header fields */
  if (fcount < 2 || fcount >4 || nitems[0] == 0 || nitems[1] == 0 ) {
    fprintf(stderr, "Invalid header\n");
    return -EINVAL_HEADER;
  }

  if (nitems[3]>0 && nitems[3]!=nitems[1]) {
    fprintf(stderr, "Incorrect number of face normals\n");
      return -EINVAL_NORMAL;
  }

  if (nitems[2]>0 && nitems[0]!=nitems[2]) {
    fprintf(stderr, "Incorrect number of vertex normals\n");
    return -EINVAL_NORMAL;
  }

  return 0;
/* #else */
} 

struct model* alloc_read_model(FILE *pf, int* header_fields) {

  struct model *raw_model;
  int i;
  float x,y,z;
  int v0, v1, v2;
  int nvert=header_fields[0], nfaces=header_fields[1];
  int nnorms=header_fields[2], nfnorms=header_fields[3];

  printf("num_faces = %d num_vert = %d\n", nfaces, nvert); 
  raw_model = (struct model*)malloc(sizeof(struct model));
  memset(raw_model, 0, sizeof(struct model));
  raw_model->num_faces = nfaces;
  raw_model->num_vert = nvert;
  raw_model->faces = (face_t*)malloc(nfaces*sizeof(face_t));
  raw_model->vertices = (vertex_t*)malloc(nvert*sizeof(vertex_t));


  raw_model->bBox[0].x = FLT_MAX;
  raw_model->bBox[0].y = FLT_MAX;
  raw_model->bBox[0].z = FLT_MAX;

  raw_model->bBox[1].x = -FLT_MAX;
  raw_model->bBox[1].y = -FLT_MAX;
  raw_model->bBox[1].z = -FLT_MAX;

  if (nfnorms > 0) {
    raw_model->face_normals = (vertex_t*)malloc(nfnorms*sizeof(vertex_t));
    raw_model->normals = (vertex_t*)malloc(nnorms*sizeof(vertex_t));
    raw_model->builtin_normals = 1;
  }
  else if (nnorms > 0) {
    raw_model->normals = (vertex_t*)malloc(nnorms*sizeof(vertex_t));
    raw_model->builtin_normals = 1;
  }

  

  for (i=0; i<nvert; i++) {
    fscanf(pf,"%f %f %f",&x, &y, &z);
    raw_model->vertices[i].x = x;
    raw_model->vertices[i].y = y;
    raw_model->vertices[i].z = z;
     if (raw_model->vertices[i].x > raw_model->bBox[1].x) 
      raw_model->bBox[1].x = raw_model->vertices[i].x;
     if (raw_model->vertices[i].x < raw_model->bBox[0].x)
      raw_model->bBox[0].x = raw_model->vertices[i].x;

    if (raw_model->vertices[i].y > raw_model->bBox[1].y) 
      raw_model->bBox[1].y = raw_model->vertices[i].y;
    if (raw_model->vertices[i].y < raw_model->bBox[0].y)
      raw_model->bBox[0].y = raw_model->vertices[i].y;

    if (raw_model->vertices[i].z > raw_model->bBox[1].z) 
      raw_model->bBox[1].z = raw_model->vertices[i].z;
    if (raw_model->vertices[i].z < raw_model->bBox[0].z)
      raw_model->bBox[0].z = raw_model->vertices[i].z;
  }
  
  for (i=0; i<nfaces; i++) {
    fscanf(pf,"%d %d %d",&v0, &v1, &v2);
    raw_model->faces[i].f0 = v0;
    raw_model->faces[i].f1 = v1;
    raw_model->faces[i].f2 = v2;
  }
  
  if (nnorms > 0) {
    for (i=0; i<nnorms; i++) {
      fscanf(pf,"%f %f %f",&x, &y, &z);
      raw_model->normals[i].x = x;
      raw_model->normals[i].y = y;
      raw_model->normals[i].z = z;
    }
  }
  if (nfnorms > 0) {
    for (i=0; i<nfnorms; i++) {
      fscanf(pf,"%f %f %f",&x, &y, &z);
      raw_model->face_normals[i].x = x;
      raw_model->face_normals[i].y = y;
      raw_model->face_normals[i].z = z;
    }
  }
  

  return raw_model;
}


struct model* read_raw_model(char *filename) {
  struct model* raw_model;
  FILE *pf;
  int header_fields[4];


  pf = fopen(filename, "r");
  if (pf==NULL) {
    perror("Error :");
    exit(-1);
  }
    
  if (read_header(pf, header_fields)) {
    fprintf(stderr, "Exiting\n");
    fclose(pf);
    exit(-1);
  } 

  raw_model = alloc_read_model(pf, header_fields);

  fclose(pf);
  return raw_model;
  

}


struct model* read_raw_model_frame(char *filename,int frame) {
  struct model* raw_model;
  FILE *pf;
  char *fullname;
  char *tmp;
  int len, header_fields[4];

  if (frame == -1) {
    len = strlen(filename)+20;
    fullname = (char*)malloc(len*sizeof(char));

    strcpy(fullname, filename);
    strcat(fullname,"_base.raw");
  }
  else {
    len = strlen(filename)+7+(int)log10(((frame>0)?frame:1))+1;
    tmp = (char*)malloc((8+(int)log10((frame>0)?frame:1))*sizeof(char));
    fullname = (char*)malloc(len*sizeof(char));
    
    strcpy(fullname,filename);
    sprintf(tmp,"_%d.raw",frame);
    strcat(fullname,tmp);

    free(tmp);   
  }

  pf = fopen(fullname,"r");
  free(fullname);
  
  if (pf==NULL) {
    perror("Error : ");
    exit(-1);
  }
  
  if (read_header(pf, header_fields)) {
    fclose(pf);
    exit(-1);
  }

  raw_model = alloc_read_model(pf, header_fields);

  fclose(pf);
  return raw_model;
}



void write_raw_model(struct model *raw_model, char *filename) {
  FILE *pf;
  int i;
  char *rootname;
  char *finalname;
  char *tmp;
  int root_length;

  if (raw_model->builtin_normals == 0 && raw_model->normals != NULL) {

    tmp = strrchr(filename, '.'); /* find last occurence of '.' */

    if (tmp == NULL) /* filename does not have an extension */
	rootname = filename;
    else {
      if (*(tmp+1) == '/') /* filename does not have an extension */
	rootname = filename;
      else {
	root_length = tmp - filename; /* number of chars before extension */
	rootname = (char*)malloc((root_length+1)*sizeof(char));
	strncpy(rootname, filename, root_length*sizeof(char));
	rootname[root_length] = '\0'; /* strncpy does not add it */
      }
    }
    finalname = (char*)malloc((strlen(rootname)+7)*sizeof(char));
    sprintf(finalname, "%s_n.raw", rootname);
  } else
    finalname = filename;


  pf = fopen(finalname,"w");
  if (pf == NULL) {
    printf("Unable to open %s\n",filename);
    exit(0);
  }
  if (raw_model->normals == NULL) {
  fprintf(pf,"%d %d\n",raw_model->num_vert,raw_model->num_faces);
  for (i=0; i<raw_model->num_vert; i++)
    fprintf(pf, "%f %f %f\n",raw_model->vertices[i].x,
	    raw_model->vertices[i].y, raw_model->vertices[i].z);

  for (i=0; i<raw_model->num_faces; i++)
    fprintf(pf, "%d %d %d\n",raw_model->faces[i].f0,
	    raw_model->faces[i].f1,raw_model->faces[i].f2);
  } else {
    fprintf(pf,"%d %d %d %d\n",raw_model->num_vert,raw_model->num_faces, 
	    raw_model->num_vert, raw_model->num_faces);
    for (i=0; i<raw_model->num_vert; i++)
      fprintf(pf, "%f %f %f\n",raw_model->vertices[i].x,
	      raw_model->vertices[i].y, raw_model->vertices[i].z);
    
    for (i=0; i<raw_model->num_faces; i++)
      fprintf(pf, "%d %d %d\n",raw_model->faces[i].f0,
	      raw_model->faces[i].f1,raw_model->faces[i].f2);
    
    for (i=0; i<raw_model->num_vert; i++)
      fprintf(pf, "%f %f %f\n", raw_model->normals[i].x, 
	      raw_model->normals[i].y, raw_model->normals[i].z);

    for (i=0; i<raw_model->num_faces; i++)
      fprintf(pf, "%f %f %f\n", raw_model->face_normals[i].x,
	      raw_model->face_normals[i].y, raw_model->face_normals[i].z);
	 
  }
  fclose(pf);
}



void write_brep_file(struct model *raw_model, char *filename, int grid_size_x,
		     int grid_size_y, int  grid_size_z,
		     vertex_t bbox_min, vertex_t bbox_max) {

  FILE *pf;
  int i;
  
  pf = fopen(filename, "w");
  if (pf == NULL) {
    fprintf(stderr, "Unable to open output file %s\n", filename);
    exit(-1);
  }
  fprintf(pf, "%f %f %f %f %f %f\n",bbox_min.x, bbox_min.y, bbox_min.z, 
	  bbox_max.x, bbox_max.y, bbox_max.z);
  fprintf(pf, "%d %d %d\n", grid_size_x, grid_size_y, grid_size_z);
  fprintf(pf, "0\n0\n%d\n%d\n", raw_model->num_vert, raw_model->num_faces);
  for (i=0; i<raw_model->num_vert; i++)
    fprintf(pf, "%f %f %f\n", raw_model->vertices[i].x, 
	    raw_model->vertices[i].y, raw_model->vertices[i].z);
  for (i=0; i<raw_model->num_faces; i++)
    fprintf(pf, "%d %d %d\n", raw_model->faces[i].f0, raw_model->faces[i].f1,
	    raw_model->faces[i].f2);
  fclose(pf);
}

/* $Id: 3dmodel_io.c,v 1.1 2001/03/12 14:50:32 aspert Exp $ */
#include <3dmodel.h>


int read_header(FILE *pf, int *nvert, int *nfaces, int *nnorms) {
  char buffer[300];
  char *tok1;
  char *tok2;
  char *tok3;
  char *delim;


  *nfaces = 0;
  *nvert = 0;
  *nnorms = 0;
  /* These are delimiters for the header */
  delim = (char*)malloc(3*sizeof(char));
  delim[0] = ' ';
  delim[1] = '\n';
  /* Scan 1st line */
  fgets(buffer, 200, pf);
  /* Extract tokens */
  tok1 = strtok(buffer, delim);
  tok2 = strtok(NULL, delim);
  tok3 = strtok(NULL, delim);
  free(delim);
  /* Check validity */
  if (tok3 != NULL) {
    *nnorms = atoi(tok3);
    *nfaces = atoi(tok2);
    *nvert = atoi(tok1);
    if (*nvert != *nnorms) {
      fprintf(stderr, "Incorrect number of normals\n");
      return 0;
    }
  } else if (tok1 != NULL && tok2 != NULL) {
    *nfaces = atoi(tok2);
    *nvert = atoi(tok1);
  } else {
    fprintf(stderr, "Invalid header\n");
    return 0;
  }
  return 1;
} 

model* alloc_read_model(FILE *pf, int nvert, int nfaces, int nnorms) {
  model *raw_model;
  int i;
  double x,y,z;
  int v0, v1, v2;


  printf("num_faces = %d num_vert = %d\n", nfaces, nvert); 
  raw_model = (model*)malloc(sizeof(model));
  raw_model->num_faces = nfaces;
  raw_model->num_vert = nvert;
  raw_model->faces = (face*)malloc(nfaces*sizeof(face));
  raw_model->vertices = (vertex*)malloc(nvert*sizeof(vertex));

  if (nnorms > 0) {
    raw_model->normals = (vertex*)malloc(nnorms*sizeof(vertex));
    raw_model->builtin_normals = 1;
  }
  else {
    raw_model->normals = NULL;
    raw_model->builtin_normals = 0;
  }

  for (i=0; i<nvert; i++) {
    fscanf(pf,"%lf %lf %lf",&x, &y, &z);
    raw_model->vertices[i].x = 1.0*x;
    raw_model->vertices[i].y = 1.0*y;
    raw_model->vertices[i].z = 1.0*z;
  }
  
  for (i=0; i<nfaces; i++) {
    fscanf(pf,"%d %d %d",&v0, &v1, &v2);
    raw_model->faces[i].f0 = v0;
    raw_model->faces[i].f1 = v1;
    raw_model->faces[i].f2 = v2;
  }
  
  if (nnorms > 0) {
    for (i=0; i<nnorms; i++) {
      fscanf(pf,"%lf %lf %lf",&x, &y, &z);
      raw_model->normals[i].x = x;
      raw_model->normals[i].y = y;
      raw_model->normals[i].z = z;
    }
  }
  return raw_model;
}


model* read_raw_model(char *filename) {
  model* raw_model;
  FILE *pf;
  int nfaces=0; 
  int nvert=0;
  int nnorms=0;


  pf = fopen(filename, "r");
  if (pf==NULL) {
    perror("Error :");
    exit(-1);
  }
    
  if (read_header(pf, &nvert, &nfaces, &nnorms) == 0) {
    fclose(pf);
    exit(-1);
  } 

  raw_model = alloc_read_model(pf, nvert, nfaces, nnorms);

  fclose(pf);
  return raw_model;
  

}


model* read_raw_model_frame(char *filename,int frame) {
  model* raw_model;
  FILE *pf;
  char *fullname;
  char *tmp;
  int nfaces, nvert, len, nnorms;

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
  
  if (read_header(pf, &nvert, &nfaces, &nnorms) == 0) {
    fclose(pf);
    exit(-1);
  }

  raw_model = alloc_read_model(pf, nvert, nfaces, nnorms);

  fclose(pf);
  return raw_model;
}


void write_raw_model(model *raw_model, char *filename) {
  FILE *pf;
  int i;
  char *rootname;
  char *delim;
  char *finalname;
  
  if (raw_model->builtin_normals == 0 && raw_model->normals != NULL) {
    delim = (char*)malloc(2*sizeof(char));
    delim[0] = '.';
    delim[1] = '\n';
    rootname = strtok(filename, delim);
    free(delim);
    finalname = (char*)malloc((strlen(rootname)+7)*sizeof(char));
    sprintf(finalname, "%s_n.raw", rootname);
  } else if (raw_model->normals == NULL)
    finalname = filename;
  else /* This model already has normals, do nothing */
    return;

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
    fprintf(pf,"%d %d %d\n",raw_model->num_vert,raw_model->num_faces, 
	    raw_model->num_vert);
    for (i=0; i<raw_model->num_vert; i++)
      fprintf(pf, "%f %f %f\n",raw_model->vertices[i].x,
	      raw_model->vertices[i].y, raw_model->vertices[i].z);
    
    for (i=0; i<raw_model->num_faces; i++)
      fprintf(pf, "%d %d %d\n",raw_model->faces[i].f0,
	      raw_model->faces[i].f1,raw_model->faces[i].f2);
    
    for (i=0; i<raw_model->num_vert; i++)
      fprintf(pf, "%f %f %f\n", raw_model->normals[i].x, 
	      raw_model->normals[i].y, raw_model->normals[i].z);
	 
  }
  fclose(pf);
}


/* $Id: isoca.c,v 1.2 2001/09/27 12:53:41 aspert Exp $ */
#include <3dutils.h>


int test_vertex(struct model *raw_model, vertex_t v) {
  int i;
  vertex_t tmp;

  for (i=0; i<raw_model->num_vert; i++) {
    substract_v(&(raw_model->vertices[i]), &v, &tmp);
    if (fabs(norm(tmp))<1e-6)
      return i;
  }
  return -1;
}

struct model* subdiv(struct model *raw_model) {
  int nfaces = 0; 
  int i;
  struct model *subd;
  vertex_t tmp1, tmp2, tmp3;
  int new1, new2, new3;


  subd = (struct model*)malloc(sizeof(struct model));
  subd->num_faces = 4*raw_model->num_faces;
  subd->faces = (face_t*)malloc(subd->num_faces*sizeof(face_t));
  subd->vertices = (vertex_t*)malloc(raw_model->num_vert*sizeof(vertex_t));
  memcpy(subd->vertices, raw_model->vertices, 
	 raw_model->num_vert*sizeof(vertex_t));
  
  subd->num_vert = raw_model->num_vert;
  
  for (i=0; i<raw_model->num_faces; i++) {

    /* Edge f0f1 */
    tmp1 = raw_model->vertices[raw_model->faces[i].f0];
    tmp2 = raw_model->vertices[raw_model->faces[i].f1];

    tmp3.x = (tmp1.x + tmp2.x)*0.5;
    tmp3.y = (tmp1.y + tmp2.y)*0.5;
    tmp3.z = (tmp1.z + tmp2.z)*0.5;
    
    new1 = test_vertex(subd, tmp3);
    if (new1 == -1) {/* New vertex in the model */
      subd->vertices = realloc(subd->vertices, 
			       (subd->num_vert+1)*sizeof(vertex_t));
     
      subd->vertices[subd->num_vert].x = tmp3.x;
      subd->vertices[subd->num_vert].y = tmp3.y;
      subd->vertices[subd->num_vert].z = tmp3.z;
      new1 = subd->num_vert;
      subd->num_vert++;
      
    }


    /* Edge f1f2 */
    tmp1 = raw_model->vertices[raw_model->faces[i].f1];
    tmp2 = raw_model->vertices[raw_model->faces[i].f2];

    tmp3.x = (tmp1.x + tmp2.x)*0.5;
    tmp3.y = (tmp1.y + tmp2.y)*0.5;
    tmp3.z = (tmp1.z + tmp2.z)*0.5;

    new2 = test_vertex(subd, tmp3);
    if (new2 == -1) {/* New vertex in the model */
      subd->vertices = realloc(subd->vertices, 
			       (subd->num_vert+1)*sizeof(vertex_t));
      subd->vertices[subd->num_vert].x = tmp3.x;
      subd->vertices[subd->num_vert].y = tmp3.y;
      subd->vertices[subd->num_vert].z = tmp3.z;
      new2 = subd->num_vert;
      subd->num_vert++;
    }


    /*Edge f2f0 */
    tmp1 = raw_model->vertices[raw_model->faces[i].f2];
    tmp2 = raw_model->vertices[raw_model->faces[i].f0];

    tmp3.x = (tmp1.x + tmp2.x)*0.5;
    tmp3.y = (tmp1.y + tmp2.y)*0.5;
    tmp3.z = (tmp1.z + tmp2.z)*0.5;

    new3 = test_vertex(subd, tmp3);
    if (new3 == -1) {/* New vertex in the model */
      subd->vertices = realloc(subd->vertices, 
			       (subd->num_vert+1)*sizeof(vertex_t));
      subd->vertices[subd->num_vert].x = tmp3.x;
      subd->vertices[subd->num_vert].y = tmp3.y;
      subd->vertices[subd->num_vert].z = tmp3.z;
      new3 = subd->num_vert;
      subd->num_vert++;
    }

    subd->faces[nfaces].f0 = raw_model->faces[i].f0;
    subd->faces[nfaces].f1 = new1;
    subd->faces[nfaces].f2 = new3;
    nfaces++;

    subd->faces[nfaces].f0 = new1;
    subd->faces[nfaces].f1 = new2;
    subd->faces[nfaces].f2 = new3;
    nfaces++;

    subd->faces[nfaces].f0 = raw_model->faces[i].f1;
    subd->faces[nfaces].f1 = new2;
    subd->faces[nfaces].f2 = new1;
    nfaces++;

    subd->faces[nfaces].f0 = raw_model->faces[i].f2;
    subd->faces[nfaces].f1 = new2;
    subd->faces[nfaces].f2 = new3;
    nfaces++;
  }
  
  for (i=raw_model->num_vert; i<subd->num_vert; i++) 
    normalize_v(&(subd->vertices[i]));


  return subd;
}

int main(int argc, char **argv) {
  double phi,stheta;
  double z,x,y;
  int i,j;
  int n;
  char *filename;
  struct model isoca;
  struct model *or_mod, *sub_mod=NULL;
  int count_faces = 0;

  if (argc != 3) {
    fprintf(stderr, "isoca filename lev\n");
    exit(-1);
  }
  
  filename = argv[1];
  n = atoi(argv[2]);

  isoca.vertices = (vertex_t*)malloc(12*sizeof(vertex_t));
  isoca.faces = (face_t*)malloc(20*sizeof(face_t));
  isoca.num_vert = 12;
  isoca.num_faces = 20;

  z = 0.2*sqrt(5); /* cos(theta) */
  stheta = 0.4*sqrt(5);

  /* Top */
  isoca.vertices[0].x = 0.0;
  isoca.vertices[0].y = 0.0;
  isoca.vertices[0].z = 1.0;

  /* Upper vertices */
  for (i=0; i<5; i++) {
    phi = 0.4*i*M_PI;
    x = stheta*cos(phi);
    y = stheta*sin(phi);

    isoca.vertices[i+1].x = x;
    isoca.vertices[i+1].y = y;
    isoca.vertices[i+1].z = z;
  }


  z= -z;
  /* Lower vertices */
  for (i=0; i<5; i++) {
    phi = (2.0*i+1.0)*M_PI/5.0;
    x = stheta*cos(phi);
    y = stheta*sin(phi);

    isoca.vertices[i+6].x = x;
    isoca.vertices[i+6].y = y;
    isoca.vertices[i+6].z = z;
  }

  /* Bottom */
  isoca.vertices[11].x = 0.0;
  isoca.vertices[11].y = 0.0;
  isoca.vertices[11].z = -1.0;


  /* 'Upper' faces */
  for (i=1; i<6; i++) {
    isoca.faces[count_faces].f0 = 0;
    isoca.faces[count_faces].f1 = i;
    isoca.faces[count_faces].f2 = ((i+1>5)?1:i+1);
    count_faces++;
  }

  /* 'Lower' faces */
  for (i=6; i<11; i++) {
    isoca.faces[count_faces].f0 = 11;
    isoca.faces[count_faces].f1 = i;
    isoca.faces[count_faces].f2 = ((i+1>10)?6:i+1);
    count_faces++;
  }


  /* 'Middle' faces */
  i=1;
  j=6;
  while (i<6 && j<11) {
    isoca.faces[count_faces].f0 = i;
    isoca.faces[count_faces].f1 = ((i+1>5)?(i+1)%5:i+1);
    isoca.faces[count_faces].f2 = j;
    count_faces++;
    isoca.faces[count_faces].f0 = i;
    isoca.faces[count_faces].f1 = ((j-1<6)?10:j-1);
    isoca.faces[count_faces].f2 = j;
    count_faces++;
    i++;
    j++;
  }
  
  or_mod = &isoca;
  sub_mod = &isoca;
  for (i=1; i<n; i++) {
    printf("Level %d ... ", i+1);fflush(stdout);
    sub_mod = subdiv(or_mod);
    free(or_mod->faces);
    free(or_mod->vertices);
    or_mod = sub_mod;
    printf("done\n");
  }

  sub_mod->normals = NULL;
  sub_mod->face_normals = NULL;
  sub_mod->tree = NULL;
  write_raw_model(sub_mod, filename);

  free_raw_model(sub_mod);
  return 0;
}

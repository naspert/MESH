/* $Id: isoca.c,v 1.8 2002/11/05 10:18:55 aspert Exp $ */
#include <3dutils.h>
#include <subdiv.h>

static void midpoint_sph(const struct ring_info *rings, const int center, 
		 	 const int v1, 
	                 const struct model *raw_model, vertex_t *vout) {
  int center2 = rings[center].ord_vert[v1];
  vertex_t p;

  __add_v(raw_model->vertices[center], raw_model->vertices[center2], p);
  __prod_v(0.5, p, p);

  __normalize_v(p);
  
  *vout = p;
}

int main(int argc, char **argv) {
  float phi,stheta;
  float z,x,y;
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
  
  /* init the isoca structure */
  memset(&isoca, 0, sizeof(struct model));

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
 
  for (i=0; i<n; i++) {
    printf("Level %d ... ", i+1);fflush(stdout);
    sub_mod = subdiv(or_mod, midpoint_sph, NULL, NULL);
    free(or_mod->faces);
    free(or_mod->vertices);
    or_mod = sub_mod;
    printf("done\n");
  }

  sub_mod->normals = NULL;
  sub_mod->face_normals = NULL;
  sub_mod->tree = NULL;
  write_raw_model(sub_mod, filename);

  __free_raw_model(sub_mod);
  return 0;
}

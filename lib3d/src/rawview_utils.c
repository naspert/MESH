/* $Id: rawview_utils.c,v 1.1 2002/06/04 13:06:40 aspert Exp $ */
#include <3dutils.h>
#include <rawview.h>
#include <rawview_misc.h>



void set_light_on() {
  glEnable(GL_LIGHTING);
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
  glEnable(GL_LIGHT0);
  glColor3f(1.0, 1.0, 1.0);
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void set_light_off() {
  printf("Wireframe mode\n");
  glDisable(GL_LIGHTING);
  glColor3f(1.0, 1.0, 1.0);
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

int do_normals(struct model* raw_model) {
  struct info_vertex *tmp;
  int i;

  printf("Computing normals...\n");
  raw_model->area = (float*)malloc(raw_model->num_faces*sizeof(float));
  tmp = (struct info_vertex*)
    malloc(raw_model->num_vert*sizeof(struct info_vertex));
  
  raw_model->face_normals = compute_face_normals(raw_model, tmp);
  
  if (raw_model->face_normals != NULL){
    compute_vertex_normal(raw_model, tmp, raw_model->face_normals);
    for (i=0; i<raw_model->num_vert; i++) 
      free(tmp[i].list_face);
    free(tmp);
    printf("Face and vertex normals done !\n");
    return 0;
  } else {
    printf("Error - Unable to build face normals (Non-manifold model ?)\n");
    return 1;
  }
    
}

int do_spanning_tree(struct model *raw_model) {
  struct info_vertex* tmp;
  int i, ret=0;

  tmp = (struct info_vertex*)
    malloc(raw_model->num_vert*sizeof(struct info_vertex));
  for(i=0; i<raw_model->num_vert; i++) {
    tmp[i].list_face = (int*)malloc(sizeof(int));
    tmp[i].num_faces = 0;
  }
  printf("Building spanning tree\n");
  /* Compute spanning tree of the dual graph */
  raw_model->tree = bfs_build_spanning_tree(raw_model, tmp); 
  if (raw_model->tree == NULL) {
    printf("Unable to build spanning tree\n");
    ret = 1;
  }

  if (ret == 0) {
    printf("Spanning tree done\n");
  }
  for(i=0; i<raw_model->num_vert; i++) 
    free(tmp[i].list_face);
  free(tmp);
  return ret;
}

/* tree destructor */
void destroy_tree(struct face_tree *tree) {
  
  if (tree->left != NULL)
    destroy_tree(tree->left);
  if (tree->right != NULL)
    destroy_tree(tree->right);

  if (tree->left == NULL && tree->right == NULL) {
    if (tree->parent != NULL) {
      if (tree->node_type == 0)
	(tree->parent)->left = NULL;
      else
	(tree->parent)->right = NULL;
    }
    free(tree);

  }
}



/* $Id: normals.c,v 1.1 2001/03/12 14:50:32 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>



/* FIND  face containing the vertices v0 & v1 not  being face_excl */
int list_faces_excl(model *raw_model, int v0, int v1, int face_excl) {
  int i;
  face *cur;
  
 
  for (i=0; i<raw_model->num_faces; i++) {
    if (i == face_excl)
      continue;
    else {
      cur = &(raw_model->faces[i]);
      if (cur->f0==v0 && (cur->f1==v1 || cur->f2==v1))
	return i;
      else if (cur->f1==v0 && (cur->f0==v1 || cur->f2==v1))
	return i;
      else if (cur->f2==v0 && (cur->f0==v1 || cur->f1==v1))
	return i;
    }
  }
  return -1;
}

/* FIND  face containing the vertices v0 & v1  */
int* list_faces(model *raw_model, int v0, int v1) {
  int i, *res ;
  face *cur;
  int nfaces = 0;

  res = (int*)malloc(sizeof(int));
  res[0] = 0;
  for (i=0; i<raw_model->num_faces; i++) {
      cur = &(raw_model->faces[i]);
      if (cur->f0==v0 && (cur->f1==v1 || cur->f2==v1)) {
	nfaces++;
	res = (int*)realloc(res,(nfaces+1)*sizeof(int));
	res[nfaces] = i;
      }
      else if (cur->f1==v0 && (cur->f0==v1 || cur->f2==v1)) {
	nfaces++;
	res = (int*)realloc(res,(nfaces+1)*sizeof(int));
	res[nfaces] = i;
      }
      else if (cur->f2==v0 && (cur->f0==v1 || cur->f1==v1)) {
	nfaces++;
	res = (int*)realloc(res,(nfaces+1)*sizeof(int));
	res[nfaces] = i;
      }
  }
  res[0] = nfaces;
  return res;
  
}



/* find the 1-ring of vertex v */
ring_info build_star2(model *raw_model, int v) {


  int i,j,k;
  int num_edges=0; /* number of edges in the 1-ring */
  edge_v *edge_list=NULL;
  int *final_star;
  int *done;
  int star_size;
  int edge_added;
  ring_info ring;

  /* list all edges in the 1-ring */
  for (i=0; i<raw_model->num_faces; i++) {
    if (raw_model->faces[i].f0 == v) {
      num_edges ++;
      edge_list = (edge_v*)realloc(edge_list, num_edges*sizeof(edge_v));

#ifdef NORM_DEBUG
      if (edge_list == NULL) {
	printf("realloc failed %d\n", i);
	exit(-1);
      }
#endif

      edge_list[num_edges-1].v0 = raw_model->faces[i].f1;
      edge_list[num_edges-1].v1 = raw_model->faces[i].f2;      
    } else if (raw_model->faces[i].f1 == v) {
      num_edges ++;
      edge_list = (edge_v*)realloc(edge_list, num_edges*sizeof(edge_v));

#ifdef NORM_DEBUG
      if (edge_list == NULL) {
	printf("realloc failed %d\n", i);
	exit(-1);
      }
#endif

      edge_list[num_edges-1].v0 = raw_model->faces[i].f0;
      edge_list[num_edges-1].v1 = raw_model->faces[i].f2;      
    }  else if (raw_model->faces[i].f2 == v) {
      num_edges ++;
      edge_list = (edge_v*)realloc(edge_list, num_edges*sizeof(edge_v));

#ifdef NORM_DEBUG
      if (edge_list == NULL) {
	printf("realloc failed %d\n", i);
	exit(-1);
      }
#endif

      edge_list[num_edges-1].v0 = raw_model->faces[i].f0;
      edge_list[num_edges-1].v1 = raw_model->faces[i].f1;      
    }

  }

  if (num_edges == 0) {
    printf("Vertex %d has no face...\n", v);
    ring.type = 0;
    ring.size = 0;
    ring.ord_vert = NULL;
    return ring;
  }

  done = (int*)calloc(num_edges, sizeof(int));
  /* worst case allocation */
  final_star = (int*)malloc(2*num_edges*sizeof(int));

  /* Put 1st two elts in the star */
  final_star[0] = edge_list[0].v0;
  final_star[1] = edge_list[0].v1;
  star_size = 2;
  done[0] = 1;
  i = 1;


  /* build the 1-ring */
  while (i < num_edges) {
    edge_added = 0;
    for(j=0; j<num_edges; j++) {
      if (done[j] == 1)
	continue;
      if (edge_list[j].v0 == final_star[0]) {
	/* add v1 on top */
	for (k=star_size-1; k>=0; k--) 
	  final_star[k+1] = final_star[k];
	
	final_star[0] = edge_list[j].v1;
	done[j] = 1;
	i++;
	star_size++;
	edge_added = 1;
	break;
      }
      else if (edge_list[j].v1 == final_star[0]) {
	/* add v0 on top */
	for (k=star_size-1; k>=0; k--) 
	  final_star[k+1] = final_star[k];
	
	final_star[0] = edge_list[j].v0;
	done[j] = 1;
	i++;
	star_size++;
	edge_added = 1;
	break;
      }
      else if (edge_list[j].v0 == final_star[star_size-1]) {
	/* add v1 on bottom */
	final_star[star_size] = edge_list[j].v1;
	done[j] = 1;
	i++;
	star_size++;
	edge_added = 1;
	break;
      }
      else if (edge_list[j].v1 == final_star[star_size-1]) {
	/* add v0 on bottom */
	final_star[star_size] = edge_list[j].v0;
	done[j] = 1;
	i++;
	star_size++;
	edge_added = 1;
	break;
      }
    }
    if (edge_added == 0) {
      printf("Vertex %d is non-manifold\n", v);
      free(done);
      free(final_star);
      ring.type = 2;
      ring.size = 0;
      ring.ord_vert = NULL;
      return ring;
    }
  }

  if (final_star[0] == final_star[star_size-1]) {
/*     printf("Regular vertex %d\n", v); */
    star_size--;
    ring.type = 0;
  } else {
/*     printf("Boundary vertex %d\n", v); */
    ring.type = 1;
  }

  ring.size = star_size;
  ring.ord_vert = (int*)malloc(star_size*sizeof(int));
  memcpy(ring.ord_vert, final_star, star_size*sizeof(int));

#ifdef NORM_DEBUG
  printf("Vertex %d : star_size = %d num_edges = %d\n", v, star_size, 
	 num_edges);
  for (i=0; i<star_size; i++)
    printf("vertex %d \n", final_star[i]);
#endif

  free(edge_list);
  free(final_star);
  free(done);
  return ring;
}



/* Returns 0 if the model has a non-manifold edge */
int manifold_edges(model *raw_model, face_tree_ptr *tree){
  int i;

  int *test;

  test = NULL;
  for (i=0; i<raw_model->num_faces; i++) {
    if (tree[i]->left != NULL) {
      test = list_faces(raw_model, 
			tree[i]->prim_left.v0, 
			tree[i]->prim_left.v1);
      if (test[0] > 2) {
	printf("Non manifold edge : %d %d\n", tree[i]->prim_left.v0, 
	       tree[i]->prim_left.v1);
	free(test);
	return 0;
      } else
	free(test);
    }
    if (tree[i]->right != NULL) {
      test = list_faces(raw_model, 
			tree[i]->prim_right.v0, 
			tree[i]->prim_right.v1);
      if (test[0] > 2) {
	printf("Non manifold edge : %d %d\n", tree[i]->prim_left.v0, 
	       tree[i]->prim_left.v1);
	free(test);
	return 0;
      } else 
	free(test);
    }

  }
  return 1;
}

int test_cycle(face_tree_ptr tree, int test_idx) {

  if (tree->face_idx == test_idx)
    return 1;

  if ((tree->left != NULL) && (test_cycle(tree->left, test_idx) == 1))
     return 1;
  
  if ((tree->right != NULL) && (test_cycle(tree->right, test_idx) == 1))
     return 1;

  return 0;
}



/* tree destructor */
void destroy_tree(face_tree_ptr tree) {
  
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



edge_list_ptr add_edge(edge_list_ptr list, int face0, int face1, 
		       int v0, int v1) {


  list->edge.face0 = face0;
  list->edge.face1 = face1;
  list->edge.common.v0 = v0;
  list->edge.common.v1 = v1;
  list->next = (edge_list_ptr)malloc(sizeof(edge_list));
  list = list->next;
  list->next = NULL;
  return list;

}


int find_adj_edges(model *raw_model, int cur_face, edge_list_ptr list) {
  int v0, v1, v2;
  int i;
  int num_added = 0; /* Number of edges added*/
  face *cur;
  edge_list_ptr cur_item;
  
/*  printf("find_adj_edges: %d\n", cur_face);*/
  v0 = raw_model->faces[cur_face].f0;
  v1 = raw_model->faces[cur_face].f1;
  v2 = raw_model->faces[cur_face].f2;

  /* Go to the end of the list */
  cur_item = list;
  while(cur_item->next != NULL)
    cur_item = cur_item->next;
   
  for (i=0; i<raw_model->num_faces; i++) {
    if (i == cur_face)
      continue;
    else {
      cur = &(raw_model->faces[i]);
      if (cur->f0==v0 && (cur->f1==v1 || cur->f2==v1)) {
	cur_item = add_edge(cur_item,  cur_face, i, v0, v1);
	num_added++;
      }
      else if (cur->f1==v0 && (cur->f0==v1 || cur->f2==v1)) {
	cur_item = add_edge(cur_item, cur_face, i, v0, v1);
	num_added++;
      }
      else if (cur->f2==v0 && (cur->f0==v1 || cur->f1==v1)) {
	cur_item  = add_edge(cur_item, cur_face, i, v0, v1);
	num_added++;
      }
      else if (cur->f0==v0 && (cur->f1==v2 || cur->f2==v2)) {
	cur_item = add_edge(cur_item, cur_face, i, v0, v2);
	num_added++;
      }
      else if (cur->f1==v0 && (cur->f0==v2 || cur->f2==v2)) {
	cur_item = add_edge(cur_item, cur_face, i, v0, v2);
	num_added++;
      }
      else if (cur->f2==v0 && (cur->f0==v2 || cur->f1==v2)) {
	cur_item  = add_edge(cur_item, cur_face, i, v0, v2);
	num_added++;
      }
      else if (cur->f0==v1 && (cur->f1==v2 || cur->f2==v2)) {
	cur_item = add_edge(cur_item, cur_face, i, v1, v2);
	num_added++;
      }
      else if (cur->f1==v1 && (cur->f0==v2 || cur->f2==v2)) {
	cur_item = add_edge(cur_item, cur_face, i, v1, v2);
	num_added++;
      }
      else if (cur->f2==v1 && (cur->f0==v2 || cur->f1==v2)) {
	cur_item  = add_edge(cur_item, cur_face, i, v1, v2);
	num_added++;
      }
    }
  }
/*  printf("find_adj_edges: %d %d\n", list->edge.face0, list->edge.face1);*/
  return num_added;
}

face_tree_ptr* bfs_build_spanning_tree(model *raw_model) {
  int faces_traversed = 0;

  edge_list_ptr list, cur_list, old;
  face_tree_ptr *tree, cur_node, new_node, top;
  int list_size = 0, i;


  list = (edge_list_ptr)malloc(sizeof(edge_list));

  tree = (face_tree_ptr*)malloc(raw_model->num_faces*sizeof(face_tree_ptr));
  /* Initialize all cells */
  for (i=0; i<raw_model->num_faces; i++) {
    tree[i] = (face_tree_ptr)malloc(sizeof(face_tree));
    tree[i]->parent = NULL;
    tree[i]->left = NULL;
    tree[i]->right = NULL;
    tree[i]->visited = 0;
  }



  tree[0]->face_idx = 0;
  tree[0]->node_type=0;
  tree[0]->visited = 1;
  list->next = NULL;
  cur_node = tree[0]; /* Initialized to the root of the tree */
  top = tree[0];

  /* Build 1st node */
  list_size += find_adj_edges(raw_model, 0, list);   
  cur_list = list;/* local copy of the full list */
  faces_traversed ++;

  while(list_size > 0) {
    if (tree[cur_list->edge.face1]->visited == 0) { /* if 1 we have a cycle */
      /* Add the face to the tree */
      cur_node = tree[cur_list->edge.face0];
      if (cur_node->left == NULL) {
	cur_node->left = tree[cur_list->edge.face1];
	cur_node->prim_left = cur_list->edge.common;
#ifdef NORM_DEBUG
	printf("face0=%d face1=%d left %d %d\n", 
	       (cur_list->edge).face0, (cur_list->edge).face1, 
	       cur_node->prim_left.v0, cur_node->prim_left.v1);
#endif
	new_node = cur_node->left;
	new_node->parent = cur_node;
	new_node->node_type = 0;
	new_node->face_idx = cur_list->edge.face1;
	new_node->visited = 1;
	faces_traversed++;
	list_size--;
	old = cur_list;
	cur_list = cur_list->next; 

      } else if (cur_node->right == NULL) {
	cur_node->prim_right = cur_list->edge.common;
	cur_node->right = tree[cur_list->edge.face1];
#ifdef NORM_DEBUG
	printf("face0=%d face1=%d right %d %d\n", 
 	       (cur_list->edge).face0, (cur_list->edge).face1, 
	       cur_node->prim_right.v0, cur_node->prim_right.v1);
#endif
	new_node = cur_node->right;
	new_node->parent = cur_node;
	new_node->node_type = 1;
	new_node->face_idx = cur_list->edge.face1;
	new_node->visited = 1;
	faces_traversed++; 
	old = cur_list;
	list_size--;
	cur_list = cur_list->next;

      } else if (cur_node->parent==NULL) {/*Top of the tree*/
	cur_node->parent = tree[cur_list->edge.face1];
	new_node = cur_node->parent;
	top = new_node; /* We have to update this */
	new_node->left = cur_node;/*this is arbitrary...*/
	new_node->prim_left = cur_list->edge.common;
#ifdef NORM_DEBUG
	printf("face0=%d face1=%d up %d %d\n", 
 	       cur_list->edge.face0, cur_list->edge.face1,
	       new_node->prim_left.v0, new_node->prim_left.v1);
#endif
	new_node->node_type = 0;
	new_node->face_idx = cur_list->edge.face1;
	new_node->visited = 1;
	faces_traversed++;
	old = cur_list;
	cur_list = cur_list->next;
	list_size--;
      }
      else {
	printf("Non-manifold edge %d %d !\n", cur_list->edge.face0, 
	       cur_list->edge.face1);
	exit(-1);
      }
      /* Add adjacent edges to the list */
      list_size += find_adj_edges(raw_model, old->edge.face1,  
 				  cur_list); 
      
      free(old);
    } else { /* discard this face from the list */
      old = cur_list;
      cur_list = cur_list->next;
      list_size--;
      free(old);
    }
  }

#ifdef NORM_DEBUG
   printf("faces_traversed = %d num_faces = %d\n", faces_traversed,  
 	 raw_model->num_faces); 
#endif NORM_DEBUG
  
  return tree;
}



int find_center(face *cur,int v1, int v2) {
  if (cur->f0==v1) {
    if (cur->f1==v2)
      return cur->f2;
    else if (cur->f2==v2)
      return cur->f1;
  } else if (cur->f1==v1) {
    if (cur->f0==v2)
      return cur->f2;
    else if (cur->f2==v2)
      return cur->f0;
  } else {
    if (cur->f1==v2)
      return cur->f0;
    else if (cur->f0==v2)
      return cur->f1;
  } 
  printf("find_center: Error\n");
  return -1;
}

void swap_vert(edge_v *p) {
  int tmp;

  tmp = p->v0;
  p->v0 = p->v1;
  p->v1 = tmp;
}

void update_child_edges(face_tree_ptr tree, int v0, int v1, int v2) {
  
  if (tree->left != NULL) {
    if (tree->right == NULL) { /* update prim_left */

      
      if ((tree->prim_left).v0 == v1 && (tree->prim_left).v1 == v0)  
	/* prim_left = v1v0 */
	swap_vert(&(tree->prim_left));
      else if ((tree->prim_left).v0 == v0 && (tree->prim_left).v1 == v2) 
	/* prim_left = v0v2 */
	swap_vert(&(tree->prim_left));
      else if ((tree->prim_left).v0 == v2 && (tree->prim_left).v1 == v1)
	/* prim_left = v2v1 */
	swap_vert(&(tree->prim_left));
      
    } else { /* update prim_left & prim_right */
      if ((tree->prim_left).v0 == v1 && (tree->prim_left).v1 == v0)  
	/* prim_left = v1v0 */
	swap_vert(&(tree->prim_left));
      else if ((tree->prim_left).v0 == v0 && (tree->prim_left).v1 == v2) 
	/* prim_left = v0v2 */
	swap_vert(&(tree->prim_left));
      else if ((tree->prim_left).v0 == v2 && (tree->prim_left).v1 == v1)
	/* prim_left = v2v1 */
	swap_vert(&(tree->prim_left));


      if ((tree->prim_right).v0 == v1 && (tree->prim_right).v1 == v0)  
	/* prim_right = v1v0 */
	swap_vert(&(tree->prim_right));
      else if ((tree->prim_right).v0 == v0 && (tree->prim_right).v1 == v2) 
	/* prim_right = v0v2 */
	swap_vert(&(tree->prim_right));
      else if ((tree->prim_right).v0 == v2 && (tree->prim_right).v1 == v1)
	/* prim_right = v2v1 */
	swap_vert(&(tree->prim_right));


    }
  } else {
    if (tree->right != NULL) { /* update prim_right */
     if ((tree->prim_right).v0 == v1 && (tree->prim_right).v1 == v0)  
	/* prim_right = v1v0 */
	swap_vert(&(tree->prim_right));
      else if ((tree->prim_right).v0 == v0 && (tree->prim_right).v1 == v2) 
	/* prim_right = v0v2 */
	swap_vert(&(tree->prim_right));
      else if ((tree->prim_right).v0 == v2 && (tree->prim_right).v1 == v1)
	/* prim_right = v2v1 */
	swap_vert(&(tree->prim_right)); 
    }
  }
}




void build_normals(model *raw_model, face_tree_ptr tree, vertex* normals) {

  int v0=-1, v1=-1, v2=-1;
/*  double tmp;*/


  if (tree->parent == NULL) {/* root of the tree */
    if(tree->left == NULL) {
      v0 = (tree->prim_right).v0;
      v2 = (tree->prim_right).v1;
      v1 = find_center(&(raw_model->faces[tree->face_idx]), v0, v2);
    } else if (tree->right == NULL) {
      v0 = (tree->prim_left).v0;
      v1 = (tree->prim_left).v1;
      v2 = find_center(&(raw_model->faces[tree->face_idx]), v0, v1);
    } else {
      v0 = (tree->prim_left).v0;
      v1 = (tree->prim_left).v1;
      if ((tree->prim_right).v0 == v1) { /* prim_right = v1v2*/
	v2 = (tree->prim_right).v1;
      } else if ((tree->prim_right).v0 == v0) { /* prim_right = v0v2 */
	v2 = (tree->prim_right).v1;
	(tree->prim_right).v0 = v2;
	(tree->prim_right).v1 = v0;	
      } else if ((tree->prim_right).v1 == v1) { /* prim_right = v2v1 */
	v2 = (tree->prim_right).v0;
	(tree->prim_right).v0 = v1;
	(tree->prim_right).v1 = v2;
      } else if ((tree->prim_right).v1 == v0) { /* prim_right = v2v0 */
	v2 = (tree->prim_right).v0;
      } else {
	printf("Oh no...\n");
      }
    }

  } else { 
    /* Not the root -> the normal of the parent has already been computed */
    if (tree->node_type == 0) /* left_child */ {
      v1 = ((tree->parent)->prim_left).v1;
      v2 = ((tree->parent)->prim_left).v0;
    } else {
      v1 = ((tree->parent)->prim_right).v1;
      v2 = ((tree->parent)->prim_right).v0;
    }
    v0 = find_center(&(raw_model->faces[tree->face_idx]),v1, v2);
    update_child_edges(tree, v0, v1, v2);
  }
  tree->v0 = v0;
  tree->v1 = v1;
  tree->v2 = v2;

  normals[tree->face_idx] = ncrossp(raw_model->vertices[v0],
				    raw_model->vertices[v1], 
				    raw_model->vertices[v2]);
#ifdef NORM_DEBUG
  if (tree->parent != NULL) {
    printf("Parent=%d type=%d n=%f %f %f\n", tree->parent->face_idx, 
	   tree->parent->node_type,
	   normals[tree->parent->face_idx].x, 
	   normals[tree->parent->face_idx].y, 
	   normals[tree->parent->face_idx].z);
    printf("Leaf=%d type = %d n=%f %f %f\n", tree->face_idx, tree->node_type,
	   normals[tree->face_idx].x, 
	   normals[tree->face_idx].y, 
	   normals[tree->face_idx].z);
    printf("test = %f\n", 
	   scalprod(normals[tree->parent->face_idx],normals[tree->face_idx]));
    printf("Parent: v0=%d v1=%d v2=%d\n", tree->parent->v0, tree->parent->v1, 
	   tree->parent->v2);
    printf("Leaf: v0=%d v1=%d v2=%d\n", tree->v0, tree->v1, tree->v2);
    printf("*\n");
  } else {
    printf("root=%d type=%d n=%f %f %f\n", tree->face_idx, tree->node_type,
	   normals[tree->face_idx].x, 
	   normals[tree->face_idx].y, 
	   normals[tree->face_idx].z);
    printf("Root: v0=%d v1=%d v2=%d\n", tree->v0, tree->v1, tree->v2);
    printf("*\n");
  }
#endif


  /* Recurse the face tree */
  if(tree->left != NULL)
    build_normals(raw_model, tree->left, normals);
  if(tree->right != NULL)
    build_normals(raw_model, tree->right, normals);
  
  
  

}

/* Compute consistent normals for each face of the model */
vertex* compute_face_normals(model* raw_model) {
  
  vertex *normals;
  face_tree_ptr *tree, top;
  int i;

  printf("Building spanning tree\n");
  /* Compute spanning tree of the dual graph */

  tree = bfs_build_spanning_tree(raw_model); 
  top = tree[0];
  while (top->parent != NULL)
    top = top->parent;
  printf("Spanning tree done\n");
  /* Finally, compute the normals for each face */
  
  if (manifold_edges(raw_model, tree) == 0) {
    for (i=0; i<raw_model->num_faces; i++)
      free(tree[i]);
    free(tree);
    return NULL;
  }
  printf("The model is manifold ...\n");
  normals = (vertex*)malloc(raw_model->num_faces*sizeof(vertex));
  build_normals(raw_model, top, normals);
  printf("Face normals done\n");
  
  for (i=0; i<raw_model->num_faces; i++) {
#ifdef NORM_DEBUG
    printf("face_normals[%d] = %f %f %f\n", i, normals[i].x, normals[i].y,  
	   normals[i].z); 
#endif
    free(tree[i]);
  }
  free(tree);
  

  return normals;
}


/* Compute a "normal" for each vertex */
void compute_vertex_normal(model* raw_model, info_vertex* curv,  
			   vertex *model_normals) {
  int i,j;
  vertex tmp, p1, p2, p3;

  for(i=0; i<raw_model->num_vert; i++) {
    curv[i].list_face = (int*)malloc(sizeof(int));
    curv[i].num_faces = 0;
  }


  /* For each vertex, build list of faces */
  for (i=0; i<raw_model->num_vert; i++) {
    for(j=0; j<raw_model->num_faces; j++)
      if(raw_model->faces[j].f0 == i ||
	 raw_model->faces[j].f1 == i ||
	 raw_model->faces[j].f2 == i) {
	if(curv[i].num_faces > 0) {
	  curv[i].list_face = (int*)realloc(curv[i].list_face, 
			      (curv[i].num_faces + 1)*sizeof(int));
	  
	}
	curv[i].list_face[curv[i].num_faces] = j;
	curv[i].num_faces ++;
      }

  }




  /* Compute area of each face */
  for (i=0; i<raw_model->num_faces; i++) {
    p1 = raw_model->vertices[raw_model->faces[i].f0];
    p2 = raw_model->vertices[raw_model->faces[i].f1];
    p3 = raw_model->vertices[raw_model->faces[i].f2];
    
    raw_model->area[i] = tri_area(p1, p2, p3);

  }

  /* Alloc array for normal of each vertex */
  raw_model->normals = (vertex*)malloc(raw_model->num_vert*sizeof(vertex));


  for (i=0; i<raw_model->num_vert; i++) {
    tmp.x = 0.0;
    tmp.y = 0.0;
    tmp.z = 0.0;
    for (j=0; j<curv[i].num_faces; j++) {
      tmp.x += model_normals[curv[i].list_face[j]].x * 
	raw_model->area[curv[i].list_face[j]];
      tmp.y += model_normals[curv[i].list_face[j]].y * 
	raw_model->area[curv[i].list_face[j]];
      tmp.z += model_normals[curv[i].list_face[j]].z * 
	raw_model->area[curv[i].list_face[j]];
    }
  
    normalize(&tmp);
    raw_model->normals[i] = tmp;
#ifdef NORM_DEBUG
    printf("%d : %f %f %f\n",i, raw_model->normals[i].x, 
	   raw_model->normals[i].y, raw_model->normals[i].z); 
#endif
    
  }

  
}



















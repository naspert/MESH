/* $Id: normals.c,v 1.5 2001/06/29 14:16:19 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>


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



/* Compares two edges (s.t. they are in lexico. order after qsort) */
int compar(const void* ed0, const void* ed1) {
  edge_sort *e0, *e1;

  e0 = (edge_sort*)ed0;
  e1 = (edge_sort*)ed1;

  if (e0->prim.v0 > e1->prim.v0)
    return 1;
  else if (e0->prim.v0 < e1->prim.v0)
    return -1;
  else {/* e1->v0 = e0->v0 */
    if(e0->prim.v1 > e1->prim.v1)
      return 1;
    else if(e0->prim.v1 < e1->prim.v1)
      return -1;
    else 
      return 0;
  }
}

/* Adds an edge in the dual graph and returns the last elt. in the graph */
void add_edge_dg(struct dual_graph_info *dual_graph, 
		 edge_sort e0, edge_sort e1) {

  int n = dual_graph->num_edges_dual;

  dual_graph->edges = realloc(dual_graph->edges, (n+1)*sizeof(edge_dual));
  dual_graph->edges[n].face0 = e0.face;
  dual_graph->edges[n].face1 = e1.face;
  dual_graph->edges[n].common = e0.prim;

}

/* Returns the number of edges from the dual graph or -1 if a non-manifold */
/* edge is encoutered. The list of faces surrounding each vertex is done */
/* at the same time */
int build_edge_list(model *raw_model, struct dual_graph_info *dual_graph, 
		    info_vertex *curv){
  edge_sort *list;
  int i;
  int v0, v1, v2;
  int nedges=0;

  
  list = (edge_sort*)malloc(3*raw_model->num_faces*sizeof(edge_sort));

  
  for (i=0; i<raw_model->num_faces; i++) {
    v0 = raw_model->faces[i].f0;
    v1 = raw_model->faces[i].f1;
    v2 = raw_model->faces[i].f2;

    /* Update the list of faces surrounding each vertex */
    if(curv[v0].num_faces > 0) 
      curv[v0].list_face = (int*)realloc(curv[v0].list_face, 
					(curv[v0].num_faces + 1)*sizeof(int));
    if(curv[v1].num_faces > 0) 
      curv[v1].list_face = (int*)realloc(curv[v1].list_face, 
					(curv[v1].num_faces + 1)*sizeof(int));
    if(curv[v2].num_faces > 0) 
      curv[v2].list_face = (int*)realloc(curv[v2].list_face, 
					(curv[v2].num_faces + 1)*sizeof(int));
      

    curv[v0].list_face[curv[v0].num_faces] = i;
    curv[v0].num_faces ++;
    curv[v1].list_face[curv[v1].num_faces] = i;
    curv[v1].num_faces ++;
    curv[v2].list_face[curv[v2].num_faces] = i;
    curv[v2].num_faces ++;

    if(v0 > v1) {
      list[nedges].prim.v0 = v1;
      list[nedges].prim.v1 = v0;
    } else {
      list[nedges].prim.v0 = v0;
      list[nedges].prim.v1 = v1;
    }
    list[nedges].face = i;
    nedges++;

    if(v1 > v2) {
      list[nedges].prim.v0 = v2;
      list[nedges].prim.v1 = v1;
    } else {
      list[nedges].prim.v0 = v1;
      list[nedges].prim.v1 = v2;
    }
    list[nedges].face = i;
    nedges++;
    
    if(v2 > v0) {
      list[nedges].prim.v0 = v0;
      list[nedges].prim.v1 = v2;
    } else {
      list[nedges].prim.v0 = v2;
      list[nedges].prim.v1 = v0;
    }
    list[nedges].face = i;
    nedges++;
    
  }
  
  qsort(list, nedges, sizeof(edge_sort), compar);


  
  for (i=0; i<3*raw_model->num_faces-1; i++) {
    if (compar(&(list[i]), &(list[i+1])) == 0) {/*New entry in the dual graph*/
      add_edge_dg(dual_graph, list[i], list[i+1]);
      dual_graph->num_edges_dual++;
      
      if (i<3*raw_model->num_faces-2 && 
	  compar(&(list[i+1]), &(list[i+2]))==0) {
	printf("Non-manifold edge %d %d\n", list[i].prim.v0, list[i].prim.v1);
	return -1;
      }
    }
  }
      
  free(list);        

#ifdef NORM_DEBUG
  printf("[build_edge_list]:%d edges in dual graph\n", 
	 dual_graph->num_edges_dual);
#endif

  return dual_graph->num_edges_dual;

}

/* Remove dual edges containing 'cur_face' from 'dual_graph' and put'em */
/* into 'bot' and update 'ne_dual'*/
/* we assume that 'bot' points on the last non-NULL elt of the and we return */
/* the last elt. of this list afterwards */
edge_list_ptr find_dual_edges(int cur_face,  int*nfound, 
			      struct dual_graph_info *dual_graph, 
			      edge_list_ptr bot) {


  int i, tmp_dual=dual_graph->num_edges_dual;
  int test=0;
  
  for (i=0; i<dual_graph->num_edges_dual; i++) {

    if(dual_graph->done[i])
      continue;

    if (dual_graph->edges[i].face0 == cur_face) {

      bot->edge = dual_graph->edges[i];
      bot->next = (edge_list_ptr)malloc(sizeof(edge_list));
      bot = bot->next;
      bot->next = NULL;

      dual_graph->done[i] = 1;
      tmp_dual--;
      (*nfound)++;
      if (++test==3)  
      /* the model is manifold -> each face has at most 3 neighbours */
	return bot;
      continue;
    } else if (dual_graph->edges[i].face1 == cur_face) {
      bot->edge.face1 = dual_graph->edges[i].face0;
      bot->edge.face0 = dual_graph->edges[i].face1;
      bot->edge.common = dual_graph->edges[i].common;
      bot->next = (edge_list_ptr)malloc(sizeof(edge_list));
      bot = bot->next;
      bot->next = NULL;

      dual_graph->done[i] = 1;
      tmp_dual--;
      (*nfound)++;
      if (++test==3)  
      /* the model is manifold -> each face has at most 3 neighbours */
	return bot;
    }

  }


  return bot;
}


/* Builds the spanning tree of the dual graph */
face_tree_ptr* bfs_build_spanning_tree(model *raw_model, info_vertex *curv) {
  int faces_traversed = 0;

  edge_list_ptr  cur_list, old;
  edge_list_ptr  *list, bot;
  struct dual_graph_info *dual_graph;

  
  face_tree_ptr *tree, cur_node, new_node, top;
  int list_size = 0, i;
  int ne_dual = 0;

  printf("Dual graph build ");
  dual_graph = (struct dual_graph_info*)malloc(sizeof(struct dual_graph_info));
  dual_graph->edges = NULL;
  dual_graph->num_edges_dual = 0;
  ne_dual = build_edge_list(raw_model, dual_graph, curv);
  dual_graph->done = (int*)calloc(dual_graph->num_edges_dual, sizeof(int));
  printf("done\n");
  if (ne_dual == -1) {
    printf("No edges in dual graph ??\n");
    free(dual_graph);
    return NULL;
  }

#ifdef NORM_DEBUG
  for (i=0; i<dual_graph->num_edges_dual; i++) 
    printf("[bfs_build_spanning_tree]: %d dual %d %d primal %d %d\n", 
	   dual_graph->num_edges_dual, dual_graph->edges[i].face0, 
	   dual_graph->edges[i].face1, dual_graph->edges[i].common.v0, 
	   dual_graph->edges[i].common.v1);
  
#endif

  list = (edge_list_ptr*)malloc(sizeof(edge_list_ptr)); 
  *list = (edge_list_ptr)malloc(sizeof(edge_list));
  (*list)->next = NULL;
  (*list)->prev = NULL;
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

  cur_node = tree[0]; /* Initialized to the root of the tree */
  top = tree[0];
  bot = *list; 
  (*list)->next = NULL;
  /* Build 1st node */
  
  bot = find_dual_edges(0, &list_size, dual_graph, bot);   

  cur_list = *list;
  faces_traversed ++;
 
  while(list_size > 0) {
    if (tree[cur_list->edge.face1]->visited == 0) { /* if 1 we have a cycle */
      /* Add the face to the tree */
      cur_node = tree[cur_list->edge.face0];
      if (cur_node->left == NULL) {
	cur_node->left = tree[cur_list->edge.face1];
	cur_node->prim_left = cur_list->edge.common;
#ifdef NORM_DEBUG
	printf("[bfs_build_spanning_tree]: face0=%d face1=%d left %d %d\n", 
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
	printf("[bfs_build_spanning_tree]: face0=%d face1=%d right %d %d\n", 
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
	printf("[bfs_build_spanning_tree]: face0=%d face1=%d up %d %d\n", 
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
	return NULL;
      }
      /* Add adjacent edges to the list */


      bot = find_dual_edges(old->edge.face1, &list_size, 
			    dual_graph, bot);
      

      free(old);
    } else { /* discard this face from the list */
      old = cur_list;

#ifdef NORM_DEBUG
       printf("[bfs_build_spanning_tree]: Removing %d %d\n", 
	      cur_list->edge.face0, cur_list->edge.face1);
#endif

      cur_list = cur_list->next;
      list_size--;
      free(old);
    }
  }

#ifdef NORM_DEBUG
  printf("[bfs_build_spanning_tree]:faces_traversed = %d num_faces = %d\n", 
	 faces_traversed, raw_model->num_faces); 
#endif
  free(bot->next); /* should be alloc'd but nothing inside */
  free(bot);
  free(dual_graph->edges);
  free(dual_graph->done);
  free(dual_graph);
  free(list);
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
    printf("Leaf=%d type=%d n=%f %f %f\n", tree->face_idx, tree->node_type,
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
vertex* compute_face_normals(model* raw_model, info_vertex *curv) {
  
  vertex *normals;
  face_tree_ptr *tree, top;
  int i;


  for(i=0; i<raw_model->num_vert; i++) {
    curv[i].list_face = (int*)malloc(sizeof(int));
    curv[i].num_faces = 0;
  }
  printf("Building spanning tree\n");
  /* Compute spanning tree of the dual graph */


  tree = bfs_build_spanning_tree(raw_model, curv); 
  if (tree == NULL)
    return NULL;
  top = tree[0];
  while (top->parent != NULL)
    top = top->parent;
  printf("Spanning tree done\n");
  /* Finally, compute the normals for each face */
  
  printf("The model is manifold ...\n");
  normals = (vertex*)malloc(raw_model->num_faces*sizeof(vertex));
  build_normals(raw_model, top, normals);
  printf("Face normals done %d\n", raw_model->num_faces);
  
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



















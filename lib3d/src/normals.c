/* $Id: normals.c,v 1.30 2002/05/13 13:50:47 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>
#include <normals.h>


/* Compares two edges (s.t. they are in lexico. order after qsort) */
int compar(const void* ed0, const void* ed1) {
  int tmp;

  if ((tmp=((struct edge_sort*)ed0)->prim.v0-
       ((struct edge_sort*)ed1)->prim.v0))
    return tmp;
  else 
    return  (((struct edge_sort*)ed0)->prim.v1 - 
      ((struct edge_sort*)ed1)->prim.v1);
}

/* Adds an edge in the dual graph and returns the last elt. in the graph */
void add_edge_dg(struct dual_graph_info *dual_graph, 
		 const struct edge_sort* e0, const struct edge_sort* e1) {

  int n = dual_graph->num_edges_dual;

  dual_graph->edges = (struct edge_dual*)
    realloc(dual_graph->edges, (n+1)*sizeof(struct edge_dual));

  dual_graph->edges[n].face0 = e0->face;
  dual_graph->edges[n].face1 = e1->face;
  dual_graph->edges[n].common = e0->prim;

}

/* Returns the number of edges from the dual graph or -1 if a non-manifold */
/* edge is encoutered. The list of faces surrounding each vertex is done */
/* at the same time */
int build_edge_list(const struct model *raw_model, 
		    struct dual_graph_info *dual_graph, 
		    struct info_vertex *curv, 
		    struct dual_graph_index **dg_idx){
  struct edge_sort *list;
  int i, f;
  int v0, v1, v2;
  int nedges=0;

  
  list = (struct edge_sort*)
    malloc(3*raw_model->num_faces*sizeof(struct edge_sort));

  
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
      

    curv[v0].list_face[curv[v0].num_faces++] = i;
    curv[v1].list_face[curv[v1].num_faces++] = i;
    curv[v2].list_face[curv[v2].num_faces++] = i;

    if(v0 > v1) {
      list[nedges].prim.v0 = v1;
      list[nedges].prim.v1 = v0;
    } else {
      list[nedges].prim.v0 = v0;
      list[nedges].prim.v1 = v1;
    }
    list[nedges++].face = i;


    if(v1 > v2) {
      list[nedges].prim.v0 = v2;
      list[nedges].prim.v1 = v1;
    } else {
      list[nedges].prim.v0 = v1;
      list[nedges].prim.v1 = v2;
    }
    list[nedges++].face = i;

    
    if(v2 > v0) {
      list[nedges].prim.v0 = v0;
      list[nedges].prim.v1 = v2;
    } else {
      list[nedges].prim.v0 = v2;
      list[nedges].prim.v1 = v0;
    }
    list[nedges++].face = i;
    
  }
  
  qsort(list, nedges, sizeof(struct edge_sort), compar);

  for (i=0; i<3*raw_model->num_faces-1; i++) {
    if (!compar(&(list[i]), &(list[i+1]))) {/*New entry in the dual graph*/
      add_edge_dg(dual_graph, &(list[i]), &(list[i+1]));

      /* update the index */
      f = list[i].face;
      (*dg_idx)[f].ring[(*dg_idx)[f].face_info++] = dual_graph->num_edges_dual;


      f = list[i+1].face;      
      (*dg_idx)[f].ring[(*dg_idx)[f].face_info++] = 
        dual_graph->num_edges_dual++;

      /* test for non-manifoldness */
      if (i<3*raw_model->num_faces-2 && 
	  !compar(&(list[i+1]), &(list[i+2]))) {
	printf("Non-manifold edge %d %d\n", list[i].prim.v0, list[i].prim.v1);
	free(list);        
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
struct edge_list* find_dual_edges(const int cur_face,  int *nfound, 
				  struct dual_graph_info *dual_graph, 
				  struct edge_list *bot, 
				  const struct dual_graph_index *dg_index) {


  int i, id;
  
  

  for (i=0; i<dg_index[cur_face].face_info; i++) {
    id = dg_index[cur_face].ring[i];
    if (!BITMAP_TEST_BIT(dual_graph->done, id)) { 
/* if this edge has not been visited */
      /* add it to the list */
      if (dual_graph->edges[id].face0 == cur_face) 
	bot->edge = dual_graph->edges[id];
      else  {
	bot->edge.face1 = dual_graph->edges[id].face0;
	bot->edge.face0 = dual_graph->edges[id].face1;
	bot->edge.common = dual_graph->edges[id].common;
      } 
      bot->next = (struct edge_list*)malloc(sizeof(struct edge_list));
      bot = bot->next;
      bot->next = NULL;
      BITMAP_SET_BIT(dual_graph->done, id);
      (*nfound)++;
    }

  }
  return bot;

}


/* Builds the spanning tree of the dual graph */
struct face_tree** bfs_build_spanning_tree(const struct model *raw_model, 
					   struct info_vertex *curv) {
  int faces_traversed=0;
  int list_size=0, i;
  int ne_dual=0;
  struct edge_list  *cur_list, *old;
  struct edge_list  **list, *bot;
  struct dual_graph_info *dual_graph;
  struct dual_graph_index *dg_idx; 
  struct face_tree **tree, *cur_node, *new_node, *top;


  printf("Dual graph build.... ");fflush(stdout);

  dual_graph = (struct dual_graph_info*)malloc(sizeof(struct dual_graph_info));
  dual_graph->edges = NULL;
  dual_graph->num_edges_dual = 0;

  dg_idx = (struct dual_graph_index*)malloc(raw_model->num_faces*
					    sizeof(struct dual_graph_index));
  memset(dg_idx, 0, 
	 raw_model->num_faces*sizeof(struct dual_graph_index));


  ne_dual = build_edge_list(raw_model, dual_graph, curv, &dg_idx);
  dual_graph->done = (bitmap_t*)calloc(
    (dual_graph->num_edges_dual+BITMAP_T_BITS-1)/BITMAP_T_BITS, BITMAP_T_SZ);
  printf("done\n");
  if (ne_dual == -1) {
    printf("No edges in dual graph ??\n");
    free(dual_graph);
    free(dg_idx);
    return NULL;
  }

#ifdef NORM_DEBUG_BFS
  for (i=0; i<dual_graph->num_edges_dual; i++) 
    printf("[bfs_build_spanning_tree]: %d dual %d %d primal %d %d\n", 
	   dual_graph->num_edges_dual, dual_graph->edges[i].face0, 
	   dual_graph->edges[i].face1, dual_graph->edges[i].common.v0, 
	   dual_graph->edges[i].common.v1);
#endif

  printf("Building spanning tree...");fflush(stdout);
  list = (struct edge_list**)malloc(sizeof(struct edge_list*)); 
  *list = (struct edge_list*)malloc(sizeof(struct edge_list));
  (*list)->next = NULL;
  (*list)->prev = NULL;
  tree = (struct face_tree**)
    malloc(raw_model->num_faces*sizeof(struct face_tree*));
  /* Initialize all cells */
  for (i=0; i<raw_model->num_faces; i++) {
    tree[i] = (struct face_tree*)malloc(sizeof(struct face_tree));
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
  
  bot = find_dual_edges(0, &list_size, dual_graph, bot, dg_idx);   

  cur_list = *list;
  faces_traversed ++;
 
  while(list_size > 0) {
    if (tree[cur_list->edge.face1]->visited == 0) { /* if 1 we have a cycle */
      /* Add the face to the tree */
      cur_node = tree[cur_list->edge.face0];
      if (cur_node->left == NULL) {
	cur_node->left = tree[cur_list->edge.face1];
	cur_node->prim_left = cur_list->edge.common;

#ifdef NORM_DEBUG_BFS
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

#ifdef NORM_DEBUG_BFS
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

#ifdef NORM_DEBUG_BFS
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
			    dual_graph, bot, dg_idx);
      

      free(old);
    } else { /* discard this face from the list */
      old = cur_list;

#ifdef NORM_DEBUG_BFS
       printf("[bfs_build_spanning_tree]: Removing %d %d\n", 
	      cur_list->edge.face0, cur_list->edge.face1);
#endif

      cur_list = cur_list->next;
      list_size--;
      free(old);
    }
  }

#ifdef NORM_DEBUG_BFS
  printf("[bfs_build_spanning_tree]:faces_traversed = %d num_faces = %d\n", 
	 faces_traversed, raw_model->num_faces); 
#endif

  free(bot->next);  /* should be alloc'd but nothing inside */
  free(bot);
  free(dual_graph->edges);
  free(dual_graph->done);
  free(dual_graph);
  free(list);
  free(dg_idx);
  printf(" done\n");
  return tree;
}



int find_center(const face_t *cur,const int v1, const int v2) {
  if (cur->f0==v1) {
    if (cur->f1==v2)
      return cur->f2;
    else
      return cur->f1;
  } else if (cur->f1==v1) {
    if (cur->f0==v2)
      return cur->f2;
    else
      return cur->f0;
  } else {
    if (cur->f1==v2)
      return cur->f0;
    else
      return cur->f1;
  }
  return -1;
}



void update_child_edges(struct face_tree *tree, const int v0, 
                        const int v1, const int v2) {
  int __tmp;

#ifndef __swap_vert
#define __swap_vert(p)                          \
  do {                                          \
    __tmp = (p).v1;                             \
    (p).v1 = (p).v0;                            \
    (p).v0 = __tmp;                             \
  } while(0)
#endif

  if (tree->left != NULL) {/* update prim_left */
      if ((tree->prim_left).v0 == v1 && (tree->prim_left).v1 == v0)  
	/* prim_left = v1v0 */
	__swap_vert(tree->prim_left);
      else if ((tree->prim_left).v0 == v0 && (tree->prim_left).v1 == v2) 
	/* prim_left = v0v2 */
	__swap_vert(tree->prim_left);
      else if ((tree->prim_left).v0 == v2 && (tree->prim_left).v1 == v1)
	/* prim_left = v2v1 */
	__swap_vert(tree->prim_left);    
  }

  if (tree->right != NULL) {/* update prim_right */
    if ((tree->prim_right).v0 == v1 && (tree->prim_right).v1 == v0)  
      /* prim_right = v1v0 */
      __swap_vert(tree->prim_right);
    else if ((tree->prim_right).v0 == v0 && (tree->prim_right).v1 == v2) 
      /* prim_right = v0v2 */
      __swap_vert(tree->prim_right);
    else if ((tree->prim_right).v0 == v2 && (tree->prim_right).v1 == v1)
      /* prim_right = v2v1 */
      __swap_vert(tree->prim_right);
  }
#undef __swap_vert
}




void build_normals(const struct model *raw_model, struct face_tree *tree, 
		   vertex_t* normals) {

  int v0=-1, v1=-1, v2=-1;


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

  ncrossp_v(&raw_model->vertices[v0], &raw_model->vertices[v1], 
	    &raw_model->vertices[v2], &normals[tree->face_idx]);
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
vertex_t* compute_face_normals(const struct model* raw_model, 
			       struct info_vertex *curv) {
  
  vertex_t *normals;
  struct face_tree **tree, *top;
  int i;


  for(i=0; i<raw_model->num_vert; i++) {
    curv[i].list_face = (int*)malloc(sizeof(int));
    curv[i].num_faces = 0;
  }

  /* Compute spanning tree of the dual graph */


  tree = bfs_build_spanning_tree(raw_model, curv); 
  if (tree == NULL)
    return NULL;
  top = tree[0];
  while (top->parent != NULL)
    top = top->parent;
 
  /* Finally, compute the normals for each face */
  
  printf("The model is manifold.\n");
  normals = (vertex_t*)malloc(raw_model->num_faces*sizeof(vertex_t));
  build_normals(raw_model, top, normals);
  printf("Face normals done !\n");
  
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
void compute_vertex_normal(struct model* raw_model, 
                           const struct info_vertex* curv,  
			   const vertex_t *model_normals) {
  int i,j;
  vertex_t tmp, *p1, *p2, *p3;

  raw_model->total_area = 0.0;

  /* Compute area of each face */
  for (i=0; i<raw_model->num_faces; i++) {
    p1 = &(raw_model->vertices[raw_model->faces[i].f0]);
    p2 = &(raw_model->vertices[raw_model->faces[i].f1]);
    p3 = &(raw_model->vertices[raw_model->faces[i].f2]);
    
    raw_model->area[i] = tri_area_v(p1, p2, p3);
    raw_model->total_area += raw_model->area[i];
  }

  /* Alloc array for normal of each vertex */
  raw_model->normals = (vertex_t*)malloc(raw_model->num_vert*sizeof(vertex_t));
  

  for (i=0; i<raw_model->num_vert; i++) {
    tmp.x = 0.0;
    tmp.y = 0.0;
    tmp.z = 0.0;
    for (j=0; j<curv[i].num_faces; j++) 
      __add_prod_v(raw_model->area[curv[i].list_face[j]], 
                   model_normals[curv[i].list_face[j]], tmp, tmp);
    
    

    __normalize_v(tmp);
    raw_model->normals[i] = tmp;
#ifdef NORM_DEBUG
    printf("%d : %f %f %f\n",i, raw_model->normals[i].x, 
	   raw_model->normals[i].y, raw_model->normals[i].z); 
#endif
    
  }

  
}



















/* $Id: ring.c,v 1.11 2003/06/17 14:44:48 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2003 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne)
 *  You are not allowed to redistribute this program without the explicit
 *  permission of the author.
 *
 *  Author : Nicolas Aspert
 *
 *  Contact : 
 *     Nicolas Aspert
 *     Signal Processing Institute (ITS)
 *     Swiss Federal Institute of Technology (EPFL)
 *     1015 Lausanne
 *     Switzerland
 *
 *     Tel : +41 21 693 3632
 *     E-Mail : Nicolas.Aspert@epfl.ch
 *
 *
 */


#include <3dmodel.h>
#include <ring.h>

#if defined(DEBUG) || defined(RING_DEBUG) || defined(NORM_DEBUG)
# include <debug_print.h>
#endif

#ifndef REMOVE_ELT_FROM_LIST
#define REMOVE_ELT_FROM_LIST(elt)               \
do {                                            \
  struct edge_v *__tmp;                         \
  __tmp = elt->prev;                            \
  elt = elt->next;                              \
  if (elt->prev != NULL)                        \
    free(elt->prev);                            \
                                                \
  elt->prev = __tmp;                            \
  if (__tmp != NULL)                            \
    __tmp->next = elt;                          \
} while(0)
#endif

#ifndef ALLOC_NEXT_SLOT
#define ALLOC_NEXT_SLOT(list_tail, id)                                      \
do {                                                                        \
    list_tail[id]->next = (struct edge_v*)calloc(1, sizeof(struct edge_v)); \
    list_tail[id]->next->prev = list_tail[id];                              \
    list_tail[id] = list_tail[id]->next;                                    \
} while(0)
#endif

void build_star_global(const struct model *raw_model, 
                       struct ring_info *ring) {
  int i, l;
  int *num_edges=NULL; /* number of edges in the 1-ring */
  struct edge_v **edge_list_primal, **cur_list_tail, *list_tail=NULL, *tmp;
  int *final_star;
  int *face_star;
  int vid;
  int star_size, n_faces, edge_added;
  
  edge_list_primal = (struct edge_v**)
    malloc(raw_model->num_vert*sizeof(struct edge_v*));
  cur_list_tail = (struct edge_v**)
    malloc(raw_model->num_vert*sizeof(struct edge_v*));
  for (i=0; i<raw_model->num_vert; i++) {
    edge_list_primal[i] = (struct edge_v*)calloc(1, sizeof(struct edge_v));
    cur_list_tail[i] = edge_list_primal[i];
  }
  memset(ring, 0, raw_model->num_vert*sizeof(struct ring_info));

  num_edges = (int*)calloc(raw_model->num_vert, sizeof(int));
  
  
  /* List all the edges in the 1-rings */
  for (i=0; i<raw_model->num_faces; i++) {

    vid = raw_model->faces[i].f0; /* current vertex */
    num_edges[vid]++;
    cur_list_tail[vid]->v0 = raw_model->faces[i].f1;
    cur_list_tail[vid]->v1 = raw_model->faces[i].f2;
    cur_list_tail[vid]->face = i;
    ALLOC_NEXT_SLOT(cur_list_tail, vid);

    
    vid = raw_model->faces[i].f1;
    num_edges[vid]++;
    cur_list_tail[vid]->v0 = raw_model->faces[i].f0;
    cur_list_tail[vid]->v1 = raw_model->faces[i].f2;
    cur_list_tail[vid]->face = i;
    ALLOC_NEXT_SLOT(cur_list_tail, vid);
    
    vid = raw_model->faces[i].f2;
    num_edges[vid]++;
    cur_list_tail[vid]->v0 = raw_model->faces[i].f0;
    cur_list_tail[vid]->v1 = raw_model->faces[i].f1;
    cur_list_tail[vid]->face = i;
    ALLOC_NEXT_SLOT(cur_list_tail, vid);
  }
  free(cur_list_tail);

  for (i=0; i<raw_model->num_vert; i++) {
    if(num_edges[i] == 0) {
      ring[i].type = -1; 
      list_tail = edge_list_primal[i];
#ifdef DEBUG
      DEBUG_PRINT("Vertex %d has no face...\n", i);
#endif
      goto singularity_encountered; 
    }

    /* worst case allocation */
    final_star = (int*)malloc(2*num_edges[i]*sizeof(int));
    face_star = (int*)malloc(2*num_edges[i]*sizeof(int));
    
    /* Put 1st elts in the star */
    final_star[0] = edge_list_primal[i]->v0;
    final_star[1] = edge_list_primal[i]->v1;
    face_star[0] = edge_list_primal[i]->face;

    /* de-queue 1st elt */
    tmp = edge_list_primal[i];
    edge_list_primal[i] = edge_list_primal[i]->next;
    edge_list_primal[i]->prev=NULL;
    free(tmp);
    list_tail = edge_list_primal[i];

    star_size = 2;
    n_faces = 1;

    l = 1;

    
    while (l < num_edges[i]) {
      edge_added = 0;
      while (list_tail->prev != NULL)
        list_tail = list_tail->prev;

      while (list_tail != NULL && edge_added == 0) {
        if (list_tail->v0 == final_star[0]) {
          /* add v1 on top */
          memmove(&(final_star[1]), final_star, star_size*sizeof(int));
          memmove(&(face_star[1]), face_star, n_faces*sizeof(int));
          
          final_star[0] = list_tail->v1;
          face_star[0] = list_tail->face;
	

          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          REMOVE_ELT_FROM_LIST(list_tail);

        }
        else if (list_tail->v1 == final_star[0]) {
          /* add v0 on top */
          memmove(&(final_star[1]), final_star, star_size*sizeof(int));
          memmove(&(face_star[1]), face_star, n_faces*sizeof(int));
       
          final_star[0] = list_tail->v0;
          face_star[0] = list_tail->face;

          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          REMOVE_ELT_FROM_LIST(list_tail);


        }
        else if (list_tail->v0 == final_star[star_size-1]) {
          /* add v1 on bottom */
          final_star[star_size] = list_tail->v1;
          face_star[n_faces] = list_tail->face;

          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          REMOVE_ELT_FROM_LIST(list_tail);

        }
        else if (list_tail->v1 == final_star[star_size-1]) {
          /* add v0 on bottom */
          final_star[star_size] = list_tail->v0;
          face_star[n_faces] = list_tail->face;

          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          REMOVE_ELT_FROM_LIST(list_tail);

        } else
          list_tail = list_tail->next;
      }

      if (edge_added == 0) {
        printf("Vertex %d is non-manifold\n", i);
      /*   free(done); */
        free(final_star);
        free(face_star);
        ring[i].type = 2;
        /* 
         *  What follows is implicitely done by memset-ing the
         *  structure to 0 :
         *
         *  ring[i].size = 0;
         *  ring[i].n_faces = 0;
         *  ring[i].ord_face = NULL;
         *  ring[i].ord_vert = NULL;
         *
         */
        goto singularity_encountered;
      }
    }

    if (final_star[0] == final_star[star_size-1]) {    /* Regular vertex */
      star_size--;
      ring[i].type = 0;
    } else     /* Boundary vertex */
      ring[i].type = 1;

    ring[i].size = star_size;
    ring[i].ord_vert = (int*)malloc(star_size*sizeof(int));
    memcpy(ring[i].ord_vert, final_star, star_size*sizeof(int));
#ifdef DEBUG
    DEBUG_PRINT("vertex %d: valence=%d\n", i, star_size);
#endif
    ring[i].n_faces = n_faces;
    ring[i].ord_face = (int*)malloc(n_faces*sizeof(int));
    memcpy(ring[i].ord_face, face_star, n_faces*sizeof(int));

#ifdef RING_DEBUG
    DEBUG_PRINT("vertex %d Tr: ", i);
    for (j=0; j<ring[i].n_faces; j++) {
      printf("%d ", ring[i].ord_face[j]);
    }
    printf("\n");
#endif

    free(final_star);
    free(face_star);

  /* Make sure everything is correctly freed */
  singularity_encountered:
    while (list_tail != NULL) {
      tmp = list_tail;
      list_tail = list_tail->next;
      free(tmp);
    }
  }
  free(edge_list_primal);
  free(num_edges);
}

/* find the 1-ring of vertex v */
void build_star(const struct model *raw_model, int v, struct ring_info *ring) {

  int i, j;
  int num_edges=0; /* number of edges in the 1-ring */
  struct edge_v *edge_list_primal=NULL;
  int *final_star;
  int *face_star;
  unsigned char *done;
  int star_size;
  int n_faces;
  int edge_added;


  /* list all edges in the 1-ring */
  for (i=0; i<raw_model->num_faces; i++) {
    if (raw_model->faces[i].f0 == v) {
      num_edges ++;
      edge_list_primal = (struct edge_v*)
	realloc(edge_list_primal, num_edges*sizeof(struct edge_v));
      
#ifdef NORM_DEBUG
      if (edge_list_primal == NULL) {
	DEBUG_PRINT("realloc failed %d\n", i);
	exit(-1);
      }
#endif

      edge_list_primal[num_edges-1].v0 = raw_model->faces[i].f1;
      edge_list_primal[num_edges-1].v1 = raw_model->faces[i].f2;
      edge_list_primal[num_edges-1].face = i;
    } else if (raw_model->faces[i].f1 == v) {
      num_edges ++;
      edge_list_primal = (struct edge_v*)
	realloc(edge_list_primal, num_edges*sizeof(struct edge_v));

#ifdef NORM_DEBUG
      if (edge_list_primal == NULL) {
	DEBUG_PRINT("realloc failed %d\n", i);
	exit(-1);
      }
#endif

      edge_list_primal[num_edges-1].v0 = raw_model->faces[i].f0;
      edge_list_primal[num_edges-1].v1 = raw_model->faces[i].f2;      
      edge_list_primal[num_edges-1].face = i;

    }  else if (raw_model->faces[i].f2 == v) {
      num_edges ++;
      edge_list_primal = (struct edge_v*)
	realloc(edge_list_primal, num_edges*sizeof(struct edge_v));


      edge_list_primal[num_edges-1].v0 = raw_model->faces[i].f0;
      edge_list_primal[num_edges-1].v1 = raw_model->faces[i].f1;      
      edge_list_primal[num_edges-1].face = i;
    }

  }

  if (num_edges == 0) {
#ifdef DEBUG
    DEBUG_PRINT("Vertex %d has no face...\n", v);
#endif
    ring->type = 0;
    ring->size = 0;
    ring->ord_vert = NULL;
    return;
  }

  done = (unsigned char*)calloc(num_edges, sizeof(unsigned char));
  /* worst case allocation */
  final_star = (int*)malloc(2*num_edges*sizeof(int));
  face_star = (int*)malloc(2*num_edges*sizeof(int));

  /* Put 1st two elts in the star */
  final_star[0] = edge_list_primal[0].v0;
  final_star[1] = edge_list_primal[0].v1;
  face_star[0] = edge_list_primal[0].face;

  star_size = 2;
  n_faces = 1;
  done[0] = 1;
  i = 1;


  /* build the 1-ring */
  while (i < num_edges) {
    edge_added = 0;
    for(j=0; j<num_edges; j++) {
      if (done[j])
	continue;
      if (edge_list_primal[j].v0 == final_star[0]) {
	/* add v1 on top */
        memmove(&(final_star[1]), final_star, star_size*sizeof(int));
        memmove(&(face_star[1]), face_star, n_faces*sizeof(int));
	
	final_star[0] = edge_list_primal[j].v1;
	face_star[0] = edge_list_primal[j].face;
	
	done[j] = 1;
	i++;
	star_size++;
	n_faces++;
	edge_added = 1;
	break;
      }
      else if (edge_list_primal[j].v1 == final_star[0]) {
	/* add v0 on top */
        memmove(&(final_star[1]), final_star, star_size*sizeof(int));
        memmove(&(face_star[1]), face_star, n_faces*sizeof(int));
       
	final_star[0] = edge_list_primal[j].v0;
	face_star[0] = edge_list_primal[j].face;
	done[j] = 1;
	i++;
	star_size++;
	n_faces++;
	edge_added = 1;
	break;
      }
      else if (edge_list_primal[j].v0 == final_star[star_size-1]) {
	/* add v1 on bottom */
	final_star[star_size] = edge_list_primal[j].v1;
	face_star[n_faces] = edge_list_primal[j].face;
	done[j] = 1;
	i++;
	star_size++;
	n_faces++;
	edge_added = 1;
	break;
      }
      else if (edge_list_primal[j].v1 == final_star[star_size-1]) {
	/* add v0 on bottom */
	final_star[star_size] = edge_list_primal[j].v0;
	face_star[n_faces] = edge_list_primal[j].face;
	done[j] = 1;
	i++;
	star_size++;
	n_faces++;
	edge_added = 1;
	break;
      }
    }
    if (edge_added == 0) {
      printf("Vertex %d is non-manifold\n", v);
      free(done);
      free(final_star);
      free(face_star);
      ring->type = 2;
      ring->size = 0;
      ring->n_faces = 0;
      ring->ord_face = NULL;
      ring->ord_vert = NULL;
      return;
    }
  }

  if (final_star[0] == final_star[star_size-1]) {    /* Regular vertex */
    star_size--;
    ring->type = 0;
  } else     /* Boundary vertex */
    ring->type = 1;


  ring->size = star_size;
  ring->ord_vert = (int*)malloc(star_size*sizeof(int));
  memcpy(ring->ord_vert, final_star, star_size*sizeof(int));
  ring->n_faces = n_faces;
  ring->ord_face = (int*)malloc(n_faces*sizeof(int));
  memcpy(ring->ord_face, face_star, n_faces*sizeof(int));

#ifdef RING_DEBUG
  DEUBG_PRINT("Vertex %d : star_size=%d num_edges=%d n_faces=%d\n", v, 
              star_size, num_edges, n_faces);
  for (i=0; i<star_size; i++)
    printf("vertex %d \n", final_star[i]);
  for (i=0; i<n_faces; i++)
    printf("face %d\n", face_star[i]);
#endif

  free(edge_list_primal);
  free(final_star);
  free(face_star);
  free(done);

}


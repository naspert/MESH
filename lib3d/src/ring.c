/* $Id: ring.c,v 1.2 2002/05/13 14:47:41 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
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

void build_star_global(const struct model *raw_model, 
                       struct ring_info *ring) {
  int i, j, k, l;
  int *num_edges=NULL; /* number of edges in the 1-ring */
  struct edge_v **edge_list_primal=NULL;
  int *final_star;
  int *face_star;
  int vid; 
  unsigned char *done;
  int star_size, n_faces, edge_added;
  
  edge_list_primal = (struct edge_v**)
    calloc(raw_model->num_vert, sizeof(struct edge_v*));

  memset(ring, 0, raw_model->num_vert*sizeof(struct ring_info));

  num_edges = (int*)calloc(raw_model->num_vert, sizeof(int));
  
  
  /* List all the edges in the 1-rings */
  for (i=0; i<raw_model->num_faces; i++) {

    vid = raw_model->faces[i].f0; /* current vertex */
    num_edges[vid]++;
    edge_list_primal[vid] = (struct edge_v*)
      realloc(edge_list_primal[vid], num_edges[vid]*sizeof(struct edge_v));
    edge_list_primal[vid][num_edges[vid]-1].v0 = raw_model->faces[i].f1;
    edge_list_primal[vid][num_edges[vid]-1].v1 = raw_model->faces[i].f2;
    edge_list_primal[vid][num_edges[vid]-1].face = i;
    
    vid = raw_model->faces[i].f1;
    num_edges[vid]++;
    edge_list_primal[vid] = (struct edge_v*)
      realloc(edge_list_primal[vid], num_edges[vid]*sizeof(struct edge_v));
    edge_list_primal[vid][num_edges[vid]-1].v0 = raw_model->faces[i].f0;
    edge_list_primal[vid][num_edges[vid]-1].v1 = raw_model->faces[i].f2;
    edge_list_primal[vid][num_edges[vid]-1].face = i;
    
    vid = raw_model->faces[i].f2;
    num_edges[vid]++;
    edge_list_primal[vid] = (struct edge_v*)
      realloc(edge_list_primal[vid], num_edges[vid]*sizeof(struct edge_v));
    edge_list_primal[vid][num_edges[vid]-1].v0 = raw_model->faces[i].f0;
    edge_list_primal[vid][num_edges[vid]-1].v1 = raw_model->faces[i].f1;
    edge_list_primal[vid][num_edges[vid]-1].face = i;
  }
  
  for (i=0; i<raw_model->num_vert; i++) {
    if(num_edges[i] == 0) {
      fprintf(stderr, "Vertex %d has no face...\n", i);
      continue;
    }
    /* build ring of each vertex */
    done = (unsigned char*)calloc(num_edges[i], 
                                  sizeof(unsigned char));
    /* worst case allocation */
    final_star = (int*)malloc(2*num_edges[i]*sizeof(int));
    face_star = (int*)malloc(2*num_edges[i]*sizeof(int));
    
    /* Put 1st two elts in the star */
    final_star[0] = edge_list_primal[i][0].v0;
    final_star[1] = edge_list_primal[i][0].v1;
    face_star[0] = edge_list_primal[i][0].face;
    
    star_size = 2;
    n_faces = 1;
    done[0] = 1;
    l = 1;
    
    while (l < num_edges[i]) {
      edge_added = 0;
      for (j=0; j<num_edges[i]; j++) {
        if (done[j])
          continue;
        if (edge_list_primal[i][j].v0 == final_star[0]) {
          /* add v1 on top */
          for (k=star_size-1; k>=0; k--) 
            final_star[k+1] = final_star[k];
          for (k=n_faces-1; k>=0; k--)
            face_star[k+1] = face_star[k];
          
          
          final_star[0] = edge_list_primal[i][j].v1;
          face_star[0] = edge_list_primal[i][j].face;
	
          done[j] = 1;
          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          break;
        }
        else if (edge_list_primal[i][j].v1 == final_star[0]) {
          /* add v0 on top */
          for (k=star_size-1; k>=0; k--) 
            final_star[k+1] = final_star[k];
          for (k=n_faces-1; k>=0; k--)
            face_star[k+1] = face_star[k];
       
          final_star[0] = edge_list_primal[i][j].v0;
          face_star[0] = edge_list_primal[i][j].face;
          done[j] = 1;
          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          break;
        }
        else if (edge_list_primal[i][j].v0 == final_star[star_size-1]) {
          /* add v1 on bottom */
          final_star[star_size] = edge_list_primal[i][j].v1;
          face_star[n_faces] = edge_list_primal[i][j].face;
          done[j] = 1;
          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          break;
        }
        else if (edge_list_primal[i][j].v1 == final_star[star_size-1]) {
          /* add v0 on bottom */
          final_star[star_size] = edge_list_primal[i][j].v0;
          face_star[n_faces] = edge_list_primal[i][j].face;
          done[j] = 1;
          l++;
          star_size++;
          n_faces++;
          edge_added = 1;
          break;
        }
      }
      if (edge_added == 0) {
        printf("Vertex %d is non-manifold\n", i);
        free(done);
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
        continue;
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
#if 0
    printf("vertex %d: valence=%d\n", i, star_size);
#endif
    ring[i].n_faces = n_faces;
    ring[i].ord_face = (int*)malloc(n_faces*sizeof(int));
    memcpy(ring[i].ord_face, face_star, n_faces*sizeof(int));

    free(final_star);
    free(face_star);
    free(done);
    free(edge_list_primal[i]);
  }
  free(edge_list_primal);
  free(num_edges);
}

/* find the 1-ring of vertex v */
void build_star(const struct model *raw_model, int v, struct ring_info *ring) {

  int i, j, k;
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
      if (edge_list == NULL) {
	printf("realloc failed %d\n", i);
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
	printf("realloc failed %d\n", i);
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
    printf("Vertex %d has no face...\n", v);
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
	for (k=star_size-1; k>=0; k--) 
	  final_star[k+1] = final_star[k];
	for (k=n_faces-1; k>=0; k--)
	  face_star[k+1] = face_star[k];
       
	
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
	for (k=star_size-1; k>=0; k--) 
	  final_star[k+1] = final_star[k];
	for (k=n_faces-1; k>=0; k--)
	  face_star[k+1] = face_star[k];
       
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

#ifdef __RING_DEBUG
  printf("Vertex %d : star_size=%d num_edges=%d n_faces=%d\n", v, star_size, 
	 num_edges, n_faces);
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


/* $Id$ */
#include <3dutils.h>
#include <subdiv_methods.h>
#include <subdiv.h>
#include <block_list.h>
#include <assert.h>
#if defined(SUBDIV_DEBUG) || defined(DEBUG)
# include <debug_print.h>
#endif
#ifdef SUBDIV_TIME
# include <time.h>
#endif

/* This is the function that performs the subdivision.
   The argument 'face_midpoint_func' is the pointer to the 
   function that performs the computation of the midpoint.
   The 'update_func' stands for the function that updates 
   the postion of 'old' vertices. This is only used for 
   non-interpolating subd. (i.e. Kobbelt). For interpolating subd. 
   you just pass NULL as argument */
struct model* subdiv_sqrt3(struct model *raw_model, 
                           const struct subdiv_sqrt3_functions *sf) 
{

  struct model *subdiv_model;
  struct ring_info *rings;
  int nedges;
  int face_idx = 0;
  int i,j;
  bitmap_t *face_mp_done;
  struct block_list *temp_face=NULL, *cur;
  vertex_t *tmp_v=NULL;
#ifdef SUBDIV_DEBUG
  int vert_idx = raw_model->num_vert;
#endif
#ifdef SUBDIV_TIME
  clock_t start;
#endif

  rings = (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
    
  build_star_global(raw_model, rings);

  temp_face = (struct block_list*)malloc(sizeof(struct block_list));
  if (init_block_list(temp_face, sizeof(face_t)) != 0)
    abort();
  cur = temp_face;

  tmp_v = (vertex_t*)malloc(raw_model->num_faces*sizeof(vertex_t));
  face_mp_done = BITMAP_ALLOC(raw_model->num_faces);

#ifdef SUBDIV_TIME
  start = clock();
#endif
  for (i=0; i< raw_model->num_vert; i++) {
    nedges = rings[i].size;
    if (rings[i].type == 0) {
      for (j=0; j<nedges-1; j++) {
        /* compute face midpoint if needed */
        if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[j])) {
          sf->face_midpoint_func(rings, rings[i].ord_face[j], raw_model, 
                                 &(tmp_v[rings[i].ord_face[j]]));
          BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[j]);
        }
        if (cur->elem_filled == cur->nelem)
          cur = get_next_block(cur);
        assert(cur != NULL);

        BLOCK_LIST_TAIL(cur, face_t).f0 = i;
        BLOCK_LIST_TAIL(cur, face_t).f1 = rings[i].ord_face[j] + 
          raw_model->num_vert;
        BLOCK_LIST_TAIL_INCR(cur, face_t).f2 = rings[i].ord_face[j+1] + 
          raw_model->num_vert;

        face_idx++;
        
      }
      
      /* handle last face : closed ring*/
      if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[nedges-1])) {
        sf->face_midpoint_func(rings, rings[i].ord_face[nedges-1], raw_model, 
                               &(tmp_v[rings[i].ord_face[nedges-1]]));
        BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[nedges-1]);
      }
        if (cur->elem_filled == cur->nelem)
          cur = get_next_block(cur);
        assert(cur != NULL);

        BLOCK_LIST_TAIL(cur, face_t).f0 = i;
        BLOCK_LIST_TAIL(cur, face_t).f1 = 
          rings[i].ord_face[nedges-1] + raw_model->num_vert;
        BLOCK_LIST_TAIL_INCR(cur, face_t).f2 = rings[i].ord_face[0] + 
          raw_model->num_vert;

        face_idx++;

    } else if (rings[i].type == 1) {
      for (j=0; j<nedges-2; j++) {
        /* compute face midpoint if needed */
        if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[j])) {
          sf->face_midpoint_func(rings, rings[i].ord_face[j], raw_model, 
                                 &(tmp_v[rings[i].ord_face[j]]));
          BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[j]);
        }
        if (cur->elem_filled == cur->nelem)
          cur = get_next_block(cur);
        assert(cur != NULL);

        BLOCK_LIST_TAIL(cur, face_t).f0 = i;
        BLOCK_LIST_TAIL(cur, face_t).f1 = rings[i].ord_face[j] + 
          raw_model->num_vert;
        BLOCK_LIST_TAIL_INCR(cur, face_t).f2 = rings[i].ord_face[j+1] + 
          raw_model->num_vert;

        face_idx++;
        
      }
      
      /* handle last and first faces : open ring */
      if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[nedges-2])) {
        sf->face_midpoint_func(rings, rings[i].ord_face[nedges-2], raw_model, 
                               &(tmp_v[rings[i].ord_face[nedges-2]]));
        BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[nedges-2]);
      }
      if (cur->elem_filled == cur->nelem)
        cur = get_next_block(cur);
      assert(cur != NULL);
      
      /* FIXME: replace this by the MP of the edge every even
       * subdivision level. Anyway, this generates duplicates in
       * triangles so it _sucks_. It *must* die !! */
      BLOCK_LIST_TAIL(cur, face_t).f0 = i;
      BLOCK_LIST_TAIL(cur, face_t).f1 = rings[i].ord_face[nedges-2] + 
        raw_model->num_vert;
      BLOCK_LIST_TAIL_INCR(cur, face_t).f2 = rings[i].ord_vert[nedges-1];

      face_idx++;

      if (cur->elem_filled == cur->nelem)
        cur = get_next_block(cur);
      assert(cur != NULL);
      BLOCK_LIST_TAIL(cur, face_t).f0 = i;
      BLOCK_LIST_TAIL(cur, face_t).f1 = rings[i].ord_face[0] + 
        raw_model->num_vert;
      BLOCK_LIST_TAIL_INCR(cur, face_t).f2 = rings[i].ord_vert[0];

      face_idx++;

    } else { /* Non-manifold vertex */
      fprintf(stderr, "Vertex %d is non-manifold -> bail out\n", i);
      return NULL;
    }
  }
#ifdef SUBDIV_TIME
  printf("subdiv time = %f sec.\n", (clock()-start)/(float)CLOCKS_PER_SEC);
#endif

#ifdef SUBDIV_DEBUG
  DEBUG_PRINT("%d new vertices computed \n", raw_model->num_faces);
  DEBUG_PRINT("subdiv_model->num_vert = %d\n", 
              raw_model->num_faces + raw_model->num_vert);
#endif
  subdiv_model = (struct model*)malloc(sizeof(struct model));
  memset(subdiv_model, 0, sizeof(struct model));
  subdiv_model->num_vert = raw_model->num_vert + raw_model->num_faces;
  subdiv_model->num_faces = face_idx;
  subdiv_model->faces = 
    (face_t*)malloc(subdiv_model->num_faces*sizeof(face_t));
  subdiv_model->vertices = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));

  if (sf->update_func == NULL) /* Interpolating subdivision */
    memcpy(subdiv_model->vertices, raw_model->vertices, 
	   raw_model->num_vert*sizeof(vertex_t));
  else /* Approx. subdivision */
    sf->update_func(raw_model, subdiv_model, rings);
  
  /* copy faces midpoints */
  memcpy(&(subdiv_model->vertices[raw_model->num_vert]), tmp_v, 
         raw_model->num_faces*sizeof(vertex_t));

  if (gather_block_list(temp_face, subdiv_model->faces, 
                        subdiv_model->num_faces*sizeof(face_t)) != 0)
    abort();
  free_block_list(&temp_face);
  

#ifdef SUBDIV_DEBUG
  DEBUG_PRINT("face_idx = %d vert_idx = %d\n", face_idx, vert_idx);
#endif

  free(face_mp_done);
  free(tmp_v);
  for (i=0; i<raw_model->num_vert; i++) {
    free(rings[i].ord_vert);
    free(rings[i].ord_face);

  }
  free(rings);
  return subdiv_model;
}



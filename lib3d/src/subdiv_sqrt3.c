/* $Id: subdiv_sqrt3.c,v 1.1 2003/03/12 17:55:00 aspert Exp $ */
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
   The argument 'midpoint_func' is the pointer to the 
   function that performs the computation of the midpoint.
   The 'update_func' stands for the function that updates 
   the postion of 'old' vertices. This is only used for 
   non-interpolating subd. (i.e. Loop). For interpolating subd. 
   you just pass NULL as argument */
struct model* subdiv_sqrt3(struct model *raw_model, const int sub_method,
                           void (*face_midpoint_func)(const struct ring_info*,
                                                      const int, 
                                                      const struct model*, 
                                                      vertex_t*), 
                           void (*midpoint_func_bound)(const struct ring_info*,
                                                       const int, const int, 
                                                       const struct model*, 
                                                       vertex_t*), 
                           void (*update_func)(const struct model*, 
                                               struct model*, 
                                               const struct ring_info*) ) {

  struct model *subdiv_model;
  struct ring_info *rings;
  int nedges;
  int face_idx = 0;
  int i,j;
  bitmap_t *face_mp_done;
  face_t *temp_face;
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

 
  /* FIXME : This may be too small a storage in case of a mesh
     w. boundaries. Replace this w. a block_list maybe... */
  temp_face = (face_t*)malloc(3*raw_model->num_faces*sizeof(face_t));

  tmp_v = (vertex_t*)malloc(raw_model->num_faces*sizeof(vertex_t));
  face_mp_done = BITMAP_ALLOC(raw_model->num_faces);

  for (i=0; i< raw_model->num_vert; i++) {
    nedges = rings[i].size;
    if (rings[i].type == 0) {
      for (j=0; j<nedges-1; j++) {
        /* compute face midpoint if needed */
        if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[j])) {
          face_midpoint_func(rings, rings[i].ord_face[j], raw_model, 
                        &(tmp_v[rings[i].ord_face[j]]));
          BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[j]);
        }
        temp_face[face_idx].f0 = i;
        temp_face[face_idx].f1 = rings[i].ord_face[j] + raw_model->num_vert;
        temp_face[face_idx++].f2 = rings[i].ord_face[j+1] + 
          raw_model->num_vert;
        
      }
      
      /* handle last face : closed ring*/
      if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[nedges-1])) {
        face_midpoint_func(rings, rings[i].ord_face[nedges-1], raw_model, 
                      &(tmp_v[rings[i].ord_face[nedges-1]]));
        BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[nedges-1]);
      }
      temp_face[face_idx].f0 = i;
      temp_face[face_idx].f1 = rings[i].ord_face[rings[i].size-1] + 
        raw_model->num_vert;
      temp_face[face_idx++].f2 = rings[i].ord_face[0] + raw_model->num_vert;

    } else if (rings[i].type == 1) {
      for (j=0; j<nedges-2; j++) {
        /* compute face midpoint if needed */
        if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[j])) {
          face_midpoint_func(rings, rings[i].ord_face[j], raw_model, 
                             &(tmp_v[rings[i].ord_face[j]]));
          BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[j]);
        }
        temp_face[face_idx].f0 = i;
        temp_face[face_idx].f1 = rings[i].ord_face[j] + raw_model->num_vert;
        temp_face[face_idx++].f2 = rings[i].ord_face[j+1] + 
          raw_model->num_vert;
        
      }
      
      /* handle last and first faces : open ring */
      if (!BITMAP_TEST_BIT(face_mp_done, rings[i].ord_face[nedges-2])) {
        face_midpoint_func(rings, rings[i].ord_face[nedges-2], raw_model, 
                           &(tmp_v[rings[i].ord_face[nedges-2]]));
        BITMAP_SET_BIT(face_mp_done, rings[i].ord_face[nedges-2]);
      }
      temp_face[face_idx].f0 = i;
      temp_face[face_idx].f1 = rings[i].ord_face[nedges-2] + 
        raw_model->num_vert;
      /* FIXME: replace this by the MP of the edge */
      temp_face[face_idx++].f2 = rings[i].ord_vert[nedges-1];

      temp_face[face_idx].f0 = i;
      temp_face[face_idx].f1 = rings[i].ord_face[0] + 
        raw_model->num_vert;
      temp_face[face_idx++].f2 = rings[i].ord_vert[0];
    } else /* vertices w. weird type. Let's hit the roof for now */
      abort();
  }

#ifdef SUBDIV_DEBUG
  DEBUG_PRINT("%d new vertices computed \n", raw_model->num_faces);
  DEBUG_PRINT("subdiv_model->num_vert = %d\n", 
              raw_model->num_faces + raw_model->num_vert);
#endif
  subdiv_model = (struct model*)malloc(sizeof(struct model));
  memset(subdiv_model, 0, sizeof(struct model));
  subdiv_model->num_vert = raw_model->num_vert + raw_model->num_faces;
  subdiv_model->num_faces = face_idx;
  subdiv_model->faces = temp_face;
  subdiv_model->vertices = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));

  if (update_func == NULL) /* Interpolating subdivision */
    memcpy(subdiv_model->vertices, raw_model->vertices, 
	   raw_model->num_vert*sizeof(vertex_t));
  else /* Approx. subdivision */
    update_func(raw_model, subdiv_model, rings);
  
  /* copy faces midpoints */
  memcpy(&(subdiv_model->vertices[raw_model->num_vert]), tmp_v, 
         raw_model->num_faces*sizeof(vertex_t));
  
#ifdef SUBDIV_TIME
/*   printf("subdiv time = %f sec.\n", (clock()-start)/(float)CLOCKS_PER_SEC); */
#endif
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



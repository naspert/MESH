/* $Id: subdiv.c,v 1.37 2003/03/28 12:30:08 aspert Exp $ */
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
struct model* subdiv(struct model *raw_model, 
                     const struct subdiv_functions* sf) 
{

  struct model *subdiv_model;
  int i, i0, i1, i2;
  int v0, v1, v2;
  int u0, u1, u2;
  struct ring_info *rings;
  vertex_t p;
  int nedges = 0;
  int face_idx = 0;
  struct midpoint_info *mp_info;
  int v_idx=raw_model->num_vert;
  face_t *temp_face;
  struct block_list *tmp_v=NULL, *cur;
#ifdef SUBDIV_DEBUG
  int vert_idx = raw_model->num_vert;
#endif
#ifdef SUBDIV_TIME
  clock_t start;
#endif

  rings = (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
    
  build_star_global(raw_model, rings);

  /* Spherical subdivision needs to have normals computed */
  if (raw_model->normals == NULL && sf->id == SUBDIV_SPH) {
      raw_model->area = (float*)malloc(raw_model->num_faces*sizeof(float));
      raw_model->face_normals = compute_face_normals(raw_model, rings);
      compute_vertex_normal(raw_model, rings, raw_model->face_normals);
  }
  
  mp_info = (struct midpoint_info*)
    malloc(raw_model->num_vert*sizeof(struct midpoint_info));

  temp_face = (face_t*)malloc(4*raw_model->num_faces*sizeof(face_t));

  tmp_v = (struct block_list*)malloc(sizeof(struct block_list));
  if (init_block_list(tmp_v, sizeof(vertex_t)) != 0)
    abort();
  cur = tmp_v;

  for (i=0; i< raw_model->num_vert; i++) {

    /* This _should_ be kinda OK to ignore this vertex */
    if (rings[i].type != 0 && rings[i].type != 1) 
      continue;
    
    mp_info[i].edge_subdiv_done = BITMAP_ALLOC(rings[i].size);
    mp_info[i].size = rings[i].size;
    mp_info[i].midpoint_idx = (int*)malloc(mp_info[i].size*sizeof(int));
   
      
#ifdef SUBDIV_DEBUG
    DEBUG_PRINT("Vertex %d : star_size = %d\n", i, rings[i].size);
    for (j=0; j<rings[i].size; j++)
      printf("number %d : %d\n", j, rings[i].ord_vert[j]);
#endif
  }

#ifdef SUBDIV_TIME
  start = clock();
#endif
  for (i=0; i<raw_model->num_faces; i++) {
    v0 = raw_model->faces[i].f0;
    v1 = raw_model->faces[i].f1;
    v2 = raw_model->faces[i].f2;

    i0 = 0;
    while (rings[v0].ord_vert[i0] != v1)
      i0++;
    u0 = 0;
    while (rings[v1].ord_vert[u0] != v0)
      u0++;

    i1 = 0;
    while (rings[v1].ord_vert[i1] != v2)
      i1++;
    u1 = 0;
    while (rings[v2].ord_vert[u1] != v1)
      u1++;

    i2 = 0;
    while (rings[v2].ord_vert[i2] != v0)
      i2++;
    u2 = 0;
    while (rings[v0].ord_vert[u2] != v2)
      u2++;

    /* edge v0v1 */
    if (!BITMAP_TEST_BIT(mp_info[v0].edge_subdiv_done, i0)) {
      if (rings[v0].type == 1 || rings[v1].type == 1)
        sf->midpoint_func_bound(rings, v0, i0, raw_model, &p);
      else
        sf->midpoint_func(rings, v0, i0, raw_model, &p);
      nedges++;
      BITMAP_SET_BIT(mp_info[v0].edge_subdiv_done, i0);
      BITMAP_SET_BIT(mp_info[v1].edge_subdiv_done, u0);
      if (cur->elem_filled == cur->nelem)
        cur = get_next_block(cur);
      assert(cur != NULL);
      BLOCK_LIST_TAIL_INCR(cur, vertex_t) = p;


      mp_info[v0].midpoint_idx[i0] = v_idx;
      mp_info[v1].midpoint_idx[u0] = v_idx++;

      
    }

    /* edge v1v2 */
    if (!BITMAP_TEST_BIT(mp_info[v1].edge_subdiv_done, i1)) {
      if (rings[v1].type == 1 || rings[v2].type == 1)
        sf->midpoint_func_bound(rings, v1, i1, raw_model, &p);
      else
        sf->midpoint_func(rings, v1, i1, raw_model, &p);
      nedges++;
      BITMAP_SET_BIT(mp_info[v1].edge_subdiv_done, i1);
      BITMAP_SET_BIT(mp_info[v2].edge_subdiv_done, u1);
      if (cur->elem_filled == cur->nelem)
        cur = get_next_block(cur);
      assert(cur != NULL);
      BLOCK_LIST_TAIL_INCR(cur, vertex_t) = p;

      mp_info[v1].midpoint_idx[i1] = v_idx;
      mp_info[v2].midpoint_idx[u1] = v_idx++;
    }

    /* edge v2v0 */
    if (!BITMAP_TEST_BIT(mp_info[v2].edge_subdiv_done, i2)) {
      if (rings[v2].type == 1 || rings[v0].type == 1)
        sf->midpoint_func_bound(rings, v2, i2, raw_model, &p);
      else
        sf->midpoint_func(rings, v2, i2, raw_model, &p);
      nedges++;
      BITMAP_SET_BIT(mp_info[v2].edge_subdiv_done, i2);
      BITMAP_SET_BIT(mp_info[v0].edge_subdiv_done, u2);
      if (cur->elem_filled == cur->nelem)
        cur = get_next_block(cur);
      assert(cur != NULL);
      BLOCK_LIST_TAIL_INCR(cur, vertex_t) = p;

      mp_info[v2].midpoint_idx[i2] = v_idx;
      mp_info[v0].midpoint_idx[u2] = v_idx++;

    }

    /* We have all the edges of the current tr. that are subdivided */
    temp_face[face_idx].f0 = v0;
    temp_face[face_idx].f1 = mp_info[v0].midpoint_idx[i0];
    temp_face[face_idx++].f2 = mp_info[v2].midpoint_idx[i2];
    
    temp_face[face_idx].f0 = v1;
    temp_face[face_idx].f1 = mp_info[v0].midpoint_idx[i0];
    temp_face[face_idx++].f2 = mp_info[v1].midpoint_idx[i1];

    temp_face[face_idx].f0 = v2;
    temp_face[face_idx].f1 = mp_info[v1].midpoint_idx[i1];
    temp_face[face_idx++].f2 = mp_info[v2].midpoint_idx[i2];

    temp_face[face_idx].f0 = mp_info[v0].midpoint_idx[i0];
    temp_face[face_idx].f1 = mp_info[v1].midpoint_idx[i1];
    temp_face[face_idx++].f2 = mp_info[v2].midpoint_idx[i2];
  }
 
  





#ifdef SUBDIV_DEBUG
  DEBUG_PRINT("%d new vertices computed \n", nedges);
  DEBUG_PRINT("subdiv_model->num_vert = %d\n", nedges+raw_model->num_vert);
#endif
  subdiv_model = (struct model*)malloc(sizeof(struct model));
  memset(subdiv_model, 0, sizeof(struct model));
  subdiv_model->num_vert = raw_model->num_vert + nedges;
  subdiv_model->num_faces = 4*raw_model->num_faces;
  subdiv_model->faces = temp_face;
  subdiv_model->vertices = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));

  if (sf->update_func == NULL) /* Interpolating subdivision */
    memcpy(subdiv_model->vertices, raw_model->vertices, 
	   raw_model->num_vert*sizeof(vertex_t));
  else /* Approx. subdivision */
    sf->update_func(raw_model, subdiv_model, rings);
  
  if (gather_block_list(tmp_v, &(subdiv_model->vertices[raw_model->num_vert]), 
                        nedges*sizeof(vertex_t)) != 0)
    abort();
  free_block_list(&tmp_v);
  
#ifdef SUBDIV_TIME
  printf("subdiv time = %f sec.\n", (clock()-start)/(float)CLOCKS_PER_SEC);
#endif
#ifdef SUBDIV_DEBUG
  DEBUG_PRINT("face_idx = %d vert_idx = %d\n", face_idx, vert_idx);
#endif

  for (i=0; i<raw_model->num_vert; i++) {
    if (rings[i].type == 0 || rings[i].type == 1) {
      free(mp_info[i].midpoint_idx);
      free(mp_info[i].edge_subdiv_done);
    }
    free(rings[i].ord_vert);
    free(rings[i].ord_face);
 
  }
  free(rings);
  free(mp_info);
  return subdiv_model;
}



/* $Id: subdiv.c,v 1.18 2002/02/26 14:46:47 aspert Exp $ */
#include <3dutils.h>
#include <subdiv_methods.h>
#include <assert.h>

/* This is the function that performs the subdivision.
   The argument 'midpoint_func' is the pointer to the 
   function that performs the computation of the midpoint.
   The 'update_func' stands for the function that updates 
   the postion of 'old' vertices. This is only used for 
   non-interpolating subd. (i.e. Loop). For interpolating subd. 
   you just pass NULL as argument */
struct model* subdiv(struct model *raw_model, 
		     void (*midpoint_func)(struct ring_info*, int, int, 
					   struct model*, vertex_t*), 
		     void (*midpoint_func_bound)(struct ring_info*, int, int, 
						 struct model*, vertex_t*), 
		     void (*update_func)(struct model*, struct model*, 
					 struct ring_info*) ) {
  struct ring_info *rings;
  struct model *subdiv_model;
  int i, j;
  int v0, v1, v2;
  int u0=-1, u1=-1, u2=-1;
  unsigned char ufound_bm;
  vertex_t p;
  int nedges = 0;
  int vert_idx = raw_model->num_vert;
  int face_idx = 0;
  struct midpoint_info *mp_info;
  int v_idx=raw_model->num_vert;
  face_t *temp_face;


  rings = (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
  
  mp_info = (struct midpoint_info*)
    malloc(raw_model->num_vert*sizeof(struct midpoint_info));


  build_star_global(raw_model, &rings);

  for (i=0; i<raw_model->num_vert; i++) {

    mp_info[i].size = rings[i].size;
    mp_info[i].midpoint_idx = (int*)malloc(mp_info[i].size*sizeof(int));
    /* Initialize the values of this array to -1 */
    memset(mp_info[i].midpoint_idx, 0xff, mp_info[i].size*sizeof(int));
    mp_info[i].midpoint = (vertex_t*)malloc(mp_info[i].size*sizeof(vertex_t));
      
#ifdef __SUBDIV_DEBUG
    printf("Vertex %d : star_size = %d\n", i, rings[i].size);
    for (j=0; j<rings[i].size; j++)
      printf("number %d : %d\n", j, rings[i].ord_vert[j]);
#endif

  }
  


  for (i=0; i<raw_model->num_vert; i++) {
    for (j=0; j<rings[i].size; j++) {

      /* this edge is already subdivided */
      if (rings[i].ord_vert[j] < i) 
	continue; 

      /* 2 boundary v. -> do nothing */
      if (rings[i].type==1 || rings[rings[i].ord_vert[j]].type==1) {
	if (midpoint_func_bound == NULL)
	  continue;
	else {
	  midpoint_func_bound(rings, i, j, raw_model, &p);
	  nedges ++;
	  mp_info[i].midpoint_idx[j] = v_idx;
	  mp_info[i].midpoint[j] = p;
	  v2 = rings[i].ord_vert[j];
	  v0 = 0;
	  while (rings[v2].ord_vert[v0] != i)
	    v0++;
	  mp_info[v2].midpoint_idx[v0] = v_idx++;
	  mp_info[v2].midpoint[v0] = p;
	  continue;
	}
      }
	  
      
      midpoint_func(rings, i, j, raw_model, &p);
      nedges ++;


#ifdef __SUBDIV_DEBUG
      printf("i=%d j=%d  rings[%d].ord_vert[%d]=%d\n",i,j, 
	      i, j, rings[i].ord_vert[j]);
      printf("nedges-1=%d\n",nedges-1);
#endif


      mp_info[i].midpoint_idx[j] = v_idx;
      mp_info[i].midpoint[j] = p;
      v2 = rings[i].ord_vert[j];
      v0 = 0;
      while (rings[v2].ord_vert[v0] != i)
	v0++;
      mp_info[v2].midpoint_idx[v0] = v_idx++;
      mp_info[v2].midpoint[v0] = p;
    }

  }



  printf("%d new vertices computed \n", nedges);
  printf("subdiv_model->num_vert = %d\n", nedges+raw_model->num_vert);

  subdiv_model = (struct model*)malloc(sizeof(struct model));
  memset(subdiv_model, 0, sizeof(struct model));
  subdiv_model->num_vert = raw_model->num_vert + nedges;
  subdiv_model->num_faces = 4*raw_model->num_faces;
  subdiv_model->faces = (face_t*)
    malloc(subdiv_model->num_faces*sizeof(face_t));
  subdiv_model->vertices = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));

  if (update_func == NULL) /* Interpolating subdivision */
    memcpy(subdiv_model->vertices, raw_model->vertices, 
	   raw_model->num_vert*sizeof(vertex_t));
  else /* Approx. subdivision */
    update_func(raw_model, subdiv_model, rings);


  
  for (j=0; j<raw_model->num_faces; j++) {
    v0 = raw_model->faces[j].f0;
    v1 = raw_model->faces[j].f1;
    v2 = raw_model->faces[j].f2;

    ufound_bm = 0;

    i = 0;
    while (rings[v0].ord_vert[i] != v1)
      i++;
    if (mp_info[v0].midpoint_idx[i] != -1) {
      u0 = mp_info[v0].midpoint_idx[i];
      subdiv_model->vertices[u0] = 
	mp_info[v0].midpoint[i];
      ufound_bm |= U0_FOUND;
    } 
     
    i = 0;
    while (rings[v1].ord_vert[i] != v2)
      i++;
    if (mp_info[v1].midpoint_idx[i] != -1) {
      u1 = mp_info[v1].midpoint_idx[i];
      subdiv_model->vertices[u1] = 
	mp_info[v1].midpoint[i];
      ufound_bm |= U1_FOUND;
    } 

    i = 0;
    while (rings[v2].ord_vert[i] != v0)
      i++;
    if (mp_info[v2].midpoint_idx[i] != -1) {
      u2 = mp_info[v2].midpoint_idx[i];
      subdiv_model->vertices[u2] = 
	mp_info[v2].midpoint[i];
      ufound_bm |= U2_FOUND;
    } 

  
#ifdef __BUTTERFLY_CREASE_DEBUG
    if (ufound_bm != 7)
      fprintf(stderr, "ufound_bm = %d\n", ufound_bm);
#endif
    switch(ufound_bm) {
    case 7:
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u2;    
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v2;
      subdiv_model->faces[face_idx].f1 = u1;
      subdiv_model->faces[face_idx].f2 = u2;
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = u2;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      break;
    case 6: /* u1 & u2 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = v1;
      subdiv_model->faces[face_idx].f2 = u2;    
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u2;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v2;
      subdiv_model->faces[face_idx].f1 = u1;
      subdiv_model->faces[face_idx].f2 = u2;
      face_idx++;
      break;
    case 5: /* u0 & u2 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u2;    
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v2;
      subdiv_model->faces[face_idx].f1 = u2;
      subdiv_model->faces[face_idx].f2 = u0;
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = v2;
      face_idx++;
      break;
    case 4: /* u2 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = v1;
      subdiv_model->faces[face_idx].f2 = u2;
      face_idx++;

      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u2;
      subdiv_model->faces[face_idx].f2 = v2;
      face_idx++;
      break;
    case 3: /* u0 & u1 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u1;
      subdiv_model->faces[face_idx].f2 = v2;    
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      break;
    case 2: /* u1 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u1;
      subdiv_model->faces[face_idx].f2 = v2;
      face_idx++;

      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = v0;
      subdiv_model->faces[face_idx].f2 = u1;
      face_idx++;
      break;
    case 1: /* u0 found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = v2;
      face_idx++;

      subdiv_model->faces[face_idx].f0 = v1;
      subdiv_model->faces[face_idx].f1 = u0;
      subdiv_model->faces[face_idx].f2 = v2;
      face_idx++;
      break;
    case 0: /* none found */
      subdiv_model->faces[face_idx].f0 = v0;
      subdiv_model->faces[face_idx].f1 = v1;
      subdiv_model->faces[face_idx].f2 = v2;    
      face_idx++;
      break;
    default: /* should never get here */
      fprintf(stderr, "Trouble ufound_bm = %d\n", ufound_bm);
    }
  }


  printf("face_idx = %d vert_idx = %d\n", face_idx, vert_idx);
  temp_face = (face_t*)malloc(face_idx*sizeof(face_t));
  memcpy(temp_face, subdiv_model->faces, face_idx*sizeof(face_t));
  free(subdiv_model->faces);
  subdiv_model->faces = temp_face;
  subdiv_model->num_faces = face_idx;

  for (i=0; i<raw_model->num_vert; i++) {
    free(rings[i].ord_vert);
    free(rings[i].ord_face);
    free(mp_info[i].midpoint_idx);
    free(mp_info[i].midpoint);
  }
  free(rings);
  free(mp_info);
  return subdiv_model;
}

int main(int argc, char **argv) {
  char *infile, *outfile;
  struct model *or_model, *sub_model=NULL;
  struct info_vertex* tmp_vert;
  int i, lev, nlev=1;
  int sub_method=-1;


  if (argc != 4 && argc != 5) {
    fprintf(stderr, 
	    "Usage: subdiv_sph [-sph, -but, -loop, -loopb] infile outfile n_lev\n");
    exit(1);
  }
  if (strcmp(argv[1], "-sph") == 0) 
    sub_method = SUBDIV_SPH;
  else if (strcmp(argv[1], "-but") == 0) 
    sub_method = SUBDIV_BUTTERFLY;
  else if (strcmp(argv[1], "-loop") == 0)
    sub_method = SUBDIV_LOOP;
  else if (strcmp(argv[1], "-loopb") == 0)
    sub_method = SUBDIV_LOOP_BOUNDARY;
  else {
    fprintf(stderr, "Invalid subdivision method %s\n", argv[1]);
    fprintf(stderr, 
	    "Usage: subdiv_sph [-sph, -but, -loop, -loopb] infile outfile n_lev\n");
    exit(1);
  }

  infile = argv[2];
  outfile = argv[3];

  if (argc==5)
    nlev = atoi(argv[4]);
  
  if (nlev < 1)
    nlev = 1;

  or_model = read_raw_model(infile);

  for (lev=0; lev<nlev; lev++) {
    if (or_model->normals == NULL && sub_method == SUBDIV_SPH) {
      tmp_vert = (struct info_vertex*)
	malloc(or_model->num_vert*sizeof(struct info_vertex));
      or_model->area = (float*)malloc(or_model->num_faces*sizeof(float));
      or_model->face_normals = compute_face_normals(or_model, tmp_vert);
      compute_vertex_normal(or_model, tmp_vert, or_model->face_normals);
      for (i=0; i<or_model->num_vert; i++) 
 	free(tmp_vert[i].list_face); 
      free(tmp_vert);
    }

    /* performs the subdivision */
    switch (sub_method) {
    case SUBDIV_SPH:
      sub_model = subdiv(or_model, compute_midpoint_sph, NULL, NULL);
      break;
    case SUBDIV_LOOP:
      sub_model = subdiv(or_model, compute_midpoint_loop, NULL, 
			 update_vertices_loop);
      break;
    case SUBDIV_LOOP_BOUNDARY:
      sub_model = subdiv(or_model, compute_midpoint_loop, 
			 compute_midpoint_loop_crease,
			 update_vertices_loop_crease);
      break;
    case SUBDIV_BUTTERFLY:
      sub_model = subdiv(or_model, compute_midpoint_butterfly, 
			 compute_midpoint_butterfly_crease, NULL);
      break;
    default:
      fprintf(stderr, "ERROR : Invalid subdivision method found = %d\n", 
	      sub_method);
      exit(1);
      break;
    }

    
    __free_raw_model(or_model);
    
    or_model = sub_model;
  }
  write_raw_model(sub_model, outfile);

  __free_raw_model(sub_model);
  return 0;
}

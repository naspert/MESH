/* $Id: loop.c,v 1.2 2001/09/24 11:59:27 aspert Exp $ */
#include <3dutils.h>


vertex compute_midpoint(ring_info *rings, int center, int v1, 
			model *raw_model) {

  vertex np, tmp;
  ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  int n = rings[center].size;
  int p0, p1;

  add_v(&(raw_model->vertices[center]), &(raw_model->vertices[center2]), 
	&np);
  prod_v(0.375, &np, &np);

  p0 = ring.ord_vert[(v1+1)%n];
  if (v1 > 0)
    p1 = ring.ord_vert[v1-1];
  else
    p1 = ring.ord_vert[n-1];
 
  add_v(&(raw_model->vertices[p0]), &(raw_model->vertices[p1]), 
	&tmp);

  add_prod_v(0.125, &tmp, &np, &np);


  return np;
}

void update_old_vertices(model *or_model, model *subdiv_model, 
			 ring_info *rings) {
  int i, j, v, n;
  double beta;
  vertex tmp;
  for (i=0; i<or_model->num_vert; i++) {
    n = rings[i].size;
 
    if (n == 3)
      beta = 3.0/16.0;
    else
      beta = 3.0/(8.0*n);

    prod_v(1.0-n*beta, &(or_model->vertices[i]), &tmp);

    for (j=0; j<n; j++) {
      v = rings[i].ord_vert[j];
      add_prod_v(beta, &(or_model->vertices[v]), &tmp, &tmp);
    }
    subdiv_model->vertices[i] = tmp;
  }

}

model* subdiv(model *raw_model, edge_sub **edge_list_ptr, 
	      int **midpoint_idx_ptr, int *num_edges) {
  ring_info *rings;
  model *subdiv_model;
  int i, j;
  int v0, v1, v2;
  int u0=-1, u1=-1, u2=-1;
  edge_v edge;
  vertex p;
  edge_sub *edge_list=NULL; 
  int nedges = 0;
  int vert_idx = raw_model->num_vert;
  int face_idx = 0;
  int *done; /* *midpoint_idx; */
  int *midpoint_idx;

  rings = (ring_info*)malloc(raw_model->num_vert*sizeof(ring_info));
  
  for (i=0; i<raw_model->num_vert; i++) {
    build_star(raw_model, i, &(rings[i]));

#ifdef _DEBUG
    printf("Vertex %d : star_size = %d\n", i, rings[i].size);
    for (j=0; j<rings[i].size; j++)
      printf("number %d : %d\n", j, rings[i].ord_vert[j]);
#endif

    if (rings[i].type == 1) {
      free(rings);
      printf("Boundary vertex unsupported\n");
      return NULL;
    }
  }
  
  
  for (i=0; i<raw_model->num_vert; i++) {
    for (j=0; j<rings[i].size; j++) {
      if (rings[i].ord_vert[j] < i) /* this edge is already subdivided */
	continue; 
      p = compute_midpoint(rings, i, j, raw_model);
      nedges ++;
      edge_list = (edge_sub*)realloc(edge_list, nedges*sizeof(edge_sub));

#ifdef _DEBUG
      printf("i=%d j=%d  rings[%d].ord_vert[%d]=%d\n",i,j, 
	      i, j, rings[i].ord_vert[j]);
      printf("nedges-1=%d n_idx=%d\n",nedges-1, n_idx);
#endif

      edge_list[nedges-1].edge.v0 = i;

#ifdef _DEBUG
      printf("edge.v0=%d\n", edge_list[nedges-1].edge.v0);
#endif

      edge_list[nedges-1].edge.v1 = rings[i].ord_vert[j];

#ifdef _DEBUG
      printf("edge.v1=%d\n", edge_list[nedges-1].edge.v1);
#endif

      edge_list[nedges-1].p = p;

      
    }

  }



  printf("%d edges found in model \n", nedges);

  subdiv_model = (model*)malloc(sizeof(model));
  subdiv_model->num_vert = raw_model->num_vert + nedges;
  subdiv_model->num_faces = 4*raw_model->num_faces;
  subdiv_model->faces = (face*)malloc(subdiv_model->num_faces*sizeof(face));
  subdiv_model->vertices = 
    (vertex*)malloc(subdiv_model->num_vert*sizeof(vertex));

  
  update_old_vertices(raw_model, subdiv_model, rings);



  
  done = (int*)calloc(nedges, sizeof(int));
  midpoint_idx = (int*)malloc(nedges*sizeof(int));
  
  for (j=0; j<raw_model->num_faces; j++) {
    v0 = raw_model->faces[j].f0;
    v1 = raw_model->faces[j].f1;
    v2 = raw_model->faces[j].f2;

    /* v0 v1 */
    if (v0 < v1) {
      edge.v0 = v0;
      edge.v1 = v1;
    } else {
      edge.v1 = v0;
      edge.v0 = v1;
    }
    for (i=0; i<nedges; i++) {
      if(edge_list[i].edge.v0 != edge.v0)
	continue;
      if(edge_list[i].edge.v1 == edge.v1) {
	if (done[i] == 0) {
	  subdiv_model->vertices[vert_idx] = edge_list[i].p;
	  u0 = vert_idx;
	  done[i] = 1;
	  midpoint_idx[i] = vert_idx;
	  vert_idx++;
	} else
	  u0 = midpoint_idx[i];
	break;
      }
    }
    /* v1 v2 */
    if (v1 < v2) {
      edge.v0 = v1;
      edge.v1 = v2;
    } else {
      edge.v1 = v1;
      edge.v0 = v2;
    }
    for (i=0; i<nedges; i++) {
      if(edge_list[i].edge.v0 != edge.v0)
	continue;
      if(edge_list[i].edge.v1 == edge.v1) {
	if (done[i] == 0) {
	  subdiv_model->vertices[vert_idx] = edge_list[i].p;
	  u1 = vert_idx;
	  midpoint_idx[i] = vert_idx;
	  done[i] = 1;
	  vert_idx++;
	} else 
	  u1 = midpoint_idx[i];
	break;
      }
    }
    /* v2 v0 */
    if (v2 < v0) {
      edge.v0 = v2;
      edge.v1 = v0;
    } else {
      edge.v1 = v2;
      edge.v0 = v0;
    }
    for (i=0; i<nedges; i++) {
      if(edge_list[i].edge.v0 != edge.v0)
	continue;
      if(edge_list[i].edge.v1 == edge.v1) {
	if (done[i] == 0) {
	  subdiv_model->vertices[vert_idx] = edge_list[i].p;
	  u2 = vert_idx;
	  midpoint_idx[i] = vert_idx;
	  done[i] = 1;
	  vert_idx++;
	} else
	  u2 = midpoint_idx[i];
	break;
      }
    }
    

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

  }

  printf("face_idx = %d vert_idx = %d\n", face_idx, vert_idx);

  free(rings);
  free(done);

  *num_edges = nedges;
  *midpoint_idx_ptr = midpoint_idx;
  *edge_list_ptr = edge_list;
  return subdiv_model;
}

int main(int argc, char **argv) {
  char *infile, *outfile;
  model *or_model, *sub_model;
  info_vertex* tmp_vert;
  int *midpoint_idx,  num_edges;
  edge_sub *edge_list;



  if (argc != 3) {
    fprintf(stderr, "Usage: loop infile outfile\n");
    exit(0);
  }
  infile = argv[1];
  outfile = argv[2];


  or_model = read_raw_model(infile);
  if (or_model->normals == NULL) {
    tmp_vert = (info_vertex*)malloc(or_model->num_vert*sizeof(info_vertex));
    or_model->area = (double*)malloc(or_model->num_faces*sizeof(double));
    or_model->face_normals = compute_face_normals(or_model, tmp_vert);
    compute_vertex_normal(or_model, tmp_vert, or_model->face_normals);
    free(tmp_vert);
    free(or_model->area);
  }



/* performs the subdivision */
  sub_model = subdiv(or_model, &edge_list, &midpoint_idx, &num_edges);
  

  sub_model->builtin_normals = 0;
  sub_model->normals = NULL;
 

#ifdef COMP_SUB_NORMALS
  tmp_vert = (info_vertex*)malloc(sub_model->num_vert*sizeof(info_vertex));
  sub_model->area = (double*)malloc(sub_model->num_faces*sizeof(double));
  sub_model->face_normals = compute_face_normals(sub_model, tmp_vert);
  compute_vertex_normal(sub_model, tmp_vert, sub_model->face_normals);
  free(tmp_vert);
  free(sub_model->area);
#endif


#ifdef COMP_SUB_NORMALS_DEBUG
  printf("[main]: %d edges\n", num_edges);
  for (i=0; i<num_edges; i++) {
    printf("edge %d %d mpoint %d\n", edge_list[i].edge.v0, 
	   edge_list[i].edge.v1, midpoint_idx[i]);
    printf("n0 = %f\t%f\t%f\n", sub_model->normals[edge_list[i].edge.v0].x, 
	   sub_model->normals[edge_list[i].edge.v0].y, 
	   sub_model->normals[edge_list[i].edge.v0].z);
    printf("n1 = %f\t%f\t%f\n", sub_model->normals[edge_list[i].edge.v1].x, 
	   sub_model->normals[edge_list[i].edge.v1].y, 
	   sub_model->normals[edge_list[i].edge.v1].z);
    printf("np = %f\t%f\t%f\n", sub_model->normals[midpoint_idx[i]].x, 
	   sub_model->normals[midpoint_idx[i]].y, 
	   sub_model->normals[midpoint_idx[i]].z); 
  }
  free(midpoint_idx);
  free(edge_list);
#endif


  write_raw_model(sub_model, outfile);
  free(sub_model->faces);
  free(sub_model->vertices);
  

  free(sub_model);
  free_raw_model(or_model);
  return 0;
}

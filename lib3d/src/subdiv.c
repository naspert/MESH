/* $Id: subdiv.c,v 1.2 2001/03/13 13:26:11 aspert Exp $ */
#include <3dutils.h>

/* v0 & v1 are the indices in rings[center].ord_vert */
vertex compute_midpoint(ring_info *rings, int center,  int v1, 
			model *raw_model) {
  double *s, *t;
  double qt=0.0, qs=0.0;
  double w=0.0; /* This is a parameter for Butterfly subdivision */
  int j;
  vertex p, r;
  int n = rings[center].size;
  ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size; /* size of opp. ring */
  int v2 = 0; /* index of center vertex in opp. ring */


#ifdef SUBDIV_DEBUG
  printf("Subdiv edge %d %d\n", center, center2);
  printf("n=%d m=%d\n", n, m);
  printf("center %d = %f %f %f\n", center,  raw_model->vertices[center].x, 
	 raw_model->vertices[center].y, 
	 raw_model->vertices[center].z);
  printf("center2 %d = %f %f %f\n", center2, raw_model->vertices[center2].x, 
	 raw_model->vertices[center2].y, 
	 raw_model->vertices[center2].z);
#endif

  if (n != 6 && m != 6) {/* double irreg */    
    while (ring_op.ord_vert[v2] != center)
      v2++;
    
    s = (double*)malloc(n*sizeof(double));
    t = (double*)malloc(m*sizeof(double));

    /* Compute values of stencil for end-vertex */
    if (m > 4) {
      for (j=0; j<m; j++) {
 	t[j] = (0.25 + cos(2*M_PI*j/(double)m) + 
		0.5*cos(4*M_PI*j/(double)m))/(double)m;
      }
      qt = 0.75;
      
    } else if (m == 4) {
      t[0] = 3.0/8.0;
      t[1] = 0.0;
      t[2] = -1.0/8.0;
      t[3] = 0.0;
      qt = 0.75;
    } else if (m == 3) {
      t[0] = 5.0/12.0;
      t[1] = -1.0/12.0;
      t[2] = t[1];
      qt = 0.75;
    }


    /* Compute values of stencil for center vertex */
    if (n > 4) {
      for (j=0; j<n; j++) {
	s[j] = (0.25 + cos(2*M_PI*j/(double)n) + 
		0.5*cos(4*M_PI*j/(double)n))/(double)n;
      }
      qs = 0.75;
      
    } else if (n == 4) {
      s[0] = 3.0/8.0;
      s[1] = 0.0;
      s[2] = -1.0/8.0;
      s[3] = 0.0;
      qs = 0.75;
    } else if (n == 3) {
      s[0] = 5.0/12.0;
      s[1] = -1.0/12.0;
      s[2] = s[1];
      qs = 0.75;
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    r.x = 0.0;
    r.y = 0.0;
    r.z = 0.0;

    /* Apply stencil to 1st vertex */
    for (j=0; j<n; j++) {
      p.x += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].x;
      p.y += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].y;
      p.z += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].z;
#ifdef SUBDIV_DEBUG
      printf("s[%d]=%f\n",j, s[j]);
      printf("v = %f %f %f\n", raw_model->vertices[ring.ord_vert[(v1+j)%n]].x,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].y,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].z);
      printf("idx = %d\n", (v1+j)%n);
      printf("%d: p = %f %f %f\n", j,p.x, p.y, p.z);
#endif
    }
    p.x += qs*raw_model->vertices[center].x;
    p.y += qs*raw_model->vertices[center].y;
    p.z += qs*raw_model->vertices[center].z;

    
    /* Apply stencil to end vertex */
    for (j=0; j<m; j++) {
      r.x += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].x;
      r.y += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].y;
      r.z += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].z;

    }
    r.x += qt*raw_model->vertices[center2].x;
    r.y += qt*raw_model->vertices[center2].y;
    r.z += qt*raw_model->vertices[center2].z;


    r.x *= 0.5;
    r.y *= 0.5;
    r.z *= 0.5;
    p.x *= 0.5;
    p.y *= 0.5;
    p.z *= 0.5;
    
    /* take the average */
    p.x += r.x;
    p.y += r.y;
    p.z += r.z;
    
    free(s);
    free(t);
  }
  else if (n == 6 && m == 6) {/* regular */
    /* apply the 10 point stencil */
    s = (double*)malloc(6*sizeof(double));
    s[0] = 0.25 - 2.0*w;
    s[1] = 1.0/8.0 + 2*w;
    s[2] = -s[1];
    s[3] = 2.0*w;
    s[4] = s[2];
    s[5] = s[1];
    qs = 0.75;

    while (ring_op.ord_vert[v2] != center)
      v2++;

    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    r.x = 0.0;
    r.y = 0.0;
    r.z = 0.0;

    /* Apply stencil to 1st vertex */
    for (j=0; j<6; j++) {
      p.x += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%6]].x;
      p.y += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%6]].y;
      p.z += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%6]].z;

    }
    p.x += qs*raw_model->vertices[center].x;
    p.y += qs*raw_model->vertices[center].y;
    p.z += qs*raw_model->vertices[center].z;

    /* Apply stencil to end vertex */
    for (j=0; j<6; j++) {
      p.x += s[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%6]].x;
      p.y += s[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%6]].y;
      p.z += s[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%6]].z;

    }
    p.x += qs*raw_model->vertices[center2].x;
    p.y += qs*raw_model->vertices[center2].y;
    p.z += qs*raw_model->vertices[center2].z;
    
    p.x *= 0.5;
    p.y *= 0.5;
    p.z *= 0.5;

    free(s);
  }
  else if (n!=6 && m==6){ /* only one irreg. vertex */
    s = (double*)malloc(n*sizeof(double));
    if (n > 4) {
      for (j=0; j<n; j++) {
	s[j] = (0.25 + cos(2*M_PI*j/(double)n) + 
		0.5*cos(4*M_PI*j/(double)n))/(double)n;
      }
      qs = 0.75;
      
    } else if (n == 4) {
      s[0] = 3.0/8.0;
      s[1] = 0.0;
      s[2] = -1.0/8.0;
      s[3] = 0.0;
      qs = 0.75;
    } else if (n == 3) {
      s[0] = 5.0/12.0;
      s[1] = -1.0/12.0;
      s[2] = s[1];
      qs = 0.75;
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    for (j=0; j<n; j++) {
      p.x += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].x;
      p.y += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].y;
      p.z += s[j]*raw_model->vertices[ring.ord_vert[(v1+j)%n]].z;

    }
    p.x += qs*raw_model->vertices[center].x;
    p.y += qs*raw_model->vertices[center].y;
    p.z += qs*raw_model->vertices[center].z;
  } else if (n==6 && m!=6) {
    t = (double*)malloc(m*sizeof(double));

    while (ring_op.ord_vert[v2] != center)
      v2++;

    if (m > 4) {
      for (j=0; j<m; j++) {
	t[j] = (0.25 + cos(2*M_PI*j/(double)m) + 
		0.5*cos(4*M_PI*j/(double)m))/(double)m;
      }
      qt = 0.75;
      
    } else if (m == 4) {
      t[0] = 3.0/8.0;
      t[1] = 0.0;
      t[2] = -1.0/8.0;
      t[3] = 0.0;
      qt = 0.75;
    } else if (m == 3) {
      t[0] = 5.0/12.0;
      t[1] = -1.0/12.0;
      t[2] = t[1];
      qt = 0.75;
    }
    
    p.x = 0.0;
    p.y = 0.0;
    p.z = 0.0;

    for (j=0; j<m; j++) {
      p.x += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].x;
      p.y += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].y;
      p.z += t[j]*raw_model->vertices[ring_op.ord_vert[(v2+j)%m]].z;

    }
    p.x += qt*raw_model->vertices[center2].x;
    p.y += qt*raw_model->vertices[center2].y;
    p.z += qt*raw_model->vertices[center2].z;
    free(t);
  } 
  
  return p;
}


model* subdiv(model *raw_model) {
  ring_info *rings;
  model *subdiv_model;
  int i, j;
  int v0, v1, v2;
  int u0=-1, u1=-1, u2=-1;
  edge_v edge;
  vertex p;
  edge_sub *edge_list=NULL;
  int nedges=0;
  int vert_idx = raw_model->num_vert;
  int face_idx = 0;
  int *done, *midpoint_idx;

  rings = (ring_info*)malloc(raw_model->num_vert*sizeof(ring_info));
  
  for (i=0; i<raw_model->num_vert; i++) {
    rings[i] = build_star2(raw_model, i);
#ifdef SUBDIV_DEBUG
    printf("Vertex %d : star_size = %d\n", i, rings[i].size);
    for (j=0; j<rings[i].size; j++)
      printf("number %d : %d\n", j, rings[i].ord_vert[j]);
#endif
    if (rings[i].type == 1) {
      free(rings);
      printf("Boundary vertex %d unsupported\n", i);
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
      edge_list[nedges-1].edge.v0 = i;
      edge_list[nedges-1].edge.v1 = rings[i].ord_vert[j];
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

  memcpy(subdiv_model->vertices, raw_model->vertices, 
	 raw_model->num_vert*sizeof(vertex));

  
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
  
/*   printf("face_idx = %d vert_idx = %d\n", face_idx, vert_idx); */
  free(edge_list);
  free(rings);
  free(done);
  free(midpoint_idx);
  return subdiv_model;
}

int main(int argc, char **argv) {
  model *raw_model, *sub_model;
  char *in_filename;
  char *out_filename;

  if (argc != 3) {
    fprintf(stderr, "Usage: subdiv infile outfile\n");
    exit(0);
  }
  in_filename = argv[1];
  out_filename = argv[2];
  
  raw_model = read_raw_model(in_filename);
  sub_model = subdiv(raw_model);
  sub_model->builtin_normals = 0;
  sub_model->normals = NULL;
  write_raw_model(sub_model, out_filename);
  free(sub_model->faces);
  free(sub_model->vertices);
  free(sub_model);  
  free(raw_model->faces);
  free(raw_model->vertices);
  free(raw_model);  
  return 0;
}


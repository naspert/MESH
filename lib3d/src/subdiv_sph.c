/* $Id: subdiv_sph.c,v 1.1 2001/04/27 14:02:02 aspert Exp $ */
#include <3dutils.h>

#ifdef EST_NORMALS
vertex *est_normals;
int n_idx = 0;
#endif

vertex compute_midpoint(ring_info *rings, int center, int v1, 
			model *raw_model) {

  int center2 = rings[center].ord_vert[v1];
  ring_info ring_op = rings[center2];
  int v2 = 0;
  vertex n,p, vj, dir, m, u, v, np1, np2, np;
  double r, ph, lambda, pl_off, nr, nph, dz, rp;

#ifdef EST_NORMALS
  double th0, th1, thn;
  vertex n0, n1, nm0, nm1;
  vertex p0p1, est_p_norm, est_p_offset;
#endif

  n = raw_model->normals[center];
  p = raw_model->vertices[center];
  vj = raw_model->vertices[rings[center].ord_vert[v1]];
  
  pl_off = -scalprod(p,n);

  dir.x = vj.x - p.x;
  dir.y = vj.y - p.y;
  dir.z = vj.z - p.z;
  
  r = norm(dir);
  
  lambda = -(pl_off + scalprod(vj, n));
  
  m.x = lambda*n.x;
  m.y = lambda*n.y;
  m.z = lambda*n.z;

  u.x = vj.x + m.x;
  u.y = vj.y + m.y;
  u.z = vj.z + m.z;

  v.x = u.x - p.x;
  v.y = u.y - p.y;
  v.z = u.z - p.z;

  if (lambda >= 0.0)
    ph = -atan(norm(m)/norm(v));
  else
    ph = atan(norm(m)/norm(v));
#ifdef EST_NORMALS
  th0 = ph; /* should be useful */
  n0 = n;
#endif

#ifdef _DEBUG
  printf("p[%d] = %f %f %f\n", center, p.x, p.y, p.z);
  printf("n[%d] = %f %f %f\n", center, n.x, n.y, n.z);
  printf("v[%d] = %f %f %f\n", rings[center].ord_vert[v1], vj.x, vj.y, vj.z);
  printf("pl_off = %f\n", pl_off);
  printf("r = %f\n", r);
  printf("phi = %f\n", ph*180.0/M_PI);
  printf("u.n + d = %f\n", scalprod(u,n)+pl_off);
  printf("test %f %f %f\n", norm(v), r*cos(ph), norm(v)-r*cos(ph));
#endif

  nr = 0.5*r;
  nph = 0.5*ph;

  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize(&v);
  np1.x = v.x*rp;
  np1.y = v.y*rp;
  np1.z = v.z*rp;

  np1.x += dz*n.x + p.x;
  np1.y += dz*n.y + p.y;
  np1.z += dz*n.z + p.z;

  while (ring_op.ord_vert[v2] != center)
      v2++;
  
  n = raw_model->normals[center2];
  p = raw_model->vertices[center2];
  vj = raw_model->vertices[ring_op.ord_vert[v2]];
  
  pl_off = -scalprod(p,n);

  dir.x = vj.x - p.x;
  dir.y = vj.y - p.y;
  dir.z = vj.z - p.z;
  
  r = norm(dir);
  
  lambda = -(pl_off + scalprod(vj, n));
  
  m.x = lambda*n.x;
  m.y = lambda*n.y;
  m.z = lambda*n.z;

  u.x = vj.x + m.x;
  u.y = vj.y + m.y;
  u.z = vj.z + m.z;

  v.x = u.x - p.x;
  v.y = u.y - p.y;
  v.z = u.z - p.z;

  if (lambda >= 0.0)
    ph = -atan(norm(m)/norm(v));
  else
    ph = atan(norm(m)/norm(v));

#ifdef EST_NORMALS
  th1 = ph;
  n1 = n;
#endif

#ifdef _DEBUG
  printf("p[%d] = %f %f %f\n", center, p.x, p.y, p.z);
  printf("n[%d] = %f %f %f\n", center, n.x, n.y, n.z);
  printf("v[%d] = %f %f %f\n", rings[center].ord_vert[v1], vj.x, vj.y, vj.z);
  printf("pl_off = %f\n", pl_off);
  printf("r = %f\n", r);
  printf("phi = %f\n", ph*180.0/M_PI);
  printf("u.n + d = %f\n", scalprod(u,n)+pl_off);
  printf("test %f %f %f\n", norm(v), r*cos(ph), norm(v)-r*cos(ph));
#endif

  nr = 0.5*r;
  nph = 0.5*ph;

  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize(&v);
  np2.x = v.x*rp;
  np2.y = v.y*rp;
  np2.z = v.z*rp;

  np2.x += dz*n.x + p.x;
  np2.y += dz*n.y + p.y;
  np2.z += dz*n.z + p.z;

/*   printf("np2 = %f %f %f\n", np2.x, np2.y, np2.z); */
  np.x = 0.5*(np1.x + np2.x);
  np.y = 0.5*(np1.y + np2.y);
  np.z = 0.5*(np1.z + np2.z);

#ifdef EST_NORMALS
/* compute the equation of the plane containing p0, p1 and n0 */
  p0p1.x = p1.x - p0.x;
  p0p1.y = p1.y - p0.y;
  p0p1.z = p1.z - p0.z;
  
  est_p_norm = crossprod(p0p1, n0);
  normalize(&est_p_norm);
  est_p_offset = -scalprod(p0, est_p_norm);
/* compute the equation of the plane containing p0, p1 and n1 */


#endif

  return np;
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
    rings[i] = build_star2(raw_model, i);
/*     printf("Vertex %d : star_size = %d\n", i, rings[i].size); */
/*     for (j=0; j<rings[i].size; j++) */
/*       printf("number %d : %d\n", j, rings[i].ord_vert[j]); */
/*     if (rings[i].type == 1) { */
/*       free(rings); */
/*       printf("Boundary vertex unsupported\n"); */
/*       return NULL; */
/*     } */
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
#ifdef EST_NORMALS
      edge_list[nedges-1].n = est_normals[n_idx];
      n_idx++;
#endif
      
/*       printf("final :%f %f %f\n", p.x, p.y, p.z); */
    }

  }

#ifdef EST_NORMALS
  free(est_normals);
#endif

  printf("%d edges found in model \n", nedges);

  subdiv_model = (model*)malloc(sizeof(model));
  subdiv_model->num_vert = raw_model->num_vert + nedges;
  subdiv_model->num_faces = 4*raw_model->num_faces;
  subdiv_model->faces = (face*)malloc(subdiv_model->num_faces*sizeof(face));
  subdiv_model->vertices = 
    (vertex*)malloc(subdiv_model->num_vert*sizeof(vertex));

  memcpy(subdiv_model->vertices, raw_model->vertices, 
	 raw_model->num_vert*sizeof(vertex));

#ifdef EST_NORMALS
  subdiv_model->est_normals = 
    (vertex*)malloc(subdiv_model->num_vert*sizeof(vertex));
  memcpy(subdiv_model->est_normals, raw_model->normals, 
	 raw_model->num_vert*sizeof(vertex));
#endif
  
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
#ifdef EST_NORMALS
	  subdiv_model->est_normals[vert_idx] = edge_list[i].n;
#endif
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
#ifdef EST_NORMALS
	  subdiv_model->est_normals[vert_idx] = edge_list[i].n;
#endif
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
#ifdef EST_NORMALS
	  subdiv_model->est_normals[vert_idx] = edge_list[i].n;
#endif
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
#ifdef COMP_SUB_NORMALS
  for (i=0; i<nedges; i++) {
    printf("edge: %d %d midpoint: %d\n",edge_list[i].edge.v0, 
	   edge_list[i].edge.v1, midpoint_idx[i]);
  }
#endif  
  printf("face_idx = %d vert_idx = %d\n", face_idx, vert_idx);
/*   free(edge_list); */
  free(rings);
  free(done);
/*   free(midpoint_idx); */
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
#ifdef COMP_SUB_NORMALS
  int i;
#endif

  if (argc != 3) {
    fprintf(stderr, "Usage: subdiv_sph infile outfile\n");
    exit(0);
  }
  infile = argv[1];
  outfile = argv[2];
  
  or_model = read_raw_model(infile);
  if (or_model->normals == NULL) {
    tmp_vert = (info_vertex*)malloc(or_model->num_vert*sizeof(info_vertex));
    or_model->area = (double*)malloc(or_model->num_faces*sizeof(double));
    or_model->face_normals = compute_face_normals(or_model);
    compute_vertex_normal(or_model, tmp_vert, or_model->face_normals);
    free(tmp_vert);
    free(or_model->area);
  }

#ifdef EST_NORMALS
  est_normals = (vertex*)malloc(raw_model->num_faces*sizeof(vertex));
  /* this should large enough ! */
#endif

/* performs the subdivision */
  sub_model = subdiv(or_model, &edge_list, &midpoint_idx, &num_edges);
  
/* */
  sub_model->builtin_normals = 0;
  sub_model->normals = NULL;

#ifdef COMP_SUB_NORMALS
  tmp_vert = (info_vertex*)malloc(sub_model->num_vert*sizeof(info_vertex));
  sub_model->area = (double*)malloc(sub_model->num_faces*sizeof(double));
  sub_model->face_normals = compute_face_normals(sub_model);
  compute_vertex_normal(sub_model, tmp_vert, sub_model->face_normals);
  free(tmp_vert);
  free(sub_model->area);
#endif


#ifdef COMP_SUB_NORMALS
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
#ifdef EST_NORMALS
    printf("est_n = %f\t%f\t%f\n", sub_model->est_n[midpoint_idx[i]].x,
	   sub_model->est_n[midpoint_idx[i]].y,
	   sub_model->est_n[midpoint_idx[i]].z);
#endif
  }
  free(midpoint_idx);
  free(edge_list);
#endif
  write_raw_model(sub_model, outfile);
  free(sub_model->faces);
  free(sub_model->vertices);

#ifdef COMP_SUB_NORMALS
  free(sub_model->normals);
#endif
#ifdef EST_NORMALS
  free(sub_model->est_normals);
#endif

  free(sub_model);  
  free(or_model->faces);
  free(or_model->vertices);
  free(or_model->face_normals);  
  free(or_model->normals);
  free(or_model);  
  return 0;
}

/* $Id: subdiv_sph.c,v 1.5 2001/09/27 12:53:42 aspert Exp $ */
#include <3dutils.h>

#undef EST_NORMALS
#ifdef EST_NORMALS
vertex_t *est_normals;
int n_idx = 0;
int use_est_normals = -1;
#endif

vertex_t compute_midpoint(struct ring_info *rings, int center, int v1, 
			  struct model *raw_model) {

  int center2 = rings[center].ord_vert[v1];
  struct ring_info ring_op = rings[center2];
  int v2 = 0;
  vertex_t n,p, vj, dir, m, u, v, np1, np2, np;
  double r, ph, lambda, pl_off, nr, nph, dz, rp, g;

#ifdef EST_NORMALS
  double th0, th1, tmp, est_p_offset;
  vertex_t n0, n1, nm0, nm1, tmp_norm_mp, p0, p1;
  vertex_t p0p1, est_p_norm;
  vertex_t b1;
#endif

  n = raw_model->normals[center];
  p = raw_model->vertices[center];
  vj = raw_model->vertices[rings[center].ord_vert[v1]];
  
  pl_off = -scalprod_v(&p, &n);

  substract_v(&vj, &p, &dir);
  
  r = norm_v(&dir);
  
  lambda = -(pl_off + scalprod_v(&vj, &n));
  
  prod_v(lambda, &n, &m);

  add_v(&vj, &m, &u);

  substract_v(&u, &p, &v);


  if (lambda >= 0.0)
    ph = -atan(norm_v(&m)/norm_v(&v));
  else
    ph = atan(norm_v(&m)/norm_v(&v));

#ifdef EST_NORMALS
  th0 = ph; /* should be useful */
  n0 = n;
  p0 = p;
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

  /* Compute the new position */
  nr = 0.5*r;
  if (ph < -M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 + 1.0)*(ph/M_PI_4 + 1.0));
    nph = g*ph;
  } else if (ph > M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 - 1.0)*(ph/M_PI_4 - 1.0));
    nph = g*ph;
  } else {
    nph = 0.5*ph; 
  }

  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize_v(&v);
  
  prod_v(rp, &v, &np1);
  
 
  np1.x += dz*n.x + p.x;
  np1.y += dz*n.y + p.y;
  np1.z += dz*n.z + p.z;

  while (ring_op.ord_vert[v2] != center)
      v2++;
  
  n = raw_model->normals[center2];
  p = raw_model->vertices[center2];
  vj = raw_model->vertices[ring_op.ord_vert[v2]];
  
  pl_off = -scalprod_v(&p, &n);
  
  substract_v(&vj, &p, &dir);
  

  
  r = norm_v(&dir);
  
  lambda = -(pl_off + scalprod_v(&vj, &n));
  
  prod_v(lambda, &n, &m);

  add_v(&vj, &m, &u);

  substract_v(&u, &p, &v);


  if (lambda >= 0.0)
    ph = -atan(norm_v(&m)/norm_v(&v));
  else
    ph = atan(norm_v(&m)/norm_v(&v));

#ifdef EST_NORMALS
  th1 = ph;
  n1 = n;
  p1 = p;
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
  if (ph < -M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 + 1.0)*(ph/M_PI_4 + 1.0));
    nph = g*ph;
  } else if (ph > M_PI_4) {
    g = 0.5*(1.0 + (ph/M_PI_4 - 1.0)*(ph/M_PI_4 - 1.0));
    nph = g*ph;
  } else {
    nph = 0.5*ph; 
  } 



  dz = nr*sin(nph);
  rp = nr*cos(nph);

  normalize_v(&v);
  prod_v(rp, &v, &np2);

  np2.x += dz*n.x + p.x;
  np2.y += dz*n.y + p.y;
  np2.z += dz*n.z + p.z;


  add_v(&np1, &np2, &np);
  prod_v(0.5, &np, &np);


#ifdef EST_NORMALS
  p0p1.x = p1.x - p0.x;
  p0p1.y = p1.y - p0.y;
  p0p1.z = p1.z - p0.z;

  /* compute the equation of the plane containing p0, p1 and n0 */  
  est_p_norm = crossprod(p0p1, n0);
  normalize(&est_p_norm);
  est_p_offset = -scalprod(p0, est_p_norm);
  /* the other basis vector of the plane is obtained through 
     Gram-Schmidt orthogonalization */
  tmp = scalprod(n0, p0p1);
  add_prod_v(-tmp, &n0, &p0p1m &b1);
  normalize(&b1);
  /* make sure the basis is direct */
  tmp = scalprod(est_p_norm, crossprod(b1, n0));
  if (tmp < -0.5) {
    b1.x = -b1.x;
    b1.y = -b1.y;
    b1.z = -b1.z;
  }
  /* Now the basis (b1, n0, est_p_norm) should be an orthn. direct basis */
  /* The expression of the estimated normal at the midpoint is *easy* */
  tmp_norm_mp.x = (sin(th0/2.0) + th0/2.0*cos(th0/2.0))/sqrt(1+th0*th0/4.0);
  tmp_norm_mp.y = (cos(th0/2.0) - th0/2.0*sin(th0/2.0))/sqrt(1+th0*th0/4.0);
  tmp_norm_mp.z = 0.0;

  nm0.x = b1.x*tmp_norm_mp.x + n0.x*tmp_norm_mp.y;
  nm0.y = b1.y*tmp_norm_mp.x + n0.y*tmp_norm_mp.y;
  nm0.z = b1.z*tmp_norm_mp.x + n0.z*tmp_norm_mp.y;

  /* compute the equation of the plane containing p0, p1 and n1 */
  est_p_norm = crossprod(p0p1, n1);
  normalize(&est_p_norm);
  est_p_offset = -scalprod(p0, est_p_norm);
  /* the other basis vector of the plane is obtained through 
     Gram-Schmidt orthogonalization */
  tmp = scalprod(n1, p0p1);
  add_prod_v(-tmp, &n1, &p0p1, &b1);
  normalize(&b1);
  /* make sure the basis is direct */
  tmp = scalprod(est_p_norm, crossprod(b1, n1));
  if (tmp < -0.5) {
    b1.x = -b1.x;
    b1.y = -b1.y;
    b1.z = -b1.z;
  }
  /* Now the basis (b1, n0, est_p_norm) should be an orthn. direct basis */
  /* The expression of the estimated normal at the midpoint is *easy* */
  tmp_norm_mp.x = (sin(th1/2.0) + th1/2.0*cos(th1/2.0))/sqrt(1+th1*th1/4.0);
  tmp_norm_mp.y = (cos(th1/2.0) - th1/2.0*sin(th1/2.0))/sqrt(1+th1*th1/4.0);
  tmp_norm_mp.z = 0.0;

  nm1.x = b1.x*tmp_norm_mp.x + n1.x*tmp_norm_mp.y;
  nm1.y = b1.y*tmp_norm_mp.x + n1.y*tmp_norm_mp.y;
  nm1.z = b1.z*tmp_norm_mp.x + n1.z*tmp_norm_mp.y;

  est_normals[n_idx].x = 0.5*(nm0.x + nm1.x);
  est_normals[n_idx].y = 0.5*(nm0.y + nm1.y);
  est_normals[n_idx].z = 0.5*(nm0.z + nm1.z);
  
  normalize(&(est_normals[n_idx]));
#endif

  return np;
}

struct model* subdiv(struct model *raw_model, struct edge_sub **edge_list_ptr, 
		     int **midpoint_idx_ptr, int *num_edges) {
  struct ring_info *rings;
  struct model *subdiv_model;
  int i, j;
  int v0, v1, v2;
  int u0=-1, u1=-1, u2=-1;
  struct edge_v edge;
  vertex_t p;
  struct edge_sub *edge_list=NULL; 
  int nedges = 0;
  int vert_idx = raw_model->num_vert;
  int face_idx = 0;
  int *done; /* *midpoint_idx; */
  int *midpoint_idx;

  rings = (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
  
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
      edge_list = (struct edge_sub*)realloc(edge_list, 
					    nedges*sizeof(struct edge_sub));
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
#ifdef EST_NORMALS
      edge_list[nedges-1].n = est_normals[n_idx];
      n_idx++;
#endif
      
    }

  }

#ifdef EST_NORMALS
  free(est_normals);
#endif

  printf("%d edges found in model \n", nedges);

  subdiv_model = (struct model*)malloc(sizeof(struct model));
  subdiv_model->num_vert = raw_model->num_vert + nedges;
  subdiv_model->num_faces = 4*raw_model->num_faces;
  subdiv_model->faces = (face_t*)
    malloc(subdiv_model->num_faces*sizeof(face_t));
  subdiv_model->vertices = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));

  memcpy(subdiv_model->vertices, raw_model->vertices, 
	 raw_model->num_vert*sizeof(vertex_t));

#ifdef EST_NORMALS
  subdiv_model->est_normals = 
    (vertex_t*)malloc(subdiv_model->num_vert*sizeof(vertex_t));
  memcpy(subdiv_model->est_normals, raw_model->normals, 
	 raw_model->num_vert*sizeof(vertex_t));
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
#ifdef COMP_SUB_NORMALS_DEBUG
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
  struct model *or_model, *sub_model;
  struct info_vertex* tmp_vert;
  int *midpoint_idx,  num_edges;
  struct edge_sub *edge_list;
#ifdef COMP_SUB_NORMALS_DEBUG
  int i;
#endif

#ifdef EST_NORMALS
  if (argc == 4) {
    infile = argv[2];
    outfile = argv[3];
    if (strcmp(argv[1], "--estimate-normals") == 0) {
      use_est_normals = 1;
    } else if (strcmp(argv[1], "--compute-normals") == 0) {
      use_est_normals = 0;
    } else {
      fprintf(stderr, "Usage: subdiv_sph [--estimate-normals, --compute-normals] infile outfile\n");
      exit(0);
    }
  } else {
    fprintf(stderr, "Usage: subdiv_sph [--estimate-normals, --compute-normals] infile outfile\n");
    exit(0);
  }
#else
  if (argc != 3) {
    fprintf(stderr, "Usage: subdiv_sph infile outfile\n");
    exit(0);
  }
  infile = argv[1];
  outfile = argv[2];
#endif

  or_model = read_raw_model(infile);
  if (or_model->normals == NULL) {
    tmp_vert = (struct info_vertex*)
      malloc(or_model->num_vert*sizeof(struct info_vertex));
    or_model->area = (double*)malloc(or_model->num_faces*sizeof(double));
    or_model->face_normals = compute_face_normals(or_model, tmp_vert);
    compute_vertex_normal(or_model, tmp_vert, or_model->face_normals);
    free(tmp_vert);
    free(or_model->area);
  }

#ifdef EST_NORMALS
  est_normals = (vertex_t*)malloc(3*or_model->num_faces*sizeof(vertex_t));
  /* this should large enough ! */
#endif

/* performs the subdivision */
  sub_model = subdiv(or_model, &edge_list, &midpoint_idx, &num_edges);
  
/* */
  sub_model->builtin_normals = 0;
  sub_model->normals = NULL;

#ifdef COMP_SUB_NORMALS
  tmp_vert = (struct info_vertex*)
    malloc(sub_model->num_vert*sizeof(struct info_vertex));
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
#ifdef EST_NORMALS
    printf("est_n = %f\t%f\t%f\n", sub_model->est_normals[midpoint_idx[i]].x,
	   sub_model->est_normals[midpoint_idx[i]].y,
	   sub_model->est_normals[midpoint_idx[i]].z);
    printf("n(ne) = %f\n", norm(sub_model->est_normals[midpoint_idx[i]]));
#endif
  }
  free(midpoint_idx);
  free(edge_list);
#endif

#ifdef EST_NORMALS
  if (use_est_normals == 1) {
    if (sub_model->normals != NULL) {
      free(sub_model->normals);
      free(sub_model->face_normals);
    }
    sub_model->normals = sub_model->est_normals;
  } else {
    if (sub_model->normals == NULL) {
      tmp_vert = (struct info_vertex*)
	malloc(sub_model->num_vert*sizeof(struct info_vertex));
      sub_model->area = (double*)malloc(sub_model->num_faces*sizeof(double));
      sub_model->face_normals = compute_face_normals(sub_model, tmp_vert);
      compute_vertex_normal(sub_model, tmp_vert, sub_model->face_normals);
      free(tmp_vert);
      free(sub_model->area);
    }
  }
#endif
  write_raw_model(sub_model, outfile);
  free(sub_model->faces);
  free(sub_model->vertices);


#ifdef EST_NORMALS
  free(sub_model->est_normals);
  free(sub_model->face_normals);
  free(sub_model->normals);
#endif

  free(sub_model);  
  free(or_model->faces);
  free(or_model->vertices);
  free(or_model->face_normals);  
  free(or_model->normals);
  free(or_model);  
  return 0;
}

/* $Id: butterfly.c,v 1.3 2001/09/24 11:59:28 aspert Exp $ */
#include <3dutils.h>

/* v0 & v1 are the indices in rings[center].ord_vert */
vertex compute_midpoint(ring_info *rings, int center,  int v1, 
			model *raw_model) {
  double *s, *t;
  double qt=0.0, qs=0.0;
  double w=1.0/16.0; /* This is a parameter for Butterfly subdivision */
  int j;
  vertex p, r;
  int n = rings[center].size;
  ring_info ring = rings[center];
  int center2 = ring.ord_vert[v1];
  ring_info ring_op = rings[center2]; /* center of opp ring */
  int m = ring_op.size; /* size of opp. ring */
  int v2 = 0; /* index of center vertex in opp. ring */

#ifdef _BOUNDARY_SUBDIV_DEBUG
  printf("Subdiv edge %d %d\n", center, center2);
  printf("n=%d m=%d\n", n, m);
#endif

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
      add_prod_v(s[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%n]]), &p, 
		 &p);

#ifdef SUBDIV_DEBUG
      printf("s[%d]=%f\n",j, s[j]);
      printf("v = %f %f %f\n", raw_model->vertices[ring.ord_vert[(v1+j)%n]].x,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].y,
	     raw_model->vertices[ring.ord_vert[(v1+j)%n]].z);
      printf("idx = %d\n", (v1+j)%n);
      printf("%d: p = %f %f %f\n", j,p.x, p.y, p.z);
#endif
    }

    add_prod_v(qs, &(raw_model->vertices[center]), &p, &p);


    
    /* Apply stencil to end vertex */
    for (j=0; j<m; j++) 
      add_prod_v(t[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%m]]), 
		 &r, &r);

    add_prod_v(qt, &(raw_model->vertices[center2]), &r, &r); 


    prod_v(0.5, &r, &r);
    prod_v(0.5, &p, &p);
    add_v(&p, &r, &p);

    
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
    for (j=0; j<6; j++) 
      add_prod_v(s[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%6]]), &p, 
		 &p);

    add_prod_v(qs, &(raw_model->vertices[center]), &p, &p);


    /* Apply stencil to end vertex */
    for (j=0; j<6; j++) 
      add_prod_v(s[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%6]]), &p, 
		 &p);

    add_prod_v(qs, &(raw_model->vertices[center2]), &p, &p);


    prod_v(0.5, &p, &p);


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

    for (j=0; j<n; j++) 
      add_prod_v(s[j], &(raw_model->vertices[ring.ord_vert[(v1+j)%n]]), &p, 
		 &p);

    add_prod_v(qs, &(raw_model->vertices[center]), &p, &p);

    free(s);
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

    for (j=0; j<m; j++) 
      add_prod_v(t[j], &(raw_model->vertices[ring_op.ord_vert[(v2+j)%m]]), &p,
		 &p);


    add_prod_v(qt, &(raw_model->vertices[center2]), &p, &p);

    free(t);
  } 
  
  return p;
}


vertex reg_interior_crease_sub(ring_info *rings, int center, int v1, 
			       int center2, model *raw_model) {

  vertex p;
  ring_info ring=rings[center], ring_op=rings[center2];
  int i;


  prod_v(0.375, &(raw_model->vertices[center2]), &p);
  
  p.x += 0.625*raw_model->vertices[center].x;
  p.y += 0.625*raw_model->vertices[center].y;
  p.z += 0.625*raw_model->vertices[center].z;
  
  if (ring.ord_vert[(v1+1)%6] == ring_op.ord_vert[0]) {    
    p.x += (raw_model->vertices[ring_op.ord_vert[0]].x - 
	    raw_model->vertices[ring_op.ord_vert[3]].x)/16.0;
    p.y += (raw_model->vertices[ring_op.ord_vert[0]].y - 
	    raw_model->vertices[ring_op.ord_vert[3]].y)/16.0;
    p.z += (raw_model->vertices[ring_op.ord_vert[0]].z - 
	    raw_model->vertices[ring_op.ord_vert[3]].z)/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].x/16.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].y/16.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].z/16.0;
    
    p.x += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].x/16.0;
    p.y += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].y/16.0;
    p.z += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].z/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].x/8.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].y/8.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].z/8.0;
    
    return p;
  } else if (ring.ord_vert[(v1+1)%6] == ring_op.ord_vert[3]) {
    
    p.x += (raw_model->vertices[ring_op.ord_vert[3]].x - 
	    raw_model->vertices[ring_op.ord_vert[0]].x)/16.0;
    p.y += (raw_model->vertices[ring_op.ord_vert[3]].y - 
	    raw_model->vertices[ring_op.ord_vert[0]].y)/16.0;
    p.z += (raw_model->vertices[ring_op.ord_vert[3]].z - 
	    raw_model->vertices[ring_op.ord_vert[0]].z)/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].x/16.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].y/16.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].z/16.0;
    
    p.x += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].x/16.0;
    p.y += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].y/16.0;
    p.z += 3.0*raw_model->vertices[ring.ord_vert[(v1-1)%6]].z/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].x/8.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].y/8.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1-2)%6]].z/8.0;
    
    return p;
  } else if (ring.ord_vert[(v1+5)%6] == ring_op.ord_vert[0]){
    
    p.x += (raw_model->vertices[ring_op.ord_vert[0]].x - 
	    raw_model->vertices[ring_op.ord_vert[3]].x)/16.0;

    p.y += (raw_model->vertices[ring_op.ord_vert[0]].y - 
	    raw_model->vertices[ring_op.ord_vert[3]].y)/16.0;

    p.z += (raw_model->vertices[ring_op.ord_vert[0]].z - 
	    raw_model->vertices[ring_op.ord_vert[3]].z)/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].x/16.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].y/16.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].z/16.0;
    
    p.x += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].x/16.0;
    p.y += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].y/16.0;
    p.z += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].z/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].x/8.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].y/8.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].z/8.0;
    
    return p;
  } else if (ring.ord_vert[(v1+5)%6] == ring_op.ord_vert[3]) {
    
    p.x += (raw_model->vertices[ring_op.ord_vert[3]].x - 
	    raw_model->vertices[ring_op.ord_vert[0]].x)/16.0;
    p.y += (raw_model->vertices[ring_op.ord_vert[3]].y - 
	    raw_model->vertices[ring_op.ord_vert[0]].y)/16.0;
    p.z += (raw_model->vertices[ring_op.ord_vert[3]].z - 
	    raw_model->vertices[ring_op.ord_vert[0]].z)/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].x/16.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].y/16.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+4)%6]].z/16.0;
    
    p.x += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].x/16.0;
    p.y += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].y/16.0;
    p.z += 3.0*raw_model->vertices[ring.ord_vert[(v1+1)%6]].z/16.0;
    
    p.x -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].x/8.0;
    p.y -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].y/8.0;
    p.z -= raw_model->vertices[ring.ord_vert[(v1+2)%6]].z/8.0;
    return p;
  } else {
    printf("Incorrect ??\n");
    printf("v1=%d %d %d\n", v1, (v1+1)%6, (v1+5)%6);
    printf("%d ring n=%d t=%d:\n", center, ring.size, ring.type);
    for (i=0; i<ring.size; i++) 
      printf("ring[%d] = %d\n", i, ring.ord_vert[i]);
    printf("%d ring_op n=%d t=%d:\n", center2, ring_op.size, ring_op.type);
    for (i=0; i<ring_op.size; i++) 
      printf("ring_op[%d] = %d\n", i, ring_op.ord_vert[i]);
    exit(-1);
    
  }

}

vertex four_point_subdiv(ring_info *rings, int center, int v1, int center2,
			 int v2, model *raw_model) {
  vertex p;
  double a=-1.0/16.0, b=9.0/16.0;
  int p0, p1, p2, p3;
  ring_info ring=rings[center], ring_op=rings[center2];
  int n=ring.size, m=ring_op.size;

  p1 = center;
  p2 = center2;
  
  /* ring.ord_vert[v1] = center2 */
  /* ring_op.ord_vert[v2] = center */     
  if (v1==0 && v2==0) { 
    p0 = ring.ord_vert[n-1];
    p3 = ring_op.ord_vert[m-1];
  } else if (v1==0 && v2==m-1) {
    p0 = ring.ord_vert[n-1];
    p3 = ring_op.ord_vert[0];
  } else if (v1==n-1 && v2==0) {
    p0 = ring.ord_vert[0];
    p3 = ring_op.ord_vert[m-1];
  } else {
    p0 = ring.ord_vert[0];
    p3 = ring_op.ord_vert[0];
  }

#ifdef BOUNDARY_SUBDIV_DEBUG
  printf("p0=%d p1=%d p2=%d p3=%d\n", p0, p1, p2, p3);
#endif
  
  p.x = a*(raw_model->vertices[p0].x + raw_model->vertices[p3].x) + 
    b*(raw_model->vertices[p1].x + raw_model->vertices[p2].x);
  p.y = a*(raw_model->vertices[p0].y + raw_model->vertices[p3].y) + 
    b*(raw_model->vertices[p1].y + raw_model->vertices[p2].y);
  p.z = a*(raw_model->vertices[p0].z + raw_model->vertices[p3].z) + 
    b*(raw_model->vertices[p1].z + raw_model->vertices[p2].z);
  return p;
}

vertex extr_crease_sub(ring_info ring, int center, int v1, model *raw_model) {
 double  *ci, c0, thk;
 int n=ring.size, j;
 vertex p;

#ifdef BOUNDARY_SUBDIV_DEBUG
 double sum=0.0;
#endif

 ci = (double*)malloc(n*sizeof(double));
 thk = M_PI/(double)(n-2);
 c0 = 1.0 - sin(thk)*sin(v1*thk)/((1.0 - cos(thk))*((double)(n-2)));
 
#ifdef BOUNDARY_SUBDIV_DEBUG
 sum += c0;
 printf("n=%d\tc0=%f\tthk=%f\n", n, c0, thk);
#endif

 ci[0] =  0.25*(cos(v1*thk) - 
   sin(2*thk)*sin(2*thk*v1)/((cos(thk)-cos(2*thk))*(n-2.0)));
 ci[n-1] = ci[0];

#ifdef BOUNDARY_SUBDIV_DEBUG
 sum += 2*ci[0];
 printf("c[0]=c[%d]=%f\n", n-1, ci[0]);
#endif

 p.x = c0*raw_model->vertices[center].x;
 p.y = c0*raw_model->vertices[center].y;
 p.z = c0*raw_model->vertices[center].z;

 p.x += ci[0]*raw_model->vertices[ring.ord_vert[0]].x;
 p.y += ci[0]*raw_model->vertices[ring.ord_vert[0]].y;
 p.z += ci[0]*raw_model->vertices[ring.ord_vert[0]].z;

 for (j=1; j<n-1; j++) {
   ci[j] = (sin(v1*thk)*sin(j*thk) + 
	    0.5*sin(2*v1*thk)*sin(2*j*thk))/(double)(n-1);

#ifdef BOUNDARY_SUBDIV_DEBUG
   sum += ci[j];
   printf("c[%d]=%f\n", j, ci[j]);
#endif

   p.x += ci[j]*raw_model->vertices[ring.ord_vert[j]].x;
   p.y += ci[j]*raw_model->vertices[ring.ord_vert[j]].y;
   p.z += ci[j]*raw_model->vertices[ring.ord_vert[j]].z;
 }

#ifdef BOUNDARY_SUBDIV_DEBUG
 printf("sum = %f\n", sum);
#endif

 p.x += ci[n-1]*raw_model->vertices[ring.ord_vert[n-1]].x;
 p.y += ci[n-1]*raw_model->vertices[ring.ord_vert[n-1]].y;
 p.z += ci[n-1]*raw_model->vertices[ring.ord_vert[n-1]].z;

 free(ci);
 return p;
 
}

vertex compute_midpoint_boundary(ring_info *rings, int center, int v1, 
				 model *raw_model) {

  ring_info ring=rings[center];
  int center2=ring.ord_vert[v1];
  ring_info ring_op=rings[center2]; /* center of opp. ring */
  int n=ring.size, m=ring_op.size; /* size of opp. ring */
  int v2 = 0; /* index of center vertex in opp. ring */
  vertex p, p2;
  int old_size=-1;

  /* find the edge in the opp. ring */
  while (ring_op.ord_vert[v2] != center)
    v2++;

  if (ring.type==1 && ring_op.type==1 && ((v1==0 && v2==0) ||
					  (v1==0 && v2==m-1) ||
					  (v1==n-1 && v2==0) ||
					  (v1==n-1 && v2==m-1))) {
    /* ************************ */
    /* apply the 4 point scheme */
    /* ************************ */

#ifdef BOUNDARY_SUBDIV_DEBUG
    printf("'4 point scheme %d %d\n", center, center2);
#endif
    return four_point_subdiv(rings, center, v1, center2, v2, raw_model);

  } else {				      
    if (ring.type == 0) { /* the current vertex is regular */
  /* as a consequence (see test in 'subdiv'), 'center2' is a boundary vertex */
      if (ring.size == 6) {
	/* the non-boundary vertex is regular */
	if (ring_op.size == 4) {
	  /* the boundary vertex is also regular i.e. valence=4 */
	  
	  /* **************************************** */
	  /* apply the 'regular interior-crease' rule */
	  /* **************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'regular interior %d -crease %d' rule\n", center, center2);
#endif
	  return reg_interior_crease_sub(rings, center, v1, center2, 
					 raw_model);
	} else {
	  /* the boundary vertex 'center2' is extr. */
	  
	  /* ************************************* */
	  /* apply the 'extraordinary crease' rule */
	  /* ************************************* */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'extr. crease' rule %d\n", center2);
#endif
	  return extr_crease_sub(ring_op, center2, v2, raw_model);
	}
      } else {
	/* the non-boundary vertex is extr. */
	if (ring_op.size == 4) {
	  /* the boundary vertex regular */
	  
	  /* *************************************** */
	  /* apply the 'interior extraordinary' rule */
	  /* *************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'extr. interior' rule %d\n", center);
#endif
	  /* trick to use the 'compute_midpoint' func. */
	  rings[center2].size = 6; 
	  rings[center2].ord_vert = (int*)realloc(rings[center2].ord_vert, 
						  6*sizeof(int));
	  rings[center2].ord_vert[4] = -1;
	  rings[center2].ord_vert[5] = -1; 
	  /* so that we don't have any surprises */

	  p = compute_midpoint(rings, center, v1, raw_model);
	  rings[center2].size = 4; /* set the size to the correct value */
	  ring_op = rings[center2];
	  return p;
	} else {
	  /* the boundary vertex is also extr. */
	  
	  /* ********************************************************** */
	  /* apply the average of 'extr. int.' and 'extr. crease' rules */
	  /* ********************************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("av. 'extr. interior %d - extr. crease %d' rule\n", 
		 center, center2);
#endif
	  p2 = extr_crease_sub(ring_op, center2, v2, raw_model);
	  old_size = ring_op.size;
	  if (old_size >= 6) {
	    rings[center2].size = 6;
	    p = compute_midpoint(rings, center, v1, raw_model);
	  } else if (old_size < 6) {
	    rings[center2].size = 6;
	    rings[center2].ord_vert = (int*)realloc(rings[center2].ord_vert, 
						    6*sizeof(int));
	    p = compute_midpoint(rings, center, v1, raw_model);
	  } 
	  rings[center2].size = old_size;
	  ring_op = rings[center2]; /* just to be sure ... */
	  p.x *= 0.5;
	  p.y *= 0.5;
	  p.z *= 0.5;
	  p.x += 0.5*p2.x;
	  p.y += 0.5*p2.y;
	  p.z += 0.5*p2.z;

	  return p;
	}
      }
    } else if (ring_op.type == 0) { 
      /* 'center' is regular but 'center2' is not */
      if (ring_op.size == 6) {
	/* the non-boundary vertex is regular */
	if (ring.size == 4) {
	  
	  /* **************************************** */
	  /* apply the 'regular interior-crease' rule */
	  /* **************************************** */
	  
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'regular interior %d -crease %d' rule\n", center2, center);
#endif
	  return reg_interior_crease_sub(rings, center2, v2, center, 
					 raw_model);
	} else {
	  
	  /* ************************************* */
	  /* apply the 'extraordinary crease' rule */
	  /* ************************************* */	  
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'extr. crease' rule %d\n", center);
#endif
	  return extr_crease_sub(ring, center, v1, raw_model);
	}
	
      } else {
	/* the non-boundary vertex is extr. */
	if (ring.size == 4) {
	  
	  /* *************************************** */
	  /* apply the 'interior extraordinary' rule */
	  /* *************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("'extr. interior' rule %d\n", center2);
#endif
	  /* trick to use the 'compute_midpoint' func. */
	  rings[center].size = 6; 
	  rings[center].ord_vert = (int*)realloc(rings[center].ord_vert, 
						 6*sizeof(int));
	  rings[center].ord_vert[4] = -1;
	  rings[center].ord_vert[5] = -1; 
	  /* so that we don't have any surprises */

	  p = compute_midpoint(rings, center2, v2, raw_model);
	  rings[center].size = 4;
	  ring = rings[center];
	  return p;
	} else {
	  
	  /* ********************************************************** */
	  /* apply the average of 'extr. int.' and 'extr. crease' rules */
	  /* ********************************************************** */
	  
#ifdef BOUNDARY_SUBDIV_DEBUG	
	  printf("av. 'extr. interior %d - extr. crease %d' rule\n", 
		 center2, center);
#endif
	  p2 = extr_crease_sub(ring, center, v1, raw_model);
	  old_size = ring.size;
	  if (old_size >= 6) {
	    rings[center].size = 6;
	    p = compute_midpoint(rings, center2, v2, raw_model);
	  } else if (old_size < 6) {
	    rings[center].size = 6;
	    rings[center].ord_vert = (int*)realloc(rings[center].ord_vert, 
						   6*sizeof(int));
	    p = compute_midpoint(rings, center2, v2, raw_model);
	  } 
	  rings[center].size = old_size;
	  ring = rings[center];
	  p.x *= 0.5;
	  p.y *= 0.5;
	  p.z *= 0.5;
	  p.x += 0.5*p2.x;
	  p.y += 0.5*p2.y;
	  p.z += 0.5*p2.z;

	  return p;
	}	
      }
    } else { 
      /* this edge connects 2 boundary vertices ... hmrgph */
      if (ring.size == 4 && ring_op.size == 4) {
	
	/* ************************************* */
	/* apply the 'crease-crease 1 or 2' rule */
	/* ************************************* */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	printf("'crease 1 or 2' rule %d %d\n", center, center2);
#endif
	exit(-1);
	
      } else if (ring.size != 4) {
	
	/* *************************************** */
	/* apply the 'extr. crease' rule on 'ring' */
	/* *************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	printf("'extr. crease' rule %d\n", center);
#endif
	return extr_crease_sub(ring, center, v1, raw_model);
      } else {
	
	/* ****************************************** */
	/* apply the 'extr. crease' rule on 'ring_op' */
	/* ****************************************** */
#ifdef BOUNDARY_SUBDIV_DEBUG	
	printf("'extr. crease' rule %d\n", center2);
#endif
	return extr_crease_sub(ring_op, center2, v2, raw_model);
      }
      
    }
  }
  /* if this is reached, then it sucks */
  printf("Where the hell are you son ?\n");
  p.x = 0.0;
  p.y = 0.0;
  p.z = 0.0;
  return p;
  
}

model* subdiv(model *raw_model) {
  ring_info *rings;
  model *subdiv_model;
  int i, j, k;
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
    build_star(raw_model, i, &(rings[i]));
#ifdef SUBDIV_DEBUG
    printf("Vertex %d : star_size = %d\n", i, rings[i].size);
    for (j=0; j<rings[i].size; j++)
      printf("number %d : %d\n", j, rings[i].ord_vert[j]);
#endif
/*     if (rings[i].type == 1) { */
/*       free(rings); */
/*       printf("Boundary vertex %d unsupported\n", i); */
/*       return NULL; */
/*     } */
  }
  
#ifdef SUBDIV_DEBUG
  for (i=0; i<raw_model->num_vert; i++) {
    for (j=0; j<rings[i].size; j++)
      printf("Vertex %d : type(ring[%d]) = %d %d\n", i, rings[i].ord_vert[j],
	     rings[rings[i].ord_vert[j]].type, rings[i].type);
  }
#endif

  for (i=0; i<raw_model->num_vert; i++) {
    for (j=0; j<rings[i].size; j++) {
      if (rings[i].ord_vert[j] < i) /* this edge is already subdivided */
	continue; 
      else if (rings[i].type == 0 && rings[rings[i].ord_vert[j]].type == 0) { 
	/* fully regular edge */
	p = compute_midpoint(rings, i, j, raw_model);
	nedges ++;
	edge_list = (edge_sub*)realloc(edge_list, nedges*sizeof(edge_sub));
	edge_list[nedges-1].edge.v0 = i;
	edge_list[nedges-1].edge.v1 = rings[i].ord_vert[j];
	edge_list[nedges-1].p = p;
      } else if (rings[i].type == 1 || 
		 rings[rings[i].ord_vert[j]].type == 1) { 
	/* A non-regular edge has been found. */
	/* Test whether it is a boundary or not */
	p = compute_midpoint_boundary(rings, i, j, raw_model);
	nedges ++;
	edge_list = (edge_sub*)realloc(edge_list, nedges*sizeof(edge_sub));
	edge_list[nedges-1].edge.v0 = i;
	edge_list[nedges-1].edge.v1 = rings[i].ord_vert[j];
	edge_list[nedges-1].p = p;
      } else {
	printf("Non-manifold neighborhood found for %d %d. Quitting\n", i, 
	       rings[i].ord_vert[j]);
	for (k=0; k<raw_model->num_vert; k++)
	  free(rings[k].ord_vert);
	free(rings);
	free(edge_list);
	exit(0);
      }
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
  for (i=0; i<raw_model->num_vert; i++)
    free(rings[i].ord_vert);
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


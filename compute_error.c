/* $Id: compute_error.c,v 1.17 2001/08/08 15:54:43 dsanta Exp $ */

#include <compute_error.h>

#include <geomutils.h>
#include <mutils.h>
#include <math.h>
#include <assert.h>

/* Ratio used to derive the cell size. It is the ratio between the cubic cell
 * side length and the side length of an average equilateral triangle. */
#define CELL_TRIAG_RATIO 1

/* Define inlining directive for C99 or as compiler specific C89 extension */
#if defined(__GNUC__) /* GCC's interpretation is inverse of C99 */
# define INLINE __inline__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define INLINE inline
#else
# define INLINE /* no inline */
#endif

/* --------------------------------------------------------------------------*
 *                       Local data types                                    *
 * --------------------------------------------------------------------------*/

/* A list of samples of a surface in 3D space. */
struct sample_list {
  vertex* sample; /* Array of sample 3D coordinates */
  int n_samples;  /* The number of samples in the array */
};

/* A list of cells */
struct cell_list {
  int *cell;   /* The array of the linear indices of the cells in the list */
  int n_cells; /* The number of elemnts in the array */
};

/* Storage for triangle sample errors. */
struct triag_sample_error {
  double **err;      /* Error array with 2D addressing. Sample (i,j) has the
                      * error stored at err[i][j], where i varies betwen 0 and
                      * n_samples-1 inclusive and j varies between 0 and
                      * n_samples-i-1 inclusive. */
  int n_samples;     /* The number of samples in each triangle direction */
  double *err_lin;   /* Error array with 1D adressing, which varies from 0 to
                      * n_samples_tot-1 inclusive. It refers to the same
                      * location as err, thus any change to err is reflected
                      * in err_lin and vice-versa. The order in the 1D array
                      * is all errors for i equal 0 and j from 0 to
                      * n_samples-1, followed by errors for i equal 1 and j
                      * from 1 to n_samples-2, and so on. */
  int n_samples_tot; /* The total number of samples in the triangle */
};

/* A list of triangles with their associated information */
struct triangle_list {
  struct triangle_info *triangles; /* The triangles */
  int n_triangles;                 /* The number of triangles */
  double area;                     /* The total triangle area */
};

/* A triangle and useful associated information. If a vertex of the triangle
 * has an angle of 90 degrees or more, that vertex is C. That way the
 * projection of C on AB is always inside AB. */
struct triangle_info {
  vertex a;            /* The A vertex of the triangle */
  vertex b;            /* The B vertex of the triangle */
  vertex c;            /* The C vertex of the triangle. The projection of C
                        * on AB is always inside the AB segment. */
  vertex ab;           /* The AB vector */
  vertex ac;           /* The AC vector */
  vertex bc;           /* The BC vector */
  double ab_len_sqr;   /* The square of the length of AB */
  double ac_len_sqr;   /* The square of the length of AC */
  double bc_len_sqr;   /* The square of the length of BC */
  double ab_1_len_sqr; /* One over the square of the length of AB */
  double ac_1_len_sqr; /* One over the square of the length of AC */
  double bc_1_len_sqr; /* One over the square of the length of BC */
  vertex d;            /* The perpendicular projection of C on AB. Always in
                        * the AB segment. */
  vertex dc;           /* The DC vector */
  double dc_1_len_sqr; /* One over the square of the length of DC */
  double da_1_max_coord; /* One over the max (in absolute value) coordinate of
                          * DA.  */
  double db_1_max_coord; /* One over the max (in absolute value) coordinate of
                          * DB */
  int da_max_c_idx;    /* The index of the coordinate corresponding to
                        * da_max_coord: 0 for X, 1 for Y, 2 for Z*/
  int db_max_c_idx;    /* The index of the coordinate corresponding to
                        * db_max_coord: 0 for X, 1 for Y, 2 for Z*/
  vertex normal;       /* The (unit length) normal of the ABC triangle
                        * (orinted with the right hand rule turning from AB to
                        * AC). */
};

/* --------------------------------------------------------------------------*
 *                    Local utility functions                                *
 * --------------------------------------------------------------------------*/

/* Returns, in vout, the negative of vector v. */
static INLINE void neg_v(const vertex *v, vertex *vout)
{
  vout->x = -v->x;
  vout->y = -v->y;
  vout->z = -v->z;
}

/* Compares integers *i0 and *i1. Returns -1 if *i0 < *i1, 0 if *i0 == *i1 and
 * 1 if *i0 > *i1. To be used with the qsort function of the standard C
 * library. */
static int intcmp(const void *i0, const void *i1)
{
  int *f0, *f1;

  f0 = (int*)i0;
  f1 = (int*)i1;

  if (*f0 == *f1)
    return 0;
  else if (*f0 < *f1)
    return -1;
  else
    return 1;
}

/* Reallocates the buffers of tse to store the sample errors for a triangle
 * sampling with n samples in each direction. If n is zero, the buffer is
 * freed. If tse->err is NULL a new buffer is allocated. If tse->n_samples
 * equals n nothing is done. The allocation never fails (if out of memory the
 * program is stopped, as with xrealloc()) */
static void realloc_triag_sample_error(struct triag_sample_error *tse, int n)
{
  int i;
  if (tse->n_samples == n) return;
  tse->n_samples = n;
  tse->n_samples_tot = n*(n+1)/2;
  /* Allocate everything in one chunk (faster and allows for 1D and 2D
   * addressing). */
  tse->err = xrealloc(tse->err,tse->n_samples_tot*sizeof(**(tse->err))+
                      n*sizeof(*(tse->err)));
  if (n != 0) {
    tse->err[0] = (double*) (tse->err+n);
    for (i=1; i<n; i++) {
      tse->err[i] = tse->err[i-1]+(n-(i-1));
    }
    tse->err_lin = tse->err[0];
  } else {
    tse->err_lin = NULL;
  }
}

/* --------------------------------------------------------------------------*
 *                            Local functions                                *
 * --------------------------------------------------------------------------*/

/* Initializes the triangle '*t' using the '*a' '*b' and '*c' vertices and
 * calculates all the relative fields of the struct. */
static void init_triangle(const vertex *a, const vertex *b, const vertex *c,
                          struct triangle_info *t)
{
  vertex ab,ac,bc;
  vertex da,db;

  /* Get the vertices in the proper ordering (the orientation is not
   * changed). C should be the vertex with an angle of 90 degrees or more, if
   * there is one. */
  substract_v(b,a,&ab);
  substract_v(c,a,&ac);
  substract_v(c,b,&bc);
  if (scalprod_v(&ab,&ac) <= 0) { /* A has angle of 90 or more => A to C */
    t->c = *a;
    t->a = *b;
    t->b = *c;
    t->ab = bc;
    neg_v(&ab,&(t->ac));
    neg_v(&ac,&(t->bc));
  } else {
    if (scalprod_v(&ab,&bc) >= 0) { /* B has angle of 90 or more => B to C */
      t->b = *a;
      t->c = *b;
      t->a = *c;
      neg_v(&ac,&(t->ab));
      neg_v(&bc,&(t->ac));
      t->bc = ab;
    } else { /* no modifications needed, C can remain C */
      t->a = *a;
      t->b = *b;
      t->c = *c;
      t->ab = ab;
      t->ac = ac;
      t->bc = bc;
    }
  }
  /* Get side lengths */
  t->ab_len_sqr = norm2_v(&(t->ab));
  t->ac_len_sqr = norm2_v(&(t->ac));
  t->bc_len_sqr = norm2_v(&(t->bc));
  t->ab_1_len_sqr = 1/t->ab_len_sqr;
  t->ac_1_len_sqr = 1/t->ac_len_sqr;
  t->bc_1_len_sqr = 1/t->bc_len_sqr;
  /* Get D, projection of C on AB */
  prod_v(-scalprod_v(&(t->ac),&(t->ab))*t->ab_1_len_sqr,&(t->ab),&da);
  substract_v(&(t->a),&da,&(t->d));
  add_v(&da,&(t->ab),&db);
  add_v(&da,&(t->ac),&(t->dc));
  /* Get the D relative lengths */
  t->dc_1_len_sqr = 1/norm2_v(&(t->dc));
  /* Get max coords of DA and DB */
  if (fabs(da.x) > fabs(da.y)) {
    if (fabs(da.x) >= fabs(da.z)) {
      t->da_1_max_coord = 1/da.x;
      t->da_max_c_idx = 0;
    } else {
      t->da_1_max_coord = 1/da.z;
      t->da_max_c_idx = 2;
    }
  } else {
    if (fabs(da.y) >= fabs(da.z)) {
      t->da_1_max_coord = 1/da.y;
      t->da_max_c_idx = 1;
    } else {
      t->da_1_max_coord = 1/da.z;
      t->da_max_c_idx = 2;
    }
  }
  if (fabs(db.x) > fabs(db.y)) {
    if (fabs(db.x) >= fabs(db.z)) {
      t->db_1_max_coord = 1/db.x;
      t->db_max_c_idx = 0;
    } else {
      t->db_1_max_coord = 1/db.z;
      t->db_max_c_idx = 2;
    }
  } else {
    if (fabs(db.y) >= fabs(db.z)) {
      t->db_1_max_coord = 1/db.y;
      t->db_max_c_idx = 1;
    } else {
      t->db_1_max_coord = 1/db.z;
      t->db_max_c_idx = 2;
    }
  }
  /* Get the triangle normal (normalized) */
  crossprod_v(&(t->ab),&(t->ac),&(t->normal));
  normalize_v(&(t->normal));
}

/* Compute the square of the distance between point 'p' and triangle 't' in 3D
 * space. The distance from a point p to a triangle is defined as the
 * Euclidean distance from p to the closest point in the triangle. */
static double dist_sqr_pt_triag(const struct triangle_info *t, const vertex *p)
{
  double dpp;             /* (signed) distance point to ABC plane */
  double l,m_adc,m_bdc;   /* l amd m triangle parametrization veraibles */
  double dq_dc,ap_ab,ap_ac,bp_bc; /* scalra products */
  vertex q;               /* projection of p on ABC plane */
  double res[3];          /* residue of AQ, parallel to AB */
  vertex dq,ap,bp,cp;     /* Point to point vectors */
  double dmin_sqr;        /* minimum distance squared */
  
  /* Get Q: projection of point on ABC plane */
  substract_v(p,&(t->a),&ap);

  dpp = scalprod_v(&(t->normal),&ap); /* signed distance from p to ABC plane */
  prod_v(dpp,&(t->normal),&q);
  substract_v(p,&q,&q);

  /* Is Q inside ABC (including border) ? */
  /* NOTE: the triangle ABC is decomposed into triangles ADC and BDC and the
   * point is tested on each of these. Since D always belongs to the AB
   * segment, if the point is in any of those triangles, it is in ABC. ADC and
   * BDC have square angles at D, and thus testing the point Q for being in or
   * out is rather easy. We use the fact that Q belongs to ADC, if and only if
   * l>=0, m>=0 and l+m<=1, where DQ = l*DC+m*DA. To obtain l we use a
   * perpendicular projection of Q on DC. We proceed analogously for the BDC
   * triangle. */

  /* Get projection on DC and l */
  substract_v(&q,&(t->d),&dq);
  dq_dc = scalprod_v(&dq,&(t->dc));
  if (dq_dc < 0) { /* AB is closest to Q and thus to P */
    ap_ab = scalprod_v(&ap,&(t->ab));
    if(ap_ab > 0) {
      if (ap_ab < t->ab_len_sqr) { /* projection of P on AB is in AB */
        dmin_sqr = norm2_v(&ap) - (ap_ab*ap_ab)*t->ab_1_len_sqr;
        if (dmin_sqr < 0) dmin_sqr = 0; /* correct rounding problems */
      } else { /* B is closer */
        substract_v(p,&(t->b),&bp);
        dmin_sqr = norm2_v(&bp);
      }
    } else { /* A is closer */
      dmin_sqr = norm2_v(&ap);
    }
  } else { /* continue testing if Q belongs to ABC */
    l = dq_dc*t->dc_1_len_sqr; /* normalize l relative to the length DC */
    /* Get residue parallel to DA and DB */
    res[0] = dq.x-l*t->dc.x;
    res[1] = dq.y-l*t->dc.y;
    res[2] = dq.z-l*t->dc.z;
    /* Get m for ADC. We use the maximum coordinate of AD to avoid division by
     * zero or very small numbers. */
    m_adc = res[t->da_max_c_idx]*t->da_1_max_coord;
    if (m_adc >= 0) { /* Q on same side of DC as A */
      if (l+m_adc <= 1) {
        /* Q inside ADC and thus ABC, distance to ABC is distance to plane */
        dmin_sqr = dpp*dpp;
      } else { /* AC is closest to Q and thus to P */
        ap_ac = scalprod_v(&ap,&(t->ac));
        if(ap_ac > 0) {
          if (ap_ac < t->ac_len_sqr) { /* projection of P on AC is in AC */
            dmin_sqr = norm2_v(&ap) - (ap_ac*ap_ac)*t->ac_1_len_sqr;
            if (dmin_sqr < 0) dmin_sqr = 0; /* correct rounding problems */
          } else { /* C is closer */
            substract_v(p,&(t->c),&cp);
            dmin_sqr = norm2_v(&cp);
          }
        } else { /* A is closer */
          dmin_sqr = norm2_v(&ap);
        }
      }
    } else { /* Q on same side of DC as B, we need to test BDC */
      m_bdc = res[t->db_max_c_idx]*t->db_1_max_coord;
      /* m_bdc is always positive (up to the rounding precision), since m_adc
       * is negative. */
      if (l+m_bdc <= 1) {
        /* Q inside BDC and thus ABC, distance to ABC is distance to plane */
        dmin_sqr = dpp*dpp;
      } else { /* BC is closest to Q and thus to P */
        substract_v(p,&(t->b),&bp);
        bp_bc = scalprod_v(&bp,&(t->bc));
        if(bp_bc > 0) {
          if (bp_bc < t->bc_len_sqr) { /* projection of P on BC is in BC */
            dmin_sqr = norm2_v(&bp) - (bp_bc*bp_bc)*t->bc_1_len_sqr;
            if (dmin_sqr < 0) dmin_sqr = 0; /* correct rounding problems */
          } else { /* C is closer */
            substract_v(p,&(t->c),&cp);
            dmin_sqr = norm2_v(&cp);
          }
        } else { /* B is closer */
          dmin_sqr = norm2_v(&bp);
        }
      }
    }
  }

  return dmin_sqr;
}

/* Calculates the square of the distance between a point p in cell
 * (gr_x,gr_y,gr_z) and cell (m,n,o). The coordinates of p are relative to the
 * minimum X,Y,Z coordinates of the bounding box from where the cell grid is
 * derived. All the cells are cubic, with a side of length cell_sz. If the
 * point p is in the cell (m,n,o) the distance is zero. */
static double dist_sqr_pt_cell(const vertex *p, int gr_x, int gr_y, int gr_z,
                               int m, int n, int o, double cell_sz)
{
  double d2,tmp;

  d2 = 0;
  if (gr_x != m) { /* if not on same cell x wise */
    tmp = (m > gr_x) ? m*cell_sz-p->x : p->x-(m+1)*cell_sz;
    d2 += tmp*tmp;
  }
  if (gr_y != n) { /* if not on same cell y wise */
    tmp = (n > gr_y) ? n*cell_sz-p->y : p->y-(n+1)*cell_sz;
    d2 += tmp*tmp;
  }
  if (gr_z != o) { /* if not on same cell z wise */
    tmp = (o > gr_z) ? o*cell_sz-p->z : p->z-(o+1)*cell_sz;
    d2 += tmp*tmp;
  }
  return d2;
}

/* Convert the triangular model m to a triangle list (without connectivity
 * information) with the associated information. All the information about the
 * triangles (i.e. fields of struct triangle_info) is computed. */
static struct triangle_list* model_to_triangle_list(const model *m)
{
  int i,n;
  struct triangle_list *tl;
  struct triangle_info *triags;
  face *face_i;

  /* Initialize and allocate storage */
  n = m->num_faces;
  tl = xmalloc(sizeof(*tl));
  tl->n_triangles = n;
  triags = xmalloc(sizeof(*tl->triangles)*n);
  tl->triangles = triags;
  tl->area = 0;

  /* Convert triangles and update global data */
  for (i=0; i<n; i++) {
    face_i = &(m->faces[i]);
    init_triangle(&(m->vertices[face_i->f0]),&(m->vertices[face_i->f1]),
                  &(m->vertices[face_i->f2]),&(triags[i]));
    tl->area += sqrt(triags[i].ab_len_sqr*norm2_v(&(triags[i].dc)))/2;
  }
  return tl;
}

/* Calculates the statistics of the error samples in tse. For each triangle
 * formed by neighboring samples the error at the vertices is averaged to
 * obtain a single error for the sample triangle. The overall mean error is
 * obtained by calculating the mean of the errors of the sample triangles. The
 * other statistics are obtained analogously. Note that all sample triangles
 * have exactly the same area, and thus the calculation is independent of the
 * triangle shape. */
static void error_stat_triag(const struct triag_sample_error *tse,
                             struct face_error *fe)
{
  int n,i,j,imax,jmax;
  double err_local;
  double err_min, err_max, err_tot, err_sqr_tot;
  double **s_err;

  err_min = DBL_MAX;
  err_max = 0;
  err_tot = 0;
  err_sqr_tot = 0;
  n = tse->n_samples;
  s_err = tse->err;
  /* Do sample triangles for which the point (i,j) is closer to the point
   * (0,0) than the side of the sample triangle opposite (i,j). There are
   * (n-1)*n/2 of these. */
  for (i=0, imax=n-1; i<imax; i++) {
    for (j=0, jmax=imax-i; j<jmax; j++) {
      err_local = s_err[i][j]+s_err[i][j+1]+s_err[i+1][j];
      err_tot += err_local;
      err_sqr_tot += err_local*err_local;
    }
  }
  /* Do the other triangles. There are (n-2)*(n-1)/2 of these. */
  for (i=1; i<n; i++) {
    for (j=1, jmax=n-i; j<jmax; j++) {
      err_local = s_err[i-1][j]+s_err[i][j-1]+s_err[i][j];
      err_tot += err_local;
      err_sqr_tot += err_local*err_local;
    }
  }
  /* Get min max */
  for (i=0; i<tse->n_samples_tot; i++) {
    err_local = tse->err_lin[i];
    if (err_min > err_local) err_min = err_local;
    if (err_max < err_local) err_max = err_local;
  }
  /* Finalize error measures */
  fe->min_error = err_min;
  fe->max_error = err_max;
  fe->mean_error = err_tot/(((n-1)*n/2+(n-2)*(n-1)/2)*3);
  fe->mean_sqr_error = err_sqr_tot/(((n-1)*n/2+(n-2)*(n-1)/2)*3);
}

/* Samples a triangle (a,b,c) using n samples in each direction. The sample
 * points are returned in the sample_list s. The dynamic array 's->sample' is
 * realloc'ed to the correct size (if no storage has been previously allocated
 * it should be NULL). The total number of samples is n*(n+1)/2. The order for
 * samples (i,j) in s->sample is all samples for i equal 0 and j from 0 to n-1,
 * followed by all samples for i equal 1 and j from 0 to n-2, and so on, where
 * i and j are the sampling indices along the ab and ac sides, respectively. */
static void sample_triangle(const vertex *a, const vertex *b, const vertex *c,
                            int n, struct sample_list* s)
{
  vertex u,v;     /* basis parametrization vectors */
  vertex a_cache; /* local (on stack) copy of a for faster access */
  int i,j,maxj,k; /* counters and limits */

  /* initialize */
  a_cache = *a;
  s->n_samples = n*(n+1)/2;
  s->sample = xrealloc(s->sample,sizeof(vertex)*s->n_samples);
  /* get basis vectors */
  substract_v(b,a,&u);
  substract_v(c,a,&v);
  prod_v(1/(double)(n-1),&u,&u);
  prod_v(1/(double)(n-1),&v,&v);
  /* Sample triangle */
  for (k = 0, i = 0; i < n; i++) {
    for (j = 0, maxj = n-i; j < maxj; j++) {
      s->sample[k].x = a_cache.x+i*u.x+j*v.x;
      s->sample[k].y = a_cache.y+i*u.y+j*v.y;
      s->sample[k++].z = a_cache.z+i*u.z+j*v.z;
    }
  }
}

/* Returns the list of cells that intersect each triangle. The returned object
 * is an array of length tl->n_triangles, where each element i contains the list
 * of cells intersecting triangle i. The returned array is malloc'ed, as well
 * as the array of each cell list. */
static struct cell_list *cells_in_triangles(struct triangle_list *tl,
                                            struct size3d grid_sz,
                                            double cell_sz, vertex bbox_min)
{
  struct cell_list *cl;       /* the cell list to return */
  int h,i,j;                  /* counters */
  int m,n,o;                  /* 3D cell indices for samples */
  int cell_idx,cell_idx_prev; /* linear (1D) cell indices */
  int n_cells;                /* number of cells */
  struct sample_list sl;      /* samples from a triangle */
  int *tmp;                   /* temp storage for cell list */
  int cell_stride_z;          /* spacement for Z index in 3D addressing of
                               * cell list */
  int m_a,n_a,o_a,m_b,n_b,o_b,m_c,n_c,o_c; /* 3D cell indices for vertices */
  int tmpi,max_cell_dist;     /* maximum cell distance along any axis */
  int n_samples;              /* number of samples to use for triangles */

  /* Initialize */
  cell_stride_z = grid_sz.x*grid_sz.y;
  cl= xmalloc((tl->n_triangles)*sizeof(*cl));
  tmp = NULL;
  sl.sample = NULL;

  /* Get intersecting cells for each triangle */
  for(i=0;i<tl->n_triangles;i++){
    h = 0;
    cl[i].cell = NULL;

    /* Get the cells in which the triangle vertices are */
    m_a = (int)floor((tl->triangles[i].a.x-bbox_min.x)/cell_sz);
    n_a = (int)floor((tl->triangles[i].a.y-bbox_min.y)/cell_sz);
    o_a = (int)floor((tl->triangles[i].a.z-bbox_min.z)/cell_sz);
    m_b = (int)floor((tl->triangles[i].b.x-bbox_min.x)/cell_sz);
    n_b = (int)floor((tl->triangles[i].b.y-bbox_min.y)/cell_sz);
    o_b = (int)floor((tl->triangles[i].b.z-bbox_min.z)/cell_sz);
    m_c = (int)floor((tl->triangles[i].c.x-bbox_min.x)/cell_sz);
    n_c = (int)floor((tl->triangles[i].c.y-bbox_min.y)/cell_sz);
    o_c = (int)floor((tl->triangles[i].c.z-bbox_min.z)/cell_sz);

    if (m_a == m_b && m_a == m_c && n_a == n_b && n_a == n_c &&
        o_a == o_b && o_a == o_c) {
      /* The ABC triangle fits entirely into one cell => fast case */
      cl[i].cell = xmalloc(sizeof(*(cl->cell))*1);
      cl[i].cell[0] = m_a+n_a*grid_sz.x+o_a*cell_stride_z;
      cl[i].n_cells = 1;
      continue;
    }

    /* Triangle does not fit in one cell, how many cells does the triangle
     * span ? */
    max_cell_dist = abs(m_a-m_b);
    if ((tmpi = abs(m_a-m_c)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(m_b-m_c)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(n_a-n_b)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(n_a-n_c)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(n_b-n_c)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(o_a-o_b)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(o_a-o_c)) > max_cell_dist) max_cell_dist = tmpi;
    if ((tmpi = abs(o_b-o_c)) > max_cell_dist) max_cell_dist = tmpi;
    /* Sample the triangle so as to have twice the samples in any direction
     * than the number of cells spanned in that direction. */
    n_samples = 2*(max_cell_dist+1);
    sample_triangle(&(tl->triangles[i].a),&(tl->triangles[i].b),
                    &(tl->triangles[i].c),n_samples,&sl);
    /* Get the intersecting cells from the samples */
    cell_idx_prev = -1;
    for(j=0;j<sl.n_samples;j++){
      /* Get cell in which the sample is. Due to rounding in the triangle
       * sampling process we check the indices to be within bounds. */
      m=(int)floor((sl.sample[j].x-bbox_min.x)/cell_sz);
      if(m >= grid_sz.x) {
        m = grid_sz.x - 1;
      } else if (m < 0) {
        m = 0;
      }
      n=(int)floor((sl.sample[j].y-bbox_min.y)/cell_sz);
      if (n >= grid_sz.y) {
        n = grid_sz.y - 1;
      } else if (n < 0) {
        n = 0;
      }
      o=(int)floor((sl.sample[j].z-bbox_min.z)/cell_sz);
      if (o >= grid_sz.z) {
        o = grid_sz.z - 1;
      } else if (o < 0) {
        o = 0;
      }
      cell_idx = m + n*grid_sz.x + o*cell_stride_z;

      /* Include cell index in list only if not the same as previous one
       * (avoid too many duplicates). */
      if (cell_idx != cell_idx_prev) {
        tmp = (int *)xrealloc(tmp, (h+1)*sizeof(int));
        tmp[h++] = cell_idx;
        cell_idx_prev = cell_idx;
      }
    }

    /* Remove duplicate cell indices, sorting makes it easier and faster to
     * remove them. */
    qsort(tmp, h, sizeof(int), intcmp);
    cell_idx_prev = -1;
    n_cells = 0;
    for (j=0; j<h; j++) {
      if (tmp[j] != cell_idx_prev) {
        cl[i].cell = (int*)xrealloc(cl[i].cell,(n_cells+1)*sizeof(*(cl->cell)));
        cl[i].cell[n_cells++] = tmp[j];
        cell_idx_prev = tmp[j];
      }
    }
    cl[i].n_cells = n_cells;

  }
  free(sl.sample);
  free(tmp);
 
  return cl;
}

/* Returns the list of triangle indices that intersect a cell, for each cell
 * in the grid. The object returned is an array, where element i is the list
 * of triangle indices that intersect the cell with linear index i. Each list
 * is terminated -1. The returned array and subarrays are malloc'ed
 * independently. */
static int** triangles_in_cells(const struct cell_list *cl,
                                const struct triangle_list *tl,
                                const struct size3d grid_sz)
{

  int i,j,k,maxi,maxj,maxk; /* counters and limits */
  int **tab; /* Table containing the indices of intersecting triangles for
              * each cell. */
  int *nt;   /* Array with the number of intersecting triangles found so far
              * for each cell */

  nt = xcalloc(grid_sz.x*grid_sz.y*grid_sz.z,sizeof(*nt));
  tab = xcalloc(grid_sz.x*grid_sz.y*grid_sz.z,sizeof(*tab));

  /* Include each triangle in the intersecting cells */
  for (j=0, maxj=tl->n_triangles; j<maxj; j++) {
    for (k=0, maxk=cl[j].n_cells; k<maxk; k++) {
      i = cl[j].cell[k];
      tab[i] = (int*) xrealloc(tab[i],(nt[i]+1)*sizeof(*tab));
      tab[i][nt[i]++] = j;
    }
  }
  /* Mark the end of the list for each cell with -1 */
  for(i=0, maxi=grid_sz.x*grid_sz.y*grid_sz.z; i<maxi; i++){
    tab[i] = (int*) xrealloc(tab[i],(nt[i]+1)*sizeof(*tab));
    tab[i][nt[i]] = -1;
  }
  free(nt);
  return tab;
}

/* Returns the distance from point p to the surface defined by the triangle
 * list tl. The distance from a point to a surface is defined as the distance
 * from a point to the closets point on the surface. To speed up the search
 * for the closest triangle in the surface the bounding box of the model is
 * subdivided in cubic cells. The list of triangles that intersect each cell
 * is given by faces_in_cell, as returned by the triangles_in_cells()
 * function. The side of the cubic cells is of length cell_sz, and there are
 * (grid_sz.x,grid_sz.y,grid_sz.z) cells in teh X,Y,Z directions. Cell (0,0,0)
 * starts at bbox_min, which is the minimum coordinates of the (axis aligned)
 * bounding box. Static storage is used by this function, so it is not
 * reentrant (i.e. thread safe). */
static double dist_pt_surf(vertex p, const struct triangle_list *tl,
                           int **faces_in_cell, struct size3d grid_sz,
                           double cell_sz, vertex bbox_min)
{
  vertex p_rel;         /* coordinates of p relative to bbox_min */
  struct size3d grid_coord; /* coordinates of cell in which p is */
  int k;                /* cell index distance of current scan */
  int m,n,o;            /* 3D cell indices */
  double dmin_sqr;      /* minimum distance squared */
  double dist_sqr;      /* current distance squared */
  int tfcl_idx;         /* triangle index in faces in cell list */
  int t_idx;            /* triangle index in triangle list */
  int dmin_update;      /* flag to signal update of dmin_sqr */
  int min_m,max_m,min_n,max_n,min_o,max_o; /* cell indices loop limits */
  int cell_stride_z;    /* spacement for Z index in 3D addressing of cell
                         * list */
  int *cell_tl;         /* list of triangles intersecting the current cell */
  struct triangle_info *triags; /* local pointer to triangle array */
  static int *cell_list_m; /* list of cells to scan for the current k */
  static int *cell_list_n;
  static int *cell_list_o;
  static int cell_list_sz; /* size of cell_list_{m,n,o} */
  int cll;              /* length of cell list */
  int j;                /* counter */

  /* Reusing the buffers from call to call gives significant speedup. */
  if (cell_list_m == NULL) {
    cell_list_sz = grid_sz.x*grid_sz.y*grid_sz.z;
    cell_list_m = xmalloc(sizeof(*cell_list_m)*cell_list_sz);
    cell_list_n = xmalloc(sizeof(*cell_list_n)*cell_list_sz);
    cell_list_o = xmalloc(sizeof(*cell_list_o)*cell_list_sz);
  } else if (cell_list_sz < grid_sz.x*grid_sz.y*grid_sz.z) {
    cell_list_sz = grid_sz.x*grid_sz.y*grid_sz.z;
    cell_list_m = xrealloc(cell_list_m,sizeof(*cell_list_m)*cell_list_sz);
    cell_list_n = xrealloc(cell_list_n,sizeof(*cell_list_n)*cell_list_sz);
    cell_list_o = xrealloc(cell_list_o,sizeof(*cell_list_o)*cell_list_sz);
  }

  /* NOTE: tests have shown it is faster to scan each triangle, even
   * repeteadly, than to track which triangles have been scanned (too much
   * time spent initializing tracking info to zero). */

  cell_stride_z = grid_sz.y*grid_sz.x;
  triags = tl->triangles;
  /* Get relative coordinates of point */
  substract_v(&p,&bbox_min,&p_rel);
  /* Get the cell coordinates of where point is. Since the bounding box bbox
   * is that of the model 2, the grid coordinates can be out of bounds (in
   * which case we limit them) */
  grid_coord.x = floor(p_rel.x/cell_sz);
  if (grid_coord.x < 0) {
    grid_coord.x = 0;
  } else if (grid_coord.x >= grid_sz.x) {
    grid_coord.x = grid_sz.x-1;
  }
  grid_coord.y = floor(p_rel.y/cell_sz);
  if (grid_coord.y < 0) {
    grid_coord.y = 0;
  } else if (grid_coord.y >= grid_sz.y) {
    grid_coord.y = grid_sz.y-1;
  }
  grid_coord.z = floor(p_rel.z/cell_sz);
  if (grid_coord.z < 0) {
    grid_coord.z = 0;
  } else if (grid_coord.z >= grid_sz.z) {
    grid_coord.z = grid_sz.z-1;
  }

  /* Scan cells, at sequentially increasing index distance k */
  k = 0;
  dmin_sqr = DBL_MAX;
  do {
    dmin_update = 0;
    /* Get the list of cells at distance k in X Y or Z direction, which has
     * not been previously tested. */
    if (k == 0) {
      cll = 1;
      cell_list_m[0] = grid_coord.x;
      cell_list_n[0] = grid_coord.y;
      cell_list_o[0] = grid_coord.z;
    } else {
      cll = 0;
      min_m = max(grid_coord.x-k,0);
      max_m = min(grid_coord.x+k,grid_sz.x-1);
      min_n = max(grid_coord.y-k,0);
      max_n = min(grid_coord.y+k,grid_sz.y-1);
      min_o = max(grid_coord.z-k,0);
      max_o = min(grid_coord.z+k,grid_sz.z-1);
      if ((o = grid_coord.z-k) >= 0) { /* bottom layer */
        for (n = min_n; n <= max_n; n++) {
          for (m = min_m; m <= max_m; m++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
      if ((o = grid_coord.z+k) < grid_sz.z) { /* top layer */
        for (n = min_n; n <= max_n; n++) {
          for (m = min_m; m <= max_m; m++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
      if ((n = grid_coord.y-k) >= 0) { /* back layer */
        for (o = min_o+1; o < max_o; o++) {
          for (m = min_m; m <= max_m; m++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
      if ((n = grid_coord.y+k) < grid_sz.y) { /* front layer */
        for (o = min_o+1; o < max_o; o++) {
          for (m = min_m; m <= max_m; m++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
      if ((m = grid_coord.x-k) >= 0) { /* left layer */
        for (o = min_o+1; o < max_o; o++) {
          for (n = min_n+1; n <= max_n; n++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
      if ((m = grid_coord.x+k) < grid_sz.x) { /* right layer */
        for (o = min_o+1; o < max_o; o++) {
          for (n = min_n+1; n <= max_n; n++) {
            cell_list_m[cll] = m;
            cell_list_n[cll] = n;
            cell_list_o[cll++] = o;
          }
        }
      }
    }
    /* Scan each cell in the compiled list */
    for (j=0; j<cll; j++) {
      m = cell_list_m[j];
      n = cell_list_n[j];
      o = cell_list_o[j];
      cell_tl = faces_in_cell[m+n*grid_sz.x+o*cell_stride_z];
      t_idx = cell_tl[0];
      /* If no triangles in cell skip */
      if (t_idx == -1) continue;
      /* If minimum distance from point to cell is larger than already
       * found minimum distance we can skip all triangles in the cell */
      if (dmin_sqr < dist_sqr_pt_cell(&p_rel,grid_coord.x,grid_coord.y,
                                      grid_coord.z,m,n,o,cell_sz)) {
        continue;
      }
      /* Scan all triangles (i.e. faces) in the cell */
      tfcl_idx = 0;
      do {
        dist_sqr = dist_sqr_pt_triag(&triags[t_idx],&p);
        if (dist_sqr < dmin_sqr) {
          dmin_sqr = dist_sqr;
          dmin_update = 1; /* signal update of dmin */
        }
      } while ((t_idx = cell_tl[++tfcl_idx]) != -1);
    }
    /* While no triangles have been scanned we have to continue testing in
     * farther cells. In addition, if a triangle was found with a smaller
     * distance, we need to scan all triangles in cells at distance k+1, to
     * see if there is a smaller distance. */
    k++;
  } while (dmin_sqr == DBL_MAX || dmin_update == 1);

  return sqrt(dmin_sqr);
}

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See compute_error.h */
struct face_list *faces_of_vertex(model *m)
{
  int i,j,imax,jmax;    /* indices and loop limits */
  struct face_list *fl; /* the face list to return */

  fl = xcalloc(m->num_vert,sizeof(*fl));
  for (i=0, imax=m->num_vert; i<imax; i++) {
    for (j=0, jmax=m->num_faces; j<jmax; j++) {
      if (m->faces[j].f0 == i || m->faces[j].f1 == i || m->faces[j].f2 == i) {
        fl[i].face = xrealloc(fl[i].face,(fl[i].n_faces+1)*sizeof(*(fl->face)));
        fl[i].face[fl[i].n_faces++] = j;
      }
    }
  }
  return fl;
}

/* See compute_error.h */
void dist_surf_surf(const model *m1, const model *m2, int n_spt,
                    struct face_error *fe_ptr[],
                    struct dist_surf_surf_stats *stats, int quiet)
{
  vertex bbox2_min,bbox2_max; /* min and max coords of bounding box of m2 */
  struct triangle_list *tl2;  /* triangle list for m2 */
  int **faces_in_cell_lst;    /* list of faces intersecting each cell */
  struct cell_list *cl;       /* list of cells intersecting each triangle */
  struct sample_list ts;      /* list of sample from a triangle */
  struct triag_sample_error tse; /* the errors at the triangle samples */
  int i,k,kmax;               /* counters and loop limits */
  double cell_sz;             /* side length of the cubic cells */
  struct size3d grid_sz;      /* number of cells in the X, Y and Z directions */
  struct face_error *fe;      /* The error metrics for each face of m1 */
  int report_step;            /* The step to update the progress report */

  /* Initialize */
  memset(&ts,0,sizeof(ts));
  memset(&tse,0,sizeof(tse));
  report_step = m1->num_faces/(100/2); /* report every 2 % */
  bbox2_min = m2->bBox[0];
  bbox2_max = m2->bBox[1];
  
  /* Get the triangle list from model 2 */
  tl2 = model_to_triangle_list(m2);
  
  /* Derive the grid size. For that we derive the average triangle side length
   * as the side of an equilateral triangle which's surface equals the average
   * triangle surface of m2. The cubic cell side is then CELL_TRIAG_RATIO
   * times that. */
  cell_sz = CELL_TRIAG_RATIO*sqrt(tl2->area/tl2->n_triangles*2/sqrt(3));
  grid_sz.x = (int) ceil((bbox2_max.x-bbox2_min.x)/cell_sz);
  grid_sz.y = (int) ceil((bbox2_max.y-bbox2_min.y)/cell_sz);
  grid_sz.z = (int) ceil((bbox2_max.z-bbox2_min.z)/cell_sz);

  /* Get the list of triangles in each cell */
  cl = cells_in_triangles(tl2,grid_sz,cell_sz,bbox2_min);
  faces_in_cell_lst = triangles_in_cells(cl,tl2,grid_sz);
  for (k=0, kmax=tl2->n_triangles; k<kmax; k++) {
    free(cl[k].cell);
  }
  free(cl);
  cl = NULL;

  /* Allocate storage for errors */
  *fe_ptr = xrealloc(*fe_ptr,m1->num_faces*sizeof(**fe_ptr));
  fe = *fe_ptr;
  realloc_triag_sample_error(&tse,n_spt);

  /* Initialize overall statistics */
  stats->m1_area = 0;
  stats->m2_area = tl2->area;
  stats->min_dist = DBL_MAX;
  stats->max_dist = 0;
  stats->mean_dist = 0;
  stats->rms_dist = 0;
  stats->cell_sz = cell_sz;
  stats->grid_sz = grid_sz;

  /* For each triangle in model 1, sample and calculate the error */
  if (!quiet) printf("Progress %2d %%",0);
  for (k=0, kmax=m1->num_faces; k<kmax; k++) {
    if (!quiet && k!=0 && k%report_step) {
      printf("\rProgress %2d %%",100*k/(kmax-1));
      fflush(stdout);
    }
    fe[k].face_area = tri_area(m1->vertices[m1->faces[k].f0],
                               m1->vertices[m1->faces[k].f1],
                               m1->vertices[m1->faces[k].f2]);
    sample_triangle(&(m1->vertices[m1->faces[k].f0]),
                    &(m1->vertices[m1->faces[k].f1]),
                    &(m1->vertices[m1->faces[k].f2]),n_spt,&ts);
    for (i=0; i<tse.n_samples_tot; i++) {
      tse.err_lin[i] = dist_pt_surf(ts.sample[i],tl2,faces_in_cell_lst,
                                    grid_sz,cell_sz,bbox2_min);
    }
    error_stat_triag(&tse,&fe[k]);
    /* Update overall statistics */
    stats->m1_area += fe[k].face_area;
    if (fe[k].min_error < stats->min_dist) stats->min_dist = fe[k].min_error;
    if (fe[k].max_error > stats->max_dist) stats->max_dist = fe[k].max_error;
    stats->mean_dist += fe[k].mean_error*fe[k].face_area;
    stats->rms_dist += fe[k].mean_sqr_error*fe[k].face_area;
  }
  if (!quiet) printf("\n");

  /* Finalize overall statistics */
  stats->mean_dist /= stats->m1_area;
  stats->rms_dist = sqrt(stats->rms_dist/stats->m1_area);

  /* free temporary storage */
  free(tl2->triangles);
  free(tl2);
  for (k=0, kmax=grid_sz.x*grid_sz.y*grid_sz.z; k<kmax; k++) {
    free(faces_in_cell_lst[k]);
  }
  free(faces_in_cell_lst);
  realloc_triag_sample_error(&tse,0);
  free(ts.sample);
}

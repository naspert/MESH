/* $Id: compute_error.c,v 1.30 2001/08/15 12:47:23 dsanta Exp $ */

#include <compute_error.h>

#include <geomutils.h>
#include <mutils.h>
#include <math.h>
#include <assert.h>

/* Ratio used to derive the cell size. It is the ratio between the cubic cell
 * side length and the side length of an average equilateral triangle. */
#define CELL_TRIAG_RATIO 3

/* If defined statistics for the dist_pt_surf() function are computed */
/* #define DO_DIST_PT_SURF_STATS */

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

/* Type for storing empty cell bitmap */
typedef int ec_bitmap_t;
/* The number of bytes used by ec_bitmap_t */
#define EC_BITMAP_T_SZ (sizeof(ec_bitmap_t))
/* The number of bits used by ec_bitmap_t, and also the divisor to obtain the
 * bitmap element index from a cell index */
#define EC_BITMAP_T_BITS (EC_BITMAP_T_SZ*8)
/* Bitmask to obtain the bitmap bit index from the cell index. */
#define EC_BITMAP_T_MASK (EC_BITMAP_T_BITS-1)
/* Macro to test the bit corresponding to element i in the bitmap bm. */
#define EC_BITMAP_TEST_BIT(bm,i) \
  (bm[i/EC_BITMAP_T_BITS]&(0x01<<(i&EC_BITMAP_T_MASK)))

/* List of triangles intersecting each cell */
struct t_in_cell_list {
  int ** triag_idx;         /* The list of the indices of the triangles
                             * intersecting each cell, terminated by -1
                             * (triag_idx[i] is the list for the cell with
                             * linear index i). If cell i is empty,
                             * triag_idx[i] is NULL. */
  int n_cells;              /* The number of cells in triag_idx */
  ec_bitmap_t *empty_cell;  /* A bitmap indicating which cells are empty. If
                             * cell i is empty, the bit (i&EC_BITMAP_T_MASK)
                             * of empty_cell[i/EC_BITMAP_T_BITS] is
                             * non-zero. */
};

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
  double s_area;       /* The surface area of the triangle */
};

/* Statistics of dist_pt_surf() function */
struct dist_pt_surf_stats {
  int n_cell_scans;       /* Number of cells that are scanned (i.e. distance
                           * point to cell is calculated) */
  int n_cell_t_scans;     /* Number of cells that for which their triangles are
                           * scanned */
  int n_triag_scans;      /* Number of triangles that are scanned */
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

/* Reallocates the buffers of tse to store the sample errors for a triangle
 * sampling with n samples in each direction. If tse->err and tse->err_lin is
 * NULL new buffers are allocated. If tse->n_samples equals n nothing is
 * done. The allocation never fails (if out of memory the program is stopped,
 * as with xrealloc()) */
static void realloc_triag_sample_error(struct triag_sample_error *tse, int n)
{
  int i;
  if (tse->n_samples == n) return;
  tse->n_samples = n;
  tse->n_samples_tot = n*(n+1)/2;
  tse->err = xrealloc(tse->err,n*sizeof(*(tse->err)));
  tse->err_lin = xrealloc(tse->err_lin,tse->n_samples_tot*sizeof(**(tse->err)));
  if (n != 0) {
    tse->err[0] = tse->err_lin;
    for (i=1; i<n; i++) {
      tse->err[i] = tse->err[i-1]+(n-(i-1));
    }
  }
}

/* Frees the buffers in tse (allocated with realloc_triag_sample_error()). */
static void free_triag_sample_error(struct triag_sample_error *tse)
{
  if (tse == NULL) return;
  free(tse->err);
  free(tse->err_lin);
  tse->err = NULL;
  tse->err_lin = NULL;
}

/* Computes the vertex normals assuming an oriented model. The triangle
 * information already present in tl are used to speed up the calculation. If
 * the model is not oriented, the resulting normals will be incorrect. */
static void calc_normals_as_oriented_model(model *m,
                                           const struct triangle_list *tl)
{
  int k,kmax;
  vertex *n;

  /* initialize all normals to zero */
  m->normals = xrealloc(m->normals,m->num_vert*sizeof(*(m->normals)));
  memset(m->normals,0,m->num_vert*sizeof(*(m->normals)));
  /* add face normals to vertices, weighted by face area */
  for (k=0, kmax=m->num_faces; k < kmax; k++) {
    n = &(tl->triangles[k].normal);
    prod_v(tl->triangles[k].s_area,n,n);
    add_v(n,&(m->normals[m->faces[k].f0]),&(m->normals[m->faces[k].f0]));
    add_v(n,&(m->normals[m->faces[k].f1]),&(m->normals[m->faces[k].f1]));
    add_v(n,&(m->normals[m->faces[k].f2]),&(m->normals[m->faces[k].f2]));
  }
  /* normalize final normals */
  for (k=0, kmax=m->num_vert; k<kmax; k++) {
    normalize_v(&(m->normals[k]));
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
  double dc_len_sqr;

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
  dc_len_sqr = norm2_v(&(t->dc));
  t->dc_1_len_sqr = 1/dc_len_sqr;
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
  /* Get surface area */
  t->s_area = sqrt(t->ab_len_sqr*dc_len_sqr)/2;
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
    tl->area += triags[i].s_area;
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
  if (n != 1) { /* normal case */
    fe->mean_error = err_tot/(((n-1)*n/2+(n-2)*(n-1)/2)*3);
    fe->mean_sqr_error = err_sqr_tot/(((n-1)*n/2+(n-2)*(n-1)/2)*3);
  } else { /* special case */
    fe->mean_error = tse->err_lin[0];
    fe->mean_sqr_error = tse->err_lin[0]*tse->err_lin[0];
  }
}

/* Samples a triangle (a,b,c) using n samples in each direction. The sample
 * points are returned in the sample_list s. The dynamic array 's->sample' is
 * realloc'ed to the correct size (if no storage has been previously allocated
 * it should be NULL). The total number of samples is n*(n+1)/2. The order for
 * samples (i,j) in s->sample is all samples for i equal 0 and j from 0 to n-1,
 * followed by all samples for i equal 1 and j from 0 to n-2, and so on, where
 * i and j are the sampling indices along the ab and ac sides,
 * respectively. As a special case, if n equals 1, the triangle middle point
 * is used as the sample. */
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
  if (n != 1) { /* normal case */
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
  } else { /* special case, use triangle middle point */
    s->sample[0].x = a_cache.x+0.5*u.x+0.5*v.x;
    s->sample[0].y = a_cache.y+0.5*u.y+0.5*v.y;
    s->sample[0].z = a_cache.z+0.5*u.z+0.5*v.z;
  }
}

/* Given a triangle list tl, returns the list of triangle indices that
 * intersect a cell, for each cell in the grid. The size of the grid is given
 * by grid_sz, the side length of the cubic cells by cell_sz and the minimum
 * coordinates of the bounding box of tl2 by bbox_min. The returned struct, its
 * arrays and subarrays are malloc'ed independently. */
static struct t_in_cell_list *triangles_in_cells(const struct triangle_list *tl,
                                                 struct size3d grid_sz,
                                                 double cell_sz,
                                                 vertex bbox_min)
{
  struct t_in_cell_list *lst; /* The list to return */
  struct sample_list sl;      /* samples from a triangle */
  int **tab;                  /* Table containing the indices of intersecting
                               * triangles for each cell. */
  int *nt;                    /* Array with the number of intersecting
                               * triangles found so far for each cell */
  ec_bitmap_t *ecb;           /* The empty cell bitmap */
  int cell_idx,cell_idx_prev; /* linear (1D) cell indices */
  int cell_stride_z;          /* spacement for Z index in 3D addressing of
                               * cell list */
  int i,j,h,imax;             /* counters and loop limits */
  int m_a,n_a,o_a,m_b,n_b,o_b,m_c,n_c,o_c; /* 3D cell indices for vertices */
  int tmpi,max_cell_dist;     /* maximum cell distance along any axis */
  int n_samples;              /* number of samples to use for triangles */
  int m,n,o;                  /* 3D cell indices for samples */
  int *c_buf;                 /* temp storage for cell list */
  int c_buf_sz;               /* the size of c_buf */

  /* Initialize */
  cell_stride_z = grid_sz.x*grid_sz.y;
  c_buf = NULL;
  c_buf_sz = 0;
  sl.sample = NULL;
  lst = xmalloc(sizeof(*lst));
  nt = xcalloc(grid_sz.x*grid_sz.y*grid_sz.z,sizeof(*nt));
  tab = xcalloc(grid_sz.x*grid_sz.y*grid_sz.z,sizeof(*tab));
  ecb = xcalloc((grid_sz.x*grid_sz.y*grid_sz.z+EC_BITMAP_T_BITS-1)/
                EC_BITMAP_T_BITS,EC_BITMAP_T_SZ);
  lst->triag_idx = tab;
  lst->n_cells = grid_sz.x*grid_sz.y*grid_sz.z;
  lst->empty_cell = ecb;

  /* Get intersecting cells for each triangle */
  for (i=0, imax=tl->n_triangles; i<imax;i++) {
    /* Get the cells in which the triangle vertices are. For non-negative
     * values, cast to int is equivalent to floor and probably faster (here
     * negative values can not happen since bounding box is obtained from the
     * vertices in tl). */
    m_a = (int)((tl->triangles[i].a.x-bbox_min.x)/cell_sz);
    n_a = (int)((tl->triangles[i].a.y-bbox_min.y)/cell_sz);
    o_a = (int)((tl->triangles[i].a.z-bbox_min.z)/cell_sz);
    m_b = (int)((tl->triangles[i].b.x-bbox_min.x)/cell_sz);
    n_b = (int)((tl->triangles[i].b.y-bbox_min.y)/cell_sz);
    o_b = (int)((tl->triangles[i].b.z-bbox_min.z)/cell_sz);
    m_c = (int)((tl->triangles[i].c.x-bbox_min.x)/cell_sz);
    n_c = (int)((tl->triangles[i].c.y-bbox_min.y)/cell_sz);
    o_c = (int)((tl->triangles[i].c.z-bbox_min.z)/cell_sz);

    if (m_a == m_b && m_a == m_c && n_a == n_b && n_a == n_c &&
        o_a == o_b && o_a == o_c) {
      /* The ABC triangle fits entirely into one cell => fast case */
      cell_idx = m_a+n_a*grid_sz.x+o_a*cell_stride_z;
      tab[cell_idx] = xrealloc(tab[cell_idx],(nt[cell_idx]+2)*sizeof(**tab));
      tab[cell_idx][nt[cell_idx]++] = i;
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
    h = 0;
    for(j=0;j<sl.n_samples;j++){
      /* Get cell in which the sample is. Due to rounding in the triangle
       * sampling process we check the indices to be within bounds. As above,
       * we can use cast to int instead of floor (probably faster) */
      m=(int)((sl.sample[j].x-bbox_min.x)/cell_sz);
      if(m >= grid_sz.x) {
        m = grid_sz.x - 1;
      } else if (m < 0) {
        m = 0;
      }
      n=(int)((sl.sample[j].y-bbox_min.y)/cell_sz);
      if (n >= grid_sz.y) {
        n = grid_sz.y - 1;
      } else if (n < 0) {
        n = 0;
      }
      o=(int)((sl.sample[j].z-bbox_min.z)/cell_sz);
      if (o >= grid_sz.z) {
        o = grid_sz.z - 1;
      } else if (o < 0) {
        o = 0;
      }

      /* Include cell index in list only if not the same as previous one
       * (avoid too many duplicates). */
      cell_idx = m + n*grid_sz.x + o*cell_stride_z;
      if (cell_idx != cell_idx_prev) {
        if (c_buf_sz <= h) {
          c_buf = xrealloc(c_buf, (h+1)*sizeof(*c_buf));
          c_buf_sz++;
        }
        c_buf[h++] = cell_idx;
        cell_idx_prev = cell_idx;
      }
    }

    /* Include triangle in intersecting cell lists, without duplicate. */
    for (j=0; j<h; j++) {
      cell_idx = c_buf[j];
      if (nt[cell_idx] == 0 || tab[cell_idx][nt[cell_idx]-1] != i) {
        tab[cell_idx] = xrealloc(tab[cell_idx],(nt[cell_idx]+2)*sizeof(**tab));
        tab[cell_idx][nt[cell_idx]++] = i;
      }
    }
  }

  /* Terminate lists with -1 and set empty cell bitmap */
  for(i=0, imax=grid_sz.x*grid_sz.y*grid_sz.z; i<imax; i++){
    if (nt[i] == 0) { /* mark empty cell in bitmap */
      ecb[i/EC_BITMAP_T_BITS] |= 0x01<<(i&EC_BITMAP_T_MASK);
    } else {
      tab[i][nt[i]] = -1;
    }
  }

  free(nt);
  free(sl.sample);
  free(c_buf);
  return lst;
}

/* Returns the distance from point p to the surface defined by the triangle
 * list tl. The distance from a point to a surface is defined as the distance
 * from a point to the closets point on the surface. To speed up the search
 * for the closest triangle in the surface the bounding box of the model is
 * subdivided in cubic cells. The list of triangles that intersect each cell
 * is given by fic, as returned by the triangles_in_cells() function. The side
 * of the cubic cells is of length cell_sz, and there are
 * (grid_sz.x,grid_sz.y,grid_sz.z) cells in teh X,Y,Z directions. Cell (0,0,0)
 * starts at bbox_min, which is the minimum coordinates of the (axis aligned)
 * bounding box. Static storage is used by this function, so it is not
 * reentrant (i.e. thread safe). If DO_DIST_PT_SURF_STATS is defined at
 * compile time, the statistics stats are updated (no reset to zero occurs,
 * the counters are increased). */
static double dist_pt_surf(vertex p, const struct triangle_list *tl,
                           const struct t_in_cell_list *fic,
#ifdef DO_DIST_PT_SURF_STATS
                           struct dist_pt_surf_stats *stats,
#endif
                           struct size3d grid_sz, double cell_sz,
                           vertex bbox_min)
{
  vertex p_rel;         /* coordinates of p relative to bbox_min */
  struct size3d grid_coord; /* coordinates of cell in which p is */
  int k;                /* cell index distance of current scan */
  int kmax;             /* maximum limit for k (avoid infinite loops) */
  int m,n,o;            /* 3D cell indices */
  int cell_idx;         /* linear cell index */
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
  static int *cell_list;/* list of cells to scan for the current k */
  static int cell_list_sz; /* size of cell_list_{m,n,o} */
  int cll;              /* length of cell list */
  int j;                /* counter */
  int tmpi;             /* temporary integer */
  ec_bitmap_t *fic_empty_cell; /* stack copy of fic->empty_cell (faster) */
  int **fic_triag_idx;  /* stack copy of fic->triag_idx (faster) */

  /* Reusing the buffers from call to call gives significant speedup. The size
   * used is an (larger) approximation than the real maximum. */
  tmpi = 2*grid_sz.x*grid_sz.y+2*grid_sz.x*grid_sz.z+2*grid_sz.y*grid_sz.z;
  if (cell_list == NULL) {
    cell_list_sz = tmpi;
    cell_list = xmalloc(sizeof(*cell_list)*cell_list_sz);
  } else if (cell_list_sz < tmpi) {
    cell_list_sz = tmpi;
    cell_list = xrealloc(cell_list,sizeof(*cell_list)*cell_list_sz);
  }

  /* NOTE: tests have shown it is faster to scan each triangle, even
   * repeteadly, than to track which triangles have been scanned (too much
   * time spent initializing tracking info to zero). */

  /* Initialize */
  cell_stride_z = grid_sz.y*grid_sz.x;
  triags = tl->triangles;
  fic_empty_cell = fic->empty_cell;
  fic_triag_idx = fic->triag_idx;

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
  kmax = max3(grid_sz.x,grid_sz.y,grid_sz.z);
  dmin_sqr = DBL_MAX;
  do {
    dmin_update = 0;
    /* Get the list of cells at distance k in X Y or Z direction, which has
     * not been previously tested. Only non-empty cells are included in the
     * list. */
    if (k == 0) {
      cell_idx = grid_coord.x+grid_coord.y*grid_sz.x+grid_coord.z*cell_stride_z;
      if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
        cll = 1;
        cell_list[0] = cell_idx;
      } else { /* empty cell */
        cll = 0;
      }
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
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
      if ((n = grid_coord.y-k) >= 0) { /* back layer */
        for (o = min_o+1; o < max_o; o++) {
          for (m = min_m; m <= max_m; m++) {
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
      if ((m = grid_coord.x-k) >= 0) { /* left layer */
        for (o = min_o+1; o < max_o; o++) {
          for (n = min_n+1; n <= max_n; n++) {
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
      if ((m = grid_coord.x+k) < grid_sz.x) { /* right layer */
        for (o = min_o+1; o < max_o; o++) {
          for (n = min_n+1; n <= max_n; n++) {
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
      if ((n = grid_coord.y+k) < grid_sz.y) { /* front layer */
        for (o = min_o+1; o < max_o; o++) {
          for (m = min_m; m <= max_m; m++) {
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
      if ((o = grid_coord.z+k) < grid_sz.z) { /* top layer */
        for (n = min_n; n <= max_n; n++) {
          for (m = min_m; m <= max_m; m++) {
            cell_idx = m+n*grid_sz.x+o*cell_stride_z;
            if (!EC_BITMAP_TEST_BIT(fic_empty_cell,cell_idx)) {
              cell_list[cll++] = cell_idx;
            }
          }
        }
      }
    }
    /* Scan each (non-empty) cell in the compiled list */
    for (j=0; j<cll; j++) {
      cell_idx = cell_list[j];
      cell_tl = fic_triag_idx[cell_idx];
      o = cell_idx/cell_stride_z;
      tmpi = cell_idx%cell_stride_z;
      n = tmpi/grid_sz.x;
      m = tmpi%grid_sz.x;
      /* If minimum distance from point to cell is larger than already
       * found minimum distance we can skip all triangles in the cell */
#ifdef DO_DIST_PT_SURF_STATS
      stats->n_cell_scans++;
#endif
      if (dmin_sqr < dist_sqr_pt_cell(&p_rel,grid_coord.x,grid_coord.y,
                                      grid_coord.z,m,n,o,cell_sz)) {
        continue;
      }
      /* Scan all triangles (i.e. faces) in the cell */
#ifdef DO_DIST_PT_SURF_STATS
      stats->n_cell_t_scans++;
#endif
      tfcl_idx = 0;
      t_idx = cell_tl[0];
      do { /* cell has always one triangle at least, so do loop is OK */
#ifdef DO_DIST_PT_SURF_STATS
        stats->n_triag_scans++;
#endif
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
  } while ((dmin_sqr == DBL_MAX || dmin_update == 1) && (k <= kmax));
  if (k > kmax) { /* Something is going wrong (probably NaNs, etc.) */
    fprintf(stderr,
            "ERROR: entered infinite loop! NaN or infinte value in model ?\n"
            "       (otherwise you have stumbled on a bug, please report)\n");
    exit(1);
  }

  return sqrt(dmin_sqr);
}

/* --------------------------------------------------------------------------*
 *                          External functions                               *
 * --------------------------------------------------------------------------*/

/* See compute_error.h */
struct face_list *faces_of_vertex(model *m)
{
  int j,jmax;           /* indices and loop limits */
  int v0,v1,v2;         /* current triangle's vertex indices */
  struct face_list *fl; /* the face list to return */

  fl = xcalloc(m->num_vert,sizeof(*fl));
  for (j=0, jmax=m->num_faces; j<jmax; j++) {
    v0 = m->faces[j].f0;
    v1 = m->faces[j].f1;
    v2 = m->faces[j].f2;
    if (v0 == v1 || v0 == v2 || v1 == v2) {
      fprintf(stderr,
              "WARNING: face %d is degenerated, skipped from face list\n",j);
      continue;
    }
    fl[v0].face = xrealloc(fl[v0].face,(fl[v0].n_faces+1)*sizeof(*(fl->face)));
    fl[v0].face[fl[v0].n_faces++] = j;
    fl[v1].face = xrealloc(fl[v1].face,(fl[v1].n_faces+1)*sizeof(*(fl->face)));
    fl[v1].face[fl[v1].n_faces++] = j;
    fl[v2].face = xrealloc(fl[v2].face,(fl[v2].n_faces+1)*sizeof(*(fl->face)));
    fl[v2].face[fl[v2].n_faces++] = j;
  }
  return fl;
}

/* See compute_error.h */
void free_face_lists(struct face_list *fl, int n)
{
  int i;
  if (fl == NULL) return;
  for (i=0; i<n; i++) {
    free(fl[i].face);
  }
  free(fl);
}

/* See compute_error.h */
void dist_surf_surf(const model *m1, model *m2, int n_spt,
                    struct face_error *fe_ptr[],
                    struct dist_surf_surf_stats *stats, int calc_normals,
                    int quiet)
{
  vertex bbox2_min,bbox2_max; /* min and max coords of bounding box of m2 */
  struct triangle_list *tl2;  /* triangle list for m2 */
  struct t_in_cell_list *fic; /* list of faces intersecting each cell */
  struct sample_list ts;      /* list of sample from a triangle */
  struct triag_sample_error tse; /* the errors at the triangle samples */
  int i,k,kmax;               /* counters and loop limits */
  double cell_sz;             /* side length of the cubic cells */
  struct size3d grid_sz;      /* number of cells in the X, Y and Z directions */
  struct face_error *fe;      /* The error metrics for each face of m1 */
  int report_step;            /* The step to update the progress report */
#ifdef DO_DIST_PT_SURF_STATS
  struct dist_pt_surf_stats dps_stats; /* Statistics */
#endif

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
  fic = triangles_in_cells(tl2,grid_sz,cell_sz,bbox2_min);

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
#ifdef DO_DIST_PT_SURF_STATS
  memset(&dps_stats,0,sizeof(dps_stats));
#endif

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
      tse.err_lin[i] = dist_pt_surf(ts.sample[i],tl2,fic,
#ifdef DO_DIST_PT_SURF_STATS
                                    &dps_stats,
#endif
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
#ifdef DO_DIST_PT_SURF_STATS
  if (!quiet) {
    int n_tot_samples;
    n_tot_samples = n_spt*(n_spt+1)/2*m1->num_faces;
    printf("Average number of scanned non-empty cells per sample: %g\n",
           (double)(dps_stats.n_cell_scans)/n_tot_samples);
    printf("Average number of cells per sample for which triangles are "
           "scanned: %g\n",
           (double)(dps_stats.n_cell_t_scans)/n_tot_samples);
    printf("Average number of triangles scanned per sample: %g\n",
           (double)(dps_stats.n_triag_scans)/n_tot_samples);
  }
#endif
  /* Finalize overall statistics */
  stats->mean_dist /= stats->m1_area;
  stats->rms_dist = sqrt(stats->rms_dist/stats->m1_area);

  /* Do normals for model 2 if requested and not yet present */
  if (calc_normals && m2->normals == NULL && m2->face_normals == NULL) {
    calc_normals_as_oriented_model(m2,tl2);
  }

  /* free temporary storage */
  free(tl2->triangles);
  free(tl2);
  for (k=0, kmax=fic->n_cells; k<kmax; k++) {
    free(fic->triag_idx[k]);
  }
  free(fic->triag_idx);
  free(fic->empty_cell);
  free(fic);
  free_triag_sample_error(&tse);
  free(ts.sample);
}

/* $Id: mesh_run.c,v 1.10 2002/02/13 10:38:40 dsanta Exp $ */

#include <time.h>
#include <string.h>
#include <xalloc.h>
#include <model_analysis.h>
#include <compute_error.h>
#include <model_in.h>
#include <geomutils.h>

#include <mesh_run.h>

/* Reads a model from file 'fname' and returns the model read. If an error
 * occurs a message is printed and the program exists. */
static struct model *read_model_file(const char *fname)
{
  int rcode;
  struct model *m;
  const char *errstr;
  
  rcode = read_fmodel(&m,fname,MESH_FF_AUTO,1);
  if (rcode <= 0) {
    switch (rcode) {
    case 0:
      errstr = "no models in file";
      break;
    case MESH_NO_MEM:
      errstr = "no memory";
      break;
    case MESH_CORRUPTED:
      errstr = "corrupted file or I/O error";
      break;
    case MESH_MODEL_ERR:
      errstr = "model error";
      break;
    case MESH_NOT_TRIAG:
      errstr = "not a triangular mesh model";
      break;
    case MESH_BAD_FF:
      errstr = "unrecognized file format";
      break;
    case MESH_BAD_FNAME:
      errstr = strerror(errno);
      break;
    default:
      errstr = "unknown error";
    }
    fprintf(stderr,"ERROR: %s: %s\n",fname,errstr);
    exit(1);
  } else if (m->num_faces == 0) {
    fprintf(stderr,"ERROR: %s: empty model (no faces)\n",fname);
    exit(1);
  }
  return m;
}

/* see mesh_run.h */
void mesh_run(const struct args *args, struct model_error *model1,
              struct model_error *model2, struct outbuf *out,
              struct prog_reporter *progress)
{
  clock_t start_time;
  struct face_list *vfl;
  struct face_error *fe = NULL;
  struct face_error *fe_rev = NULL;
  struct dist_surf_surf_stats stats;
  struct dist_surf_surf_stats stats_rev;
  double bbox1_diag,bbox2_diag;
  struct model_info *m1info,*m2info;
  double abs_sampling_step,abs_sampling_dens;

  /* Read models from input files */
  memset(model1,0,sizeof(*model1));
  memset(model2,0,sizeof(*model2));
  m1info = (struct model_info*) xa_malloc(sizeof(*m1info));
  m2info = (struct model_info*) xa_malloc(sizeof(*m2info));
  outbuf_printf(out,"Reading %s ... ",args->m1_fname);
  outbuf_flush(out);
  start_time = clock();
  model1->mesh = read_model_file(args->m1_fname);
  outbuf_printf(out,"Done (%.2f secs)\n",
                (double)(clock()-start_time)/CLOCKS_PER_SEC);
  outbuf_printf(out,"Reading %s ... ",args->m2_fname);
  outbuf_flush(out);
  start_time = clock();
  model2->mesh = read_model_file(args->m2_fname);
  outbuf_printf(out,"Done (%.2f secs)\n",
                (double)(clock()-start_time)/CLOCKS_PER_SEC);

  /* Analyze models (we don't need normals for model 1, so we don't request
   * for it to be oriented). */
  start_time = clock();
  bbox1_diag = dist(model1->mesh->bBox[0], model1->mesh->bBox[1]);
  bbox2_diag = dist(model2->mesh->bBox[0], model2->mesh->bBox[1]);
  vfl = faces_of_vertex(model1->mesh);
  analyze_model(model1->mesh,vfl,m1info,0);
  model1->info = m1info;
  analyze_model(model2->mesh,NULL,m2info,1);
  model2->info = m2info;
  if(args->no_gui){
    free_face_lists(vfl,model1->mesh->num_vert);
    vfl = NULL;
  }
  /* Adjust sampling step size */
  abs_sampling_step = args->sampling_step*bbox2_diag;
  abs_sampling_dens = 1/(abs_sampling_step*abs_sampling_step);

  /* Print available model information */
  outbuf_printf(out,"\n                      Model information\n\n");
  outbuf_printf(out,"Number of vertices:     \t%11d\t%11d\n",
                model1->mesh->num_vert,model2->mesh->num_vert);
  outbuf_printf(out,"Number of triangles:    \t%11d\t%11d\n",
                model1->mesh->num_faces,model2->mesh->num_faces);
  outbuf_printf(out,"BoundingBox diagonal:   \t%11g\t%11g\n",
                bbox1_diag,bbox2_diag);
  outbuf_printf(out,"Number of disjoint parts:\t%11d\t%11d\n",
                m1info->n_disjoint_parts,m2info->n_disjoint_parts);
  outbuf_printf(out,"Manifold:               \t%11s\t%11s\n",
                (m1info->manifold ? "yes" : "no"), 
                (m2info->manifold ? "yes" : "no"));
  outbuf_printf(out,"Originally oriented:    \t%11s\t%11s\n",
                (m1info->orig_oriented ? "yes" : "no"),
                (m2info->orig_oriented ? "yes" : "no"));
  outbuf_printf(out,"Orientable:             \t%11s\t%11s\n",
                (m1info->orientable ? "yes" : "no"),
                (m2info->orientable ? "yes" : "no"));
  outbuf_printf(out,"Closed:                 \t%11s\t%11s\n",
                (m1info->closed ? "yes" : "no"),
                (m2info->closed ? "yes" : "no"));
  outbuf_flush(out);

  /* Compute the distance from one model to the other */
  dist_surf_surf(model1->mesh,model2->mesh,abs_sampling_dens,
                 args->force_sample_all,
                 &fe,&stats,!args->no_gui,(args->quiet ? NULL : progress));

  /* Print results */
  outbuf_printf(out,"Surface area:           \t%11g\t%11g\n",
                stats.m1_area,stats.m2_area);
  outbuf_printf(out,"\n       Distance from model 1 to model 2\n\n");
  outbuf_printf(out,"        \t   Absolute\t%% BBox diag\n");
  outbuf_printf(out,"        \t           \t  (Model 2)\n");
  outbuf_printf(out,"Min:    \t%11g\t%11g\n",
                stats.min_dist,stats.min_dist/bbox2_diag*100);
  outbuf_printf(out,"Max:    \t%11g\t%11g\n",
                stats.max_dist,stats.max_dist/bbox2_diag*100);
  outbuf_printf(out,"Mean:   \t%11g\t%11g\n",
                stats.mean_dist,stats.mean_dist/bbox2_diag*100);
  outbuf_printf(out,"RMS:    \t%11g\t%11g\n",
                stats.rms_dist,stats.rms_dist/bbox2_diag*100);
  outbuf_printf(out,"\n");
  outbuf_flush(out);
  
 

  if (args->do_symmetric) { /* Invert models and recompute distance */
    outbuf_printf(out,"       Distance from model 2 to model 1\n\n");
    dist_surf_surf(model2->mesh,model1->mesh,abs_sampling_dens,
                   args->force_sample_all,
                   &fe_rev,&stats_rev,0,(args->quiet ? NULL : progress));
    free_face_error(fe_rev);
    fe_rev = NULL;
    outbuf_printf(out,"        \t   Absolute\t%% BBox diag\n");
    outbuf_printf(out,"        \t           \t  (Model 2)\n");
    outbuf_printf(out,"Min:    \t%11g\t%11g\n",
                  stats_rev.min_dist,stats_rev.min_dist/bbox2_diag*100);
    outbuf_printf(out,"Max:    \t%11g\t%11g\n",
                  stats_rev.max_dist,stats_rev.max_dist/bbox2_diag*100);
    outbuf_printf(out,"Mean:   \t%11g\t%11g\n",
                  stats_rev.mean_dist,stats_rev.mean_dist/bbox2_diag*100);
    outbuf_printf(out,"RMS:    \t%11g\t%11g\n",
                  stats_rev.rms_dist,stats_rev.rms_dist/bbox2_diag*100);
    outbuf_printf(out,"\n");

    /* Print symmetric distance measures */
    outbuf_printf(out,
                  "       Symmetric distance between model 1 and model 2\n\n");
    outbuf_printf(out,"        \t   Absolute\t%% BBox diag\n");
    outbuf_printf(out,"        \t           \t  (Model 2)\n");
    outbuf_printf(out,"Min:    \t%11g\t%11g\n",
                  max(stats.min_dist,stats_rev.min_dist),
                  max(stats.min_dist,stats_rev.min_dist)/bbox2_diag*100);
    outbuf_printf(out,"Max:    \t%11g\t%11g\n",
                  max(stats.max_dist,stats_rev.max_dist),
                  max(stats.max_dist,stats_rev.max_dist)/bbox2_diag*100);
    outbuf_printf(out,"Mean:   \t%11g\t%11g\n",
                  max(stats.mean_dist,stats_rev.mean_dist),
                  max(stats.mean_dist,stats_rev.mean_dist)/bbox2_diag*100);
    outbuf_printf(out,"RMS:    \t%11g\t%11g\n",
                  max(stats.rms_dist,stats_rev.rms_dist),
                  max(stats.rms_dist,stats_rev.rms_dist)/bbox2_diag*100);
    outbuf_printf(out,"\n");
  }


  outbuf_printf(out,"               \t       Absolute\t   %% BBox diag\t     "
                "Expected samples\n"
                "               \t               \t       model 2\t   "
                "model 1\tmodel 2\n");
  if (!args->do_symmetric) {
    outbuf_printf(out,"Sampling step: \t%15g\t%14g\t%10d\t%7d\n",
                  abs_sampling_step,abs_sampling_step/bbox2_diag*100,
                  (int)(stats.m1_area*abs_sampling_dens),0);
    outbuf_printf(out,"\n");
    outbuf_printf(out,"        \t    Total\t    Avg. / triangle\t\t"
                  "Tot (%%) area of\n"
                  "        \t         \tmodel 1 \tmodel 2 \t"
                  "sampled triang.\n");
    outbuf_printf(out,"Samples:\t%9d\t%7.2g\t%15.2g\t%18.2f\n",stats.m1_samples,
                  ((double)stats.m1_samples)/model1->mesh->num_faces,
                  ((double)stats.m1_samples)/model2->mesh->num_faces,
                  stats.st_m1_area/stats.m1_area*100.0);
  } else {
    outbuf_printf(out,"Sampling step: \t%15g\t%14g\t%10d\t%7d\n",
                  abs_sampling_step,abs_sampling_step/bbox2_diag*100,
                  (int)(stats.m1_area*abs_sampling_dens),
                  (int)(stats.m2_area*abs_sampling_dens));
    outbuf_printf(out,"\n");
    outbuf_printf(out,"        \t    Total\t    Avg. / triangle\t\t"
                  "Tot (%%) area of\n"
                  "        \t         \tmodel 1 \tmodel 2 \t"
                  "sampled triang.\n");
    outbuf_printf(out,"Samples (1->2):\t%9d\t%7.2g\t%15.2g\t%18.2f\n",
                  stats.m1_samples,
                  ((double)stats.m1_samples)/model1->mesh->num_faces,
                  ((double)stats.m1_samples)/model2->mesh->num_faces,
                  stats.st_m1_area/stats.m1_area*100.0);
    outbuf_printf(out,"Samples (2->1):\t%9d\t%7.2g\t%15.2g\t%18.2f\n",
                  stats_rev.m1_samples,
                  ((double)stats_rev.m1_samples)/model1->mesh->num_faces,
                  ((double)stats_rev.m1_samples)/model2->mesh->num_faces,
                  stats_rev.st_m1_area/stats_rev.m1_area*100.0);
  }
  outbuf_printf(out,"\n");
  if (!args->do_symmetric) {
    outbuf_printf(out,
                  "                       \t     X\t    Y\t   Z\t   Total\n");
    outbuf_printf(out,"Partitioning grid size:\t%6d\t%5d\t%4d\t%8d\n",
                  stats.grid_sz.x,stats.grid_sz.y,stats.grid_sz.z,
                  stats.grid_sz.x*stats.grid_sz.y*stats.grid_sz.z);
    outbuf_printf(out,"\nAvg. number of triangles per non-empty cell:\t%.2f\n",
                  stats.n_t_p_nec);
    outbuf_printf(out,"Proportion of non-empty cells:          \t%.2f%%\n",
                  (double)stats.n_ne_cells/(stats.grid_sz.x*stats.grid_sz.y*
                                            stats.grid_sz.z)*100.0);
  } else {
    outbuf_printf(out,"                                \t     "
                  "X\t    Y\t   Z\t   Total\n");
    outbuf_printf(out,"Partitioning grid size (1 to 2):\t%6d\t%5d\t%4d\t%8d\n",
                  stats.grid_sz.x,stats.grid_sz.y,stats.grid_sz.z,
                  stats.grid_sz.x*stats.grid_sz.y*stats.grid_sz.z);
    outbuf_printf(out,"Partitioning grid size (2 to 1):\t%6d\t%5d\t%4d\t%8d\n",
                  stats_rev.grid_sz.x,stats_rev.grid_sz.y,stats_rev.grid_sz.z,
                  stats_rev.grid_sz.x*stats_rev.grid_sz.y*stats_rev.grid_sz.z);
    outbuf_printf(out,"\nAvg. number of triangles per non-empty cell (1 to 2):"
                  "\t%.2f\n",stats.n_t_p_nec);
    outbuf_printf(out,"Avg. number of triangles per non-empty cell (2 to 1):"
                  "\t%.2f\n",stats_rev.n_t_p_nec);
    outbuf_printf(out,
                  "Proportion of non-empty cells (1 to 2):          \t%.2f%%\n",
                  (double)stats.n_ne_cells/(stats.grid_sz.x*stats.grid_sz.y*
                                            stats.grid_sz.z)*100.0);
    outbuf_printf(out,
                  "Proportion of non-empty cells (2 to 1):          \t%.2f%%\n",
                  (double)stats_rev.n_ne_cells/
                  (stats_rev.grid_sz.x*stats_rev.grid_sz.y*
                   stats_rev.grid_sz.z)*100.0);
  }
  outbuf_printf(out,"\n");
  outbuf_printf(out,"Execution time (secs.):\t%.2f\n",
                (double)(clock()-start_time)/CLOCKS_PER_SEC);
  outbuf_flush(out);

  if(args->no_gui){
    free_face_error(fe);
    fe = NULL;
  } else {
    /* Get the per vertex error metric */
    calc_vertex_error(model1,fe,vfl);
    /* Free now useless data */
    free_face_error(fe);
    fe = NULL;
    free_face_lists(vfl,model1->mesh->num_vert);
    vfl = NULL;
  }
}

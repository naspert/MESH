/* $Id: mesh.cpp,v 1.1 2001/09/12 12:44:41 dsanta Exp $ */

#include <time.h>
#include <string.h>
#include <qapplication.h>
#include <qevent.h>
#include <qobject.h>
#include <qkeycode.h>

#include <compute_error.h>
#include <model_analysis.h>
#include <ScreenWidget.h>
#include <InitWidget.h>

/* To store the parsed arguments */
struct args {
  char *m1_fname; /* filename of model 1 */
  char *m2_fname; /* filename of model 2 */
  int  no_gui;    /* text only flag */
  int quiet;      /* do not display extra info flag*/
  double sampling_step; /* The sampling step, as fraction of the bounding box
                         * diagonal of model 2. */
  int do_symmetric; /* do symmetric error measure */
};

/* Prints usage information to the out stream */
static void print_usage(FILE *out)
{
  fprintf(out,"MESH: Measuring Distance between Surfaces with the "
          "Hausdorff distance\n");
  fprintf(out,"\n");
  fprintf(out,"usage: mesh [[options] file1 file2]\n");
  fprintf(out,"\n");
  fprintf(out,"The program measures the distance from the 3D model in\n");
  fprintf(out,"file1 to the one in file2. The models must be given as\n");
  fprintf(out,"triangular meshes in RAW format, optionally with normals.\n");
  fprintf(out,"After the distance is calculated the result is displayed\n");
  fprintf(out,"as overall measures in text form and as a detailed distance\n");
  fprintf(out,"map in graphical form.\n");
  fprintf(out,"If no options nor filenames are given a dialog is shown\n");
  fprintf(out,"to select the input file names as well as the parameters.\n");
  fprintf(out,"\n");
  fprintf(out,"options:");
  fprintf(out,"\n");
  fprintf(out,"  -h\tDisplays this help message and exits.\n");
  fprintf(out,"\n");
  fprintf(out,"  -s\tCalculate a symmetric distance measure. It calculates\n");
  fprintf(out,"    \tthe distance in the two directions and uses the max\n");
  fprintf(out,"    \tas the symmetric distance (Hausdorff distance).\n");
  fprintf(out,"\n");
  fprintf(out,"  -q\tQuiet, do not print progress meter.\n");
  fprintf(out,"\n");
  fprintf(out,"  -t\tDisplay only textual results, do not display the GUI.\n");
  fprintf(out,"\n");
  fprintf(out,"  -l s\tSet the sampling step to s, which is a percentage of\n");
  fprintf(out,"      \tthe bounding box diagonal of the second model. The\n");
  fprintf(out,"      \ttriangles of the first model are sampled, in order\n");
  fprintf(out,"      \tto calculate an approximation of the distance, so\n");
  fprintf(out,"      \tthat the maximum distance between samples in any\n");
  fprintf(out,"      \ttriangle is no more than s. Each triangle of the\n");
  fprintf(out,"      \tfirst model gets at least one sample, whatever the\n");
  fprintf(out,"      \tsampling step. The default is 0.5.\n");
}

/* Initializes *pargs to default values and parses the command line arguments
 * in argc,argv. */
static void parse_args(int argc, char **argv, struct args *pargs)
{
  char *endptr;
  int i;

  memset(pargs,0,sizeof(*pargs));
  pargs->sampling_step = 0.5;
  i = 1;
  while (i < argc) {
    if (argv[i][0] == '-') { /* Option */
      if (strcmp(argv[i],"-h") == 0) { /* help */
        print_usage(stdout);
        exit(0);
      } else if (strcmp(argv[i],"-t") == 0) { /* text only */
        pargs->no_gui = 1;
      } else if (strcmp(argv[i],"-q") == 0) { /* quiet */
        pargs->quiet = 1;
      } else if (strcmp(argv[i],"-s") == 0) { /* symmetric distance */
        pargs->do_symmetric = 1;
      } else if (strcmp(argv[i],"-l") == 0) { /* sampling step */
        if (argc <= i+1) {
          fprintf(stderr,"ERROR: missing argument for -l option\n");
          exit(1);
        }
        pargs->sampling_step = strtod(argv[++i],&endptr);
        if (argv[i][0] == '\0' || *endptr != '\0' ||
            pargs->sampling_step <= 0) {
          fprintf(stderr,"ERROR: invalid number for -l option\n");
          exit(1);
        }
      } else { /* unrecognized option */
        fprintf(stderr,
                "ERROR: unknown option in command line, use -h for help\n");
        exit(1);
      }
    } else { /* file name */
      if (pargs->m1_fname == NULL) {
        pargs->m1_fname = argv[i];
      } else if (pargs->m2_fname == NULL) {
        pargs->m2_fname = argv[i];
      } else {
        fprintf(stderr,
                "ERROR: too many arguments in command line, use -h for help\n");
        exit(1);
      }
    }
    i++; /* next argument */
  }
  pargs->sampling_step /= 100; /* convert percent to fraction */
}

/*****************************************************************************/
/*             fonction principale                                           */
/*****************************************************************************/


int main( int argc, char **argv )
{
  clock_t start_time;
  model_error model1,model2;
  struct face_list *vfl;
  int i;
  QString m1,n1,o1;
  struct face_error *fe = NULL;
  struct face_error *fe_rev = NULL;
  struct dist_surf_surf_stats stats;
  struct dist_surf_surf_stats stats_rev;
  double bbox1_diag,bbox2_diag;
  struct args pargs;
  ScreenWidget *c;
  QApplication *a;
  struct model_info m1info,m2info;

  /* Initialize application */
  i = 0;
  while (i<argc) {
    if (strcmp(argv[i],"-t") == 0) break; /* text version requested */
    if (strcmp(argv[i],"-h") == 0) break; /* just asked for command line help */
    i++;
  }
  if (i == argc) { /* no text version requested, initialize QT */
    a = new QApplication( argc, argv );
  } else {
    a = NULL; /* No QT app needed */
  }

  /* Parse arguments */
  parse_args(argc,argv,&pargs);
  /* Display starting dialog if insufficient arguments */
  if (argc > 1) {
    if (pargs.m1_fname == NULL || pargs.m2_fname == NULL) {
      fprintf(stderr,"ERROR: missing file name(s) in command line\n");
      exit(1);
    }
  } else {
    InitWidget *b;
    b = new InitWidget() ;
    a->setMainWidget(b);
    b->show(); 
    a->exec();
    
    pargs.m1_fname = b->mesh1;
    pargs.m2_fname = b->mesh2;
    pargs.sampling_step = atof(b->step)/100;
  }
  
  /* Read models from input files */
  memset(&model1,0,sizeof(model1));
  memset(&model2,0,sizeof(model2));
  model1.mesh = read_raw_model(pargs.m1_fname);
  model2.mesh = read_raw_model(pargs.m2_fname);

  /* Analyze models (we don't need normals for model 1, so we don't request
   * for it to be oriented). */
  start_time = clock();
  bbox1_diag = dist(model1.mesh->bBox[0], model1.mesh->bBox[1]);
  bbox2_diag = dist(model2.mesh->bBox[0], model2.mesh->bBox[1]);
  vfl = faces_of_vertex(model1.mesh);
  analyze_model(model1.mesh,vfl,&m1info,0);
  model1.info = &m1info;
  analyze_model(model2.mesh,NULL,&m2info,1);
  model2.info = &m2info;
  if(pargs.no_gui){
    free_face_lists(vfl,model1.mesh->num_vert);
    vfl = NULL;
  }
  /* Adjust sampling step size */
  pargs.sampling_step *= bbox2_diag;

  /* Print available model information */
  printf("\n                      Model information\n\n");
  printf("Number of vertices:     \t%11d\t%11d\n",
         model1.mesh->num_vert,model2.mesh->num_vert);
  printf("Number of triangles:    \t%11d\t%11d\n",
         model1.mesh->num_faces,model2.mesh->num_faces);
  printf("BoundingBox diagonal:   \t%11g\t%11g\n",
         bbox1_diag,bbox2_diag);
  printf("Number of disjoint parts:\t%11d\t%11d\n",
         m1info.n_disjoint_parts,m2info.n_disjoint_parts);
  printf("Manifold:               \t%11s\t%11s\n",
         (m1info.manifold ? "yes" : "no"), (m2info.manifold ? "yes" : "no"));
  printf("Originally oriented:    \t%11s\t%11s\n",
         (m1info.orig_oriented ? "yes" : "no"),
         (m2info.orig_oriented ? "yes" : "no"));
  printf("Orientable:             \t%11s\t%11s\n",
         (m1info.orientable ? "yes" : "no"),
         (m2info.orientable ? "yes" : "no"));
  printf("Closed:                 \t%11s\t%11s\n",
         (m1info.closed ? "yes" : "no"),
         (m2info.closed ? "yes" : "no"));
  fflush(stdout);

  /* Compute the distance from one model to the other */
  dist_surf_surf(model1.mesh,model2.mesh,pargs.sampling_step,
                 &fe,&stats,!pargs.no_gui,pargs.quiet);

  /* Print results */
  printf("Surface area:           \t%11g\t%11g\n",
         stats.m1_area,stats.m2_area);
  printf("\n       Distance from model 1 to model 2\n\n");
  printf("        \t   Absolute\t%% BBox diag\n");
  printf("        \t           \t  (Model 2)\n");
  printf("Min:    \t%11g\t%11g\n",
         stats.min_dist,stats.min_dist/bbox2_diag*100);
  printf("Max:    \t%11g\t%11g\n",
         stats.max_dist,stats.max_dist/bbox2_diag*100);
  printf("Mean:   \t%11g\t%11g\n",
         stats.mean_dist,stats.mean_dist/bbox2_diag*100);
  printf("RMS:    \t%11g\t%11g\n",
         stats.rms_dist,stats.rms_dist/bbox2_diag*100);
  printf("\n");
  fflush(stdout);

  if (pargs.do_symmetric) { /* Invert models and recompute distance */
    printf("       Distance from model 2 to model 1\n\n");
    dist_surf_surf(model2.mesh,model1.mesh,pargs.sampling_step,
                   &fe_rev,&stats_rev,0,pargs.quiet);
    free_face_error(fe_rev);
    fe_rev = NULL;
    printf("        \t   Absolute\t%% BBox diag\n");
    printf("        \t           \t  (Model 2)\n");
    printf("Min:    \t%11g\t%11g\n",
           stats_rev.min_dist,stats_rev.min_dist/bbox2_diag*100);
    printf("Max:    \t%11g\t%11g\n",
           stats_rev.max_dist,stats_rev.max_dist/bbox2_diag*100);
    printf("Mean:   \t%11g\t%11g\n",
           stats_rev.mean_dist,stats_rev.mean_dist/bbox2_diag*100);
    printf("RMS:    \t%11g\t%11g\n",
           stats_rev.rms_dist,stats_rev.rms_dist/bbox2_diag*100);
    printf("\n");

    /* Print symmetric distance measures */
    printf("       Symmetric distance between model 1 and model 2\n\n");
    printf("        \t   Absolute\t%% BBox diag\n");
    printf("        \t           \t  (Model 2)\n");
    printf("Min:    \t%11g\t%11g\n",
           max(stats.min_dist,stats_rev.min_dist),
           max(stats.min_dist,stats_rev.min_dist)/bbox2_diag*100);
    printf("Max:    \t%11g\t%11g\n",
           max(stats.max_dist,stats_rev.max_dist),
           max(stats.max_dist,stats_rev.max_dist)/bbox2_diag*100);
    printf("Mean:   \t%11g\t%11g\n",
           max(stats.mean_dist,stats_rev.mean_dist),
           max(stats.mean_dist,stats_rev.mean_dist)/bbox2_diag*100);
    printf("RMS:    \t%11g\t%11g\n",
           max(stats.rms_dist,stats_rev.rms_dist),
           max(stats.rms_dist,stats_rev.rms_dist)/bbox2_diag*100);
    printf("\n");
  }

  printf("               \t       Absolute\t   %% BBox diag model 2\n");
  printf("Sampling step: \t%15g\t%22g\n",pargs.sampling_step,
         pargs.sampling_step/bbox2_diag*100);
  printf("\n");
  if (!pargs.do_symmetric) {
    printf("        \t    Total\tAvg. / triangle\t"
           "      Avg. / triangle\n"
           "        \t         \t     of model 1\t"
           "           of model 2\n");
    printf("Samples:\t%9d\t%15.2f\t%21.2f\n",
           stats.m1_samples,((double)stats.m1_samples)/model1.mesh->num_faces,
           ((double)stats.m1_samples)/model2.mesh->num_faces);
  } else {
    printf("                 \t    Total\tAvg. / triangle\t"
           "      Avg. / triangle\n"
           "                 \t         \t     of model 1\t"
           "           of model 2\n");
    printf("Samples (1 to 2):\t%9d\t%15.2f\t%21.2f\n",
           stats.m1_samples,((double)stats.m1_samples)/model1.mesh->num_faces,
           ((double)stats.m1_samples)/model2.mesh->num_faces);
    printf("Samples (2 to 1):\t%9d\t%15.2f\t%21.2f\n",
           stats_rev.m1_samples,
           ((double)stats_rev.m1_samples)/model1.mesh->num_faces,
           ((double)stats_rev.m1_samples)/model2.mesh->num_faces);
  }
  printf("\n");
  if (!pargs.do_symmetric) {
    printf("                       \t     X\t    Y\t   Z\t   Total\n");
    printf("Partitioning grid size:\t%6d\t%5d\t%4d\t%8d\n",
           stats.grid_sz.x,stats.grid_sz.y,stats.grid_sz.z,
           stats.grid_sz.x*stats.grid_sz.y*stats.grid_sz.z);
  } else {
    printf("                                \t     X\t    Y\t   Z\t   Total\n");
    printf("Partitioning grid size (1 to 2):\t%6d\t%5d\t%4d\t%8d\n",
           stats.grid_sz.x,stats.grid_sz.y,stats.grid_sz.z,
           stats.grid_sz.x*stats.grid_sz.y*stats.grid_sz.z);
    printf("Partitioning grid size (2 to 1):\t%6d\t%5d\t%4d\t%8d\n",
           stats_rev.grid_sz.x,stats_rev.grid_sz.y,stats_rev.grid_sz.z,
           stats_rev.grid_sz.x*stats_rev.grid_sz.y*stats_rev.grid_sz.z);
  }
  printf("\n");
  printf("Execution time (secs.):\t%.2f\n",
         (double)(clock()-start_time)/CLOCKS_PER_SEC);
  fflush(stdout);

  if(pargs.no_gui){
    free_face_error(fe);
    fe = NULL;
  } else {
    /* Get the per vertex error metric */
    calc_vertex_error(&model1,fe,vfl);
    /* Free now useless data */
    free_face_error(fe);
    fe = NULL;
    free_face_lists(vfl,model1.mesh->num_vert);
    vfl = NULL;

   
    c = new ScreenWidget(&model1, &model2);
    QObject::connect(c->quitBut, SIGNAL(clicked()), 
                     a, SLOT(quit()));
    a->setMainWidget( c );
    c->show(); 
    return a->exec();
  }
}

/* $Id: mesh.cpp,v 1.16 2002/02/13 10:38:40 dsanta Exp $ */

#include <time.h>
#include <string.h>
#include <qapplication.h>
#include <qprogressdialog.h>
#include <ScreenWidget.h>
#include <InitWidget.h>

#include <mesh_run.h>
#include <3dmodel_io.h>

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
  fprintf(out,"triangular meshes in RAW or VRML2 formats, optionally with\n");
  fprintf(out,"normals. The VRML parser reads IndexedFaceSets nodes only,\n");
  fprintf(out,"ignoring all transformations, and does not support USE tags\n");
  fprintf(out,"(DEF tags are ignored). The file type is autodetected.\n");
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
  fprintf(out,"      \tthat the sampling density (number of samples per\n");
  fprintf(out,"      \tunit surface) is 1/(s^2) (i.e. one sample per square\n");
  fprintf(out,"      \tof side length s). A probabilistic model is used so\n");
  fprintf(out,"      \tthat the resulting number is as close as possible to\n");
  fprintf(out,"      \tthe target. The default is 0.5\n\n");
  fprintf(out,"  -f\tForce to have at least one sample per triangle of\n");
  fprintf(out,"    \tmodel 1. Otherwise, depending on the sampling step\n");
  fprintf(out,"    \tsize and number of triangles in model 1, it can happen\n");
  fprintf(out,"    \tthat some triangles of model 1 have no samples.\n");
  fprintf(out,"    \tForcing at least one sample per triangle can improve\n");
  fprintf(out,"    \tprecision in some situations, at the expense of larger\n");
  fprintf(out,"    \trunning time.\n\n");
  fprintf(out,"  -wlog\tDisplay textual results in a window instead of on\n");
  fprintf(out,"       \tstandard output. Not compatible with the -t option.\n");
  fprintf(out,"\n");
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
      } else if (strcmp(argv[i], "-f") == 0) { /* sample all triangles */
        pargs->force_sample_all = 1;
      } else if (strcmp(argv[i], "-wlog") == 0) { /* log into window */
	pargs->do_wlog = 1;
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
  if (pargs->no_gui && pargs->do_wlog) {
    fprintf(stderr, "ERROR: incompatible options -t and -wlog\n");
    exit(1);
  }
  pargs->sampling_step /= 100; /* convert percent to fraction */
}

/*****************************************************************************/
/*             fonction principale                                           */
/*****************************************************************************/


int main( int argc, char **argv )
{
  int i;
  QString m1,n1,o1;
  struct args pargs;
  QApplication *a;
  InitWidget *b;
  ScreenWidget *c; 
  TextWidget *textOut;
  QProgressDialog *qProg;
  struct model_error model1,model2;
  int rcode;
  struct outbuf *log;
  struct prog_reporter pr;

  /* Initialize application */
  a = NULL;
  b = NULL;
  c = NULL;
  qProg = NULL;
  memset(&model1,0,sizeof(model1));
  memset(&model2,0,sizeof(model2));
  memset(&pr,0,sizeof(pr));
  log = NULL;
  i = 0;
  while (i<argc) {
    if (strcmp(argv[i],"-t") == 0) /* text version requested */
      break; 
    if (strcmp(argv[i],"-h") == 0) /* just asked for command line help */
      break; 
    i++;
  }
  if (i == argc) { /* no text version requested, initialize QT */
    a = new QApplication( argc, argv );
    if (a != NULL) a->connect( a, SIGNAL(lastWindowClosed()), 
			       a, SLOT(quit()) );
  } else {
    a = NULL; /* No QT app needed */
  }

  /* Parse arguments */
  parse_args(argc,argv,&pargs);
  /* Display starting dialog if insufficient arguments */
  if (pargs.m1_fname != NULL || pargs.m2_fname != NULL) {
    if (pargs.m1_fname == NULL || pargs.m2_fname == NULL) {
      fprintf(stderr,"ERROR: missing file name(s) in command line\n");
      exit(1);
    }
    if (!pargs.do_wlog) {
      log = outbuf_new(stdio_puts,stdout);
    }
    else {
      textOut = new TextWidget();
      log = outbuf_new(TextWidget_puts,textOut);
      textOut->show();
    }
    if (pargs.no_gui) {
      pr.prog = stdio_prog;
      pr.cb_out = stdout;
    } else {
      qProg = new QProgressDialog("Calculating distance",0,100);
      qProg->setMinimumDuration(1500);
      pr.prog = QT_prog;
      pr.cb_out = qProg;
    }
    mesh_run(&pargs,&model1,&model2, log, &pr);
  } else {
    b = new InitWidget(pargs, &model1, &model2);
    b->show(); 
  }
  if (a != NULL) {
    if (pargs.m1_fname != NULL || pargs.m2_fname != NULL) {
      c = new ScreenWidget(&model1, &model2);
      a->setMainWidget(c);
      c->show(); 
    }
    rcode = a->exec();
  } else {
    rcode = 0;
  }
  /* Free model data */
  if (model1.mesh != NULL) free_raw_model(model1.mesh);
  free(model1.verror);
  free(model1.info);
  if (model2.mesh != NULL) free_raw_model(model2.mesh);
  free(model2.verror);
  free(model2.info);
  /* Free widgets */
  outbuf_delete(log);
  delete qProg;
  delete b;
  delete c;
  delete a; // QApplication must be last QT thing to delete
  /* Return exit code */
  return rcode;
}

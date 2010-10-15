/* $Id$ */
#include <3dutils.h>
#include <subdiv.h>
#include <subdiv_methods.h>


int main(int argc, char **argv) {
  char *infile, *outfile;
  struct model *or_model, *sub_model=NULL;
  int lev, nlev=1, rcode, nonopt_argc=1;
  int sub_method=-1, use_binary=0;
  struct subdiv_methods sm={BUTTERFLY_SUBDIV_FUNCTIONS, 
			    LOOP_SUBDIV_FUNCTIONS,
                            SPHERICAL_OR_SUBDIV_FUNCTIONS, 
			    SPHERICAL_ALT_SUBDIV_FUNCTIONS,
                            KOBBELTSQRT3_SUBDIV_FUNCTIONS};
  

  struct subdiv_functions *tmp_func=NULL;
  
  if (argc < 4 || argc > 6) {
    fprintf(stderr, 
	    "Usage: subdiv [-sph_or, -sph_alt, -but, -loop, -ksqrt3][-bin]"
            " infile outfile n_lev\n");
    exit(1);
  }
  
  for (nonopt_argc=1; nonopt_argc<=2; nonopt_argc++) {
#ifdef DEBUG
    printf("argv[%d] = %s\n", nonopt_argc, argv[nonopt_argc]);
#endif
    if (strcmp(argv[nonopt_argc], "-sph_or") == 0) {
      tmp_func = &(sm.spherical_or);
      sub_method = SUBDIV_SPH_OR;
    }
    else if (strcmp(argv[nonopt_argc], "-sph_alt") == 0) {
      tmp_func = &(sm.spherical_alt);
      sub_method = SUBDIV_SPH_ALT;
    }
    else if (strcmp(argv[nonopt_argc], "-but") == 0) {
      tmp_func = &(sm.butterfly);
      sub_method = SUBDIV_BUTTERFLY;
    }
    else if (strcmp(argv[nonopt_argc], "-loop") == 0) {
      tmp_func = &(sm.loop);
      sub_method = SUBDIV_LOOP;
    }
    else if (strcmp(argv[nonopt_argc], "-ksqrt3") == 0) 
      sub_method = SUBDIV_KOB_SQRT3;
    else if (strcmp(argv[nonopt_argc], "-bin") == 0)
      use_binary = 1;
    else
      break;
      
  }
  if (sub_method == -1) {
    fprintf(stderr, "Invalid subdivision method\n");
    fprintf(stderr, 
	    "Usage: subdiv [-sph_or, -sph_alt, -but, -loop, -ksqrt3][-bin]"
            " infile outfile n_lev\n");
    exit(1);
  }

  infile = argv[nonopt_argc];
  outfile = argv[++nonopt_argc];

  if (argc == nonopt_argc+2)
    nlev = atoi(argv[++nonopt_argc]);
  
  if (nlev < 1)
    nlev = 1;

  rcode = read_fmodel(&or_model, infile, MESH_FF_AUTO, 0);

  if (rcode < 0) {
    fprintf(stderr, "Unable to read model - error code %d\n", rcode);
    exit(-1);
  }  


  for (lev=0; lev<nlev; lev++) {

    /* performs the subdivision */
    switch (sub_method) {
    case SUBDIV_KOB_SQRT3: /* handle sqrt3 stuff separately */
      sub_model = subdiv_sqrt3(or_model, &(sm.kob_sqrt3));
      break;
    default: /* 4-to-1 split */
      sub_model = subdiv(or_model, tmp_func);     
      break;
    }

    
    __free_raw_model(or_model);
    
    or_model = sub_model;
  }
  write_raw_model(sub_model, outfile, use_binary);

  __free_raw_model(sub_model);
  return 0;
}

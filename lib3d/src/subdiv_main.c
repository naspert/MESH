/* $Id: subdiv_main.c,v 1.7 2003/03/12 17:55:00 aspert Exp $ */
#include <3dutils.h>
#include <subdiv.h>
#include <subdiv_methods.h>


int main(int argc, char **argv) {
  char *infile, *outfile;
  struct model *or_model, *sub_model=NULL;
  int lev, nlev=1, rcode;
  int sub_method=-1;


  if (argc != 4 && argc != 5) {
    fprintf(stderr, 
	    "Usage: subdiv [-sph, -but, -loop, -ksqrt3]"
            " infile outfile n_lev\n");
    exit(1);
  }
  if (strcmp(argv[1], "-sph") == 0) 
    sub_method = SUBDIV_SPH;
  else if (strcmp(argv[1], "-but") == 0) 
    sub_method = SUBDIV_BUTTERFLY;
  else if (strcmp(argv[1], "-loop") == 0)
    sub_method = SUBDIV_LOOP;
  else if (strcmp(argv[1], "-ksqrt3") == 0)
    sub_method = SUBDIV_KOB_SQRT3;
  else {
    fprintf(stderr, "Invalid subdivision method %s\n", argv[1]);
    fprintf(stderr, 
	    "Usage: subdiv [-sph, -but, -loop, -ksqrt3]"
            " infile outfile n_lev\n");
    exit(1);
  }

  infile = argv[2];
  outfile = argv[3];

  if (argc==5)
    nlev = atoi(argv[4]);
  
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
    case SUBDIV_SPH:
      sub_model = subdiv(or_model, SUBDIV_SPH, compute_midpoint_sph, 
                         compute_midpoint_sph_crease, NULL);
      break;
    case SUBDIV_LOOP:
      sub_model = subdiv(or_model, SUBDIV_LOOP, compute_midpoint_loop, 
                         compute_midpoint_loop_crease,
                         update_vertices_loop);
      break;
    case SUBDIV_BUTTERFLY:
      sub_model = subdiv(or_model, SUBDIV_BUTTERFLY, 
                         compute_midpoint_butterfly, 
                         compute_midpoint_butterfly_crease, NULL);
      break;
    case SUBDIV_KOB_SQRT3:
      sub_model = subdiv_sqrt3(or_model, SUBDIV_KOB_SQRT3, 
                               compute_face_midpoint_kobsqrt3, 
                               NULL, update_vertices_kobsqrt3);
      break;
    default:
      fprintf(stderr, "ERROR : Invalid subdivision method found = %d\n", 
              sub_method);
      exit(1);
      break;
    }

    
    __free_raw_model(or_model);
    
    or_model = sub_model;
  }
  write_raw_model(sub_model, outfile);

  __free_raw_model(sub_model);
  return 0;
}

/* $Id: subdiv_main.c,v 1.3 2002/10/31 10:26:13 aspert Exp $ */
#include <3dutils.h>
#include <subdiv.h>
#include <subdiv_methods.h>


int main(int argc, char **argv) {
  char *infile, *outfile;
  struct model *or_model, *sub_model=NULL;
  struct info_vertex* tmp_vert;
  int i, lev, nlev=1, rcode;
  int sub_method=-1;


  if (argc != 4 && argc != 5) {
    fprintf(stderr, 
	    "Usage: subdiv [-sph, -but, -loop, -loopb]"
            " infile outfile n_lev\n");
    exit(1);
  }
  if (strcmp(argv[1], "-sph") == 0) 
    sub_method = SUBDIV_SPH;
  else if (strcmp(argv[1], "-but") == 0) 
    sub_method = SUBDIV_BUTTERFLY;
  else if (strcmp(argv[1], "-loop") == 0)
    sub_method = SUBDIV_LOOP;
  else if (strcmp(argv[1], "-loopb") == 0)
    sub_method = SUBDIV_LOOP_BOUNDARY;
  else {
    fprintf(stderr, "Invalid subdivision method %s\n", argv[1]);
    fprintf(stderr, 
	    "Usage: subdiv [-sph, -but, -loop, -loopb]"
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
    if (or_model->normals == NULL && sub_method == SUBDIV_SPH) {
      tmp_vert = (struct info_vertex*)
	malloc(or_model->num_vert*sizeof(struct info_vertex));
      or_model->area = (float*)malloc(or_model->num_faces*sizeof(float));
      or_model->face_normals = compute_face_normals(or_model, tmp_vert);
      compute_vertex_normal(or_model, tmp_vert, or_model->face_normals);
      for (i=0; i<or_model->num_vert; i++) 
 	free(tmp_vert[i].list_face); 
      free(tmp_vert);
    }

    /* performs the subdivision */
    switch (sub_method) {
    case SUBDIV_SPH:
      sub_model = subdiv(or_model, compute_midpoint_sph, 
                         compute_midpoint_sph_crease, NULL);
      break;
    case SUBDIV_LOOP:
      sub_model = subdiv(or_model, compute_midpoint_loop, NULL, 
			 update_vertices_loop);
      break;
    case SUBDIV_LOOP_BOUNDARY:
      sub_model = subdiv(or_model, compute_midpoint_loop, 
			 compute_midpoint_loop_crease,
			 update_vertices_loop_crease);
      break;
    case SUBDIV_BUTTERFLY:
      sub_model = subdiv(or_model, compute_midpoint_butterfly, 
			 compute_midpoint_butterfly_crease, NULL);
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

/* $Id: lapl.c,v 1.1 2003/01/09 11:59:58 aspert Exp $ */
#include <3dmodel.h>
#include <geomutils.h>
#include <ring.h>
#include <3dmodel_io.h>
#include <model_in.h>

int main(int argc, char **argv) {
  char *in_fname, *out_fname;
  struct model *raw_model;
  vertex_t *new_vert, tmp;
  struct ring_info *ring;
  int i, j, rcode, n_lev=1, lev;

  if (argc != 3 && argc != 4) {
    fprintf(stderr, "Usage: lapl infile outfile [n_lev]\n");
    exit(-1);
  }

  in_fname = argv[1];
  out_fname = argv[2];
  
  if (argc == 4) {
    i = atoi(argv[3]);
    n_lev = (i < 1)?1:i;
  }

  rcode = read_fmodel(&raw_model, in_fname, MESH_FF_AUTO, 0);
  if (rcode < 0) {
    fprintf(stderr, "Unable to read model, error code = %d\n", rcode);
    exit(rcode);
  }

  ring = 
    (struct ring_info*)malloc(raw_model->num_vert*sizeof(struct ring_info));
  new_vert =  (vertex_t*)malloc(raw_model->num_vert*sizeof(vertex_t));
  
  for (lev=0; lev<n_lev; lev++) {
    build_star_global(raw_model, ring);

    for (i=0; i<raw_model->num_vert; i++) {
      tmp.x = tmp.y = tmp.z = 0.0;
      for (j=0; j<ring[i].size; j++) 
        __add_v(raw_model->vertices[ring[i].ord_vert[j]], tmp, tmp);
      
      __prod_v(1.0/(float)ring[i].size, tmp, tmp);
      __add_v(raw_model->vertices[i], tmp, new_vert[i]);
    }
    
    memcpy(raw_model->vertices, new_vert, 
           raw_model->num_vert*sizeof(vertex_t));

    for (j=0; j<raw_model->num_vert; j++) {
      free(ring[j].ord_vert);
      free(ring[j].ord_face);
    }
  }
  write_raw_model(raw_model, out_fname);
  free(new_vert);
 
  free(ring);
  __free_raw_model(raw_model);

  return 0;
}

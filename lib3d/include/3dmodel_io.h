/* $Id: 3dmodel_io.h,v 1.7 2001/10/23 09:29:36 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#ifdef __cplusplus
extern "C" {
#endif
  int read_header(FILE*, int*, int*, int*, int*);
  struct model* alloc_read_model(FILE*, int, int, int, int);
  struct model* read_raw_model(char*);
  struct model* read_raw_model_frame(char*, int);
  void write_raw_model(struct model*, char*);
  void free_raw_model(struct model*);
  void write_brep_file(struct model*, char*, int, int, int, 
		       vertex_t, vertex_t);
#ifdef __cplusplus
}
#endif


#endif

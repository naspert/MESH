/* $Id: 3dmodel_io.h,v 1.6 2001/09/27 11:44:42 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#ifdef __cplusplus
extern "C" {
#endif
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

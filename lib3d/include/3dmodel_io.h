/* $Id: 3dmodel_io.h,v 1.9 2002/02/26 13:18:15 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#define EINVAL_HEADER 0x10
#define EINVAL_NORMAL 0x20

#ifdef __cplusplus
extern "C" {
#endif
  int read_header(FILE*, int*);
  struct model* alloc_read_model(FILE*, int*);
  struct model* read_raw_model(char*);
  struct model* read_raw_model_frame(char*, int);
  void write_raw_model(struct model*, char*);
  void write_brep_file(struct model*, char*, int, int, int, 
		       vertex_t, vertex_t);
#ifdef __cplusplus
}
#endif


#endif

/* $Id: 3dmodel_io.h,v 1.4 2001/07/02 12:50:02 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#ifdef __cplusplus
extern "C" {
#endif
  model* read_raw_model(char*);
  model* read_raw_model_frame(char*, int);
  void write_raw_model(model*, char*);
  void write_brep_file(model*, char*, int, int, int, vertex, vertex);
#ifdef __cplusplus
}
#endif


#endif

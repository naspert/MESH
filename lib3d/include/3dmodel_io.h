/* $Id: 3dmodel_io.h,v 1.3 2001/04/26 11:40:21 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#ifdef __cplusplus
extern "C" {
#endif
  model* read_raw_model(char*);
  model* read_raw_model_frame(char*, int);
  void write_raw_model(model*, char*);
#ifdef __cplusplus
}
#endif


#endif

/* $Id: 3dmodel_io.h,v 1.2 2001/04/25 11:29:56 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#ifdef _CPLUSPLUS
extern "C" {
#endif
  model* read_raw_model(char*);
  model* read_raw_model_frame(char*, int);
  void write_raw_model(model*, char*);
#ifdef _CPLUSPLUS
}
#endif


#endif

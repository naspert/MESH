/* $Id: 3dmodel_io.h,v 1.10 2003/03/26 08:59:24 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

#define EINVAL_HEADER 0x10
#define EINVAL_NORMAL 0x20

#ifdef __cplusplus
extern "C" {
#endif
  struct model* read_raw_model(const char*);
  void write_raw_model(const struct model*, char*, const int);
#ifdef __cplusplus
}
#endif


#endif

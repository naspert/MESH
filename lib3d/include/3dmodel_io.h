/* $Id: 3dmodel_io.h,v 1.1 2001/03/12 14:50:32 aspert Exp $ */
#include <3dmodel.h>

#ifndef _3DMODEL_IO_PROTO
#define _3DMODEL_IO_PROTO

model* read_raw_model(char*);
model* read_raw_model_frame(char*, int);
void write_raw_model(model*, char*);

#endif

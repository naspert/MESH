/* $Id: compute_error.h,v 1.1 2001/04/30 07:50:47 jacquet Exp $ */
#include <3dmodel.h>
#include <geomutils.h>

#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO

#ifdef __cplusplus
extern "C" {
#endif
  double distance(vertex,vertex,vertex);
  sample* echantillon(vertex, vertex, vertex, double);
  cellules* liste(int, model*);
  int** cublist(cellules*,int,model*);
  double pcd(vertex,model*,int**,int,FILE*);

#ifdef __cplusplus
}
#endif


#endif

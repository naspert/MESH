/* $Id: compute_error.h,v 1.1 2001/04/27 07:14:12 jacquet Exp $ */
#include <3dmodel.h>
#include <geomutils.h>

#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO

#ifdef _CPLUSPLUS
extern "C" {
#endif
  double distance(vertex,vertex,vertex);
  sample* echantillon(vertex, vertex, vertex, double);
  cellules* liste(int, model*);
  int** cublist(cellules*,int,model*);
  double pcd(vertex,model*,int**,int);

#ifdef _CPLUSPLUS
}
#endif


#endif

/* $Id: compute_error.h,v 1.2 2001/05/03 08:01:24 jacquet Exp $ */
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
  void listoffaces(model *,int*,int**);

#ifdef __cplusplus
}
#endif


#endif

/* $Id: compute_error.h,v 1.3 2001/05/30 12:41:33 jacquet Exp $ */
#include <3dmodel.h>
#include <geomutils.h>

#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO


#ifdef __cplusplus
extern "C" {
#endif
  double dist_pt_cellule(vertex, int, int, int, int, int, int, double);
  double dist_pt_surf(vertex A,vertex B,vertex C,vertex point,vertex normal);
  double distance(vertex,vertex,vertex);
  double echantillondist(vertex, vertex, vertex,vertex, double);
  sample* echantillon(vertex, vertex, vertex, double);
  cellules* liste(model*,double,vertex,double);
  int** cublist(cellules*,model*,vertex);
  double pcd(vertex,model*,int**,vertex,double);
  void listoffaces(model *,int*,int**);
  double err_moy(double **,sample *,int);

#ifdef __cplusplus
}
#endif


#endif
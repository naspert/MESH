/* $Id: ColorMap.h,v 1.3 2001/10/01 16:46:42 dsanta Exp $ */
#ifndef COLORMAP_H
#define COLORMAP_H

#include <3dutils.h>

#ifdef __cplusplus
extern "C" {
#endif
double** HSVtoRGB(void);
void free_colormap(double **);
#ifdef __cplusplus
}
#endif
#endif

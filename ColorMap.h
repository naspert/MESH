/* $Id: ColorMap.h,v 1.4 2001/11/12 15:25:02 dsanta Exp $ */
#ifndef COLORMAP_H
#define COLORMAP_H

#ifdef __cplusplus
extern "C" {
#endif
double** HSVtoRGB(void);
void free_colormap(double **);
#ifdef __cplusplus
}
#endif
#endif

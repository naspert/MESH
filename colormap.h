/* $Id: colormap.h,v 1.1 2002/02/20 18:09:07 dsanta Exp $ */

#ifndef _COLORMAP_PROTO
#define _COLORMAP_PROTO

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* Returns an HSV colormap with hue uniformly spaced from 240 to zero degrees
 * and with len entries. The colormap is returned in a matrix of 3 columns and
 * len rows, each row representing the RGB value of that colormap entry. RGB
 * values are given in the [0,1] range. The colormap values are obtained by
 * varying the hue for colors with full saturation and value. Hue goes from
 * blue (240 degrees) passing through cyan, green, yellow and red (zero
 * degrees). len should be no less than 2. */
float** colormap_hsv(int len);

/* Frees a colormap */
void free_colormap(float **cmap);

END_DECL
#undef END_DECL

#endif /* _COLORMAP_PROTO */

/* $Id: colormap.c,v 1.2 2002/02/21 09:15:18 dsanta Exp $ */

#include <colormap.h>
#include <stdlib.h>
#include <math.h>
#include <xalloc.h>

/* Transforms a hue (in degrees) to the RGB equivalent. The saturation and
 * value are taken as the maximum. */
static void hue2rgb(float hue, float *r, float *g, float *b) {
  float p,n;
  int k;

  /* Get principal component of angle */
  hue -= 360*(float)floor(hue/360);
  /* Get section */
  hue /= 60;
  k = (int)floor(hue);
  if (k==6) {
    k = 5;
  } else if (k < 0) {
    k = 0;
  }
  p = hue-k;
  n = 1-p;

  /* Get RGB values based on section */
  switch (k) {
  case 0:
    *r = 1;
    *g = p;
    *b = 0;
    break;
  case 1:
    *r = n;
    *g = 1;
    *b = 0;
    break;
  case 2:
    *r = 0;
    *g = 1;
    *b = p;
    break;
  case 3:
    *r = 0;
    *g = n;
    *b = 1;
    break;
  case 4:
    *r = p;
    *g = 0;
    *b = 1;
    break;
  case 5:
    *r = 1;
    *g = 0;
    *b = n;
    break;
  default:
    abort(); /* should never get here */
  }
}

/* see colormap.h */
float** colormap_hsv(int len)
{
  float **cmap,step;
  int i;

  if (len <= 1) return NULL;

  cmap = xa_malloc(len*sizeof(*cmap));
  *cmap = xa_malloc(3*len*sizeof(**cmap));
  for (i=1; i<len; i++) {
    cmap[i] = cmap[i-1]+3;
  }

  step = 240/(float)(len-1);
  for (i=0; i<len ; i++) {
    hue2rgb(step*(len-1-i),cmap[i],cmap[i]+1,cmap[i]+2);
  }
  return cmap;
}

/* see colormap.h */
void free_colormap(float **cmap)
{
  if (cmap != NULL) {
    free(*cmap);
    free(cmap);
  }
}

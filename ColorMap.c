/* $Id: ColorMap.c,v 1.9 2001/11/12 15:25:02 dsanta Exp $ */
#include <ColorMap.h>
#include <xalloc.h>
#include <3dutils.h>

/* This function generates a colormap for the HSV colorspace using */
/* normalized RGB values. */
double** HSVtoRGB(void)
{
  double **colormap;
  double f, p, q, t;
  double r=-1, g=-1, b=-1, hue;
  int i, j=0, h;

  colormap = (double **)xa_malloc(8*sizeof(double*));
  
  for(i=0;i<8;i++)
    colormap[i] = (double *)xa_malloc(3*sizeof(double));
  

  for(h=0; h<240; h+=30){
    hue = ((double)h)/60.0;
    i = (int)floor(hue);
    f = hue - (double)i;
    p = 0.0;
    q = 1.0 - f;
    t = f;

    switch(i){
    case 0:
      r = 1.0; 
      g = t; 
      b = p;
      break;
    case 1:
      r = q; 
      g = 1.0; 
      b = p;
      break;
    case 2:
      r = p; 
      g = 1.0; 
      b = t;
      break;
    case 3:
      r = p; 
      g = q; 
      b = 1.0;
      break;
    case 4:
      r = t; 
      g = p;
      b = 1.0;
      break;
    case 5:
      r = 1.0; 
      g = p; 
      b = q;
      break;
    default:
      fprintf(stderr,"HSVtoRGB: out of range value for hue!\n");
      break;
    }

    colormap[7-j][0] = max(min(r,1.0),0.0);
    colormap[7-j][1] = max(min(g,1.0),0.0);
    colormap[7-j][2] = max(min(b,1.0),0.0);
    j++;  
  }

  return(colormap);
}

void free_colormap(double **cmap)
{
  int i;

  if (cmap != NULL) {
    for (i=0; i<8; i++) {
      free(cmap[i]);
    }
    free(cmap);
  }
}

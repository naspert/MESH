/* $Id: ColorMap.c,v 1.3 2001/08/06 11:48:34 aspert Exp $ */
#include <ColorMap.h>




/*****************************************************************************/
/*            creation d'une colormap                                        */
/*****************************************************************************/
double** HSVtoRGB()
{
  double **colormap;
  double f, p, q, t;
  double r=-1, g=-1, b=-1, hue;
  int i, j=0, h;

  colormap = (double **)malloc(8*sizeof(double*));
  
  for(i=0;i<8;i++)
    colormap[i] = (double *)malloc(3*sizeof(double));
  

  for(h=0; h<240; h+=30){
    hue = ((double)h)/60.0;
    i = floor(hue);
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
      printf("Out of range value for hue\n");
      break;
    }

    colormap[7-j][0] = r;
    colormap[7-j][1] = g;
    colormap[7-j][2] = b;
    j++;  
  }

  return(colormap);
}

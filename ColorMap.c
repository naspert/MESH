#include <ColorMap.h>

#include <ColorMap.h>


/*****************************************************************************/
/*            creation d'une colormap                                        */
/*****************************************************************************/
double** HSVtoRGB()
{
  double **colormap;
  double f,p,q,t,r,g,b,h,hue;
  int i,j=0;

  colormap=(double **)malloc(8*sizeof(double*));
  for(i=0;i<8;i++){
    colormap[i]=(double *)malloc(3*sizeof(double));
  }

  for(h=0.0;h<240.0;h=h+240.0/8.0){
    hue=h/60.0;
    i=floor(hue);
    f=hue-i;
    p=0.0;
    q=1-f;
    t=f;
    switch(i){
    case 0:r=1; g=t; b=p;break;
    case 1:r=q; g=1; b=p;break;
    case 2:r=p; g=1; b=t;break;
    case 3:r=p; g=q; b=1;break;
    case 4:r=t; g=p; b=1;break;
    case 5:r=1; g=p; b=q;break;
    
    }
    colormap[7-j][0]=r;
    colormap[7-j][1]=g;
    colormap[7-j][2]=b;
    j++;  
  }

  return(colormap);
}

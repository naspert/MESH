/* $Id: colormap.c,v 1.10 2003/01/13 12:46:07 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2003 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */







#include <colormap.h>
#include <stdlib.h>
#include <math.h>
#include <xalloc.h>

/* Transforms a hue (in degrees) to the RGB equivalent. The saturation and
 * value are taken as the maximum. 
 */
static void hue2rgb(float hue, float *r, float *g, float *b) {
  float p,n;
  int k;

  /* Get principal component of angle */
  hue -= 360*(float)floor(hue/360);
  /* Get section */
  hue /= 60;
  k = (int)floor(hue);
  if (k == 6) {
    k = 5;
  } else if (k < 0) {
    k = 0;
  }
  p = hue - k;
  n = 1 - p;

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
  for (i=0; i<len ; i++) 
    hue2rgb(step*(len-1-i),cmap[i],cmap[i]+1,cmap[i]+2);
  
  return cmap;
}

/* see colormap.h */
float** colormap_gs(int len)
{
  float **cmap,step;
  int i;

  if (len <= 1) return NULL;

  cmap = xa_malloc(len*sizeof(*cmap));
  *cmap = xa_malloc(3*len*sizeof(**cmap));
  for (i=1; i<len; i++) {
    cmap[i] = cmap[i-1]+3;
  }

  step = 1/(float)(len-1);
  for (i=0; i<len ; i++) 
    cmap[i][0] = cmap[i][1] = cmap[i][2] = step*(len - 1 - i);
  
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

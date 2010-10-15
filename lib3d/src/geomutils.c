/* $Id$ */


/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
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







#include <3dmodel.h>
#define _GEOMUTILS_C_
#include <geomutils.h> /* catch the inlined functions */


/* Rotates the point 'p' around the axis 'u' */
void rotate_3d(vertex_t p, vertex_t u, double theta, vertex_t *vout) {
  double cth=cos(theta);
  double sth=sin(theta);
  double a=1.0-cth;
  double tmp;

  
  __normalize_v(u); /* useful ? */

  tmp = (cth + a*u.x*u.x)*p.x;
  tmp += (a*u.x*u.y - sth*u.z)*p.y;
  tmp += (a*u.x*u.z +u.y*sth)*p.z;
  vout->x = (float) tmp;

  tmp = (a*u.x*u.y + u.z*sth)*p.x;
  tmp += (cth +a*u.y*u.y)*p.y;
  tmp += (a*u.y*u.z -u.x*sth)*p.z;
  vout->y = (float) tmp;

  tmp = (a*u.x*u.z -u.y*sth)*p.x;
  tmp += (u.y*u.z*a + u.x*sth)*p.y;
  tmp += (u.z*u.z*a + cth)*p.z;
  vout->z = (float) tmp;
  
}

/* Computes the radius and center of the 2D circle passing by p1 p2 and p3 */
void compute_circle2d(vertex_t p1, vertex_t p2, vertex_t p3, 
		      float *r, vertex_t *center) {
    vertex_t v1, v2;
    float cp; 
    float num, np1, np2, np3;


    v1.x = p2.x - p1.x;
    v1.y = p2.y - p1.y;
    
    v2.x = p3.x - p1.x;
    v2.y = p3.y - p1.y;
    
    cp = v1.x*v2.y - v1.y*v2.x;

    if (fabs(cp)>1e-10) {
	np1 = __norm_v(p1);
	np2 = __norm_v(p2);
	np3 = __norm_v(p3);
	num = np1*(p2.y - p3.y) + np2*(p3.y - p1.y) +
	    np3*(p1.y - p2.y);
	center->x = num/(2.0f*cp);
	num = np1*(p3.x - p2.x) + np2*(p1.x - p3.x) +
	    np3*(p2.x - p1.x);
	center->y = num/(2.0f*cp);
    }

    v1.x = center->x - p1.x;
    v1.y = center->y - p1.y;
    v1.z = 0.0;

    *r = __norm_v(v1);

}

/* Compute the center and radius of the circle passing
   by the points p1,p2 and p3 */
void compute_circle3d(vertex_t p1, vertex_t p2, vertex_t p3, 
                      float *r, vertex_t *center) {
    vertex_t u,v,w;
    vertex_t m1, m2, tmp;
    int i;
    float det;
    float a[9], b[3];

    __substract_v(p2, p1, u);
    __substract_v(p3, p1, v);


    __crossprod_v(u, v, w);

   
    /* The center of the circle is the 
       intersection of 3 planes */

    __add_v(p1, p2, m1);
    __prod_v(0.5f, m1, m1);

    __add_v(p1, p3, m2);
    __prod_v(0.5f, m2, m2);


    /* Is the equation system OK ? */
    det = u.x*(v.y*w.z - v.z*w.y) - 
	v.x*(u.y*w.z - u.z*w.y) + w.x*(u.y*v.z - u.z*v.y);
    
    if (fabs(det) < 1e-6) {
	printf("Error solving system\n");
	exit(0);
    }

    /* Brute force matrix inversion */
    a[0] = (v.y*w.z - v.z*w.y)/det;
    a[1] = (u.z*w.y - u.y*w.z)/det;
    a[2] = (u.y*v.z - u.z*v.y)/det;
    a[3] = (v.z*w.x - v.x*w.z)/det;
    a[4] = (u.x*w.z - u.z*w.x)/det;
    a[5] = (u.z*v.x - v.z*u.x)/det;
    a[6] = (v.x*w.y - w.x*v.y)/det;
    a[7] = (u.y*w.x - w.y*u.x)/det;
    a[8] = (u.x*v.y - v.x*u.y)/det;

    b[0] = u.x*m1.x + u.y*m1.y + u.z*m1.z;
    b[1] = v.x*m2.x + v.y*m2.y + v.z*m2.z;
    b[2] = 0.0;

   
    center->x = 0.0;
    for (i=0; i<3; i++)
	 center->x += a[i]*b[i];
    
    center->y = 0.0;
    for (i=0; i<3; i++)
	 center->y += a[i+3]*b[i];

    center->z = 0.0;
    for (i=0; i<3; i++)
	 center->z += a[i+6]*b[i];

    substract_v(center, &p1, &tmp);
    tmp.x = center->x - p1.x;
    tmp.y = center->y - p1.y;
    tmp.z = center->z - p1.z;

    *r = __norm_v(tmp);
    
}

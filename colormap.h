/* $Id: colormap.h,v 1.8 2003/01/13 12:46:07 aspert Exp $ */


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

/* Returns a uniform grayscale colormap. This is convenient to make
 * grayscale screenshots (especially when printing a color page in a
 * paper costs 1000$ :-) */
float** colormap_gs(int len);

/* Frees a colormap */
void free_colormap(float **cmap);

END_DECL
#undef END_DECL

#endif /* _COLORMAP_PROTO */

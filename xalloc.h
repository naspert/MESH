/* $Id: xalloc.h,v 1.7 2002/08/30 09:18:44 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
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







/*
 * xalloc: non-failing memory allocation functions
 */

#ifndef _XALLOC_PROTO
#define _XALLOC_PROTO

#include <stddef.h>

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

/* Give hints for more optimization */
#if defined(__GNUC__) && (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define XALLOC_ATTR __attribute__ ((__malloc__))
#else
#define XALLOC_ATTR
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* Same as malloc, but exits if out of memory. */
void * xa_malloc(size_t size) XALLOC_ATTR;

/* Same as calloc, but exits if out of memory. */
void * xa_calloc(size_t nmemb, size_t size) XALLOC_ATTR;

/* Same as realloc, but exits if out of memory. Not suitable for the
 * XALLOC_ATTR since it might return the same address multiple times. */
void * xa_realloc(void *ptr, size_t size);

END_DECL
#undef END_DECL

#endif /* _XALLOC_PROTO */

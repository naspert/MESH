/* $Id: debug_print.h,v 1.5 2003/01/13 12:40:54 aspert Exp $ */

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

#ifndef DEBUG_PRINT_PROTO
#define DEBUG_PRINT_PROTO

#include <stdio.h>

#if defined(__GNUC__) && (__GNUC__>2 || __GNUC__==2 && __GNUC_MINOR__>=95) 

# define DEBUG_PRINT(format, args...)                           \
do {                                                            \
  printf("[%s:%d:%s]: ", __FILE__, __LINE__, __FUNCTION__);     \
  printf(format , ## args );                                    \
} while(0)

#elif defined(__STDC__)
/* Microsoft Visual C++ and Sun/SGI compilers do not know about
 * __FUNCTION__, nor about varargs macros... */
/* Of course, if this is in a loop, do not forget the {} brackets, 
 * 'cause this is kind of a hack.... */
# define DEBUG_PRINT printf("[%s:%d]: ", __FILE__, __LINE__);printf
#else 
/* we just alias DEBUG_PRINT to 'printf', still works, but almost
 * useless for debugging, since we don't know at all what line prints
 * information... */
# define DEBUG_PRINT printf
#endif


#endif

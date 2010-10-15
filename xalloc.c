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








#include <xalloc.h>
#include <stdlib.h>
#include <stdio.h>

static void _xa_outofmem(size_t size)
{
  fprintf(stderr,"Out of memory (requested %u bytes). Exit\n",size);
  fprintf(stderr,
          "To locate the offending function call, put a breakpoint\n"
          "with a debugger in the _xa_outofmem function.\n");
  exit(1);
}

void * xa_malloc(size_t size)
{
  void *ptr;
  ptr = malloc(size);
  if (ptr == NULL) _xa_outofmem(size);
  return ptr;
}

void * xa_calloc(size_t nmemb, size_t size)
{
  void *ptr;
  ptr = calloc(nmemb,size);
  if (ptr == NULL) _xa_outofmem(nmemb*size);
  return ptr;
}

void * xa_realloc(void *ptr, size_t size)
{
  void *new_ptr;
  new_ptr = realloc(ptr,size);
  if (new_ptr == NULL && size != 0) _xa_outofmem(size);
  return new_ptr;
}

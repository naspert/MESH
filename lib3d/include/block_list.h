/* $Id: block_list.h,v 1.3 2003/01/13 12:46:08 aspert Exp $ */

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

/* Single-linked list of memory blocks - structure declaration and
 * prototypes of the handlers, written by N. Aspert, just because
 * Win32 systems are _slow_ at realloc-ing big arrays ;-) */

#ifndef BLOCKLIST_PROTO
#define BLOCKLIST_PROTO
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif 

/* --------------------------------------------------------------------------
   DATA BLOCKS LINKED LIST STRUCTURE
   -------------------------------------------------------------------------- */

/* Number of elements alloc-ed in each 'data' field of the
 * 'block_list', i.e. "data=malloc(BL_NELEM*size_elem)" */
#define BL_NELEM 512

struct block_list {
  int nelem; /* number of elements allocated in the block */
  int elem_filled; /* number of elements actually filled in the block */
  size_t size_elem; /* size of one element */
  void *data; /* nelem*size_elem bytes */
  struct block_list *next; /* next data block, NULL if last block*/
};


/* Error codes - always negative (and non-overlapping with those from
 * model_in.h) */
#define BLK_NULL         -16
#define DATA_NULL        -17
#define GATHER_FAIL_MEM  -18

/* --------------------------------------------------------------------------
   EXPORTED FUNCTIONS
   -------------------------------------------------------------------------- */

/* See block_list.c for details */
int init_block_list(struct block_list *, size_t);

struct block_list* get_next_block(struct block_list*);

int gather_block_list(struct block_list* , void*, size_t);

void free_block_list(struct block_list**);

#ifdef __cplusplus
}
#endif

#endif

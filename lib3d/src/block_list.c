/* $Id: block_list.c,v 1.5 2004/04/30 07:50:22 aspert Exp $ */

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


/* Basic handling functions for chained memory block structure, written by
 * N. Aspert 
 * You can initialize a list, add new blocks, gather them into an
 * array and destroy a list, which should be sufficient. It is
 * significantly faster (up to a factor 2 !) than just realloc-ing a
 * big array on Win32 systems. 
 */


#include <block_list.h>
#ifdef DEBUG
# include <debug_print.h>
#endif

/* Create a block_list element having elements of size 'sz_elem' */
int init_block_list(struct block_list *blk, size_t sz_elem) 
{
  if (blk == NULL)
    return BLK_NULL;
  blk->size_elem = sz_elem;
  blk->elem_filled = 0;
  blk->next = NULL;
  blk->data = malloc(BL_NELEM*sz_elem);
  if (blk->data == NULL)
    return DATA_NULL;
  blk->nelem = BL_NELEM;
  return 0;
}

/* Append a new 'block_list' element to the block pointed by 'blk'. It
 * has the same size/sz_elem, and returns a pointer to the new element
 * or NULL if an error occured.
 */
struct block_list* get_next_block(struct block_list *blk)
{
  int rcode;
  if (blk == NULL)
    return NULL;
  
  blk->next = (struct block_list*)malloc(sizeof(struct block_list));
  if (blk->next == NULL)
    return NULL;

  rcode = init_block_list(blk->next, blk->size_elem);
#ifdef DEBUG
  DEBUG_PRINT("new block = %d bytes\n", blk->size_elem*BL_NELEM);
#endif
 if(rcode<0)
   return NULL;
 
 return blk->next;
}


/* Copy all the data of the block_list pointed by 'head_blk' into the
 * array 'dest'. Of course, 'dest' *MUST* be alloc-ed correctly to handle
 * all the data contained in the list. If the size of the destination
 * array (passed in sz_dest) is not sufficient, the function returns
 * GATHER_FAIL_MEM, and 0 otherwise. */
int gather_block_list(struct block_list *head_blk, void *dest, size_t sz_dest) 
{
  size_t tmp=0;
  int rcode = 0;
  struct block_list *cur_blk=head_blk;
  char *pt_dest=(char*)dest;
  while (cur_blk != NULL) {
    tmp += cur_blk->elem_filled*cur_blk->size_elem;
    if (tmp > sz_dest) {
      rcode = GATHER_FAIL_MEM;
      break;
    }
    memcpy(pt_dest, cur_blk->data, cur_blk->elem_filled*cur_blk->size_elem);
#ifdef DEBUG
    DEBUG_PRINT("%d bytes gathered\n", tmp);
#endif
    pt_dest = pt_dest + cur_blk->elem_filled*cur_blk->size_elem;
    cur_blk = cur_blk->next;
  }
  return rcode;
} 

/* Trashs the whole block_list pointed by 'blk', as well as all its
 * siblings */
void free_block_list(struct block_list **blk) 
{
  struct block_list *cur_blk=*blk, *old_blk;

  while (cur_blk != NULL) {
    free(cur_blk->data);
    old_blk = cur_blk;
    cur_blk = old_blk->next;
    free(old_blk);
  }
}


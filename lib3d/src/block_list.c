/* $Id: block_list.c,v 1.1 2002/11/04 15:27:46 aspert Exp $ */

#include <block_list.h>

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
  printf("[get_next_block]new block = %d bytes\n", blk->size_elem*BL_NELEM);
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
    printf("[gather_block_list]%d bytes gathered\n", tmp);
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


/* $Id: block_list.h,v 1.1 2002/11/04 15:27:45 aspert Exp $ */

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

/* $Id: xalloc.c,v 1.1 2001/08/17 08:58:52 dsanta Exp $ */

#include <xalloc.h>
#include <stdlib.h>
#include <stdio.h>

static void _xa_outofmem(size_t size)
{
  fprintf(stderr,"Out of memory (requested %ud bytes). Exit\n",size);
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

/* $Id: mutils.c,v 1.2 2001/08/16 14:59:14 dsanta Exp $ */

#include <mutils.h>
#include <stdlib.h>
#include <stdio.h>

static void xoutofmem(size_t size)
{
  fprintf(stderr,"Out of memory (requested %ud bytes). Exit\n",size);
  fprintf(stderr,
          "To locate the offending function call, put a breakpoint\n"
          "with a debugger in the xoutofmem function.\n");
  exit(1);
}

void * xmalloc(size_t size)
{
  void *ptr;
  ptr = malloc(size);
  if (ptr == NULL) xoutofmem(size);
  return ptr;
}

void * xcalloc(size_t nmemb, size_t size)
{
  void *ptr;
  ptr = calloc(nmemb,size);
  if (ptr == NULL) xoutofmem(nmemb*size);
  return ptr;
}

void * xrealloc(void *ptr, size_t size)
{
  void *new_ptr;
  new_ptr = realloc(ptr,size);
  if (new_ptr == NULL && size != 0) xoutofmem(size);
  return new_ptr;
}

void xfree(void *ptr)
{
  free(ptr);
}

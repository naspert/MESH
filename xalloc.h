/* $Id: xalloc.h,v 1.1 2001/08/17 08:58:52 dsanta Exp $ */

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

BEGIN_DECL
#undef BEGIN_DECL

/* Same as malloc, but exits if out of memory. */
void * xa_malloc(size_t size);

/* Same as calloc, but exits if out of memory. */
void * xa_calloc(size_t nmemb, size_t size);

/* Same as realloc, but exits if out of memory. */
void * xa_realloc(void *ptr, size_t size);

END_DECL
#undef END_DECL

#endif /* _XALLOC_PROTO */

/* $Id: debug_print.h,v 1.1 2002/11/13 12:18:22 aspert Exp $ */

#ifndef DEBUG_PRINT_PROTO
#define DEBUG_PRINT_PROTO

#include <stdio.h>

#if defined(__GNUC__) && (__GNUC__>2 || __GNUC__==2 && __GNUC_MINOR__>=95)

# define DEBUG_PRINT(format, args...)                           \
do {                                                            \
  printf("[%s:%d:%s]: ", __FILE__, __LINE__, __FUNCTION__);     \
  printf(format , ## args );                                    \
} while(0)

#else 
/* we just alias DEBUG_PRINT to 'printf', still works, be almost
 * useless for debugging, since we don't know at all what line prints
 * information... */
# warning DEBUG mode without gcc gives *terse* reporting...
# define DEBUG_PRINT printf
#endif


#endif

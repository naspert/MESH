/* $Id: debug_print.h,v 1.3 2002/11/13 13:45:03 aspert Exp $ */

#ifndef DEBUG_PRINT_PROTO
#define DEBUG_PRINT_PROTO

#include <stdio.h>

#if defined(__GNUC__) && (__GNUC__>2 || __GNUC__==2 && __GNUC_MINOR__>=95) 

# define DEBUG_PRINT(format, args...)                           \
do {                                                            \
  printf("[%s:%d:%s]: ", __FILE__, __LINE__, __FUNCTION__);     \
  printf(format , ## args );                                    \
} while(0)

#elif defined(_MSC_VER) || defined(__STDC__)
/* Microsoft Visual C++ does not know about __FUNCTION__, nor about
 * varargs macros... */
/* Of course, if this is in a loop, do not forget the brackets, 'cause
 * this is kind of a hack.... */
# define DEBUG_PRINT printf("[%s:%d]: ", __FILE__, __LINE__);printf
#else 
/* we just alias DEBUG_PRINT to 'printf', still works, be almost
 * useless for debugging, since we don't know at all what line prints
 * information... */
# define DEBUG_PRINT printf
#endif


#endif

/* $Id: mesh.h,v 1.1 2002/02/26 10:27:48 dsanta Exp $ */

#ifndef MESH_H
#define MESH_H

#ifdef __cplusplus
#define BEGIN_CDECL extern "C" {
#define END_CDECL }
#else
#define BEGIN_CDECL
#define END_CDECL
#endif

BEGIN_CDECL /* Start things exported to C and C++ */
#undef BEGIN_CDECL

/* The mesh version string */
extern const char *version;

/* The copyright string */
extern const char *copyright;

END_CDECL /* End things exported to C and C++ */
#undef END_CDECL

#ifdef __cplusplus /* Start things that are C++ only */

#endif /* End things that are C++ only */

#endif /* MESH_H */

/* $Id: model_in.h,v 1.1 2002/02/04 15:56:20 dsanta Exp $ */

/*
 * Functions to read 3D model data from files
 *
 * Author: Diego Santa Cruz
 *
 * Currently supported file formats:
 *
 * - Raw ascii:
 *      Reads vertices, faces, vertex normals and face normals.
 *
 * - VRML 2 (aka VRML97):
 *      Only the IndexedFaceSet nodes are read. All transformations are
 *      ignored. DEF/USE and PROTO tags are not parsed. Vertices, faces,
 *      vertex normals and face normals are read. Due to lack of support in
 *      'struct model' for indexed vertex normals, they convertex to
 *      non-indexed ones by taking the last appearing normal for each vertex
 *      (i.e. if multiple normals exist for a vertex only the last one is
 *      considered). Likewise for indexed face normals.
 *
 * - Inventor 2:
 *      Not yet implemented but detected
 *
 * - Ply ascii:
 *      Not yet implemented but detected
 *
 */

#ifndef _MODEL_IN_PROTO
#define _MODEL_IN_PROTO

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*/

#include <3dmodel.h>

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* --------------------------------------------------------------------------
   FILE FORMATS
   -------------------------------------------------------------------------- */

#define MESH_FF_AUTO      0 /* Autodetect file format */
#define MESH_FF_RAW       1 /* Raw ascii */
#define MESH_FF_VRML      2 /* VRML 2 utf8 (a.k.a. VRML97) */
#define MESH_FF_IV        3 /* Inventor 2 ascii */
#define MESH_FF_PLY       4 /* Ply ascii */

/* --------------------------------------------------------------------------
   ERROR CODES (always negative)
   -------------------------------------------------------------------------- */

#define MESH_NO_MEM    -1   /* not enough memory */
#define MESH_CORRUPTED -2   /* corrupted file or I/O error */
#define MESH_MODEL_ERR -3   /* error in model */
#define MESH_NOT_TRIAG -4   /* not a triangular mesh */
#define MESH_BAD_FF    -5   /* not a recongnized file format */
#define MESH_BAD_FNAME -6   /* Could not open file name */

/* --------------------------------------------------------------------------
   EXTERNAL FUNCTIONS
   -------------------------------------------------------------------------- */

/* Reads the 3D triangular mesh models from the input '*data' stream, in the
 * file format specified by 'fformat'. The model meshes are returned in the
 * new '*models_ref' array (allocate via malloc). If succesful it returns the
 * number of meshes read or the negative error code (MESH_CORRUPTED,
 * MESH_NOT_TRIAG, etc.). If an error occurs '*models_ref' is not modified. If
 * 'fformat' is MESH_FF_AUTO the file format is autodetected. If 'concat' is
 * non-zero only one mesh is returned, which is the concatenation of the the
 * ones read. */
int read_model(struct model **models_ref, FILE *data, int fformat, int concat);

/* Reads the 3D triangular mesh models from the input file '*fname' file, in
 * the file format specified by 'fformat'. The model meshes are returned in
 * the new '*models_ref' array (allocated via malloc). It returns the number
 * of meshes read, if succesful, or the negative error code (MESH_CORRUPTED,
 * MESH_NOT_TRIAG, etc.) otherwise. If an error occurs opening the file
 * MESH_BAD_FNAME is returned (the detailed error is given in errno). If an
 * error occurs '*models_ref' is not modified. If 'fformat' is MESH_FF_AUTO
 * the file format is autodetected. If 'concat' is non-zero only one mesh is
 * returned, which is the concatenation of the the ones read. */
int read_fmodel(struct model **models_ref, const char *fname,
                int fformat, int concat);


END_DECL
#undef END_DECL

#endif /* _MODEL_IN_PROTO */

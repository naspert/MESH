/* $Id: model_in.h,v 1.6 2002/04/11 08:51:04 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
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
 *   Submitted to ICME 2002, available on http://mesh.epfl.ch
 *
 */

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

/* 
 * 'zlib' is not part of the Window$ platforms. Comment out this to use zlib
 * under Windows.
 */
#ifdef WIN32
#define DONT_USE_ZLIB
#endif

#ifndef DONT_USE_ZLIB
# include <zlib.h>
#endif


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
#define MESH_FF_SMF       5 /* SMF format from QSlim */

/* --------------------------------------------------------------------------
   ERROR CODES (always negative)
   -------------------------------------------------------------------------- */

#define MESH_NO_MEM    -1   /* not enough memory */
#define MESH_CORRUPTED -2   /* corrupted file or I/O error */
#define MESH_MODEL_ERR -3   /* error in model */
#define MESH_NOT_TRIAG -4   /* not a triangular mesh */
#define MESH_BAD_FF    -5   /* not a recognized file format */
#define MESH_BAD_FNAME -6   /* Could not open file name */

/* -------------------------------------------------------------------------
   EXTERNAL FUNCTIONS
   ------------------------------------------------------------------------- */



struct file_data {
#ifdef DONT_USE_ZLIB
  FILE *f;
#else
  gzFile f;
#endif
  unsigned char *block; /* data block = 1KB */
  int nbytes; /* actual number of bytes in block */
  int pos; /* current position in block */
  int eof_reached;
};


/* 
 * common part - make the [un]getc function point to the custom
 * versions that read from a buffer 
 */
# undef getc
# define getc buf_getc
# undef ungetc
# define ungetc buf_ungetc


/*
 * Adapt all calls to the situation. If we use zlib, use the gz* functions. 
 * If not, use the f* ones. 
 */
#ifdef DONT_USE_ZLIB
# undef loc_fopen
# define loc_fopen fopen
# undef loc_fclose
# define loc_fclose fclose
# undef loc_fread
# define loc_fread fread
# undef loc_feof
# define loc_feof feof
# undef loc_getc
# define loc_getc fgetc
#else
# undef loc_fopen
# define loc_fopen gzopen
# undef loc_fclose
# define loc_fclose gzclose
# undef loc_fread /* why the $%^ are they using different args ??? */
# define loc_fread(ptr, size, nemb, stream) gzread(stream, ptr, (nemb)*(size))
# undef loc_feof
# define loc_feof gzeof
# undef loc_getc
# define loc_getc gzgetc
#endif

/* Reads the 3D triangular mesh models from the input '*data' stream, in the
 * file format specified by 'fformat'. The model meshes are returned in the
 * new '*models_ref' array (allocate via malloc). If succesful it returns the
 * number of meshes read or the negative error code (MESH_CORRUPTED,
 * MESH_NOT_TRIAG, etc.). If an error occurs '*models_ref' is not modified. If
 * 'fformat' is MESH_FF_AUTO the file format is autodetected. If 'concat' is
 * non-zero only one mesh is returned, which is the concatenation of the the
 * ones read. */
int read_model(struct model **models_ref, struct file_data *data, 
               int fformat, int concat);

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

/* $Id: model_in.h,v 1.17 2002/08/27 07:46:03 aspert Exp $ */


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
 *   Accepted for publication, ICME 2002, available on http://mesh.epfl.ch
 *
 */



/*
 * Functions to read 3D model data from files
 *
 * Author: Diego Santa Cruz
 *
 * N. Aspert is guilty for all the cruft to read gzipped files + Inventor + 
 * SMF and PLY parsers.
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
 *      'struct model' for indexed vertex normals, they are converted to
 *      non-indexed ones by taking the last appearing normal for each vertex
 *      (i.e. if multiple normals exist for a vertex only the last one is
 *      considered). Likewise for indexed face normals.
 *
 * - Inventor 2:
 *     Only the Coordinate3-point and IndexedFaceSet-coordIndex fields
 *     are read. Normals and everything else is (hopefully) silently
 *     ignored. 
 *
 * - SMF : 
 *     Only the vertices and triangular faces are read (which should be
 *     sufficient to read the output of QSlim for instance). Every
 *     other field (normals, transforms, colors, begin/end, etc.) is
 *     ignored. 
 *
 * - Ply ascii:
 *     Only the vertices and triangular faces are read. The 'property'
 *     fields are *not* read (only those describing vertex coordinates
 *     and face indices are considered, others are skipped). Binary
 *     PLY should be OK. However, changing the 'binary_big_endian'
 *     into a 'binary_big_endian' (or the contrary) in the header can
 *     improve things sometimes, especially if you converted your
 *     ASCII file into a binary one using 'ply2binary'...
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
 * under Windows (It *does* work ! I saw it on my PC !).
 */
#ifdef WIN32
# define DONT_USE_ZLIB
#endif

#ifndef DONT_USE_ZLIB
# include <zlib.h>
#endif


#ifdef __cplusplus
# define BEGIN_DECL extern "C" {
# define END_DECL }
#else
# define BEGIN_DECL
# define END_DECL
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* --------------------------------------------------------------------------
   BUFFERED FILE DATA STRUCTURE
   -------------------------------------------------------------------------- */
struct file_data {
#ifdef DONT_USE_ZLIB
  FILE *f;
#else
  gzFile f;
#endif
  unsigned char *block; /* data block = 16KB */
  int nbytes; /* actual number of bytes in block */
  int pos; /* current position in block */
  int eof_reached;
};

/* --------------------------------------------------------------------------
   FILE FORMATS
   -------------------------------------------------------------------------- */

#define MESH_FF_AUTO      0 /* Autodetect file format */
#define MESH_FF_RAW       1 /* Raw ascii */
#define MESH_FF_VRML      2 /* VRML 2 utf8 (a.k.a. VRML97) */
#define MESH_FF_IV        3 /* Inventor 2 ascii */
#define MESH_FF_PLY       4 /* Ply ascii */
#define MESH_FF_SMF       5 /* SMF format (from QSlim) */

/* --------------------------------------------------------------------------
   ERROR CODES (always negative)
   -------------------------------------------------------------------------- */

#define MESH_NO_MEM    -1   /* not enough memory */
#define MESH_CORRUPTED -2   /* corrupted file or I/O error */
#define MESH_MODEL_ERR -3   /* error in model */
#define MESH_NOT_TRIAG -4   /* not a triangular mesh */
#define MESH_BAD_FF    -5   /* not a recognized file format */
#define MESH_BAD_FNAME -6   /* Could not open file name */

/* --------------------------------------------------------------------------
   PARAMETERS FOR FF READERS
   -------------------------------------------------------------------------- */

/* Maximum allowed word length */
#define MAX_WORD_LEN 60
/* Default initial number of elements for an array */
#define SZ_INIT_DEF 240
/* Maximum number of elements by which an array is grown */
#define SZ_MAX_INCR 2048

/* Characters that are considered whitespace in VRML */
#define VRML_WS_CHARS " \t,\n\r"
/* Characters that start a comment in VRML */
#define VRML_COMM_ST_CHARS "#"
/* Characters that start a quoted string in VRML */
#define VRML_STR_ST_CHARS "\""
/* Characters that are whitespace, or that start a comment in VRML */
#define VRML_WSCOMM_CHARS VRML_WS_CHARS VRML_COMM_ST_CHARS
/* Characters that are whitespace, or that start a comment or string in VRML */
#define VRML_WSCOMMSTR_CHARS VRML_WS_CHARS VRML_COMM_ST_CHARS VRML_STR_ST_CHARS

/* -------------------------------------------------------------------------
   EXTERNAL FUNCTIONS
   ------------------------------------------------------------------------- */


/* 
 * common part - make the [un]getc function point to the custom
 * versions that read from a buffer 
 */
# undef getc
# define getc buf_getc
# undef ungetc
# define ungetc buf_ungetc

/* These two macros aliased to 'getc' and 'ungetc' in
 * 'model_in.h'. They behave (or at least should behave) identically
 * to glibc's 'getc' and 'ungetc', except that they read from a buffer
 * in memory instead of reading from a file. 'buf_getc' is able to
 * refill the buffer if there is no data left to be read in the block
 */
#ifndef buf_getc
#define buf_getc(stream)                                        \
(((stream)->nbytes > 0 && (stream)->pos < (stream)->nbytes)?    \
 ((int)(stream)->block[((stream)->pos)++]):buf_getc_func(stream))
#endif

#ifndef buf_ungetc
#define buf_ungetc(c, stream)                   \
(((c) != EOF && (stream)->pos > 0) ?            \
 ((stream)->block[--((stream)->pos)]):EOF)
#endif

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


/* Low-level functions used by model readers - see model_in.c for
 * details. Although they are exported, they should only be used in
 * the model_in*.c files  */
int buf_getc_func(struct file_data*);
int int_scanf(struct file_data*, int*);
int float_scanf(struct file_data*, float*);
int string_scanf(struct file_data*, char*);
int buf_fscanf_1arg(struct file_data*, const char*, void*);
void* grow_array(void*, size_t, int*, int);
int skip_ws_comm(struct file_data*);
int skip_ws_comm_str(struct file_data*);
int find_chars(struct file_data*, const char*);
int find_string(struct file_data*, const char*);

/* File format reader functions - should be accessed only through
 * read_[f]model. See the model_in*.c files for more details about
 * their behaviour (esp. wrt. error handling and/or [un]implemented
 * features of each file format) */
int read_raw_tmesh(struct model**, struct file_data*);
int read_smf_tmesh(struct model**, struct file_data*);
int read_ply_tmesh(struct model**, struct file_data*);
int read_vrml_tmesh(struct model**, struct file_data*, int);
int read_iv_tmesh(struct model**, struct file_data*);


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

/* $Id: model_in_ply.c,v 1.11 2002/08/26 14:27:27 aspert Exp $ */

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

/* PLY file format reader written by N. Aspert 
 * 
 * It should read ascii and binary PLY files, although some weirds
 * things can happen with binary files when the endianness is
 * incorrectly specified (yes, it already happened). Before reporting
 * a bug, check if a 'corrupted' file can be 'recovered' by
 * hand-editing the file and by changing the 'binary_big_endian' into
 * a 'binary_little_endian' field (on the contrary) does not improve
 * things, especially if you have used 'ply2binary' to create your
 * binary file (see the README for more details).
 *
 * Please note that only _triangular_ meshes are read, and that only
 * the vertices' coordinates and the indices of faces are read. All
 * other properties are skipped (more or less) silently.
 *
 */

#include <model_in.h>
#include <types.h>
#include <model_in_ply.h>



/* Just a small wrapper : skip all whitespace chars and read the
 * string that follows. Returns 1 upon success */
static int skip_ws_str_scanf(struct file_data *data, char *out) 
{
  skip_ws_comm(data);
  return string_scanf(data, out);
}

/* Skip 'nbytes' bytes in stream 'data'. Returns 0 if successfull or a
 * negative value if a failure occurs. */
static int skip_bytes(struct file_data *data, size_t nbytes) 
{
  size_t i;
  int c;
  int rcode=0;

  for (i=0; i<nbytes; i++) {
    c = getc(data);
    if (c == EOF) {
      rcode = MESH_CORRUPTED;
      break;
    }
  }
#ifdef DEBUG
  printf("[skip_bytes] Skipped %d bytes\n", nbytes);
#endif
  return rcode;
}


/* The read_[type] function reads a [type] from 'data' and puts the
 * result into 'out'. Returns 0 upon success and a negative value in
 * case of failure. */
static int read_uint16(struct file_data *data, int swap_bytes, t_uint16 *out ) 
{
  union sw_uint16 tmp;
  int c, i;
  int rcode = 0;

  if (swap_bytes == 1) {
    for (i=1; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=1; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_uint16] read %d\n", tmp.bo);
#endif
  return rcode;
}


static int read_int16(struct file_data *data, int swap_bytes, t_int16 *out ) 
{
  union sw_int16 tmp;
  int c, i;
  int rcode = 0;

  if (swap_bytes == 1) {
    for (i=1; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=1; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_int16] read %d\n", tmp.bo);
#endif
  return rcode;
}

static int read_uint32(struct file_data *data, int swap_bytes, t_uint32 *out ) 
{
  union sw_uint32 tmp;
  int i, c;
  int rcode = 0;

  if (swap_bytes == 1) {
    for (i=3; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=3; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_uint32] read %d\n", tmp.bo);
#endif
  return rcode;
}

static int read_int32(struct file_data *data, int swap_bytes, t_int32 *out ) 
{
  union sw_int32 tmp;
  int i, c;
  int rcode = 0;

  if (swap_bytes == 1) {
    for (i=3; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=3; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_int32] read %d\n", tmp.bo);
#endif
  return rcode;
}

static int read_float32(struct file_data *data, int swap_bytes, float *out ) 
{
  union sw_float32 tmp;
  int i, c;
  int rcode = 0;


  if (swap_bytes == 1) {
    for (i=3; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
#ifdef LL_DEBUG
      printf("[read_float32] c[%d] = %d\n", i, c);
#endif
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=3; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
#ifdef LL_DEBUG
      printf("[read_float32] c[%d] = %d\n", i, c);
#endif
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_float32] read %f\n", tmp.bo);
#endif

  return rcode;
}

static int read_float64(struct file_data *data, int swap_bytes, double *out ) 
{
  union sw_float32 tmp;
  int i, c;
  int rcode = 0;

  if (swap_bytes == 1) {
    for (i=7; i>=0; i--) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  } else {
    for (i=0; i<=7; i++) {
      c = getc(data);
      if (c == EOF) {
        rcode = EOF;
        break;
      }
      tmp.bs[i] = (t_uint8)c;
    }
  }

  if (rcode >= 0)
    *out = tmp.bo ;
#ifdef DEBUG
  printf("[read_float64] read %f\n", tmp.bo);
#endif
  return rcode;
}


/* Reads a property of type 'prop_type' from 'data', and puts the
 * result 'out'. Returns 0 upon success, a negative value otherwise */
static int read_ply_property(struct file_data *data, int prop_type, 
                             int swap_bytes, void *out) 
{
  int c, rcode=0;
  /* Just local pointers to copy the result from the 'read_[type]'
   * functions into... */
  t_int8 *tmp_int8;
  t_uint8 *tmp_uint8;
  t_int16 *tmp_int16;
  t_uint16 *tmp_uint16;
  t_int32 *tmp_int32;
  t_uint32 *tmp_uint32;
  float *tmp_float32;
  double *tmp_float64;

  switch(prop_type) {
  case uint8:
    c = getc(data);
    if (c == EOF) 
      rcode = EOF;
    else {
      tmp_uint8 = out;
      *tmp_uint8 = (t_uint8)c;
    }
    break;
  case int8:
    c = getc(data);
    if (c == EOF) 
      rcode = EOF;
    else {
      tmp_int8= out;
      *tmp_int8 = (t_int8)c;
    }
    break;
  case uint16:
    tmp_uint16 = out;
    rcode = read_uint16(data, swap_bytes, tmp_uint16);
    break;
  case int16:
    tmp_int16 = out;
    rcode = read_int16(data, swap_bytes, tmp_int16);
    break;
  case uint32:
    tmp_uint32 = out;
    rcode = read_uint32(data, swap_bytes, tmp_uint32);
    break;
  case int32:
    tmp_int32 = out;
    rcode = read_int32(data, swap_bytes, tmp_int32);
    break;
  case float32:
    tmp_float32 = out;
    rcode = read_float32(data, swap_bytes, tmp_float32);
    break;
  case float64:
    tmp_float64 = out;
    rcode = read_float64(data, swap_bytes, tmp_float64);
    break;
  default:
    rcode = MESH_CORRUPTED;
    break;
  }

  return rcode;
}

/* Returns the associated integer to a PLY type conatined in a string
 * of characters 'str' (see 'model_in_ply.h' for all types). It also
 * supports the 'old-fashioned' types, defined in a previous version
 * of the PLY file format. */
static int get_type(char* str) 
{
  int t;
  
  if (strcmp(str, "int8") == 0 || strcmp(str, "char") == 0)
    t = int8;
  else if (strcmp(str, "uint8") == 0 || strcmp(str, "uchar") == 0)
    t = uint8;
  else if (strcmp(str, "int16") == 0 || strcmp(str, "short") == 0)
    t = int16;
  else if (strcmp(str, "uint16") == 0 || strcmp(str, "ushort") == 0)
    t = uint16;
  else if (strcmp(str, "int32") == 0 || strcmp(str, "int") == 0)
    t = int32;
  else if (strcmp(str, "uint32") == 0 || strcmp(str, "uint") == 0)
    t = uint32;
  else if (strcmp(str, "float32") == 0 || strcmp(str, "float") == 0)
    t = float32;
  else if (strcmp(str, "float64") == 0 || strcmp(str, "double") == 0)
    t = float64;
  else
    t = not_valid;

  return t;
}

/* Try to match a property name to the ones that are currently handled
 * (i.e. vertex coord and face index). Returns the integer associated
 * to the property (see 'model_in_ply.h' for more details) */
static int get_prop_name(char *str) 
{
  int name;
  
  if (strcmp(str, "x") == 0)
    name = v_x;
  else if (strcmp(str, "y") == 0)
    name = v_y;
  else if (strcmp(str, "z") == 0)
    name = v_z;
  else if (strcmp(str, "vertex_indices") == 0)
    name = v_idx;
  else
    name = unsupported;

  return name;
}

/* Fills the 'prop' array with all the 'property' fields read from
 * 'data' before encoutering another 'element' field (or a
 * 'end_header'). Returns 0 upons success and a negative value if a
 * failure occurs. */
static int read_properties(struct file_data *data, struct ply_prop **prop) 
{
  int c;
  char stmp[MAX_WORD_LEN+1];
  int n_prop=0;
  int rcode=0;
  do {
    skip_ws_comm(data);
    c = getc(data);
    if (c == 'p') {
      c = ungetc(c, data);
      if (string_scanf(data, stmp) == 1 && strcmp(stmp, "property") == 0) {
        *prop = realloc(*prop, (n_prop+1)*sizeof(struct ply_prop));
        memset(*prop+n_prop, 0, sizeof(struct ply_prop));
        if (skip_ws_str_scanf(data, stmp) == 1) {
          if (strcmp(stmp, "list") == 0) { /* do we have a list ? */
            (*prop)[n_prop].is_list = 1;
            if (skip_ws_str_scanf(data, stmp) == 1) {/* if yes, we need to
                                                        read an additional
                                                        field */
              (*prop)[n_prop].type_list = get_type(stmp);
              if (skip_ws_str_scanf(data, stmp) != 1)
                rcode = MESH_CORRUPTED;
            }
            else
              rcode = MESH_CORRUPTED;
          }
          (*prop)[n_prop].type_prop = get_type(stmp);

          
          /* Get the property name */
          if (skip_ws_str_scanf(data, stmp) == 1) {
            (*prop)[n_prop++].prop = get_prop_name(stmp);
            skip_ws_comm(data);
          } else
            rcode = MESH_CORRUPTED;
        } else
          rcode = MESH_CORRUPTED;
      } else
        rcode = MESH_CORRUPTED;
    }
  } while (rcode >= 0 && c == 'p');
  c = ungetc(c, data);
  return (rcode < 0) ? rcode : n_prop;
}

/* Read 'n_faces' faces of the model from a binary 'data'. 
 * Returns 0 if successful and a negative value in case of trouble. 
 * Non-triangular faces are NOT supported. Moreover, the face's vertex
 * indices have to be consistent with the number of vertices in the
 * model 'n_vtcs'. The 'n_f_prop' properties associated with the faces
 * are passed in the 'face_prop' array.*/
static int read_bin_ply_faces(face_t *faces, struct file_data *data,
                              int n_faces, int n_vtcs,
                              int swap_bytes,
                              struct ply_prop *face_prop, int n_f_prop) 
{
  int i, j, f0, f1, f2;
  int rcode=0;
  t_uint8 tmp=0;

  for (i=0; i<n_faces && rcode >=0; i++) {
    for (j=0; j<n_f_prop && rcode >=0; j++) {
      if (face_prop[j].prop == v_idx) {
        if (!face_prop[j].is_list)
          rcode = MESH_CORRUPTED;
        else {
          rcode = read_ply_property(data, face_prop[j].type_list, swap_bytes,
                                    &tmp);
          if (tmp != 3) { /* Non triangular mesh -> bail out */
#ifdef DEBUG
            printf("[read_bin_ply_faces] found a %d face\n", tmp);
#endif
            rcode = MESH_NOT_TRIAG;
            break;
          }
          rcode = read_ply_property(data, face_prop[j].type_prop, swap_bytes,
                                    &f0);
          rcode = read_ply_property(data, face_prop[j].type_prop, swap_bytes,
                                    &f1);
          rcode = read_ply_property(data, face_prop[j].type_prop, swap_bytes,
                                    &f2);
          if (f0 < 0 || f1 < 0 || f2 < 0 || 
              f0 >= n_vtcs || f1 >= n_vtcs || f2 >= n_vtcs ) {
            rcode = MESH_MODEL_ERR;
            break;
          }
          faces[i].f0 = f0;
          faces[i].f1 = f1;
          faces[i].f2 = f2;
          
        }
      } else {
        rcode = skip_bytes(data,  
                           ply_sizes[face_prop[j].type_prop]*
                           sizeof(unsigned char));
      }
    }
  }

  return rcode;
}


/* Read 'n_faces' faces of the model from 'data'. 
 * Returns 0 if successful and a negative value in case of trouble. 
 * Non-triangular faces are NOT supported. Moreover, the face's vertex
 * indices have to be consistent with the number of vertices in the
 * model 'n_vtcs'. */
static int read_ascii_ply_faces(face_t *faces, struct file_data *data,
                                int n_faces, int n_vtcs)
{
  int i, c, f0, f1, f2;
  int rcode=0;


  for(i=0; i<n_faces; i++) {
    
    if (int_scanf(data, &c) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }

    if (c != 3) { /* Non-triangular mesh -> bail out */
      rcode =  MESH_NOT_TRIAG;
      break;
    }
        
    /* Read faces */
    if (int_scanf(data, &f0) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }

    if (int_scanf(data, &f1) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }

    if (int_scanf(data, &f2) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }

    /* Check if indices are consistent ...*/
    if (f0 < 0 || f1 < 0 || f2 < 0 || 
        f0 >= n_vtcs || f1 >= n_vtcs || f2 >= n_vtcs ) {
      rcode = MESH_MODEL_ERR;
      break;
    }
    /* skip the rest of the current line, which can contain
     * additional informations, but currently ignored
     */
    do {
      c = getc(data);
    } while (c != '\n' && c != '\r' && c != EOF);
    
    if (c == EOF) {
      rcode = MESH_CORRUPTED;
      break;
    }
    faces[i].f0 = f0;
    faces[i].f1 = f1;
    faces[i].f2 = f2;
  } /* end face loop */

  return rcode;
}

/* Reads 'n_vtcs' vertex points from the 'data' stream in binary format
 * and stores them in the 'vtcs' array. Zero is returned on success, or the
 * negative error code otherwise. If no error occurs the bounding box minium
 * and maximum are returned in 'bbox_min' and 'bbox_max'. The
 * 'n_v_prop' properties associated to the vertices are fed through
 * the 'v_prop' array. */
static int read_bin_ply_vertices(vertex_t *vtcs, struct file_data *data,
                                 int n_vtcs, 
                                 vertex_t *bbox_min, vertex_t *bbox_max,
                                 int swap_bytes,
                                 struct ply_prop *v_prop, int n_v_prop) 
{
  int rcode = 0;
  int i, j;
  vertex_t bbmin,bbmax;

  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;

  for (i=0; i<n_vtcs && rcode>=0; i++) {
    for (j=0; j<n_v_prop && rcode>=0; j++) {
      switch (v_prop[j].prop) {
      case v_x:
        rcode = read_ply_property(data, v_prop[j].type_prop, swap_bytes, 
                                  &(vtcs[i].x));
        if (rcode == 0) {
          if (vtcs[i].x < bbmin.x) bbmin.x = vtcs[i].x;
          if (vtcs[i].x > bbmax.x) bbmax.x = vtcs[i].x;
        }
        break;
      case v_y:
        rcode = read_ply_property(data, v_prop[j].type_prop, swap_bytes, 
                                  &(vtcs[i].y));
        if (rcode == 0) {
          if (vtcs[i].y < bbmin.y) bbmin.y = vtcs[i].y;
          if (vtcs[i].y > bbmax.y) bbmax.y = vtcs[i].y;
        }
        break;
      case v_z:
        rcode = read_ply_property(data, v_prop[j].type_prop, swap_bytes, 
                                  &(vtcs[i].z));
        if (rcode == 0) {
          if (vtcs[i].z < bbmin.z) bbmin.z = vtcs[i].z;
          if (vtcs[i].z > bbmax.z) bbmax.z = vtcs[i].z;
        }
        break;
      default:
        rcode = skip_bytes(data, 
                           ply_sizes[v_prop[j].type_prop] * 
                           sizeof(unsigned char));
        break;
      }
    }
  }

  if (n_vtcs == 0) {
    memset(&bbmin,0,sizeof(bbmin));
    memset(&bbmax,0,sizeof(bbmax));
  }

  *bbox_min = bbmin;
  *bbox_max = bbmax;
  return rcode;
}

/* Reads 'n_vtcs' vertex points from the '*data' stream in raw ascii format
 * and stores them in the 'vtcs' array. Zero is returned on success, or the
 * negative error code otherwise. If no error occurs the bounding box minium
 * and maximum are returned in 'bbox_min' and 'bbox_max'. */
static int read_ascii_ply_vertices(vertex_t *vtcs, struct file_data *data, 
                                   int n_vtcs, 
                                   vertex_t *bbox_min, vertex_t *bbox_max)
{
  vertex_t bbmin, bbmax;
  int i, c, rcode=0;

  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;

  for (i=0; i< n_vtcs; i++) {
    if (float_scanf(data, &(vtcs[i].x)) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }
    if (float_scanf(data, &(vtcs[i].y)) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }
    if (float_scanf(data, &(vtcs[i].z)) != 1) {
      rcode = MESH_CORRUPTED;
      break;
    }
    /* skip the rest of the current line, which can contain
     * additional informations (e.g. color), but currently ignored
     */
    do {
      c = getc(data);
    } while (c != '\n' && c != '\r' && c != EOF);
    
    if (c == EOF) {
      rcode = MESH_CORRUPTED;
      break;
    }

    if (vtcs[i].x < bbmin.x) bbmin.x = vtcs[i].x;
    if (vtcs[i].y < bbmin.y) bbmin.y = vtcs[i].y;
    if (vtcs[i].z < bbmin.z) bbmin.z = vtcs[i].z;
    if (vtcs[i].x > bbmax.x) bbmax.x = vtcs[i].x;
    if (vtcs[i].y > bbmax.y) bbmax.y = vtcs[i].y;
    if (vtcs[i].z > bbmax.z) bbmax.z = vtcs[i].z;
  }
   if (n_vtcs == 0) {
    memset(&bbmin,0,sizeof(bbmin));
    memset(&bbmax,0,sizeof(bbmax));
  }
  *bbox_min = bbmin;
  *bbox_max = bbmax;
  return rcode;
}

/* Reads a _triangular_ mesh from a PLY ASCII file. Note that no
 * binary formats are supported, although PLY format allows
 * them. Maybe in a further version ...
 * Only the vertices and faces are read. All other possible properties
 * (e.g. color ...) are skipped silently.
 * However, this code should be sufficient to read most common PLY files.
 * It returns the number of meshes read (i.e. 1) if successful, and a
 * negative code if it failed. */
int read_ply_tmesh(struct model **tmesh_ref, struct file_data *data) 
{
  char stmp[MAX_WORD_LEN+1];
  vertex_t bbmin, bbmax;
  struct model* tmesh;
  int rcode=1;
  int is_bin=0;
  int file_endianness=0, platform_endianness=0, swap_bytes=0;
  struct ply_prop *vertex_prop=NULL, *face_prop=NULL;
  int n_vert_prop=0, n_face_prop=0;
  int c;
  union sw_uint32 test_byte_order = {{0x01, 0x02, 0x03, 0x04}};
  
  tmesh = (struct model*)calloc(1, sizeof(struct model));

  /* read the header */
  /* check if the format is ASCII or binary */
  if (skip_ws_str_scanf(data, stmp) == 1 && strcmp(stmp, "format") == 0) {
    if (skip_ws_str_scanf(data, stmp) == 1) { 
      /* check whether we have ASCII or binary stuff */
      if (strcmp(stmp, "binary_little_endian") == 0) {
        is_bin = 1;
      } else if (strcmp(stmp, "binary_big_endian") == 0) {
        is_bin = 1;
        file_endianness = 1;
      } else if (strcmp(stmp, "ascii") != 0) {
        rcode = MESH_CORRUPTED;
      } else {
        if (skip_ws_str_scanf(data, stmp) != 1 || strcmp(stmp, "1.0") != 0) 
          rcode = MESH_CORRUPTED;
      }
    } else {
      rcode = MESH_CORRUPTED;
    }
  } else
    rcode = MESH_CORRUPTED;

  if (rcode >= 0) {
    do {
      if ((c = find_string(data, "element")) != EOF) {
        if (skip_ws_str_scanf(data, stmp) == 1) {
          if (strcmp(stmp, "vertex") == 0) {
            if ((c = int_scanf(data, &(tmesh->num_vert))) != 1)
              rcode = MESH_CORRUPTED;
            else {
              n_vert_prop = read_properties(data, &vertex_prop);
              if (n_vert_prop <= 0)
                rcode = MESH_CORRUPTED;

#ifdef DEBUG
              fprintf(stderr, "[read_ply_tmesh] num_vert = %d\n", 
                      tmesh->num_vert);
#endif
            }
          } else if (strcmp(stmp, "face") == 0) {
            if ((c = int_scanf(data, &(tmesh->num_faces))) != 1) 
              rcode = MESH_CORRUPTED;
            else {
              n_face_prop = read_properties(data, &face_prop);
              if (n_face_prop <= 0)
                rcode = MESH_CORRUPTED;

#ifdef DEBUG
              fprintf(stderr, "[read_ply_tmesh] num_faces = %d\n", 
                      tmesh->num_faces);
#endif
            }
          } else if (strcmp(stmp, "edge") == 0) {
            fprintf(stderr, "[Warning] 'edge' field not supported !\n");
          } else if (strcmp(stmp, "material") == 0)
            fprintf(stderr, "[Warning] 'material' field not supported !\n");
        } else {
          fprintf(stderr, "[Warning] Unrecognized 'element' field found."
                  " Skipping...\n");
        }
      } else
        rcode = MESH_CORRUPTED;
    } while (rcode >= 0 && (tmesh->num_faces == 0 || tmesh->num_vert == 0));
    
  /* Ignore everything else from the header */
    if ((c = find_string(data, "end_header")) == EOF)
      rcode = MESH_CORRUPTED;
    /* end_header read */
    else {

      tmesh->vertices = (vertex_t*)malloc(tmesh->num_vert*sizeof(vertex_t));
      tmesh->faces = (face_t*)malloc(tmesh->num_faces*sizeof(face_t));
      
      if (tmesh->vertices == NULL || tmesh->faces == NULL)
        rcode = MESH_NO_MEM;
      else {
        if (!is_bin) { /* ascii format */
          /* Read vertices */
          rcode = read_ascii_ply_vertices(tmesh->vertices, data, 
                                          tmesh->num_vert, 
                                          &bbmin, &bbmax);
          
          /* Read face list */
          rcode = read_ascii_ply_faces(tmesh->faces, data, 
                                       tmesh->num_faces, tmesh->num_vert);
        } else { /* binary format */
          /* check endianness of the platform, just in case ;-) */
          rcode = MESH_BAD_FF;
          if (test_byte_order.bo == TEST_BIG_ENDIAN)
            platform_endianness = 1;
          else if (test_byte_order.bo != TEST_LITTLE_ENDIAN) {
            fprintf(stderr, "Unable to probe for byte ordering\n");
            platform_endianness = -1;
          }
          swap_bytes = (file_endianness == platform_endianness)?0:1;
          skip_ws_comm(data);
          rcode = read_bin_ply_vertices(tmesh->vertices, data, tmesh->num_vert,
                                        &bbmin, &bbmax, 
                                        swap_bytes,
                                        vertex_prop, n_vert_prop);

          rcode = read_bin_ply_faces(tmesh->faces, data, tmesh->num_faces,
                                     tmesh->num_vert,
                                     swap_bytes,
                                     face_prop, n_face_prop);
        }
      }
    }
  }
  if (rcode < 0) { /* something went wrong */
    if (tmesh->vertices != NULL)
      free(tmesh->vertices);
    if (tmesh->faces != NULL)
      free(tmesh->faces);
    free(tmesh);
  } else {
    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
    *tmesh_ref = tmesh;
    rcode = 1;
  }

  return rcode;
}

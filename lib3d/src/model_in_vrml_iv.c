/* $Id: model_in_vrml_iv.c,v 1.8 2003/03/13 14:47:35 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2003 EPFL (Swiss Federal Institute of Technology,
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
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */




#include <model_in.h>
#include <block_list.h>
#ifdef DEBUG
# include <debug_print.h>
#endif

/* Advances the '*data' stream past the end of the VRML field (single, array
 * or node). Returns zero on success and an error code (MESH_CORRUPTED, etc.) 
 * on error. */
static int skip_vrml_field(struct file_data *data)
{
  int c,n_brace;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;

  if (c == '[') { /* array enclosed in [], skip until next ] */
    getc(data); /* skip [ */
    find_chars(data,"]");
    c = getc(data); /* skip ] */
  } if (c == '{') { /* a node, skip (including embedded nodes) */
    getc(data); /* skip { */
    n_brace = 1;
    do {
      c = find_chars(data,"{}");
      if (c == '{') { /* embedded node start */
        getc(data); /* skip { */
        n_brace++;
      } else if (c == '}') { /* node ending */
        getc(data); /* skip } */
        n_brace--;
      }
    } while (n_brace > 0 && c != EOF);
  } else if (c != EOF) { /* single value */
    c = find_chars(data,VRML_WS_CHARS);
  }
  return (c != EOF) ? 0 : MESH_CORRUPTED;
}

/* Frees and sets to NULL pointer fields of model 'm'. If 'm' is NULL nothing
 * is done. */
static void free_model_pfields(struct model *m) {
  if (m != NULL) {
    free(m->vertices); m->vertices = NULL;
    free(m->faces); m->faces = NULL;
    free(m->normals); m->normals = NULL;
    free(m->face_normals); m->face_normals = NULL;
  }
}

/* Converts indexed vertex normals to non-indexed vertex normals, for
 * triangular mesh models. The new array (allocated via malloc) is returned
 * in '*vnrmls_ref'. The normals are given in 'nrmls', the normals indices for
 * each vertex index of each face (including the -1 terminating each face) are
 * given in 'nrml_idcs' and the faces of the model in 'faces'. The number of
 * normals is given by 'n_nrmls', of normal indices by 'n_nrml_idcs' and of
 * faces by 'n_faces'. The maximum normal i-lXmundex is given by 'max_nidx' and the
 * maximum vertex index (of 'faces') in 'max_vidx'. If no error occurs the
 * number of normals in '*vnrmls_ref' is returned. Otherwise the negative
 * error code is returned and '*vnrmls_ref' is not modified. */
static int tidxnormals_to_vnormals(vertex_t **vnrmls_ref, vertex_t *nrmls,
                                   int n_nrmls, int *nrml_idcs, 
                                   int n_nrml_idcs,
                                   int max_nidx, int max_vidx, face_t *faces,
                                   int n_faces)
{
  vertex_t *vnrmls;
  int n_vnrmls;
  int i,j;

  if (n_nrmls <= max_nidx || (n_nrml_idcs+1)/4 < n_faces ||
      max_vidx < -1) {
    return MESH_MODEL_ERR;
  }
  n_vnrmls = max_vidx+1;
  if (n_vnrmls == 0) {
    *vnrmls_ref = NULL;
    return 0;
  }
  vnrmls = calloc(n_vnrmls,sizeof(*vnrmls));
  if (vnrmls == NULL) return MESH_NO_MEM;
  for (i=0, j=0; i<n_faces; i++) {
    if (nrml_idcs[j] < 0 || nrml_idcs[j+1] < 0 || nrml_idcs[j+2] < 0 ||
        (j+3 < n_nrml_idcs && nrml_idcs[j+3] != -1)) {
      n_vnrmls = MESH_MODEL_ERR;
      break;
    }
    vnrmls[faces[i].f0] = nrmls[nrml_idcs[j++]];
    vnrmls[faces[i].f1] = nrmls[nrml_idcs[j++]];
    vnrmls[faces[i].f2] = nrmls[nrml_idcs[j++]];
    j++; /* skip face terminating -1 */
  }
  if (n_vnrmls >= 0) {
    *vnrmls_ref = vnrmls;
  } else {
    free(vnrmls);
  }
  return n_vnrmls;
}

/* Converts indexed face normals to non-indexed face normals, for triangular
 * mesh models. The new array (allocated via malloc) is returned in
 * '*fnrmls_ref'. The normals are given in 'nrmls' and the normals indices for
 * each face are given in 'nrml_idcs'. The number of normals is given by
 * 'n_nrmls', of normal indices by 'n_nrml_idcs' and of faces by
 * 'n_faces'. The maximum normal index is given by 'max_nidx'. If no error
 * occurs the number of normals in '*fnrmls_ref' is returned. Otherwise the
 * negative error code is returned and '*fnrmls_ref' is not modified. */
static int tidxnormals_to_fnormals(vertex_t **fnrmls_ref, vertex_t *nrmls,
                                   int n_nrmls, int *nrml_idcs, int n_nrml_idcs,
                                   int max_nidx, int n_faces)
{
  vertex_t *fnrmls;
  int n_fnrmls;
  int i;

  if (n_nrmls <= max_nidx || n_nrml_idcs < n_faces) return MESH_MODEL_ERR;
  n_fnrmls = n_faces;
  if (n_fnrmls == 0) {
    *fnrmls_ref = NULL;
    return 0;
  }
  fnrmls = calloc(n_fnrmls,sizeof(*fnrmls));
  if (fnrmls == NULL) return MESH_NO_MEM;
  for (i=0; i<n_faces; i++) {
    if (nrml_idcs[i] < 0) {
      n_fnrmls = MESH_CORRUPTED;
      break;
    }
    fnrmls[i] = nrmls[nrml_idcs[i]];
  }
  if (n_fnrmls >= 0) {
    *fnrmls_ref = fnrmls;
  } else {
    free(fnrmls);
  }
  return n_fnrmls;
}


/* Gets the type of the node appearing next in the '*data' stream. The node
 * name is returned in 's', up to 'slen' characters (including the terminating
 * null). Any DEF statement is skipped, along with the node name. Returns zero
 * on success or the negative error code on error. If an error occurs 's' is
 * not modified. */
static int read_node_type(char *s, struct file_data *data, int slen)
{
  char stmp[MAX_WORD_LEN+1];
  int rcode;

  rcode = 0;
  if (skip_ws_comm(data) == EOF) return MESH_CORRUPTED;
  if (string_scanf(data,stmp) != 1) {
    rcode = MESH_CORRUPTED;
  } else {
#ifdef DEBUG
    DEBUG_PRINT("stmp = %s\n", stmp);
#endif
    if (strcmp("DEF",stmp) == 0) {
      /* DEF tag => skip node name and get node type */
      if (skip_ws_comm(data) == EOF || skip_vrml_field(data) == EOF ||
          skip_ws_comm(data) == EOF || string_scanf(data,stmp) != 1) {
        rcode = MESH_CORRUPTED;
      }
#ifdef DEBUG
      DEBUG_PRINT("stmp = %s\n", stmp);
#endif
    }
  }
  if (rcode == 0) {
    strncpy(s,stmp,slen);
    s[slen-1] = '\0'; /* make sure string is always null terminated */
  }
  return rcode;
}

/* Reads a VRML boolean field from the '*data' stream. If the field is "TRUE"
 * one is returned in '*res', if it is "FALSE" zero is returned in '*res'. Any
 * other value is an error. If an error occurs '*res' is not modified and the
 * negative error value is returned. Otherwise zero is returned. */
static int read_sfbool(int *res, struct file_data *data)
{
  char stmp[MAX_WORD_LEN+1];
  int rcode;

  rcode = 0;
  if (skip_ws_comm(data) != EOF && string_scanf(data,stmp) == 1) {
    if (strcmp(stmp,"TRUE") == 0) {
      *res = 1;
    } else if (strcmp(stmp,"FALSE") == 0) {
      *res = 0;
    } else {
      rcode = MESH_CORRUPTED;
    }
  } else {

    rcode = MESH_CORRUPTED;
  }
  return rcode;
}

/* Reads an ASCII VRML or Inventor float array. The array must start by an
 * opening bracket '[', values be separated by VRML whitespace (e.g., commas)
 * and it must finish with a closing bracket ']'. If the array has only
 * 'nelem' values the brackets can be left out (typically used for MFVec3f
 * fields with 'nelem = 3', otherwise 'nelem' should be 1 for
 * MFFloat). 
 * A valid pointer 'blk' to a 'block_list', properly initialized with
 * elements having a size of sizeof(float) must be passed to the function.
 * The number of elements in the array is returned by the function. In
 * case of error the negative error code is returned (MESH_NO_MEM,
 * MESH_CORRUPTED, etc.) */
static int read_mffloat(struct block_list *blk, struct file_data *data, 
                        int nelem)
{
  int len, n;
  int c, in_brackets;
  struct block_list *cur=blk;
  float tmpf;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;
  if (c == '[') {
    in_brackets = 1;
    getc(data); /* skip [ */
  } else {
    in_brackets = 0;
  }

  /* Get numbers until we reach closing bracket */
  len = 0;
  n = 0;
  do {

    c = skip_ws_comm(data);
    if (c == ']') {
      if (!in_brackets) {
        n = MESH_CORRUPTED;
      } else {
        getc(data); /* skip ] */
        break;
      }
    } else if (c != EOF && float_scanf(data,&tmpf) == 1) {
      if (cur->elem_filled == cur->nelem) { /* Need more storage */
        cur = get_next_block(cur);
        if (cur == NULL) {
          n = MESH_NO_MEM;
          break;
        }
      }
      TAIL_BLOCK_LIST_INCR(cur, float) = tmpf;
      n++;
    } else { /* error */
      n = MESH_CORRUPTED;
    }
  } while ((in_brackets || n < nelem) && c != EOF && n >= 0);
#ifdef DEBUG
  DEBUG_PRINT("in_brackets=%d n=%d c=%d \n", in_brackets, n, c);
#endif
  
  return n;
}

/* Reads an ASCII VRML or Inventor integer array. The array must start by an
 * opening bracket '[', values be separated by VRML whitespace (e.g., commas)
 * and it must finish with a closing bracket ']'. If the array has only one
 * value the brackets can be left out. 
 * A valid pointer 'blk' to a 'block_list' (initialized, with elements
 * having a size of sizeof(int) must be passed to the function, in
 * order to store the data read from the file.
 * The maximum value in it returned in '*max_val', the number of
 * elements in the array is returned by the function. 
 * In case of error the negative error code is
 * returned (MESH_NO_MEM, MESH_CORRUPTED, etc.)  and  '*max_val'
 * is not modified. */
static int read_mfint32(struct block_list *blk, struct file_data *data, 
                        int *max_val)
{
  int len,n;
  int c,in_brackets;
  struct block_list *cur=blk;
  int tmpi;
  int maxv;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;
  if (c == '[') {
    in_brackets = 1;
    getc(data); /* skip [ */
  } else {
    in_brackets = 0;
  }

  /* Get numbers until we reach closing bracket */
  len = 0;
  n = 0;
  maxv = INT_MIN;
  do {
    c = skip_ws_comm(data);
    if (c == ']') {
      if (!in_brackets) {
        n = MESH_CORRUPTED;
      } else {
        getc(data); /* skip ] */
        break;
      }
    } else if (c != EOF && int_scanf(data,&tmpi) == 1) {
      if (cur->elem_filled == cur->nelem) { /* Reallocate storage */
        cur = get_next_block(cur);
        if (cur == NULL) {
          n = MESH_NO_MEM;
          break;
        }
      }
      if (tmpi > maxv) maxv = tmpi;
      TAIL_BLOCK_LIST_INCR(cur, int) = tmpi;
      n++;
    } else { /* error */
      n = MESH_CORRUPTED;
    }
  } while (in_brackets && c != EOF && n >= 0);

  if (n >= 0)  /* read OK */
    *max_val = maxv;
   
  return n;
}

/* Reads an ASCII VRML mfvec3f (array of floting-point point coords). The
 * array must start by an opening bracket '[', values be separated by VRML
 * whitespace (e.g., commas) and it must finish with a closing bracket ']'. If
 * the array has only one value the brackets can be left out. The array
 * (allocated via malloc) is returned in '*a_ref'. It is NULL for empty
 * arrays. The number of elements in the array is returned by the function. In
 * case of error the negative error code is returned (MESH_NO_MEM,
 * MESH_CORRUPTED, etc.)  and '*a_ref' is not modified. */
static int read_mfvec3f(vertex_t **a_ref, struct file_data *data)
{
  struct block_list *tmp=NULL;
  float *vals=NULL;
  vertex_t *vtcs=NULL;
  int n_vals, n_vtcs;
  int i, j, rcode;

  tmp = (struct block_list*)calloc(1, sizeof(struct block_list));
  if (tmp == NULL)
    return MESH_NO_MEM;

  rcode = init_block_list(tmp, sizeof(float));
  if (rcode < 0)
    return rcode;

  n_vals = read_mffloat(tmp, data, 3);

  if (n_vals < 0) {
    free_block_list(&tmp);
    return n_vals; /* error */
  }

  /* gather data into a classical array */
  vals = (float*)malloc(n_vals*sizeof(float));
  if (vals == NULL)
    return MESH_NO_MEM;

  rcode = gather_block_list(tmp, vals, n_vals*sizeof(float));
  free_block_list(&tmp);
  if (rcode < 0)
    return rcode;

  n_vtcs = n_vals/3;
  if (n_vtcs > 0) {
    vtcs = malloc(sizeof(*vtcs)*n_vtcs);
    if (vtcs != NULL) {
      for (i=0, j=0; i<n_vtcs; i++) {
        vtcs[i].x = vals[j++];
        vtcs[i].y = vals[j++];
        vtcs[i].z = vals[j++];
      }
    } else {
      n_vtcs = MESH_NO_MEM;
    }
  } else {
    vtcs = NULL;
  }
  free(vals);
  if (n_vtcs >= 0) 
    *a_ref = vtcs;
  
  return n_vtcs;
}

/* Reads an ASCII VRML mfvec3f (array of floting-point point coords). The
 * array must start by an opening bracket '[', values be separated by VRML
 * whitespace (e.g., commas) and it must finish with a closing bracket ']'. If
 * the array has only one value the brackets can be left out. The array
 * (allocated via malloc) is returned in '*a_ref'. It is NULL for empty
 * arrays. In '*bbox_min' and '*bbox_max' the min and max of the vertices
 * bounding box is returned. The number of elements in the array is returned
 * by the function. In case of error the negative error code is returned
 * (MESH_NO_MEM, MESH_CORRUPTED, etc.)  and '*a_ref' '*bbox_min' and
 * '*bbox_max' are not modified. */
static int read_mfvec3f_bbox(vertex_t **a_ref, struct file_data *data,
                             vertex_t *bbox_min, vertex_t *bbox_max)
{
  struct block_list *tmp=NULL;
  float *vals=NULL;
  vertex_t *vtcs=NULL;
  vertex_t bbmin, bbmax;
  int n_vals, n_vtcs;
  int i, j, rcode;

  tmp = (struct block_list*)calloc(1, sizeof(struct block_list));
  if (tmp == NULL)
    return MESH_NO_MEM;

  rcode = init_block_list(tmp, sizeof(float));
  if (rcode < 0)
    return rcode;


  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;

  n_vals = read_mffloat(tmp, data, 3);

  if (n_vals < 0) {
    free_block_list(&tmp);
    return n_vals; /* error */
  }
  /* gather data into a classical array */
  vals = (float*)malloc(n_vals*sizeof(float));
  if (vals == NULL)
    return MESH_NO_MEM;

  rcode = gather_block_list(tmp, vals, n_vals*sizeof(float));
  free_block_list(&tmp);
  if (rcode < 0)
    return rcode;

  n_vtcs = n_vals/3;
  if (n_vtcs > 0) {
    vtcs = malloc(sizeof(*vtcs)*n_vtcs);
    if (vtcs != NULL) {
      for (i=0, j=0; i<n_vtcs; i++) {
        vtcs[i].x = vals[j++];
        if (vtcs[i].x < bbmin.x) bbmin.x = vtcs[i].x;
        if (vtcs[i].x > bbmax.x) bbmax.x = vtcs[i].x;
        vtcs[i].y = vals[j++];
        if (vtcs[i].y < bbmin.y) bbmin.y = vtcs[i].y;
        if (vtcs[i].y > bbmax.y) bbmax.y = vtcs[i].y;
        vtcs[i].z = vals[j++];
        if (vtcs[i].z < bbmin.z) bbmin.z = vtcs[i].z;
        if (vtcs[i].z > bbmax.z) bbmax.z = vtcs[i].z;
      }
    } else {
      n_vtcs = MESH_NO_MEM;
    }
  } else {
    vtcs = NULL;
  }
  free(vals);
  if (n_vtcs >= 0) {
    *a_ref = vtcs;
    if (n_vtcs == 0) {
      memset(&bbmin,0,sizeof(bbmin));
      memset(&bbmax,0,sizeof(bbmax));
    }
    *bbox_min = bbmin;
    *bbox_max = bbmax;
  }
  return n_vtcs;
}

/* Reads an ASCII or Inventor VRML coordIndex field, verifying that it forms a
 * triangular mesh. The array of faces (allocated via malloc) is returned in
 * '*a_ref' and the maximum vertex index in '*max_vidx'. It is NULL if there
 * are zero faces. The number of faces in the field is returned by the
 * function. In case of error the negative error code is returned
 * (MESH_NO_MEM, MESH_CORRUPTED, MESH_NOT_TRIAG, etc.)  and '*a_ref' and
 * '*max_vidx' are not modified. */
static int read_tcoordindex(face_t **a_ref, struct file_data *data, 
                            int *max_vidx)
{
  int *fidxs;
  struct block_list *tmp=NULL;
  face_t *faces;
  int n_fidxs,n_faces;
  int i, j, rcode;
  int max_idx;

  fidxs = NULL;
  max_idx = -1;
  tmp = (struct block_list*)calloc(1, sizeof(struct block_list));

  if (tmp == NULL)
    return MESH_NO_MEM;

  rcode = init_block_list(tmp, sizeof(int));
  if (rcode < 0) {
    free_block_list(&tmp);
    return rcode;
  }

  n_fidxs = read_mfint32(tmp,data,&max_idx);
  if (n_fidxs < 0) return n_fidxs; /* error */

  /* gather data */
  fidxs = (int*)malloc(n_fidxs*sizeof(int));
  if (fidxs == NULL)
    return MESH_NO_MEM;
  rcode = gather_block_list(tmp, fidxs, n_fidxs*sizeof(int));
  free_block_list(&tmp);
  if (rcode < 0)
    return rcode;


  n_faces = (n_fidxs+1)/4;
  if (n_faces > 0) {
    faces = malloc(sizeof(*faces)*n_faces);
    if (faces != NULL) {
      for (i=0, j=0; i<n_faces; i++) {
        faces[i].f0 = fidxs[j++];
        faces[i].f1 = fidxs[j++];
        faces[i].f2 = fidxs[j++];
        if (faces[i].f0 < 0 || faces[i].f1 < 0 || faces[i].f2 < 0) {
          n_faces = MESH_CORRUPTED;
          break;
        } else if (j < n_fidxs && fidxs[j++] != -1) {
          n_faces = (fidxs[j-1] >= 0) ? MESH_NOT_TRIAG : MESH_CORRUPTED;
          break;
        }
      }
    } else {
      n_faces = MESH_NO_MEM;
    }
  } else {
    faces = NULL;
  }
  free(fidxs);
  if (n_faces >= 0) {
    *a_ref = faces;
    *max_vidx = max_idx;
  }
  return n_faces;
}

/* Reads a VRML Coordinate node, returning the number of 3D points read, or a
 * negative error code if an error occurs (MESH_NO_MEM, MESH_CORRUPTED, etc.).
 * The array of points is returned in '*vtcs_ref' (allocated via malloc). It
 * is NULL if there are zero points. In '*bbox_min' and '*bbox_max' the min
 * and max of the vertices bounding box is returned. If an error occurs
 * '*vtcs_ref' '*bbox_min' and '*bbox_max' are not modified. */
static int read_vrml_coordinate(vertex_t **vtcs_ref, struct file_data *data,
                                vertex_t *bbox_min, vertex_t *bbox_max)
{
  char stmp[MAX_WORD_LEN+1];
  int c;
  int rcode;
  vertex_t *vtcs;
  int n_vtcs;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  getc(data); /* skip { */

  rcode = 0;
  n_vtcs = 0;
  vtcs = NULL;
  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      getc(data); /* skip } */
    } else if (c != EOF && string_scanf(data,stmp) == 1) { /* field */
      if (strcmp(stmp,"point") == 0) {
#ifdef DEBUG
        DEBUG_PRINT("point found\n");
#endif
        n_vtcs = read_mfvec3f_bbox(&vtcs,data,bbox_min,bbox_max);
#ifdef DEBUG
        DEBUG_PRINT("n_vtcs = %d\n", n_vtcs);
#endif
        if (n_vtcs < 0) { /* error */
          rcode = n_vtcs;
        }
      } else { /* unsupported field */
        rcode = skip_vrml_field(data);
      }
    } else { /* error */
      rcode = MESH_CORRUPTED;
    }
  } while (c != '}' && rcode == 0);
  if (rcode >= 0) { /* no error */
    *vtcs_ref = vtcs;
    rcode = n_vtcs;
  }
  return rcode;
}

/* Reads a VRML Normal node, returning the number of 3D points read, or a
 * negative error code if an error occurs (MESH_NO_MEM, MESH_CORRUPTED, etc.).
 * The array of normal vectors is returned in '*nrmls_ref' (allocated via
 * malloc). It is NULL if there are zero points. If an error occurs
 * '*nrmls_ref' is not modified. */
static int read_vrml_normal(vertex_t **nrmls_ref, struct file_data *data)
{
  char stmp[MAX_WORD_LEN+1];
  int c;
  int rcode;
  vertex_t *nrmls;
  int n_nrmls;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  getc(data); /* skip { */

  rcode = 0;
  n_nrmls = 0;
  nrmls = NULL;
  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      getc(data); /* skip } */
    } else if (c != EOF && string_scanf(data,stmp) == 1) { /* field */
      if (strcmp(stmp,"vector") == 0) {
        n_nrmls = read_mfvec3f(&nrmls,data);
        if (n_nrmls < 0) { /* error */
          rcode = n_nrmls;
        }
      } else { /* unsupported field */
        rcode = skip_vrml_field(data);
      }
    } else { /* error */
      rcode = MESH_CORRUPTED;
    }
  } while (c != '}' && rcode == 0);
  if (rcode >= 0) { /* no error */
    *nrmls_ref = nrmls;
    rcode = n_nrmls;
  }
  return rcode;
}

/* Reads a VRML IndexedFaceSet node containing a triangular mesh. The
 * resulting model is returned in '*tmesh'. Currently only vertices and faces
 * are read. If an error occurs the negative error code is returned
 * (MESH_CORRUPTED, MESH_NO_MEM, MESH_NOT_TRIAG, etc.). Otherwise zero is
 * returned. If an error occurs '*tmesh' is not modified. Otherwise all its
 * values are discarded. */
static int read_vrml_ifs(struct model *tmesh, struct file_data *data)
{
  char stmp[MAX_WORD_LEN+1];
  int c;
  vertex_t *vtcs=NULL;
  face_t *faces=NULL;
  vertex_t *normals=NULL, *vnormals=NULL, *fnormals=NULL;
  int *nrml_idcs=NULL;
  struct block_list *tmp=NULL;
  vertex_t bbmin,bbmax;
  int n_vtcs=-1, n_faces=-1, n_nrmls=-1, n_nrml_idcs=-1;
  int max_vidx=-1, max_nidx=-1;
  int nrml_per_vertex=1;
  int rcode=0;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  getc(data); /* skip { */



  memset(&bbmin,0,sizeof(bbmin));
  memset(&bbmax,0,sizeof(bbmax));

  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      getc(data); /* skip } */
    } else if (c != EOF && string_scanf(data,stmp) == 1) { /* field */
      if (strcmp(stmp,"coord") == 0) { /* Coordinates */
#ifdef DEBUG
        DEBUG_PRINT("coord found\n");
#endif
        if (n_vtcs != -1) {
          rcode = MESH_CORRUPTED;
        } else if ((rcode = read_node_type(stmp,data,MAX_WORD_LEN+1)) == 0 &&
                   strcmp(stmp,"Coordinate") == 0) {
#ifdef DEBUG
          DEBUG_PRINT("Coordinate found\n");
#endif
          n_vtcs = read_vrml_coordinate(&vtcs,data,&bbmin,&bbmax);
#ifdef DEBUG
          DEBUG_PRINT("read_vrml_coordinate done\n");
#endif
          if (n_vtcs < 0) rcode = n_vtcs; /* error */
        } else if (rcode == 0) {
          rcode = MESH_CORRUPTED;
        }
      } else if (strcmp(stmp,"coordIndex") == 0) { /* faces */
        if (n_faces != -1) {
          rcode = MESH_CORRUPTED;
        } else {
          n_faces = read_tcoordindex(&faces,data,&max_vidx);
          if (n_faces < 0) rcode = n_faces; /* error */
        }
      } else if (strcmp(stmp,"normalPerVertex") == 0) {
        rcode = read_sfbool(&nrml_per_vertex,data);
      } else if (strcmp(stmp,"normal") == 0) { /* normal vectors */
        if (n_nrmls != -1) {
          rcode = MESH_CORRUPTED;
        } else if ((rcode = read_node_type(stmp,data,MAX_WORD_LEN+1)) == 0 &&
                   strcmp(stmp,"Normal") == 0) {
          n_nrmls = read_vrml_normal(&normals,data);
          if (n_nrmls < 0) rcode = n_nrmls; /* error */
        } else if (rcode == 0) {
          rcode = MESH_CORRUPTED;
        }
      } else if (strcmp(stmp,"normalIndex") == 0) {
        if (n_nrml_idcs != -1) {
          rcode = MESH_CORRUPTED;
        } else {
          tmp = (struct block_list*)calloc(1, sizeof(struct block_list));
          if (tmp == NULL)
            break;

          rcode = init_block_list(tmp, sizeof(int));
          if (rcode < 0)
            break;

          n_nrml_idcs = read_mfint32(tmp,data,&max_nidx);
          if (n_nrml_idcs < 0) { 
            free_block_list(&tmp);
            rcode = n_nrml_idcs; /* error */
            break;
          }
          else {
            nrml_idcs = (int*)malloc(n_nrml_idcs*sizeof(int));
            if (nrml_idcs == NULL) {
              rcode = MESH_NO_MEM;
              break;
            }
            rcode = gather_block_list(tmp, nrml_idcs, n_nrml_idcs*sizeof(int));
            free_block_list(&tmp);
            if (rcode < 0)
              break;
          }
            
        }
      } else if (strcmp(stmp,"color") == 0 || strcmp(stmp,"texCoord") == 0) {
        /* unsupported node field */
        rcode = read_node_type(stmp,data,MAX_WORD_LEN+1);
        if (rcode == 0) rcode = skip_vrml_field(data);
      } else { /* unsupported non-node field */
        rcode = skip_vrml_field(data);
      }
    } else { /* error */
      rcode = MESH_CORRUPTED;
    }
  } while (c != '}' && rcode == 0);
  if (rcode == 0) {
    if (n_vtcs == -1) n_vtcs = 0;
    if (n_faces == -1) n_faces = 0;
    if (n_vtcs <= max_vidx) {
#ifdef DEBUG
      DEBUG_PRINT("n_vtcs=%d <= max_vidx=%d\n", n_vtcs, max_vidx);
#endif
      rcode = MESH_MODEL_ERR;
    } else {
      n_vtcs = max_vidx+1;
    }
    if (rcode == 0 && n_nrmls > 0) {
      if (n_nrml_idcs > 0) { /* convert indexed to direct */
        if (nrml_per_vertex) { /* vertex normals */
          n_nrmls = tidxnormals_to_vnormals(&vnormals,normals,n_nrmls,
                                            nrml_idcs,n_nrml_idcs,max_nidx,
                                            max_vidx,faces,n_faces);
          if (n_nrmls < 0) { /* error */
            rcode = n_nrmls;
          }
        } else { /* face normals */
          n_nrmls = tidxnormals_to_fnormals(&fnormals,normals,n_nrmls,
                                            nrml_idcs,n_nrml_idcs,max_nidx,
                                            n_faces);
          if (n_nrmls < 0) { /* error */
            rcode = n_nrmls;
          }
        }
      } else { /* already direct normals */
        if (nrml_per_vertex) { /* vertex normals */
          if (n_nrmls <= max_vidx) {
            rcode = MESH_MODEL_ERR;
          } else {
            vnormals = normals;
            normals = NULL;
          }
        } else { /* face normals */
          if (n_nrmls < n_faces) {
            rcode = MESH_MODEL_ERR;
          } else {
            fnormals = normals;
            normals = NULL;
          }
        }
      }
    }
  }

  if (rcode == 0) {
    memset(tmesh,0,sizeof(*tmesh)); /* reset *tmesh */
    tmesh->vertices = vtcs;
    tmesh->num_vert = n_vtcs;
    tmesh->faces = faces;
    tmesh->num_faces = n_faces;
    tmesh->normals = vnormals;
    tmesh->face_normals = fnormals;
    tmesh->builtin_normals = (vnormals != NULL);
    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
  } else {
    free(vtcs);
    free(faces);
  }
  if (normals != NULL)
    free(normals);
  if (nrml_idcs != NULL)
    free(nrml_idcs);

  return rcode;
}

/* Concatenates the 'n' models in the '*inmeshes' array into the new model
 * (allocated with malloc) pointed to by '*outmesh_ref'. Currently only
 * vertices and faces are handled. If an error occurs '*outmesh_ref' is not
 * modified and the negative error code is returned (MESH_NO_MEM,
 * etc). Otherwise zero is returned. */
static int concat_models(struct model **outmesh_ref,
                         const struct model *inmeshes, int n)
{
  struct model *mesh;
  int k,i,j;
  int vtx_off;
  int w_vtx_nrmls,w_face_nrmls;

  /* Initialize */
  mesh = calloc(1,sizeof(*mesh));
  if (mesh == NULL) return MESH_NO_MEM;
  w_vtx_nrmls = 1;
  w_face_nrmls = 1;
  for (k=0; k<n; k++) {
    mesh->num_vert += inmeshes[k].num_vert;
    mesh->num_faces += inmeshes[k].num_faces;
    if (mesh->normals == NULL) w_vtx_nrmls = 0;
    if (mesh->face_normals == NULL) w_face_nrmls = 0;
  }
  mesh->vertices = malloc(sizeof(*(mesh->vertices))*mesh->num_vert);
  mesh->faces = malloc(sizeof(*(mesh->faces))*mesh->num_faces);
  if (w_vtx_nrmls) {
    mesh->normals = malloc(sizeof(*(mesh->normals))*mesh->num_vert);
  }
  if (w_face_nrmls) {
    mesh->face_normals = malloc(sizeof(*(mesh->face_normals))*mesh->num_faces);
  }
  if (mesh->vertices == NULL || mesh->faces == NULL ||
      (w_vtx_nrmls && mesh->normals == NULL) ||
      (w_face_nrmls && mesh->face_normals == NULL)) {
    free(mesh->vertices);
    free(mesh->faces);
    free(mesh->normals);
    free(mesh->face_normals);
    free(mesh);
    return MESH_NO_MEM;
  }
  mesh->bBox[0].x = mesh->bBox[0].y = mesh->bBox[0].z = FLT_MAX;
  mesh->bBox[1].x = mesh->bBox[1].y = mesh->bBox[1].z = -FLT_MAX;
  /* Concatenate vertices */
  for (k=0, j=0; k<n; k++) {
    for (i=0; i<inmeshes[k].num_vert; i++, j++) {
      mesh->vertices[j] = inmeshes[k].vertices[i];
    }
    if (inmeshes[k].num_vert > 0) {
      if (inmeshes[k].bBox[0].x < mesh->bBox[0].x) {
        mesh->bBox[0].x = inmeshes[k].bBox[0].x;
      }
      if (inmeshes[k].bBox[0].y < mesh->bBox[0].y) {
        mesh->bBox[0].y = inmeshes[k].bBox[0].y;
      }
      if (inmeshes[k].bBox[0].z < mesh->bBox[0].z) {
        mesh->bBox[0].z = inmeshes[k].bBox[0].z;
      }
      if (inmeshes[k].bBox[1].x > mesh->bBox[1].x) {
        mesh->bBox[1].x = inmeshes[k].bBox[1].x;
      }
      if (inmeshes[k].bBox[1].y > mesh->bBox[1].y) {
        mesh->bBox[1].y = inmeshes[k].bBox[1].y;
      }
      if (inmeshes[k].bBox[1].z > mesh->bBox[1].z) {
        mesh->bBox[1].z = inmeshes[k].bBox[1].z;
      }
    }
  }
  /* Concatenate faces */
  for (k=0, j=0, vtx_off=0; k<n; k++) {
    for (i=0; i<inmeshes[k].num_faces; i++, j++) {
      mesh->faces[j].f0 = inmeshes[k].faces[i].f0+vtx_off;
      mesh->faces[j].f1 = inmeshes[k].faces[i].f1+vtx_off;
      mesh->faces[j].f2 = inmeshes[k].faces[i].f2+vtx_off;
    }
    vtx_off += inmeshes[k].num_vert;
  }
  /* Concatenate vertex normals */
  if (w_vtx_nrmls) {
    for (k=0, j=0; k<n; k++) {
      for (i=0; i<inmeshes[k].num_vert; i++, j++) {
        mesh->normals[j] = inmeshes[k].normals[i];
      }
    }
    mesh->builtin_normals = 1;
  }
  /* Concatenate face normals */
  if (w_face_nrmls) {
    for (k=0, j=0; k<n; k++) {
      for (i=0; i<inmeshes[k].num_faces; i++, j++) {
        mesh->face_normals[j] = inmeshes[k].face_normals[i];
      }
    }
  }

  /* Return resulting model */
  *outmesh_ref = mesh;
  return 0;
}


/* Reads all IndexedFaceSet meshes from the '*data' stream in VRML format. All
 * of them should be triangular meshes. Any transformations are ignored. If
 * succesful the number of read meshes is returned. Otherwise the negative
 * error code (MESH_CORRUPTED, MESH_NOT_TRIAG, MESH_NO_MEM, etc.) is
 * returned. If no error occurs the read meshes are returned in the new
 * '*tmeshes_ref' array (allocated via malloc). Otherwise '*tmeshes_ref' is
 * not modified. If 'concat' is non-zero all meshes are concatenated into one,
 * and only one is returned. */
int read_vrml_tmesh(struct model **tmeshes_ref, struct file_data *data,
                    int concat) {
  struct model *tmeshes;
  struct model *ctmesh;
  int c,i;
  int len,n_tmeshes;
  int rcode;

  rcode = 0;
  tmeshes = NULL;
  ctmesh = NULL;
  n_tmeshes = 0;
  len = 0;

  do {
    c = find_string(data,"IndexedFaceSet");
#ifdef DEBUG
    DEBUG_PRINT("found IndexedFaceSet c=%d\n", c);
#endif
    if (c != EOF) {
      if (n_tmeshes == len) {
        tmeshes = grow_array(tmeshes,sizeof(*tmeshes),&len,SZ_MAX_INCR);
        if (tmeshes == NULL) {
          rcode = MESH_NO_MEM;
          break;
        }
      }

      rcode = read_vrml_ifs(&(tmeshes[n_tmeshes]),data);

      n_tmeshes++;
    }
  } while (c != EOF && rcode == 0);

  if (rcode == 0) {
    if (concat && n_tmeshes > 1) { /* concatenate all meshes */
      rcode = concat_models(&ctmesh,tmeshes,n_tmeshes);
    }
  }

  if (rcode == 0) {
    if (ctmesh != NULL) {
      *tmeshes_ref = ctmesh;
      rcode = 1;
      ctmesh = NULL; /* avoid freeing ctmesh */
    } else {
      *tmeshes_ref = tmeshes;
      rcode = n_tmeshes;
      n_tmeshes = 0; /* avoid freeing tmeshes */
    }
  }
  
  for (i=0; i<n_tmeshes; i++) {
    free_model_pfields(tmeshes+i);
  }
  free_model_pfields(ctmesh);
  return rcode;
}



/* Reads a triangular mesh from an Inventor file (which can be
 * gzipped). Only the 'Coordinate3' and 'IndexedFaceSet' fields are
 * read. Everything else (normals, etc...) is silently
 * ignored. Multiple 'Coordinate3' and 'IndexedFaceSet' are *NOT*
 * supported. It returns the number of meshes read (i.e. 1) if
 * successful, and a negative code if it failed. */
int read_iv_tmesh(struct model **tmesh_ref, struct file_data *data) {
  int c, rcode=0;
  struct model *tmesh;
  char stmp[MAX_WORD_LEN+1];
  vertex_t bbmin, bbmax;
  vertex_t *vtcs=NULL;
  face_t *faces=NULL;
  int n_vtcs=-1, n_faces=-1, max_vidx=-1;
  

  tmesh = (struct model*)calloc(1, sizeof(*tmesh));
  c = find_string(data, "Separator");
  if (c != EOF) {

    do {
      c = skip_ws_comm(data);
      if (c == '}')
        getc(data);
      else if (c != EOF && string_scanf(data, stmp) == 1) {

        if (strcmp(stmp, "Coordinate3") == 0) { /* Coordinate3 */
#ifdef DEBUG
          DEBUG_PRINT("Coordinate3 found\n");
#endif
          if (n_vtcs != -1)
            rcode = MESH_CORRUPTED;
          else {
            n_vtcs = read_vrml_coordinate(&vtcs, data, &bbmin, &bbmax);
#ifdef DEBUG
            DEBUG_PRINT("nvtcs=%d\n", n_vtcs);
#endif
            if (n_vtcs < 0) rcode = n_vtcs; /* error */
          }
        } 
        else if (strcmp(stmp, "IndexedFaceSet") == 0) { /* IFS */
#ifdef DEBUG
          DEBUG_PRINT("IndexedFaceSet found\n");
#endif
         /* a 'coordIndex' field should not be too far ...*/
          c = find_string(data, "coordIndex"); 
          if (c == EOF) 
            return MESH_CORRUPTED;
#ifdef DEBUG
          DEBUG_PRINT("coordIndex found\n");
#endif
          if (n_faces != -1)
            rcode = MESH_CORRUPTED;
          else {
            n_faces = read_tcoordindex(&faces, data, &max_vidx);
#ifdef DEBUG
            DEBUG_PRINT("nfaces=%d\n", n_faces);
#endif
            if (n_faces < 0) rcode = n_faces; /* error */
          }
        } else /* Ignore everything else ... */
          rcode = skip_vrml_field(data); 

      } 
    } while (c != '}' && rcode == 0);

    if (rcode == 0) {
      if (n_vtcs == -1) n_vtcs = 0;
      if (n_faces == -1) n_faces = 0;
      if (n_vtcs <= max_vidx) {
        rcode = MESH_MODEL_ERR;
      } else {
        n_vtcs = max_vidx+1;
      }
    }

    if (rcode == 0) {
      memset(tmesh,0,sizeof(*tmesh)); /* reset *tmesh */
      tmesh->vertices = vtcs;
      tmesh->num_vert = n_vtcs;
      tmesh->faces = faces;
      tmesh->num_faces = n_faces;
      tmesh->bBox[0] = bbmin;
      tmesh->bBox[1] = bbmax;
      *tmesh_ref = tmesh;
      rcode = 1;
    }

  } else { /* c == EOF */
    free(vtcs);
    free(faces);
    free(tmesh);
    rcode =  MESH_CORRUPTED;
  }
  return rcode;
}


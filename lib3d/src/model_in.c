/* $Id: model_in.c,v 1.1 2002/02/04 15:56:21 dsanta Exp $ */

/*
 * Functions to read 3D model data from files
 *
 * Author: Diego Santa Cruz
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <model_in.h>

/* --------------------------------------------------------------------------
   PARAMETERS
   -------------------------------------------------------------------------- */

/* Maximum allowed word length */
#define MAX_WORD_LEN 60
/* Default initial number of elements for an array */
#define SZ_INIT_DEF 240
/* Maximum number of elements by which an array is grown */
#define SZ_MAX_INCR 2048

/* Converts argument into string, without replacing defines in argument */
#define STRING_Q(N) #N
/* Converts argument into string, replacing defines in argument */
#define STRING(N) STRING_Q(N)

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

/* --------------------------------------------------------------------------
   LOCAL FUNCTIONS
   -------------------------------------------------------------------------- */

/* Grows the array pointed to by 'p'. The current array size (in elements) is
 * given by '*len'. The array is doubled in size, if the size increment is
 * less than 'max_incr' elements, or augmented by 'max_incr' elements
 * otherwise. The new array size is returned in '*len'. The element size, in
 * bytes, is given by 'elem_sz'. If p is NULL a new array is allocated of size
 * '*len', if '*len' is not zero, or SZ_INIT_DEF otherwise. Otherwise, if
 * '*len' is zero, the new length is SZ_INIT_DEF. Otherwise the size is
 * doubled, up to a maximum increment of 'max_incr'. If 'max_incr' is zero, it
 * is taken to be infinity (i.e. the array size is always doubled). The
 * pointer to the new storage allocation is returned. If there is not enough
 * memory NULL is returned and the current array, if any, is freed, and zero
 * is returned in '*len'.  */
static void* grow_array(void *p, size_t elem_sz, int *len, int max_incr)
{
  void *new_p;
  int cur_len,new_len;

  assert(elem_sz != 0);
  assert(*len >= 0 && max_incr >= 0);
  cur_len = *len;
  if (p == NULL) { /* no array */
    new_len = (cur_len > 0) ? cur_len : SZ_INIT_DEF;
  } else if (cur_len <= 0) { /* empty array */
    new_len = SZ_INIT_DEF;
  } else {
    new_len = (max_incr <= 0 || cur_len < max_incr) ? 2*cur_len :
      cur_len+max_incr;
  }
  new_p = realloc(p,elem_sz*new_len);
  if (new_p != NULL) {
    *len = new_len;
  } else {
    free(p);
    *len = 0;
  }
  return new_p;
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
 * faces by 'n_faces'. The maximum normal index is given by 'max_nidx' and the
 * maximum vertex index (of 'faces') in 'max_vidx'. If no error occurs the
 * number of normals in '*vnrmls_ref' is returned. Otherwise the negative
 * error code is returned and '*vnrmls_ref' is not modified. */
static int tidxnormals_to_vnormals(vertex_t **vnrmls_ref, vertex_t *nrmls,
                                   int n_nrmls, int *nrml_idcs, int n_nrml_idcs,
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
    mesh->builtin_normals = 1;
  }

  /* Return resulting model */
  *outmesh_ref = mesh;
  return 0;
}

/* Advances the data stream to the next non-comment and non-whitespace
 * character in the '*data' stream. If the end-of-file is reached or an I/O
 * error occurs, EOF is returned. Otherwise the next character to be read from
 * '*data' is returned. Comments and whitespace are interpreted as in VRML
 * (note that a comma is a whitespace in VRML!).
 */
static int skip_ws_comm(FILE *data)
{
  int c;

  c = fgetc(data);
  do {
    if (c == '#' /* hash */ ) { /* Comment line, skip whole line */
      do {
        c = fgetc(data);
      } while (c != '\n' && c != '\r' && c != EOF);
      if (c != EOF) c = fgetc(data); /* Get first character of next line */
    } else if (c == ' ' || c == '\t' || c == ',' || c == '\n' || c == '\r') {
      /* VRML whitespace char, get next character */
      c = fgetc(data);
    } else if (c != EOF) { /* a meaningful character, put it back */
      c = ungetc(c,data);
      break;
    }
  } while (c != EOF);
  return c;
}

/* Like skip_ws_comm(), but also skips over VRML quoted strings. Returns the
 * next non-whitespace, non-comment and non-string character, or EOF if
 * end-of-file or I/O error is encountered. */
static int skip_ws_comm_str(FILE *data)
{
  int c;
  int in_escape;

  c = skip_ws_comm(data);
  while (c == '"') { /* skip all consecutive strings */
    fgetc(data); /* skip '"' starting string */
    in_escape = 0;
    do {
      c = fgetc(data);
      if (!in_escape) {
        if (c == '\\') in_escape = 1;
      } else {
        in_escape = 0;
        if (c == '"') c = '\0'; /* escaped double quotes, not interesting */
      }
    } while (c != '"' && c != EOF);
    if (c != EOF) {
      c = skip_ws_comm(data);
    }
  }
  return c;
}

/* Advances the '*data' stream until one of the characters in 'chars' (null
 * terminated string) is the next character to be read. Returns the matched
 * character or EOF if end-of-file or an I/O error is encountered. Characters
 * in quoted strings and comments (excluding line-termination character) of
 * the '*data' stream are never matched. */
static int find_chars(FILE *data, const char *chars)
{
  int c;
  int in_escape;

  c = fgetc(data);
  while (c != EOF && strchr(chars,c) == NULL) {
    if (c == '"') { /* start of quoted string, skip it */
      in_escape = 0;
      do {
        c = fgetc(data);
        if (!in_escape) {
          if (c == '\\') in_escape = 1;
        } else {
          in_escape = 0;
          if (c == '"') c = '\0'; /* escaped double quotes, not interesting */
        }
      } while (c != '"' && c != EOF);
      if (c != EOF) c = fgetc(data); /* next char */
    } else if (c == '#') { /* start comment, skip it */
      do {
        c = fgetc(data);
      } while (c != '\n' && c != '\r' && c != EOF);
    } else { /* unquoted char, just get next */
      c = fgetc(data);
    }
  }
  if (c != EOF) c = ungetc(c,data); /* put matched char back */
  return c;
}

/* Advances the '*data' stream until the 'string' is found as a whole
 * word. Returns the next character to be read from the '*data' stream, which
 * is the first character after the word 'string'. EOF is returned if
 * end-of-file is reached or an I/O error occurs. Comments, quoted strings and
 * whitespace in the '*data' stream are ignored. The 'string' argument shall
 * not contain whitespace, comments or quoted strings. */
static int find_string(FILE *data, const char *string)
{
  int c,i;

  do {
    c = fgetc(data);
    if (strchr(VRML_WSCOMMSTR_CHARS,c)) { /* characters we need to skip */
      if (ungetc(c,data) == EOF) return EOF;
      skip_ws_comm_str(data);
      c = fgetc(data);
    }
    i = 0;
    while (c != EOF && string[i] != '\0' && string[i] == c) {
      c = fgetc(data);
      i++;
    }
    if (!ferror(data) && string[i] == '\0' &&
        (c == EOF || strchr(VRML_WSCOMMSTR_CHARS,c) != NULL)) {
      break; /* string matched and is whole word */
    }
  } while (c != EOF);
  if (c != EOF) c = ungetc(c,data);
  return c;
}

/* Advances the '*data' stream past the end of the VRML field (single, array
 * or node). Returns zero on success and an error code (MESH_CORRUPTED, etc.) 
 * on error. */
static int skip_vrml_field(FILE *data)
{
  int c,n_brace;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;

  if (c == '[') { /* array enclosed in [], skip until next ] */
    fgetc(data); /* skip [ */
    find_chars(data,"]");
    c = fgetc(data); /* skip ] */
  } if (c == '{') { /* a node, skip (including embedded nodes) */
    fgetc(data); /* skip { */
    n_brace = 1;
    do {
      c = find_chars(data,"{}");
      if (c == '{') { /* embedded node start */
        fgetc(data); /* skip { */
        n_brace++;
      } else if (c == '}') { /* node ending */
        fgetc(data); /* skip } */
        n_brace--;
      }
    } while (n_brace > 0 && c != EOF);
  } else if (c != EOF) { /* single value */
    c = find_chars(data,VRML_WS_CHARS);
  }
  return (c != EOF) ? 0 : MESH_CORRUPTED;
}

/* Reads a VRML boolean field from the '*data' stream. If the field is "TRUE"
 * one is returned in '*res', if it is "FALSE" zero is returned in '*res'. Any
 * other value is an error. If an error occurs '*res' is not modified and the
 * negative error value is returned. Otherwise zero is returned. */
static int read_sfbool(int *res, FILE *data)
{
  char stmp[MAX_WORD_LEN+1];
  const char sfmt[] = "%" STRING(MAX_WORD_LEN) "[^" VRML_WSCOMMSTR_CHARS "]";
  int rcode;

  rcode = 0;
  if (skip_ws_comm(data) != EOF && fscanf(data,sfmt,stmp) == 1) {
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
 * fields with 'nelem = 3', otherwise 'nelem' should be 1 for MFFloat). The
 * array (allocated via malloc) is returned in '*a_ref'. It is NULL for empty
 * arrays. The number of elements in the array is returned by the function. In
 * case of error the negative error code is returned (MESH_NO_MEM,
 * MESH_CORRUPTED, etc.)  and '*a_ref' is not modified. */
static int read_mffloat(float **a_ref, FILE *data, int nelem)
{
  int len,n;
  int c,in_brackets;
  float *array;
  float tmpf;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;
  if (c == '[') {
    in_brackets = 1;
    fgetc(data); /* skip [ */
  } else {
    in_brackets = 0;
  }

  /* Get numbers until we reach closing bracket */
  array = NULL;
  len = 0;
  n = 0;
  do {
    c = skip_ws_comm(data);
    if (c == ']') {
      if (!in_brackets) {
        n = MESH_CORRUPTED;
      } else {
        fgetc(data); /* skip ] */
        break;
      }
    } else if (c != EOF && fscanf(data,"%f",&tmpf) == 1) {
      if (n == len) { /* Need more storage */
        array = grow_array(array,sizeof(*array),&len,SZ_MAX_INCR);
        if (array == NULL) {
          n = MESH_NO_MEM;
          break;
        }
      }
      array[n++] = tmpf;
    } else { /* error */
      n = MESH_CORRUPTED;
    }
  } while ((in_brackets || n < nelem) && c != EOF && n >= 0);

  if (n >= 0) { /* read OK */
    *a_ref = array;
  } else { /* An error occurred */
    free(array);
  }
  return n;
}

/* Reads an ASCII VRML or Inventor integer array. The array must start by an
 * opening bracket '[', values be separated by VRML whitespace (e.g., commas)
 * and it must finish with a closing bracket ']'. If the array has only one
 * value the brackets can be left out. The array (allocated via malloc) is
 * returned in '*a_ref' and the maximum value in it returned in '*max_val',
 * '*a_ref' is NULL for empty arrays. The number of elements in the array is
 * returned by the function. In case of error the negative error code is
 * returned (MESH_NO_MEM, MESH_CORRUPTED, etc.)  and '*a_ref' and '*max_val'
 * are not modified. */
static int read_mfint32(int **a_ref, FILE *data, int *max_val)
{
  int len,n;
  int c,in_brackets;
  int *array;
  int tmpi;
  int maxv;

  /* Identify first char */
  if ((c = skip_ws_comm(data)) == EOF) return MESH_CORRUPTED;
  if (c == '[') {
    in_brackets = 1;
    fgetc(data); /* skip [ */
  } else {
    in_brackets = 0;
  }

  /* Get numbers until we reach closing bracket */
  array = NULL;
  len = 0;
  n = 0;
  maxv = INT_MIN;
  do {
    c = skip_ws_comm(data);
    if (c == ']') {
      if (!in_brackets) {
        n = MESH_CORRUPTED;
      } else {
        fgetc(data); /* skip ] */
        break;
      }
    } else if (c != EOF && fscanf(data,"%i",&tmpi) == 1) {
      if (n == len) { /* Reallocate storage */
        array = grow_array(array,sizeof(*array),&len,SZ_MAX_INCR);
        if (array == NULL) {
          n = MESH_NO_MEM;
          break;
        }
      }
      if (tmpi > maxv) maxv = tmpi;
      array[n++] = tmpi;
    } else { /* error */
      n = MESH_CORRUPTED;
    }
  } while (in_brackets && c != EOF && n >= 0);

  if (n >= 0) { /* read OK */
    *a_ref = array;
    *max_val = maxv;
  } else { /* An error occurred */
    free(array);
  }
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
static int read_mfvec3f(vertex_t **a_ref, FILE *data)
{
  float *vals;
  vertex_t *vtcs;
  int n_vals,n_vtcs;
  int i,j;

  vals = NULL;
  n_vals = read_mffloat(&vals,data,3);
  if (n_vals < 0) {
    return n_vals; /* error */
  }
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
  if (n_vtcs >= 0) {
    *a_ref = vtcs;
  }
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
static int read_mfvec3f_bbox(vertex_t **a_ref, FILE *data,
                             vertex_t *bbox_min, vertex_t *bbox_max)
{
  float *vals;
  vertex_t *vtcs;
  vertex_t bbmin,bbmax;
  int n_vals,n_vtcs;
  int i,j;

  vals = NULL;
  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
  n_vals = read_mffloat(&vals,data,3);
  if (n_vals < 0) {
    return n_vals; /* error */
  }
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
static int read_tcoordindex(face_t **a_ref, FILE *data, int *max_vidx)
{
  int *fidxs;
  face_t *faces;
  int n_fidxs,n_faces;
  int i,j;
  int max_idx;

  fidxs = NULL;
  max_idx = -1;
  n_fidxs = read_mfint32(&fidxs,data,&max_idx);
  if (n_fidxs < 0) return n_fidxs; /* error */
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
static int read_vrml_coordinate(vertex_t **vtcs_ref, FILE *data,
                                vertex_t *bbox_min, vertex_t *bbox_max)
{
  char stmp[MAX_WORD_LEN+1];
  const char sfmt[] = "%" STRING(MAX_WORD_LEN) "[^" VRML_WSCOMMSTR_CHARS "]";
  int c;
  int rcode;
  vertex_t *vtcs;
  int n_vtcs;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  fgetc(data); /* skip { */

  rcode = 0;
  n_vtcs = 0;
  vtcs = NULL;
  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      fgetc(data); /* skip } */
    } else if (c != EOF && fscanf(data,sfmt,stmp) == 1) { /* field */
      if (strcmp(stmp,"point") == 0) {
        n_vtcs = read_mfvec3f_bbox(&vtcs,data,bbox_min,bbox_max);
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
static int read_vrml_normal(vertex_t **nrmls_ref, FILE *data)
{
  char stmp[MAX_WORD_LEN+1];
  const char sfmt[] = "%" STRING(MAX_WORD_LEN) "[^" VRML_WSCOMMSTR_CHARS "]";
  int c;
  int rcode;
  vertex_t *nrmls;
  int n_nrmls;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  fgetc(data); /* skip { */

  rcode = 0;
  n_nrmls = 0;
  nrmls = NULL;
  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      fgetc(data); /* skip } */
    } else if (c != EOF && fscanf(data,sfmt,stmp) == 1) { /* field */
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
static int read_vrml_ifs(struct model *tmesh, FILE *data)
{
  char stmp[MAX_WORD_LEN+1];
  const char sfmt[] = "%" STRING(MAX_WORD_LEN) "[^" VRML_WSCOMMSTR_CHARS "]";
  int c;
  vertex_t *vtcs;
  face_t *faces;
  vertex_t *normals,*vnormals,*fnormals;
  int *nrml_idcs;
  vertex_t bbmin,bbmax;
  int n_vtcs,n_faces,n_nrmls,n_nrml_idcs;
  int max_vidx,max_nidx;
  int nrml_per_vertex;
  int rcode;

  /* Get the opening curly bracket */
  if ((c = skip_ws_comm(data)) == EOF || c != '{') return MESH_CORRUPTED;
  fgetc(data); /* skip { */

  rcode = 0;
  vtcs = NULL;
  faces = NULL;
  normals = vnormals = fnormals = NULL;
  nrml_idcs = NULL;
  memset(&bbmin,0,sizeof(bbmin));
  memset(&bbmax,0,sizeof(bbmax));
  nrml_per_vertex = 1;
  n_vtcs = -1;
  n_faces = -1;
  n_nrmls = -1;
  n_nrml_idcs = -1;
  max_vidx = -1;
  max_nidx = -1;
  do {
    c = skip_ws_comm(data);
    if (c == '}') { /* end of node */
      fgetc(data); /* skip } */
    } else if (c != EOF && fscanf(data,sfmt,stmp) == 1) { /* field */
      fprintf(stderr,"stmp %s (rcode = %i)\n",stmp,rcode);
      if (strcmp(stmp,"coord") == 0) { /* Coordinates */
        if (n_vtcs != -1 || skip_ws_comm(data) == EOF ||
            fscanf(data,sfmt,stmp) != 1 || strcmp(stmp,"Coordinate") != 0) {
          rcode = MESH_CORRUPTED;
        } else { /* read "Coordinate" */
          n_vtcs = read_vrml_coordinate(&vtcs,data,&bbmin,&bbmax);
          if (n_vtcs < 0) rcode = n_vtcs; /* error */
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
        if (n_nrmls != -1 || skip_ws_comm(data) == EOF ||
            fscanf(data,sfmt,stmp) != 1 || strcmp(stmp,"Normal") != 0) {
          rcode = MESH_CORRUPTED;
        } else { /* read "Normal" */
          n_nrmls = read_vrml_normal(&normals,data);
          if (n_nrmls < 0) rcode = n_nrmls; /* error */
        }
      } else if (strcmp(stmp,"normalIndex") == 0) {
        if (n_nrml_idcs != -1) {
          rcode = MESH_CORRUPTED;
        } else {
          n_nrml_idcs = read_mfint32(&nrml_idcs,data,&max_nidx);
          if (n_nrml_idcs < 0) rcode = n_nrml_idcs; /* error */
        }
      } else { /* unsupported field */
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
      rcode = MESH_MODEL_ERR;
    } else {
      n_vtcs = max_vidx+1;
    }
    if (rcode == 0 && n_nrmls > 0) {
      if (n_nrml_idcs > 0) { /* convert indexed to direct */
        fprintf(stderr,"n_nrmls = %i, n_nrml_idcs = %i, max_vidx = %i, "
                "n_faces = %i, p/v = %i\n",
                n_nrmls,n_nrml_idcs,max_vidx,n_faces,nrml_per_vertex);
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
        fprintf(stderr,"n_nrmls = %i, max_vidx = %i, n_faces = %i, p/v = %i\n",
                n_nrmls,max_vidx,n_faces,nrml_per_vertex);
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
    tmesh->builtin_normals = (n_nrmls > 0);
    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
  } else {
    free(vtcs);
    free(faces);
  }
  free(normals);
  free(nrml_idcs);
  return rcode;
}

/* Reads all IndexedFaceSet meshes from the '*data' stream in VRML format. All
 * of them should be triangular meshes. Any transformations are ignored. If
 * succesful the number of read meshes is returned. Otherwise the negative
 * error code (MESH_CORRUPTED, MESH_NOT_TRIAG, MESH_NO_MEM, etc.) is
 * returned. If no error occurs the read meshes are returned in the new
 * '*tmeshes_ref' array (allocated via malloc). Otherwise '*tmeshes_ref' is
 * not modified. If 'concat' is non-zero all meshes are concatenated into one,
 * and only one is returned. */
static int read_vrml_tmesh(struct model **tmeshes_ref, FILE *data,
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

/* Reads 'n_faces' triangular faces from the '*data' stream in raw ascii
 * format and stores them in the 'faces' array. The face's vertex indices are
 * checked for consistency with the number of vertices 'n_vtcs'. Zero is
 * returned on success, or the negative error code otherwise. */
static int read_raw_faces(face_t *faces, FILE *data, int n_faces, int n_vtcs)
{
  int i;

  for (i=0; i<n_faces; i++) {
    if (fscanf(data,"%i %i %i",&(faces[i].f0),
               &(faces[i].f1),&(faces[i].f2)) != 3) {
      return MESH_CORRUPTED;
    }
    if (faces[i].f0 < 0 || faces[i].f0 >= n_vtcs ||
        faces[i].f1 < 0 || faces[i].f1 >= n_vtcs ||
        faces[i].f2 < 0 || faces[i].f2 >= n_vtcs) {
      return MESH_MODEL_ERR;
    }
  }
  return 0;
}

/* Reads 'n_vtcs' vertex points from the '*data' stream in raw ascii format
 * and stores them in the 'vtcs' array. Zero is returned on success, or the
 * negative error code otherwise. If no error occurs the bounding box minium
 * and maximum are returned in 'bbox_min' and 'bbox_max'. */
static int read_raw_vertices(vertex_t *vtcs, FILE *data, int n_vtcs,
                             vertex_t *bbox_min, vertex_t *bbox_max)
{
  int i;
  vertex_t bbmin,bbmax;

  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
  for (i=0; i<n_vtcs; i++) {
    if (fscanf(data,"%f %f %f",&(vtcs[i].x),&(vtcs[i].y),&(vtcs[i].z)) != 3) {
      return MESH_CORRUPTED;
    }
    if (vtcs[i].x < bbmin.x) bbmin.x = vtcs[i].x;
    if (vtcs[i].x > bbmax.x) bbmax.x = vtcs[i].x;
    if (vtcs[i].y < bbmin.y) bbmin.y = vtcs[i].y;
    if (vtcs[i].y > bbmax.y) bbmax.y = vtcs[i].y;
    if (vtcs[i].z < bbmin.z) bbmin.z = vtcs[i].z;
    if (vtcs[i].z > bbmax.z) bbmax.z = vtcs[i].z;
  }
  if (n_vtcs == 0) {
    memset(&bbmin,0,sizeof(bbmin));
    memset(&bbmax,0,sizeof(bbmax));
  }
  *bbox_min = bbmin;
  *bbox_max = bbmax;
  return 0;
}

/* Reads 'n' normal vectors from the '*data' stream in raw ascii format and
 * stores them in the 'nrmls' array. Zero is returned on success, or the
 * negative error code otherwise. */
static int read_raw_normals(vertex_t *nrmls, FILE *data, int n)
{
  int i;
  for (i=0; i<n; i++) {
    if (fscanf(data,"%f %f %f",&(nrmls[i].x),
               &(nrmls[i].y),&(nrmls[i].z)) != 3) {
      return MESH_CORRUPTED;
    }
  }
  return 0;
}

/* Reads the 3D triangular mesh model in raw ascii format from the '*data'
 * stream. The model mesh is returned in the new '*tmesh_ref' array (allocated
 * via malloc). It returns the number of meshes read (always one), if
 * succesful, or the negative error code (MESH_CORRUPTED, MESH_NO_MEM, etc.) 
 * otherwise. If an error occurs 'tmesh_ref' is not modified. */
static int read_raw_tmesh(struct model **tmesh_ref, FILE *data)
{
  int n_vtcs,n_faces,n_vnorms,n_fnorms;
  int n;
  char line_buf[256];
  struct model *tmesh;
  int rcode;

  rcode = 0;
  /* Read header line */
  if (fgets(line_buf,sizeof(line_buf),data) != line_buf) {
    return MESH_CORRUPTED;
  }
  n = sscanf(line_buf,"%i %i %i %i",&n_vtcs,&n_faces,&n_vnorms,&n_fnorms);
  if (n < 2 || n > 4) {
    return MESH_CORRUPTED;
  }
  if (n_vtcs < 3 || n_faces <= 0) return MESH_CORRUPTED;
  if (n > 2 && n_vnorms != n_vtcs) return MESH_CORRUPTED;
  if (n > 3 && n_fnorms != n_faces) return MESH_CORRUPTED;
  if (n <= 3) n_fnorms = 0;
  if (n <= 2) n_vnorms = 0;
  /* Allocate space and initialize mesh */
  tmesh = calloc(1,sizeof(*tmesh));
  if (tmesh == NULL) return MESH_NO_MEM;
  tmesh->num_faces = n_faces;
  tmesh->num_vert = n_vtcs;
  tmesh->vertices = malloc(sizeof(*(tmesh->vertices))*n_vtcs);
  tmesh->faces = malloc(sizeof(*(tmesh->faces))*n_faces);
  if (n_vnorms > 0) {
    tmesh->normals = malloc(sizeof(*(tmesh->normals))*n_vnorms);
  }
  if (n_fnorms > 0) {
    tmesh->face_normals = malloc(sizeof(*(tmesh->face_normals))*n_fnorms);
  }
  if (tmesh->vertices == NULL || tmesh->faces == NULL ||
      (n_vnorms > 0 && tmesh->normals == NULL) ||
      (n_fnorms > 0 && tmesh->face_normals == NULL)) {
    rcode = MESH_NO_MEM;
  }
  if (rcode == 0) {
    rcode = read_raw_vertices(tmesh->vertices,data,n_vtcs,
                              &(tmesh->bBox[0]),&(tmesh->bBox[1]));
  }
  if (rcode == 0) {
    rcode = read_raw_faces(tmesh->faces,data,n_faces,n_vtcs);
  }
  if (rcode == 0 && n_vnorms > 0) {
    rcode = read_raw_normals(tmesh->normals,data,n_vnorms);
  }
  if (rcode == 0 && n_fnorms > 0) {
    rcode = read_raw_normals(tmesh->face_normals,data,n_fnorms);
  }
  if (rcode < 0) {
    free(tmesh->vertices);
    free(tmesh->faces);
    free(tmesh->normals);
    free(tmesh->face_normals);
    free(tmesh);
  } else {
    *tmesh_ref = tmesh;
    rcode = 1;
  }
  return rcode;
}

/* Detect the file format of the '*data' stream, returning the detected
 * type. The header identifying the file format, if any, is stripped out and
 * the '*data' stream is positioned just after it. If an I/O error occurs
 * MESH_CORRUPTED is returned. If the file format can not be detected
 * MESH_BAD_FF is returned. The detected file formats are: MESH_FF_RAW,
 * MESH_FF_VRML, MESH_FF_IV, MESH_FF_PLY. */
static int detect_file_format(FILE *data)
{
  char stmp[MAX_WORD_LEN+1];
  const char swfmt[] = "%" STRING(MAX_WORD_LEN) "s";
  const char svfmt[] = "%" STRING(MAX_WORD_LEN) "[0-9V.]";
  int c;
  int rcode;
  char *eptr;
  double ver;

  c = fgetc(data);
  if (c == '#') { /* Probably VRML or Inventor */
    if (fscanf(data,swfmt,stmp) == 1) {
      if (strcmp(stmp,"VRML") == 0) {
        if (fgetc(data) == ' ' && fscanf(data,swfmt,stmp) == 1 &&
            strcmp(stmp,"V2.0") == 0 && fgetc(data) == ' ' &&
            fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"utf8") == 0 &&
            ((c = fgetc(data)) == '\n' || c == '\r' || c == ' ' || c == '\t')) {
          while (c != EOF && c != '\n' && c != '\r') { /* skip rest of header */
            c = fgetc(data);
          }
          rcode = (c != EOF) ? MESH_FF_VRML : MESH_CORRUPTED;
        } else {
          rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
        }
      } else if (strcmp(stmp,"Inventor") == 0) {
        if (fgetc(data) == ' ' && fscanf(data,svfmt,stmp) == 1 &&
            stmp[0] == 'V' && (ver = strtod(stmp+1,&eptr)) >= 2 &&
            ver < 3 && *eptr == '\0' && fgetc(data) == ' ' &&
            fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"ascii") == 0 &&
            ((c = fgetc(data)) == '\n' || c == '\r' || c == ' ' || c == '\t')) {
          while (c != EOF && c != '\n' && c != '\r') { /* skip rest of header */
            c = fgetc(data);
          }
          rcode = (c != EOF) ? MESH_FF_IV : MESH_CORRUPTED;
        } else {
          rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
        }
      } else {
        rcode = MESH_BAD_FF;
      }
    } else {
      rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
    }
  } else if (c == 'p') { /* Probably ply */
    c = ungetc(c,data);
    if (c != EOF) {
      if (fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"ply") == 0) {
        if (fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"format") == 0 &&
            fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"ascii") == 0 &&
            fscanf(data,swfmt,stmp) == 1 && strcmp(stmp,"1.0") == 0 &&
            ((c = fgetc(data)) == '\n' || fgetc(data) == '\r')) {
          rcode = MESH_FF_PLY;
        } else {
          rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
        }
      } else {
        rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
      }
    } else {
      rcode = MESH_CORRUPTED;
    }
  } else if (c >= '0' && c <= '9') { /* probably raw */
    c = ungetc(c,data);
    rcode = (c != EOF) ? MESH_FF_RAW : MESH_CORRUPTED;
  } else { /* error */
    rcode = ferror(data) ? MESH_CORRUPTED : MESH_BAD_FF;
  }
  return rcode;
}

/* see model_in.h */
int read_model(struct model **models_ref, FILE *data, int fformat, int concat)
{
  int rcode;
  struct model *models;

  if (fformat == MESH_FF_AUTO) {
    fformat = detect_file_format(data);
    if (fformat < 0) return fformat; /* error or unrecognized file format */
  }
  rcode = 0;
  models = NULL;
  switch (fformat) {
  case MESH_FF_RAW:
    rcode = read_raw_tmesh(&models,data);
    break;
  case MESH_FF_VRML:
    rcode = read_vrml_tmesh(&models,data,concat);
    break;
  default:
    rcode = MESH_BAD_FF;
  }
  if (rcode >= 0) {
    *models_ref = models;
  }
  return rcode;
}

/* see model_in.h */
int read_fmodel(struct model **models_ref, const char *fname,
                int fformat, int concat)
{
  FILE *f;
  int rcode;

  f = fopen(fname,"rb");
  if (f == NULL) return MESH_BAD_FNAME;
  rcode = read_model(models_ref,f,fformat,concat);
  fclose(f);
  return rcode;
}

/* $Id: model_in_ply.c,v 1.4 2002/08/22 17:26:34 aspert Exp $ */

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

#include <model_in.h>


/* Read 'n_faces' faces of the model from 'data'. 
 * Returns 0 if successful and a negative value in case of trouble. 
 * Non-triangular faces are NOT supported. Moreover, the face's vertex
 * indices have to be consistent with the number of vertices in the
 * model 'n_vtcs'. */
static int read_ply_faces(face_t *faces, struct file_data *data,
                          int n_faces, int n_vtcs)
{
  int i, c, f0, f1, f2;

  for(i=0; i<n_faces; i++) {
    
    if (int_scanf(data, &c) != 1) 
      return MESH_CORRUPTED;
    
    if (c != 3)  /* Non-triangular mesh -> bail out */
      return  MESH_NOT_TRIAG;
    
        
    /* Read faces */
    if (int_scanf(data, &f0) != 1) 
      return MESH_CORRUPTED;

    if (int_scanf(data, &f1) != 1) 
      return MESH_CORRUPTED;

    if (int_scanf(data, &f2) != 1) 
      return MESH_CORRUPTED;

    /* Check if indices are consistent ...*/
    if (f0 < 0 || f1 < 0 || f2 < 0 || 
        f0 >= n_vtcs || f1 >= n_vtcs || f2 >= n_vtcs ) 
      return MESH_MODEL_ERR;


    faces[i].f0 = f0;
    faces[i].f1 = f1;
    faces[i].f2 = f2;
  } /* end face loop */

  return 0;
}

/* Reads 'n_vtcs' vertex points from the '*data' stream in raw ascii format
 * and stores them in the 'vtcs' array. Zero is returned on success, or the
 * negative error code otherwise. If no error occurs the bounding box minium
 * and maximum are returned in 'bbox_min' and 'bbox_max'. */
static int read_ply_vertices(vertex_t *vtcs, struct file_data *data, 
                             int n_vtcs, 
                             vertex_t *bbox_min, vertex_t *bbox_max)
{
  vertex_t bbmin, bbmax;
  int i, c;

  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;

  for (i=0; i< n_vtcs; i++) {
    if (float_scanf(data, &(vtcs[i].x)) != 1) 
      return MESH_CORRUPTED;
    if (float_scanf(data, &(vtcs[i].y)) != 1) 
      return MESH_CORRUPTED;
    if (float_scanf(data, &(vtcs[i].z)) != 1) 
      return MESH_CORRUPTED;
    
    /* skip the rest of the current line, which can contain
     * additional informations (e.g. color), but currently ignored
     */
    do {
      c = getc(data);
    } while (c != '\n' && c != '\r' && c != EOF);
    
    if (c == EOF) 
      return MESH_CORRUPTED;
    
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
  return 0;
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
  int rcode = 1;
  int is_bin = 0;
  int endianness = 0;
  int c;
  
  tmesh = (struct model*)calloc(1, sizeof(struct model));

  /* read the header */
  /* check if the format is ASCII */
  skip_ws_comm(data);
  if (string_scanf(data, stmp) == 1 && strcmp(stmp, "format") == 0) {
    skip_ws_comm(data);
    if (string_scanf(data, stmp) == 1) { 
      /* check whether we have ASCII or binary stuff */
      if (strcmp(stmp, "binary_little_endian") == 0) {
        is_bin = 1;
      } else if (strcmp(stmp, "binary_big_endian") == 0) {
        is_bin = 1;
        endianness = 1;
      } else if (strcmp(stmp, "ascii") != 0) {
        rcode = MESH_CORRUPTED;
      } else {
        skip_ws_comm(data);
        if (string_scanf(data, stmp) != 1 || strcmp(stmp, "1.0") != 0) 
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
        skip_ws_comm(data);
        if (string_scanf(data, stmp) == 1) {
          if (strcmp(stmp, "vertex") == 0) {
            if ((c = int_scanf(data, &(tmesh->num_vert))) != 1)
              rcode = MESH_CORRUPTED;
#ifdef DEBUG
            fprintf(stderr, "[read_ply_tmesh] num_vert = %d\n", 
                    tmesh->num_vert);
#endif
          } else if (strcmp(stmp, "face") == 0) {
            if ((c = int_scanf(data, &(tmesh->num_faces))) != 1)
              rcode = MESH_CORRUPTED;
#ifdef DEBUG
            fprintf(stderr, "[read_ply_tmesh] num_faces = %d\n", 
                    tmesh->num_faces);
#endif
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
          rcode = read_ply_vertices(tmesh->vertices, data, tmesh->num_vert, 
                                    &bbmin, &bbmax);
          
          /* Read face list */
          rcode = read_ply_faces(tmesh->faces, data, 
                                 tmesh->num_faces, tmesh->num_vert);
        } else { /* binary format */
          rcode = MESH_BAD_FF;
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
    rcode = 0;
  }

  return rcode;
}

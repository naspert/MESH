/* $Id: model_in_raw.c,v 1.3 2002/08/30 09:18:46 aspert Exp $ */


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
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */




#include <model_in.h>

/* Reads 'n_faces' triangular faces from the '*data' stream in raw ascii
 * format and stores them in the 'faces' array. The face's vertex indices are
 * checked for consistency with the number of vertices 'n_vtcs'. Zero is
 * returned on success, or the negative error code otherwise. */
static int read_raw_faces(face_t *faces, struct file_data *data, 
                          int n_faces, int n_vtcs)
{
  int i;
  
  for (i=0; i<n_faces; i++) {
    if (int_scanf(data, &(faces[i].f0)) != 1)
      return MESH_CORRUPTED;
    if (int_scanf(data, &(faces[i].f1)) != 1)
      return MESH_CORRUPTED;
    if (int_scanf(data, &(faces[i].f2)) != 1)
      return MESH_CORRUPTED;

#ifdef DEBUG
    printf("i=%d f0=%d f1=%d f2=%d\n", i, faces[i].f0, faces[i].f1, faces[i].f2);
#endif
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
static int read_raw_vertices(vertex_t *vtcs, struct file_data *data, 
                             int n_vtcs,
                             vertex_t *bbox_min, vertex_t *bbox_max)
{
  int i;
  vertex_t bbmin,bbmax;

  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
  for (i=0; i<n_vtcs; i++) {

    if (float_scanf(data, &(vtcs[i].x)) != 1)
      return MESH_CORRUPTED;
    if (float_scanf(data, &(vtcs[i].y)) != 1)
      return MESH_CORRUPTED;
    if (float_scanf(data, &(vtcs[i].z)) != 1)
      return MESH_CORRUPTED;

#ifdef DEBUG    
    printf("i=%d x=%f y=%f z=%f\n", i, vtcs[i].x, vtcs[i].y, vtcs[i].z);
#endif
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
static int read_raw_normals(vertex_t *nrmls, struct file_data *data, int n)
{
  int i;
  for (i=0; i<n; i++) {
    if (float_scanf(data,  &(nrmls[i].x)) != 1)
      return MESH_CORRUPTED;
    if (float_scanf(data,  &(nrmls[i].y)) != 1)
      return MESH_CORRUPTED;
    if (float_scanf(data,  &(nrmls[i].z)) != 1)
      return MESH_CORRUPTED;

  
  }
  return 0;
}

/* Reads the 3D triangular mesh model in raw ascii format from the '*data'
 * stream. The model mesh is returned in the new '*tmesh_ref' array (allocated
 * via malloc). It returns the number of meshes read (always one), if
 * succesful, or the negative error code (MESH_CORRUPTED, MESH_NO_MEM, etc.) 
 * otherwise. If an error occurs 'tmesh_ref' is not modified. */
int read_raw_tmesh(struct model **tmesh_ref, struct file_data *data)
{
  int n_vtcs,n_faces,n_vnorms,n_fnorms;
  int n, i;
  char line_buf[256];
  int tmp;
  struct model *tmesh;
  int rcode;

  rcode = 0;
  /* Read 1st line */
  i=0;
  do {
    tmp = getc(data);
    line_buf[i] = (char)tmp;
    i++;
  } while (i < (int)sizeof(line_buf) && tmp != '\r' && tmp != '\n' && 
	   tmp != EOF);

  if (tmp == EOF || i == sizeof(line_buf))
    return MESH_CORRUPTED;

  if (tmp != EOF)
    ungetc(tmp, data);

  line_buf[--i]='\0';

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

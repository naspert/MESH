/* $Id: model_in_off.c,v 1.7 2004/10/25 11:37:41 aspert Exp $ */
/*
 *
 *  Copyright (C) 2004 EPFL (Swiss Federal Institute of Technology,
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

/* Adapted from  John Burkardt's IVCON code by P. Ilodibia and N. Aspert .
 * Provides limited OFF format reading. As usual, only triangular faces
 * are supported. Additional info such as color, normals is ignored. 
 * */


#include <model_in.h>
#ifdef DEBUG
# include <debug_print.h>
#endif

#define LINE_BUF_SIZE 256
      

static int read_off_vertices(vertex_t *vtcs, struct file_data *data, 
			     int n_vtcs, 
			     vertex_t *bbox_min, vertex_t *bbox_max)
{
  int i, k, tmp;
  vertex_t bbmin, bbmax;
  char line_buf[LINE_BUF_SIZE];
  bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
  bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;

  skip_ws_comm(data);

  for (i=0; i<n_vtcs; i++) {
    /* read one line of data */
    k=0;
    do {
      tmp = getc(data);
      line_buf[k++] = tmp;
    } while (tmp != '\n' && tmp != '\r' && tmp != EOF && k < LINE_BUF_SIZE);
    
    if (tmp == EOF || (k == LINE_BUF_SIZE && tmp != '\n' && tmp != '\r')) 
      return MESH_CORRUPTED;
    line_buf[--k] = '\0';
    if (sscanf(line_buf, "%f %f %f", 
	       &(vtcs[i].x), &(vtcs[i].y), &(vtcs[i].z)) != 3)
      return MESH_CORRUPTED;
    
    skip_ws_comm(data);
#ifdef DEBUG
    DEBUG_PRINT("i=%d x=%f y=%f z=%f\n", i, vtcs[i].x, vtcs[i].y, vtcs[i].z);
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

static int read_off_faces(face_t *faces, struct file_data *data, int n_faces,
			  int n_vtcs)
{
  int order, i, k;
  char line_buf[LINE_BUF_SIZE];
  int tmp;
  for (i=0; i<n_faces; i++) {
    /* read one line of data */
    k=0;
    do {
      tmp = getc(data);
      line_buf[k++] = tmp;
    } while (tmp != '\n' && tmp != '\r' && 
	     tmp != EOF && k < LINE_BUF_SIZE);
    
    if ((tmp == EOF && i < n_faces - 1) || 
	(k == LINE_BUF_SIZE && tmp != '\n' && tmp != '\r')) 
      return MESH_CORRUPTED;
    line_buf[--k] = '\0';
    if (sscanf(line_buf, "%d %d %d %d", &order, 
	       &(faces[i].f0), &(faces[i].f1), &(faces[i].f2)) != 4)
      return MESH_CORRUPTED;
    if (order != 3)
      return MESH_NOT_TRIAG;
    
    skip_ws_comm(data);
#ifdef DEBUG
    DEBUG_PRINT("i=%d f0=%d f1=%d f2=%d\n", i, faces[i].f0,
                faces[i].f1, faces[i].f2);
#endif

    if (faces[i].f0 < 0 || faces[i].f0 >= n_vtcs ||
        faces[i].f1 < 0 || faces[i].f1 >= n_vtcs ||
        faces[i].f2 < 0 || faces[i].f2 >= n_vtcs) 
      return MESH_MODEL_ERR;
  }
  return 0;
}

int read_off_tmesh(struct model **tmesh_ref,struct file_data *data)
{
 
  int vert_num;
  int edge_num;
  int face_num;
  char line[LINE_BUF_SIZE];
  int header_found = 0;
  int rcode = MESH_CORRUPTED;
  int tmp;
  unsigned int i;
  struct model *tmesh;
  
  if(data == NULL)
    return MESH_CORRUPTED;
  
  do {
    tmp = skip_ws_comm(data);
    i = 0;
    if (tmp == EOF)
      return MESH_CORRUPTED;
    do {/* buffer one non-comment line */
      tmp = getc(data);
      line[i] = (char)tmp;
      i++;
    } while (i < LINE_BUF_SIZE && tmp != '\n' && tmp != '\r' 
	     && tmp != EOF);
    
    if (tmp == EOF || i == LINE_BUF_SIZE)
      return MESH_CORRUPTED;
    
    line[--i] = '\0';
    
    if (header_found == 0) {/* header must be the 1st non-comment line */
      if (strstr(line, "OFF") == NULL) 
	return MESH_CORRUPTED;
      else 
	header_found++;/* read next line */
    }
  } while (tmp != EOF && header_found == 0);
  if (header_found == 0 || tmp == EOF) 
    return MESH_CORRUPTED;
  
  skip_ws_comm(data);
  /* read the number of verts, faces and edges */
  if (int_scanf(data, &vert_num) != 1)
    return MESH_CORRUPTED;
  if (int_scanf(data, &face_num) != 1)
    return MESH_CORRUPTED;
  if (int_scanf(data, &edge_num) != 1)
    return MESH_CORRUPTED;
  if (vert_num < 3 || face_num <= 0)
    return MESH_CORRUPTED;
  /* otherwise alloc needed buffers/structs */
  tmesh = calloc(1,sizeof(*tmesh));
  if(tmesh == NULL) return MESH_NO_MEM;
  tmesh->num_vert = vert_num;
  tmesh->vertices = malloc(sizeof(vertex_t)*tmesh->num_vert);
  tmesh->num_faces = face_num;
  tmesh->faces = malloc(sizeof(face_t)*tmesh->num_faces);
  if (tmesh->faces == NULL || tmesh->vertices == NULL)
    return MESH_NO_MEM;
  rcode = read_off_vertices(tmesh->vertices, data, tmesh->num_vert,
			    &(tmesh->bBox[0]), &(tmesh->bBox[1]));
  if (rcode != 0)
    goto err_read_off_tmesh;
  rcode = read_off_faces(tmesh->faces, data, 
			 tmesh->num_faces, tmesh->num_vert);
  if (rcode != 0)
    goto err_read_off_tmesh;
  
  *tmesh_ref = tmesh;
  return 1;
  
 err_read_off_tmesh:
  free(tmesh->vertices);
  free(tmesh->faces);
  free(tmesh);
  return rcode;
}
 

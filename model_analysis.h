/* $Id: model_analysis.h,v 1.13 2002/08/30 07:56:01 aspert Exp $ */


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
 *   pp. 705-708, available on http://mesh.epfl.ch
 *
 */





#ifndef _MODEL_ANALYSIS_PROTO
#define _MODEL_ANALYSIS_PROTO

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*
 */


#include <3dmodel.h>
#include <reporting.h>

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* --------------------------------------------------------------------------*
 *                       Exported data types                                 *
 * --------------------------------------------------------------------------*/

/* A list of model faces */
struct face_list {
  int *face;   /* Array of indices of the faces in the list */
  int n_faces; /* Number of faces in the array */
};

/* Model analysis information */
struct model_info {
  int orientable;       /* The model is orientable, even if not currently
                         * oriented. */
  int oriented;         /* Model is currently oriented. */
  int orig_oriented;    /* Model is originally oriented. */
  int manifold;         /* Model is manifold. */
  int closed;           /* Model is closed (i.e. defines a volume) */
  int n_degenerate;     /* The number of degenerate faces */
  int n_disjoint_parts; /* The number of disjoint (i.e. not connected) parts
                         * in the model. If there are more than one, the other
                         * fields refer to the union of all parts (i.e. if the
                         * model is manifold (or oriented, closed, etc.) all
                         * its parts are manifold, but if the model is
                         * non-manifold some parts might still be
                         * manifold). */
};

/* --------------------------------------------------------------------------*
 *                       Exported functions                                  *
 * --------------------------------------------------------------------------*/

/* Analyzes model m, returning the information in *info. Degenerate faces in m
 * are ignored in the analysis. If do_orient is non-zero and the model is
 * orientable, the model m will be modified so as to be oriented. If the model
 * is not orientable, the model will be oriented as much as possible if
 * do_orient is 2 or more. If verbose is non-zero any problems with the model
 * are reported to out, preceded by the model name name. */
void analyze_model(struct model *m, struct model_info *info, int do_orient,
                   int verbose, struct outbuf *out, const char *name);

/* Returns an array of length m->num_vert with the list of faces incident on
 * each vertex. The number of degenerate faces is returned in
 * *n_degenerate. Degenerate faces are ignored (i.e. not included as incident
 * on any vertex). */
struct face_list *faces_of_vertex(const struct model *m, int *n_degenerate);

/* Frees the storage for the array of face lists fl, of length n */
void free_face_lists(struct face_list *fl, int n);

END_DECL
#undef END_DECL

#endif /* _MODEL_ANALYSIS_PROTO */

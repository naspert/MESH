/* $Id: Error3DViewerWidget.h,v 1.2 2004/04/30 07:50:20 aspert Exp $ */

/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
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


#ifndef ERROR3DVIEWERWIDGET_H
#define ERROR3DVIEWERWIDGET_H

#include <Basic3DViewerWidget.h>
#include <ColorMapWidget.h>
#include <compute_error.h>

// This class extends Basic3DViewerWidget, adding possibility to
// display error on the surface using 
// - per vertex error (possibly adding vertices s.t. there is 1
// vert./sample)
// - per face error (use the mean of the error over the face for
// color)
// - per sample error (using textures... may cause trouble for some
// OpenGL implementations)
class Error3DViewerWidget : public Basic3DViewerWidget
{
  Q_OBJECT

public:
  Error3DViewerWidget(struct model_error *model, bool texture_enabled,
		      QWidget *parent=0,
		      const char *name=0);
  ~Error3DViewerWidget();

  // possible values for 'error_mode'
  enum ErrorMode {VERTEX_ERROR = 0, MEAN_FACE_ERROR = 1, SAMPLE_ERROR = 2};  

public slots:
  void setErrorMode(int emode);
  void setVEDownSampling(int n);
  void setColorMap(int newSpace);

protected:
  void initializeGL();
  void rebuildList();
  

private:
  // functions 
  void genErrorTextures();
  int fillTexture(const struct face_error *fe, GLubyte *texture) const;
  static int ceilLog2(int v);
  void setGLColorForError(float error) const;
  void drawMeanFaceErrorT() const;
  void drawVertexErrorT() const;
  void drawTexSampleErrorT() const;

  

  float **colormap;
  ColorMapWidget::colorSpace csp;
  struct model_error *model;
  GLuint *etex_id; // texture IDs for per triangle sample error
  int *etex_sz;    // texture size for each of etex_id textures
  const GLfloat no_err_value; // gray value for when there is no error for
                              // primitive
  int downsampling; // downsampling factor for vertex error
  int error_mode;
  bool texture_enabled;
};


#endif

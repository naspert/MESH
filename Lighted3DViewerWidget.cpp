/* $Id: Lighted3DViewerWidget.cpp,v 1.1 2003/04/17 10:45:37 aspert Exp $ */

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

#include <Lighted3DViewerWidget.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <geomutils.h>



Lighted3DViewerWidget::Lighted3DViewerWidget(struct model_error *model_err,
					     QWidget *parent, const char *name)
  : Basic3DViewerWidget(model_err->mesh, parent, name)
{
  this->model = model_err;
  two_sided_material = 1;
  not_orientable_warned = 0;
}

// slot to toggle light
void Lighted3DViewerWidget::setLight(bool state) 
{
  GLboolean light_state;
  struct model *r_model = model->mesh;
  if (!getGLInitialized()) {
    fprintf(stderr,
            "Lighted3DViewerWidget::setLight() called"
	    " before GL context is initialized!\n");
    return;
  }

  // Get state from renderer
  makeCurrent();
  light_state = glIsEnabled(GL_LIGHTING);
  if (light_state != !state) // harmless
    printf("Mismatched state between qcbLight/light_state\n");
  if (light_state==GL_FALSE){ // We are now switching to lighted mode
    if (r_model->normals !=NULL){// Are these ones computed ?
      glEnable(GL_LIGHTING);
    } else { // Normals should have been computed
      fprintf(stderr,"ERROR: normals were not computed!\n");
    }
  }
  else if (light_state==GL_TRUE){// We are now switching to
                                   // non-lighted mode
    glDisable(GL_LIGHTING);
  }
  checkGLErrors("setLight()");
  updateGL();
}

// Slot to switch between 1-sided and 2-sided material (for open meshes)
void Lighted3DViewerWidget::setTwoSidedMaterial(bool state) 
{
  
  if (state != (bool)two_sided_material) // harmless ...
    printf("Mismatched state qcbTwoSide/two_sided_material\n");
  two_sided_material = !two_sided_material;
  if (getGLInitialized()) { // only update GL if already initialized
    makeCurrent();
    QApplication::setOverrideCursor(Qt::waitCursor);
    rebuildList();
    updateGL();
    QApplication::restoreOverrideCursor();
  }
}

// Slot to invert normals (directly in the model)
void Lighted3DViewerWidget::invertNormals(bool state) 
{
  GLboolean lightState=state;
  struct model *r_model = model->mesh;
  int i;

  if (!getGLInitialized()) {
    fprintf(stderr,"RawWidget::invertNormals() called before GL context is "
            "initialized!\n");
    return;
  }

  makeCurrent();
  QApplication::setOverrideCursor(Qt::waitCursor);
  lightState = glIsEnabled(GL_LIGHTING);
  if (lightState == GL_TRUE && r_model->normals != NULL) {
    for (i=0; i<r_model->num_vert; i++) 
      neg_v(&(r_model->normals[i]), &(r_model->normals[i]));
      
    rebuildList();
    updateGL();
  }
  QApplication::restoreOverrideCursor();
 
}

// Herited from Basic3DViewerWidget
void Lighted3DViewerWidget::initializeGL() 
{
  Basic3DViewerWidget::initializeGL();
  if (!this->model->info->closed)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  if (model->mesh->normals !=NULL){// Are these ones computed ?
      if (!this->model->info->orientable && !not_orientable_warned) {
        not_orientable_warned = 1;
        QMessageBox::warning(this,"Not orientable model",
                             "Model is not orientable.\n"
                             "Surface shading is probably incorrect.");
      }
      glEnable(GL_LIGHTING);
    } else {// Normals should have been computed
      fprintf(stderr,"ERROR: normals were not computed!\n");
    }
}

// Herited from Basic3DViewerWidget
void Lighted3DViewerWidget::rebuildList()
{
  // Surface material characteristics for lighted mode
  static const float front_amb_mat[4] = {0.11f, 0.06f, 0.11f, 1.0f};
  static const float front_diff_mat[4] = {0.43f, 0.47f, 0.54f, 1.0f};
  static const float front_spec_mat[4] = {0.33f, 0.33f, 0.52f, 1.0f};
  static const float front_mat_shin = 10.0f;
  static const float back_amb_mat[4] = {0.3f, 0.3f, 0.3f, 1.0f};
  static const float back_diff_mat[4] = {0.5f, 0.5f, 0.5f, 1.0f};
  static const float back_spec_mat[4] = {0.33f, 0.33f, 0.52f, 1.0f};
  static const float back_mat_shin = 10.0f;

  if (two_sided_material) {
    glMaterialfv(GL_FRONT,GL_AMBIENT,front_amb_mat);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,front_diff_mat);
    glMaterialfv(GL_FRONT,GL_SPECULAR,front_spec_mat);
    glMaterialf(GL_FRONT,GL_SHININESS,front_mat_shin);
    glMaterialfv(GL_BACK,GL_AMBIENT,back_amb_mat);
    glMaterialfv(GL_BACK,GL_DIFFUSE,back_diff_mat);
    glMaterialfv(GL_BACK,GL_SPECULAR,back_spec_mat);
    glMaterialf(GL_BACK,GL_SHININESS,back_mat_shin);
  } else {
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,front_amb_mat);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,front_diff_mat);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,front_spec_mat);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,front_mat_shin);
  }
  Basic3DViewerWidget::rebuildList();

}

// Herited from Basic3DViewerWidget
void Lighted3DViewerWidget::keyPressEvent(QKeyEvent *k) 
{
  Basic3DViewerWidget::keyPressEvent(k);
  switch (k->key()) {
  case Key_F2:
    emit toggleLight();
    break;
  case Key_F4:
    emit toggleNormals();
    break;
  case Key_F5:
    emit toggleTwoSidedMaterial();
    break;
  default:
    break;
  }
}

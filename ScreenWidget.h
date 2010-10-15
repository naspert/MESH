/* $Id$ */


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







#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

/* QT includes */
#include <qwidget.h>
#include <q3groupbox.h>
//#include <qvgroupbox.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include "compute_error.h"


class ScreenWidget : public QWidget {
  Q_OBJECT
public:
  ScreenWidget(struct model_error *model1, struct model_error *model2,
               const struct args *pargs, QWidget *parent=0, 
               const char *name=0);

signals:
  void dsValChange(int n); // wired to Error3DViewerWidget::setVEDownSampling

protected slots:
  void quit();
  void infoLeftModel();
  void infoRightModel();
  void disableSlider(int errMode);
  void disableSync(bool state);
  void trapChanges(int n);
  void aboutKeys();
  void aboutBugs();
  void aboutMesh();
  void updatecbStatus(bool state);
  

private:
  Q3GroupBox *qgbSlider;
  QSlider *qslidDispSampDensity;
  QSpinBox *qspSampDensity, *qspTimerSpeed;
  QCheckBox *qcbInvNorm, *qcbTwoSide, *qcbTimer;
  QPushButton *syncBut;

  // local copies of the parameters passed to the constructor
  struct model_error *locMod1, *locMod2; 

  enum whichModel {LEFT_MODEL=0, RIGHT_MODEL=1}; // values of 'id'
                                                 // parameter in 'infoModel'
  void infoModel(struct model_error *model, int id);
  void changeGroupBoxTitle(int n);
};


#endif

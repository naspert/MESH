/* $Id: InitWidget.h,v 1.24 2003/01/13 12:46:05 aspert Exp $ */


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







#ifndef INITW_H
#define INITW_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <mesh_run.h>
#include <ScreenWidget.h>
#include <TextWidget.h>

class InitWidget : public QWidget {
  Q_OBJECT

public:
  InitWidget(struct args defArgs,
             struct model_error *m1, struct model_error *m2,
             QWidget *parent=0, const char *name=0 );
  ~InitWidget();

signals:
void signalrunDone();

private slots:
  void loadMesh1();
  void loadMesh2();
  void getParameters();
  void incompleteFields();
  void meshRun();

private:
  void meshSetUp();
  QLineEdit *qledMesh1, *qledMesh2, *qledSplStep, *qledMinSplFreq;
  QCheckBox *chkSymDist, *chkLogWindow, *chkVerbNMV, *chkTexture;
  struct args pargs;
  struct model_error *model1,*model2;
  ScreenWidget *c;
  TextWidget *textOut;
  struct outbuf *log;
  QPixmap *qpxMeshIcon;
};

extern "C" {
  void QT_prog(void *out, int p);
}

#endif

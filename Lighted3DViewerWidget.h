/* $Id: Lighted3DViewerWidget.h,v 1.1 2003/04/17 10:45:37 aspert Exp $ */

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


#ifndef LIGHTED3DVIEWERWIDGET_H
#define LIGHTED3DVIEWERWIDGET_H

#include <Basic3DViewerWidget.h>
#include <compute_error.h>


// This class adds lighting capabilities to Basic3DViewerWidget (and
// is used for the right GL widget in ScreenWidget)
class Lighted3DViewerWidget : public Basic3DViewerWidget
{
  Q_OBJECT

public:
  Lighted3DViewerWidget(struct model_error *model, 
			QWidget *parent=0, const char *name=0); // constructor
  ~Lighted3DViewerWidget() { }; // Destructor
  
public slots:
  void setTwoSidedMaterial(bool state);
  void setLight(bool state);
  void invertNormals(bool state);

signals:
  void toggleLight(); // wired to setLight
  void toggleTwoSidedMaterial(); // wired to setTwoSidedMaterial
  void toggleNormals(); // wired to invertNormals

protected:
  void initializeGL();
  void keyPressEvent(QKeyEvent*);
  void rebuildList();

private:  
  struct model_error *model;
  int not_orientable_warned;
  int two_sided_material;
};

#endif

/* $Id: ColorMapWidget.h,v 1.17 2002/05/08 12:05:26 aspert Exp $ */


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
 *   Submitted to ICME 2002, available on http://mesh.epfl.ch
 *
 */

#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>
#include <compute_error.h>

/* Colorbar constants */
#ifndef CBAR_WIDTH
# define CBAR_WIDTH 25
#endif
#ifndef CBAR_STEP
# define CBAR_STEP 2
#endif
#ifndef N_LABELS
# define N_LABELS 9
#endif


class ColorMapWidget : public QWidget
{
Q_OBJECT
public:
  ColorMapWidget(const struct model_error *model1_error,
		 QWidget *parent=0, const char *name=0 );
 ~ColorMapWidget();
 QSize sizeHint() const;
 QSize minimumSizeHint() const;
 enum scaleMode {LIN_SCALE=0, LOG_SCALE=1};
 enum colorSpace {GRAYSCALE, HSV};

public slots:
  void doHistogram(int scaleType);
  void setColorMap(int newSpace); 

protected:
  void paintEvent(QPaintEvent *); 

 private:
  const struct model_error *me;
  int *histogram;
  float **colormap;
  int cmap_len;
  double dmax, dmin;
  scaleMode scaleState; // stores the state of the scale (LIN or LOG)
  colorSpace colorState; // stores the state of the color space used
                         // (GRAYSCALE or HSV)
};

#endif

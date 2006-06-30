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







#include <ColorMapWidget.h>
#include <qapplication.h>
#include <qpainter.h>
#include <colormap.h>
#include <qfont.h>
#include <math.h>

/* Constructor of the ColorMapWidget class */
/* Only initializes a few values and build a HSV colormap */
ColorMapWidget::ColorMapWidget(const struct model_error *model1_error,
			       QWidget *parent, 
			       const char *name):QWidget(parent,name) {

  setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding));
  setBackgroundColor(Qt::black);

  me = model1_error;
  colormap = NULL;
  histogram = NULL;
  scaleState = LIN_SCALE;
  colorState = HSV;
  dmax = me->max_error;
  dmin = me->min_error;
  cmap_len = -1;
}

QSize ColorMapWidget::sizeHint() const {
  return QSize(75,512);
}

QSize ColorMapWidget::minimumSizeHint() const {
  return QSize(75,256);
}

ColorMapWidget::~ColorMapWidget() {
  delete [] histogram;
  free_colormap(colormap);
}

void ColorMapWidget::doHistogram(int scaleType) {
  double drange,off;
  double *serror;
  int i,bin_idx,max_cnt,n;
  int len = cmap_len/CBAR_STEP;

  // This is a potentially slow operation
  QApplication::setOverrideCursor(Qt::waitCursor);

  delete [] histogram;
  histogram = new int[len];
  memset(histogram, 0, sizeof(*histogram)*len);

  n = me->n_samples;
  drange = me->max_error - me->min_error;
  off = me->min_error;
  serror = me->fe[0].serror;
  for (i=0; i<n; i++) {
    bin_idx = (int) ((serror[i]-off)/drange*len);
    if (bin_idx >= len) bin_idx = len-1;
    histogram[bin_idx]++;
  }

  max_cnt = 0;
  for (i=0; i<len; i++) 
    if (max_cnt < histogram[i]) max_cnt = histogram[i];

  scaleState = (scaleMode)scaleType;
  if (scaleType == LIN_SCALE) {
    for (i=0; i<len; i++)
      histogram[i] = (int)floor(histogram[i]/(double)max_cnt*CBAR_WIDTH+0.5);
  } else if (scaleType == LOG_SCALE) {
    for (i=0; i<len; i++) {
      if (histogram[i] != 0)
        histogram[i] = (int)floor(log((double)histogram[i])/
                                  log((double)max_cnt)*CBAR_WIDTH + 0.5);
    }
  } else 
    fprintf(stderr, "Invalid scale specified\n");
  
  update();

  QApplication::restoreOverrideCursor();
}

void ColorMapWidget::setColorMap(int newSpace) {

  if (colorState != newSpace) {
    colorState = (colorSpace)newSpace;
    free_colormap(colormap);

    if (colorState == HSV)
      colormap = colormap_hsv(cmap_len);
    else if (colorState == GRAYSCALE)
      colormap = colormap_gs(cmap_len);
    else 
      fprintf(stderr, "Invalid color space specified\n");
    doHistogram(scaleState);
  }
}

/* This function generates the bar graph that will be displayed aside */
/* the models. Each color is displayed with the mean error value associated. */
/* The 'QPaintEvent' parameter is not used (it seems to be needed when */
/* calling the QPainter stuff. */
void ColorMapWidget::paintEvent(QPaintEvent *) {
  QFont f(QApplication::font());
  double res;
  QPainter p;
  QString tmpDisplayedText;
  int i,h,yoff,ysub,cidx;
  int lscale;
  double scale;

  lscale = (int) floor(log10(dmax));
  scale = pow(10.0,lscale);
  f.setPointSize(9);
  p.begin(this);
  p.setFont(f);
  QFontMetrics fm(p.fontMetrics());
  yoff = fm.lineSpacing();
  ysub = fm.ascent()/2;
  tmpDisplayedText.sprintf("x 1 e %i",lscale);
  p.setPen(Qt::white);
  p.drawText(10,yoff,tmpDisplayedText);
  h = height() - 3*yoff;
  h = h/CBAR_STEP*CBAR_STEP;
  yoff *= 2;
  if (cmap_len != h) {
    free_colormap(colormap);
    cmap_len = h;
    if (colorState == HSV)
      colormap = colormap_hsv(cmap_len);
    else if (colorState == GRAYSCALE)
      colormap = colormap_gs(cmap_len);
    else 
      fprintf(stderr, "Invalid color space specified\n");

    doHistogram(scaleState);
  }
  p.drawText(40, yoff+ysub, tmpDisplayedText.sprintf( "%.3f",dmax/scale));
  for(i=0; i<N_LABELS-1; i++) {
    res = dmax - (i+1)*(dmax - dmin)/(double)(N_LABELS-1);
    p.drawText(40, yoff+ysub+(int)((i+1)*(h/(double)(N_LABELS-1))),
               tmpDisplayedText.sprintf( "%.3f",res/scale));
  }
  for(i=0; i<cmap_len; i++){
    cidx = cmap_len-1-i;
    p.setPen(QColor((int)(255*colormap[cidx][0]), (int)(255*colormap[cidx][1]),
		    (int)(255*colormap[cidx][2])));
    p.drawLine(10, yoff+i, 10+histogram[cidx/CBAR_STEP], yoff+i);
  }
  p.end();
}

/* $Id: ColorMapWidget.cpp,v 1.14 2002/02/01 09:04:24 aspert Exp $ */
#include <ColorMapWidget.h>
#include <qapplication.h>
#include <qpainter.h>
#include <ColorMap.h>
#include <qfont.h>
#include <math.h>

/* Constructor of the ColorMapWidget class */
/* Only initializes a few values and build a HSV colormap */
ColorMapWidget::ColorMapWidget(double dmoymin, double dmoymax, 
			       QWidget *parent, 
			       const char *name):QWidget(parent,name) {

  setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding));
  setBackgroundColor(Qt::black);

  colormap = HSVtoRGB();
  dmax = dmoymax;
  dmin = dmoymin;

}

QSize ColorMapWidget::sizeHint() const {
  return QSize(75,512);
}

QSize ColorMapWidget::minimumSizeHint() const {
  return QSize(75,256);
}

ColorMapWidget::~ColorMapWidget() {
  free_colormap(colormap);
}

void ColorMapWidget::rescale(double dmoymin, double dmoymax) {
  dmax = dmoymax;
  dmin = dmoymin;
  // This will call painEvent()
  update();
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
  int h,yoff,ysub;
  int lscale;
  double scale;

  lscale = (int) floor(log10(dmax));
  scale = pow(10,lscale);
  f.setPointSize(9);
  p.begin(this);
  p.setFont(f);
  QFontMetrics fm(p.fontMetrics());
  yoff = fm.lineSpacing();
  ysub = fm.ascent()/2;
  tmpDisplayedText.sprintf("x 1 e %i",lscale);
  p.setPen(Qt::white);
  p.drawText(10,yoff,tmpDisplayedText);
  h = height()-3*yoff;
  yoff *= 2;
  tmpDisplayedText.sprintf( "%.3f",dmax/scale);
  p.drawText(35, yoff+ysub, tmpDisplayedText);
  for(int i=0; i<8; i++){
    p.setBrush(QColor((int)(255*colormap[7-i][0]), 
		      (int)(255*colormap[7-i][1]), 
		      (int)(255*colormap[7-i][2])));
    p.setPen(QColor((int)floor(255*colormap[7-i][0]), 
		    (int)floor(255*colormap[7-i][1]),
		    (int)floor(255*colormap[7-i][2])));
    p.drawRect(10, yoff+i*(h/8), 20, (h+7)/8);
    p.setPen(Qt::white);
    res = dmax - (i+1)*(dmax - dmin)/8.0;
    tmpDisplayedText.sprintf( "%.3f",res/scale);
    p.drawText(35, yoff+ysub+(i+1)*(h/8), tmpDisplayedText);
  }
  p.end();
}

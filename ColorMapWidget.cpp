/* $Id: ColorMapWidget.cpp,v 1.5 2001/08/07 08:58:48 aspert Exp $ */
#include <ColorMapWidget.h>


/* Constructor of the ColorMapWidget class */
/* Only initializes a few values and build a HSV colormap */
ColorMapWidget::ColorMapWidget(double dmoymin, double dmoymax, 
			       QWidget *parent, 
			       const char *name):QWidget(parent,name) {
  setMinimumSize( 70, 512 );
  setMaximumSize( 70, 512 );
  setBackgroundColor( black );

  colormap = HSVtoRGB();
  dmax = dmoymax;
  dmin = dmoymin;

}

/* This function generates the bar graph that will be displayed aside */
/* the models. Each color is displayed with the mean error value associated. */
/* The 'QPaintEvent' parameter is not used (it seems to be needed when */
/* calling the QPainter stuff. */
void ColorMapWidget::paintEvent(QPaintEvent *) {
  double res;
  QPainter p;
  QString tmpDisplayedText;

  p.begin(this);
  for(int i=0; i<8; i++){
    p.setBrush(QColor((int)(255*colormap[7-i][0]), 
		      (int)(255*colormap[7-i][1]), 
		      (int)(255*colormap[7-i][2])));
    p.setPen(QColor((int)floor(255*colormap[7-i][0]), 
		    (int)floor(255*colormap[7-i][1]),
		    (int)floor(255*colormap[7-i][2])));
    p.drawRect(10, i*64, 20, 64);
    p.setPen(Qt::white);
    p.setFont(QFont("courier", 8));
    res = dmax - (dmax - dmin)*i/7;
    tmpDisplayedText.sprintf( "%5f",res);
    p.drawText(35, (i+1)*64, tmpDisplayedText);
  }
  p.end();
}

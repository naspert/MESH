#include <ColorMapWidget.h>

/***************************************************************************/
/*        definition de la classe ColorMapWidget                           */
/***************************************************************************/



ColorMapWidget::ColorMapWidget(QWidget *parent, const char *name)
  :QWidget(parent,name)
{
  setMinimumSize( 20, 500 );
  setMaximumSize( 20, 500 );
  setBackgroundColor( black );
  colormap=HSVtoRGB();
}

void ColorMapWidget::paintEvent(QPaintEvent *)
{
  QPainter p;
  p.begin(this);

  for(int i=0; i<16;i++){
    p.setBrush(QColor(255*colormap[15-i][0],255*colormap[15-i][1],255*colormap[15-i][2]));
    p.setPen(QColor(floor(255*colormap[15-i][0]),floor(255*colormap[15-i][1]),floor(255*colormap[15-i][2])));
    p.drawRect(0,i*32,20,32);
  }
  p.end();
}

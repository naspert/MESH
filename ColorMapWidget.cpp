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

  for(int i=0; i<8;i++){
    p.setBrush(QColor(255*colormap[7-i][0],255*colormap[7-i][1],255*colormap[7-i][2]));
    p.setPen(QColor(floor(255*colormap[7-i][0]),floor(255*colormap[7-i][1]),floor(255*colormap[7-i][2])));
    p.drawRect(0,i*64,20,64);
  }
  p.end();
}

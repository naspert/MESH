/* $Id: ColorMapWidget.h,v 1.11 2002/02/21 12:45:00 aspert Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>
#include <compute_error.h>

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
 enum {LIN_SCALE=0, LOG_SCALE=1};

public slots:
  void doHistogram(int scaleType);

protected:
  void paintEvent(QPaintEvent *); 

 private:
  const struct model_error *me;
  int *histogram;
  float **colormap;
  int cmap_len;
  int len;
  double dmax, dmin;
};

#endif

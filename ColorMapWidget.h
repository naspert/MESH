/* $Id: ColorMapWidget.h,v 1.14 2002/02/28 12:07:16 aspert Exp $ */
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

public slots:
  void doHistogram(int scaleType);

protected:
  void paintEvent(QPaintEvent *); 

 private:
  const struct model_error *me;
  int *histogram;
  float **colormap;
  int cmap_len;
  double dmax, dmin;
  int scaleState; // stores the state of the scale (LIN or LOG)
};

#endif

#include <mesh_run.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qfile.h>

#ifndef _TEXTWIDGET_PROTO
#define _TEXTWIDGET_PROTO

class TextWidget : public QWidget {

public:
  TextWidget(QWidget *parent=0, const char *name=0);
  void openFile(FILE *in_stream);
  TextWidget(FILE *in_stream, QWidget *parent=0, const char *name=0);


private:
  QVBoxLayout *layout;
  QTextView *view;
  QPushButton *butClose;
  QString text;
  QFile qfIn;

};
#endif

/* $Id: TextWidget.h,v 1.4 2002/02/26 07:54:17 aspert Exp $ */
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>

#ifndef _TEXTWIDGET_PROTO
#define _TEXTWIDGET_PROTO

#ifdef __cplusplus
extern "C" {
#endif
  /* Called within a C function via its pointer -> make it "extern C" */
  void TextWidget_puts(void *out, const char *str);

#ifdef __cplusplus
}
#endif

class TextWidget : public QWidget {

public:
  TextWidget(QWidget *parent=0, const char *name=0);
  void append(const QString &str);
  QSize sizeHint() const;


private:
  QGridLayout *layout;
  QTextView *view;
  QPushButton *butClose;
};
#endif

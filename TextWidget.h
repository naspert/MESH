#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>

#ifndef _TEXTWIDGET_PROTO
#define _TEXTWIDGET_PROTO

extern "C" {
  void TextWidget_puts(void *out, const char *str);
}

class TextWidget : public QWidget {

public:
  TextWidget(QWidget *parent=0, const char *name=0);
  void append(const QString &str);
  QSize sizeHint() const;


private:
  QVBoxLayout *layout;
  QTextView *view;
  QPushButton *butClose;
};
#endif

/* $Id: TextWidget.cpp,v 1.5 2002/02/26 16:34:51 dsanta Exp $ */

#include <TextWidget.h>
#include <qapplication.h>

TextWidget::TextWidget(QWidget *parent, const char *name)
  : QWidget(parent, name) {
  QFont font(QApplication::font());

  layout = new QGridLayout(this,2,1);
  view = new QTextView(this);
  view->setTextFormat(Qt::PlainText);
  font.setFamily("courier");
  view->setFont(font);
  layout->addWidget(view,0,0);
  butClose = new QPushButton("Close", this);
  layout->addWidget(butClose,1,0,Qt::AlignCenter);

  connect(butClose, SIGNAL(clicked()), this, SLOT(close()));

  setCaption("Mesh execution log");
}

TextWidget::~TextWidget()
{
  delete view;
  delete layout;
  delete butClose;
}

QSize TextWidget::sizeHint() const {
  return QSize(550,500);
}

void TextWidget::append(const QString &str) {
  // The append() method of QT's TextView is buggy. Use the recommended
  // workaround.
  view->setText(view->text()+str);
  qApp->processEvents(100);
}

void TextWidget_puts(void *out, const char *str) {
  TextWidget *tw;

  tw = (TextWidget*) out;
  tw->append(QString(str));
}

#include <TextWidget.h>

TextWidget::TextWidget(QWidget *parent, const char *name)
  : QWidget(parent, name) {
  
  layout = new QVBoxLayout(this);
  layout->setResizeMode(QLayout::FreeResize);
  view = new QTextView(this);
  view->setMinimumSize(600, 600);
  view->setTextFormat(Qt::PlainText);
  view->setFont(QFont("courier", 9));
  layout->addWidget(view);
  butClose = new QPushButton("Close", this);
  butClose->setMinimumSize(40, 20);
  butClose->setMaximumSize(40, 20);
  layout->addWidget(butClose);

  connect(butClose, SIGNAL(clicked()), this, SLOT(close()));

}

void TextWidget::openFile(FILE *in_stream) {
  int readChar;
  QString buf;

  qfIn.open(IO_ReadOnly, in_stream);
  while (!qfIn.atEnd()) {
    readChar = qfIn.readLine(buf, 500);
    if (!buf.isEmpty())
      text += buf;
  }
  qfIn.close();
  view->setText(text);
}

// This one is a shortcut that merges the previous ones
TextWidget::TextWidget(FILE *in_stream, QWidget *parent, const char *name)
  : QWidget(parent, name) {

  QString buf;
  int readChar;


  qfIn.open(IO_ReadOnly, in_stream);
  layout = new QVBoxLayout(this);
  layout->setResizeMode(QLayout::FreeResize);
  view = new QTextView(this);
  view->setMinimumSize(600, 600);
  view->setTextFormat(Qt::PlainText);
  view->setFont(QFont("courier", 9));
  layout->addWidget(view);
  butClose = new QPushButton("Close", this);
  butClose->setMinimumSize(40, 20);
  butClose->setMaximumSize(40, 20);
  layout->addWidget(butClose);

  connect(butClose, SIGNAL(clicked()), this, SLOT(close()));

  while (!qfIn.atEnd()) {
    readChar = qfIn.readLine(buf, 500);
    if (!buf.isEmpty())
      text += buf;
  }
  qfIn.close();
  view->setText(text);
}


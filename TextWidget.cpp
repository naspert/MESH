#include <TextWidget.h>
#include <qapplication.h>

TextWidget::TextWidget(QWidget *parent, const char *name)
  : QWidget(parent, name) {
  QFont font(QApplication::font());

  layout = new QVBoxLayout(this);
  layout->setResizeMode(QLayout::FreeResize);
  view = new QTextView(this);
  view->setTextFormat(Qt::PlainText);
  font.setFamily("courier");
  view->setFont(font);
  layout->addWidget(view);
  butClose = new QPushButton("Close", this);
  butClose->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,
                                      QSizePolicy::Minimum));
  layout->addWidget(butClose);

  connect(butClose, SIGNAL(clicked()), this, SLOT(close()));

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

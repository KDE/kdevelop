#include <qlayout.h>
#include <qframe.h>


#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>


#include "ckonsolewidget.h"
#include "ckonsolewidget.moc"

class CKonsoleWidgetPrivate
{
public:

  CKonsoleWidgetPrivate()
    : part(0)
  {
  }

  void setDirectory(QString dirname)
  {
    if (url == dirname)
      return;
    url = dirname;
    if (part) {
      KURL url(dirname);
      part->openURL(url);
    }
  }

  void activate(QWidget *parent)
  {
    if (part)
      return;

    KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");
    if (!factory)
      return;

    part = (KParts::ReadOnlyPart *) factory->create(parent);
    if (!part)
      return;
    part->widget()->setFocusPolicy(QWidget::WheelFocus);
    parent->setFocusProxy(part->widget());
    part->widget()->setFocus();

    if (part->widget()->inherits("QFrame"))
      ((QFrame*)part->widget())->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    
    QVBoxLayout *vbox = new QVBoxLayout(parent);
    vbox->addWidget(part->widget());
  
    if (part && !url.isEmpty())
      setDirectory(url);

    part->widget()->show();
  }
  

private:
  
  QString url;
  KParts::ReadOnlyPart *part;

};


CKonsoleWidget::CKonsoleWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  d = new CKonsoleWidgetPrivate;
}


CKonsoleWidget::~CKonsoleWidget()
{
  delete d;
}


void CKonsoleWidget::setDirectory(QString dirname)
{
  d->setDirectory(dirname);
}


void CKonsoleWidget::showEvent(QShowEvent *ev)
{
  QWidget::showEvent(ev);
  d->activate(this);
}

#include <qlayout.h>
#include <qframe.h>
#include <qdir.h>


#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include "kdevcore.h"
#include "kdevproject.h"
#include "konsoleviewpart.h"
#include "konsoleviewwidget.h"


class KonsoleWidgetPrivate
{
public:

  KonsoleWidgetPrivate()
    : part(0)
  {
  }

  void setDirectory(QString dirname)
  {
    if (url == dirname)
      return;
    url = dirname;
    if (part)
      part->openURL(KURL(dirname));
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
  

  QString url;
  KParts::ReadOnlyPart *part;
  KDevPart *m_part;

};


KonsoleViewWidget::KonsoleViewWidget(KonsoleViewPart *part)
 : QWidget(0, "konsole widget")
{
  d = new KonsoleWidgetPrivate;
  d->m_part = part;

  connect(part->core(), SIGNAL(wentToSourceFile(const QString &)),
          this, SLOT(wentToSourceFile(const QString &)));
}


KonsoleViewWidget::~KonsoleViewWidget()
{
  delete d;
}


void KonsoleViewWidget::wentToSourceFile(const QString &fileName)
{
  QString dir = fileName;

  // strip filename
  int pos = dir.findRev('/');
  if (pos > 0)
    dir = dir.left(pos);
  
  kdDebug(9000) << "GOTO: " << dir << endl;

  setDirectory(dir);
}


void KonsoleViewWidget::setDirectory(QString dirname)
{
  d->setDirectory(dirname);
}


void KonsoleViewWidget::showEvent(QShowEvent *ev)
{
  QWidget::showEvent(ev);
  d->activate(this);
}


#include "konsoleviewwidget.moc"

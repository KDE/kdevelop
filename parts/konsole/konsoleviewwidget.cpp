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


KonsoleViewWidget::KonsoleViewWidget(KonsoleViewPart *part)
 : QWidget(0, "konsole widget"), part(0)
{
  connect(part->core(), SIGNAL(wentToSourceFile(const QString &)),
          this, SLOT(wentToSourceFile(const QString &)));

  vbox = new QVBoxLayout(this);
}


KonsoleViewWidget::~KonsoleViewWidget()
{
}


void KonsoleViewWidget::activate()
{
  if (part)
    return;

  KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");
  if (!factory)
    return;

  part = (KParts::ReadOnlyPart *) factory->create(this);
  if (!part)
    return;

  part->widget()->setFocusPolicy(QWidget::WheelFocus);
  setFocusProxy(part->widget());
  part->widget()->setFocus();

  if (part->widget()->inherits("QFrame"))
    ((QFrame*)part->widget())->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    
  vbox->addWidget(part->widget());
  
  if (part && !url.isEmpty())
    setDirectory(url);

  part->widget()->show();

  connect(part, SIGNAL(destroyed()), this, SLOT(partDestroyed()));
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
  if (url == dirname)
	return;
  url = dirname;
  if (part)
	part->openURL(KURL(dirname));
}


void KonsoleViewWidget::showEvent(QShowEvent *ev)
{
  QWidget::showEvent(ev);
  activate();
}


void KonsoleViewWidget::partDestroyed()
{
  part = 0;
}


#include "konsoleviewwidget.moc"

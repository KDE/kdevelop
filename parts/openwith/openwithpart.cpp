#include "openwithpart.h"

#include <kpopupmenu.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <kaction.h>

#include "kdevpartcontroller.h"
#include "kdevcore.h"


K_EXPORT_COMPONENT_FACTORY(libkdevopenwith, KGenericFactory<OpenWithPart>("kdevopenwith"));

OpenWithPart::OpenWithPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name ? name : "OpenWithPart")
{
  connect(core(), SIGNAL(contextMenu(QPopupMenu*,const Context *)),
	  this, SLOT(fillContextMenu(QPopupMenu*,const Context *)));
}


OpenWithPart::~OpenWithPart()
{
}


void OpenWithPart::fillContextMenu(QPopupMenu *popup, const Context *context)
{
  if (!context->hasType("file"))
    return;

  popup->insertSeparator();

  const FileContext *ctx = static_cast<const FileContext*>(context);
  if (ctx->isDirectory())
      return;

  m_url = KURL(ctx->fileName());

  KPopupMenu *sub = new KPopupMenu(popup);

  sub->insertItem(i18n("Open as UTF-8"), this, SLOT(openAsEncoding(int)));
  
  QString mimeType = KMimeType::findByURL(m_url, 0, true, true)->name();
  KTrader::OfferList offers = KTrader::self()->query(mimeType, "Type == 'Application'");

  if (offers.count() > 0)
  {

    KTrader::OfferList::Iterator it;
    for (it = offers.begin(); it != offers.end(); ++it)
    {
      KAction *action = new KAction((*it)->name(), 0, 0, (*it)->desktopEntryPath().latin1());
      connect(action, SIGNAL(activated()), this, SLOT(openWithService()));
      action->plug(sub);
    }
    sub->insertSeparator();
    
    popup->insertItem(i18n("Open With"), sub);

    // make sure the generic "Open with ..." entry gets appended to the submenu
    popup = sub;
  }

  popup->insertItem(i18n("Open With..."), this, SLOT(openWithDialog()));
}


void OpenWithPart::openWithService()
{
  KService::Ptr ptr = KService::serviceByDesktopPath(sender()->name());
  if (ptr)
  {
    KURL::List list;
    list << m_url;

    KRun::run(*ptr, list);
  }
}


void OpenWithPart::openWithDialog()
{
  KURL::List list;
  list << m_url;
  KRun::displayOpenWithDialog(list);
}


void OpenWithPart::openAsEncoding(int /*id*/)
{
  partController()->setEncoding("utf8");
  partController()->editDocument(m_url);
}

#include "openwithpart.moc"

#include "openwithpart.h"

#include <qfile.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include <kpopupmenu.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <kaction.h>

#include "kdevpartcontroller.h"
#include "kdevcore.h"
#include "kdevplugininfo.h"
#include "urlutil.h"

static const KDevPluginInfo data("kdevopenwith");
K_EXPORT_COMPONENT_FACTORY(libkdevopenwith, KDevGenericFactory<OpenWithPart>(data))

OpenWithPart::OpenWithPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent, name ? name : "OpenWithPart")
{
  connect(core(), SIGNAL(contextMenu(Q3PopupMenu*,const Context *)),
	  this, SLOT(fillContextMenu(Q3PopupMenu*,const Context *)));
}


OpenWithPart::~OpenWithPart()
{
}


void OpenWithPart::fillContextMenu(Q3PopupMenu *popup, const Context *context)
{
  if (!context->hasType( Context::FileContext ))
    return;

  const FileContext *ctx = static_cast<const FileContext*>(context);
  m_url = ctx->urls().first();
  if (URLUtil::isDirectory(m_url))
      return;

  popup->insertSeparator();

  KPopupMenu *sub = new KPopupMenu(popup);

  int id = sub->insertItem(i18n("Open as UTF-8"), this, SLOT(openAsEncoding(int)));
  sub->setWhatsThis(id, i18n("<b>Open as UTF-8</b><p>Open this file in KDevelop as UTF-8 encoded text."));

  QString mimeType = KMimeType::findByURL(m_url, 0, true, true)->name();
  KTrader::OfferList offers = KTrader::self()->query(mimeType, "Type == 'Application'");

  if (offers.count() > 0)
  {

    KTrader::OfferList::Iterator it;
    for (it = offers.begin(); it != offers.end(); ++it)
    {
      KAction *action = new KAction((*it)->name(), (*it)->icon(), 0, 0, QFile::encodeName( (*it)->desktopEntryPath() ).data());
      connect(action, SIGNAL(activated()), this, SLOT(openWithService()));
      action->plug(sub);
    }
    sub->insertSeparator();

    id = popup->insertItem(i18n("Open With"), sub);
    popup->setWhatsThis(id, i18n("<b>Open With</b><p>Lists all applications that can be used to open the selected file."));

    // make sure the generic "Open with ..." entry gets appended to the submenu
    popup = sub;
  }

  id = popup->insertItem(i18n("Open With..."), this, SLOT(openWithDialog()));
  popup->setWhatsThis(id, i18n("<b>Open With...</b><p>Provides a dialog to choose the application to open the selected file."));
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

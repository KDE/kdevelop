#include "openwithpart.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qmenu.h>

#include <kdevprojectmodel.h>
#include <kdevgenericfactory.h>
#include <kdevpartcontroller.h>
#include <kdevcore.h>
#include <kdevplugininfo.h>
#include <urlutil.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <kaction.h>


static const KDevPluginInfo data("kdevopenwith");
K_EXPORT_COMPONENT_FACTORY(libkdevopenwith, KDevGenericFactory<OpenWithPart>(data))

OpenWithPart::OpenWithPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent)
{
  setObjectName(QString::fromUtf8(name));
  
  connect(core(), SIGNAL(contextMenu(QMenu*,const Context*)),
          this, SLOT(fillContextMenu(QMenu*,const Context*)));
}

OpenWithPart::~OpenWithPart()
{
}

void OpenWithPart::fillContextMenu(QMenu *popup, const Context *context)
{
  kdDebug(9000) << "context: " << context->type() << endl;

  m_url = KURL();

  if (context->hasType(Context::FileContext))
    {
      const FileContext *ctx = static_cast<const FileContext*>(context);
      m_url = ctx->urls().first();
    }
  else if (context->hasType(Context::ProjectItemContext))
    {
      const ProjectItemContext *ctx = static_cast<const ProjectItemContext*>(context);
      const KDevProjectItem *item = ctx->item();
      Q_ASSERT(item != 0);
      if (item->file())
        m_url.setPath(item->file()->fileInfo().absoluteFilePath());
    }
  else
    {
      // nothing to do
      return;
    }

  populateMenu(popup);
}

void OpenWithPart::populateMenu(QMenu *popup)
{
  popup->addSeparator();
  
  QAction *action = popup->addAction(i18n("Open With"));
  action->setWhatsThis(i18n("<b>Open With</b><p>Lists all applications that can be used to open the selected file."));

  connect(action, SIGNAL(triggered()), this, SLOT(openWithDialog()));
}

void OpenWithPart::openWithService()
{
  if (KService::Ptr ptr = KService::serviceByDesktopPath(sender()->name()))
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

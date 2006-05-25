#include "openwithpart.h"

#include <qfile.h>

#include <kpopupmenu.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <krun.h>
#include <kaction.h>
#include <kcharsets.h>

#include "kdevpartcontroller.h"
#include "kdevcore.h"
#include "kdevplugininfo.h"
#include "urlutil.h"

typedef KDevGenericFactory<OpenWithPart> OpenWithFactory;

static const KDevPluginInfo data("kdevopenwith");
K_EXPORT_COMPONENT_FACTORY(libkdevopenwith, OpenWithFactory(data))

OpenWithPart::OpenWithPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent, name ? name : "OpenWithPart")
{
  setInstance( OpenWithFactory::instance() );

  connect(core(), SIGNAL(contextMenu(QPopupMenu*,const Context *)),
	  this, SLOT(fillContextMenu(QPopupMenu*,const Context *)));
}


OpenWithPart::~OpenWithPart()
{
}


void OpenWithPart::fillContextMenu(QPopupMenu *popup, const Context *context)
{
	if ( !context->hasType( Context::FileContext ) ) return;

	const FileContext *ctx = static_cast<const FileContext*>(context);
	m_urls = ctx->urls();

	popup->insertSeparator();

	popup->insertItem( i18n("Open as UTF-8"), this, SLOT(openAsUtf8()) );

	KPopupMenu * openAsPopup = new KPopupMenu( popup );

	int id = popup->insertItem( i18n("Open As"), openAsPopup );
	popup->setWhatsThis(id, i18n("<b>Open As</b><p>Lists all encodings that can be used to open the selected file."));

	QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();

	int i = 0;
	QStringList::const_iterator it = encodings.constBegin();
	while ( it != encodings.constEnd() )
	{
		openAsPopup->insertItem( *it, this, SLOT(openAsEncoding(int)), 0, i );
		++it;
		i++;
	}

	KPopupMenu * openWithPopup = new KPopupMenu( popup );

	// hmmm.. what to do here? open every different mimetype seperately? Just go with the first for now..
	QString mimeType = KMimeType::findByURL(m_urls.first(), 0, true, true)->name();
	KTrader::OfferList offers = KTrader::self()->query(mimeType, "Type == 'Application'");
	if (offers.count() > 0)
	{
		KTrader::OfferList::Iterator it;
		for (it = offers.begin(); it != offers.end(); ++it)
		{
			KAction *action = new KAction((*it)->name(), (*it)->icon(), 0, 0, QFile::encodeName( (*it)->desktopEntryPath() ).data());
			connect(action, SIGNAL(activated()), this, SLOT(openWithService()));
			action->plug( openWithPopup );
		}
	}

	id = popup->insertItem( i18n("Open With"), openWithPopup );
	popup->setWhatsThis(id, i18n("<b>Open With</b><p>Lists all applications that can be used to open the selected file."));

	openWithPopup->insertSeparator();

	// make sure the generic "Open with ..." entry gets appended to the submenu
	id = openWithPopup->insertItem(i18n("Open With..."), this, SLOT(openWithDialog()));
	openWithPopup->setWhatsThis(id, i18n("<b>Open With...</b><p>Provides a dialog to choose the application to open the selected file."));
}


void OpenWithPart::openWithService()
{
  KService::Ptr ptr = KService::serviceByDesktopPath(sender()->name());
  if (ptr)
  {
    KRun::run(*ptr, m_urls);
  }
}


void OpenWithPart::openWithDialog()
{
  KRun::displayOpenWithDialog( m_urls );
}


void OpenWithPart::openAsEncoding( int id )
{
	QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();
	QString encoding;
	if ( id <= encodings.count() && id >= 0 )
	{
		encoding = KGlobal::charsets()->encodingForName( encodings[ id ] );
	}

	KURL::List::iterator it = m_urls.begin();
	while ( it != m_urls.end() )
	{
		partController()->setEncoding( encoding );
		partController()->editDocument( *it );
		++it;
	}
}

void OpenWithPart::openAsUtf8( )
{
	KURL::List::iterator it = m_urls.begin();
	while ( it != m_urls.end() )
	{
		partController()->setEncoding( "utf8" );
		partController()->editDocument( *it );
		++it;
	}
}

#include "openwithpart.moc"

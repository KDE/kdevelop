#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <qwidget.h>
#include <qdir.h>
#include <qfile.h>
#include <kparts/factory.h>

#include <ktrader.h>

#include "docmanager.h"


DocManager::DocManager () : QObject (0L, 0L)
{
  docs.setAutoDelete (true);
}

DocManager::~DocManager ()
{

}

KTextEditor::Document *DocManager::createDoc (QString type)
{
  KLibFactory *factory = 0;
  KTrader::OfferList offers = KTrader::self()->query("KTextEditor/Document");
  KService::Ptr service = *offers.begin();

  factory = KLibLoader::self()->factory( QFile::encodeName(service->library()) );
  KTextEditor::Document *doc = (KTextEditor::Document *) static_cast<KParts::Factory *>(factory)->createPart( (QWidget *)0L, "nix", this, "nix", "KTextEditor::Document", 0L );

  docs.append (doc);
  return doc;
}

bool DocManager::deleteDoc (KTextEditor::Document *doc)
{
  docs.remove (doc);
  return true;
}
#include "docmanager.moc"

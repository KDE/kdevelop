#include <kiconloader.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <qwidget.h>
#include <qdir.h>

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

  factory = KLibLoader::self()->factory( service->library().latin1() );
  KTextEditor::Document *doc = static_cast<KTextEditor::Document *>(factory->create(this, 0, "KTextEditor::Document"));

  docs.append (doc);
  return doc;
}

bool DocManager::deleteDoc (KTextEditor::Document *doc)
{
  docs.remove (doc);
  return true;
}

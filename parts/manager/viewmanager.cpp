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

#include "viewmanager.h"

ViewManager::ViewManager (QWidget *parent, DocManager *dm) : QTabWidget (parent)
{
  views.setAutoDelete (true);
  this->dm = dm;
}

ViewManager::~ViewManager()
{
  views.setAutoDelete (false);
}

void ViewManager::createView (KTextEditor::Document *doc)
{
  KTextEditor::View *view = doc->createView (this, "nix");
  addTab (view, QString ("test"));
  view->show();
  showPage (view);
  views.append (view);
}

void ViewManager::closeView (KTextEditor::View *view)
{
  removePage (view);
}

void ViewManager::slotDocumentNew()
{
  KTextEditor::Document *doc = dm->createDoc ("");
  createView (doc);
}

void ViewManager::slotDocumentOpen()
{
  QString path = QString::null;
  KURL::List urls = KFileDialog::getOpenURLs(path, QString::null, 0L, i18n("Open File"));
  if(urls.isEmpty())
      return;
  for (KURL::List::Iterator i=urls.begin(); i != urls.end(); ++i)
  {
    KTextEditor::Document *doc = dm->createDoc ("");
    createView (doc);
    doc->openURL( *i );
  }
}

void ViewManager::slotDocumentClose()
{
  KTextEditor::View *view = (KTextEditor::View *) currentPage();
  KTextEditor::Document *doc = view->document();
  closeView (view);
  dm->deleteDoc(doc);
}
#include "viewmanager.moc"

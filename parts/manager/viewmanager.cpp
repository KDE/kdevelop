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

#include "viewmanager.h"

ViewManager::ViewManager (QWidget *parent, DocManager *dm) : QTabWidget (parent)
{

}

ViewManager::~ViewManager()
{

}

void ViewManager::createView (KTextEditor::Document *doc)
{
  KTextEditor::View *view = doc->createView (this, "nix");
  addTab (view, QString ("test"));
}

void ViewManager::closeView (KTextEditor::View *view)
{

}


/*
 * KDevelop Class viewer
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "classtree.h"

#include <QHeaderView>
#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include <klocale.h>
#include <kicon.h>
#include <kaction.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "classmodel.h"
#include "classbrowserplugin.h"
#include "duchainbase.h"
#include "duchain.h"
#include "duchainlock.h"
#include "declaration.h"

using namespace KDevelop;

ClassWidget::ClassWidget(QWidget* parent, ClassBrowserPlugin* plugin)
  : QWidget(parent)
  , m_plugin(plugin)
  , m_tree(new ClassTree(this, plugin))
  , m_currentMode(ModeProject)
{
  setObjectName("Class Browser Tree");
  setWindowTitle(i18n("Classes"));
  setWindowIcon(KIcon("class"));

  KAction* action = new KAction(i18n( "Scope" ), this);
  action->setToolTip(i18n("Select how much of the code model to display."));
  addAction(action);

  QMenu *modeMenu = new QMenu(this);
  action->setMenu(modeMenu);
  QActionGroup* ag = new QActionGroup(modeMenu);
  ag->setExclusive(true);
  QAction *currentdoc = ag->addAction( i18n( "&Current Document" ) );
  currentdoc->setData(ModeCurrentDocument);
  QAction* project = ag->addAction( i18n( "&Project" ) );
  project->setData(ModeProject);
  project->trigger();
  QAction* all = ag->addAction( i18n( "&All" ) );
  all->setData(ModeAll);
  modeMenu->addActions(ag->actions());

  connect( ag, SIGNAL( triggered(QAction*) ), this, SLOT( setMode(QAction*) ) );

  action = new KAction(i18n( "Filter" ), this);
  action->setToolTip(i18n("Filter the class browser by item type"));
  addAction(action);

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setMargin(0);
  vbox->addWidget( m_tree );
  setLayout( vbox );

  setWhatsThis( i18n( "Code View" ) );
}

ClassWidget::~ClassWidget()
{
}

ClassTree::ClassTree(QWidget* parent, ClassBrowserPlugin* plugin)
  : QTreeView(parent)
  , m_plugin(plugin)
{
  setModel(model());
  header()->hide();

  connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
}

ClassTree::~ClassTree()
{
}

ClassModel* ClassWidget::model()
{
  return m_plugin->model();
}

ClassModel* ClassTree::model()
{
  return m_plugin->model();
}

void ClassWidget::setMode(QAction* action)
{
  Modes newMode = static_cast<Modes>(action->data().toInt());

  if (newMode != m_currentMode) {
    switch (m_currentMode) {
      case ModeCurrentDocument:
        model()->setFilterDocument(0L);
        disconnect(m_plugin->core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), model(), SLOT(setFilterDocument(KDevelop::IDocument*)));
        break;

      case ModeProject:
        model()->setFilterByProject(false);
        break;

      default:
        break;
    }

    m_currentMode = newMode;

    switch (m_currentMode) {
      case ModeCurrentDocument:
        model()->setFilterDocument(m_plugin->core()->documentController()->activeDocument());
        connect(m_plugin->core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), model(), SLOT(setFilterDocument(KDevelop::IDocument*)));
        break;

      case ModeProject:
        model()->setFilterByProject(true);
        break;

      default:
        break;
    }
  }
}

void ClassTree::contextMenuEvent(QContextMenuEvent* e)
{
  QMenu *menu = new QMenu(this);
  QModelIndex index = indexAt(e->pos());
  if (index.isValid()) {
    DUChainReadLocker readLock(DUChain::lock());

    DUChainBasePointer* base = model()->objectForIndex(index);

    if (base && base->data()) {
      QAction* openDec = menu->addAction(i18n("Open &Declaration"), this, SLOT(openDeclaration()));
      openDec->setData(QVariant::fromValue(*base));
      QAction* openDef = menu->addAction(i18n("Open De&finition"), this, SLOT(openDefinition()));
      openDef->setData(QVariant::fromValue(*base));

      Declaration* dec = 0;
      
      if (DUContext* d = dynamic_cast<DUContext*>(base->data())) {
        dec = d->owner();
      } else if (0 != (dec = dynamic_cast<Declaration*>(base->data()))) {
	// ### do something here
      }

      if(!dec || !dec->definition())
        openDef->setEnabled(false);
    }
  }
  menu->exec(QCursor::pos());
}

void ClassTree::itemActivated(const QModelIndex& index)
{
  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer* base = model()->objectForIndex(index);
  if (base && base->data()) {
    KUrl url = KUrl((*base)->url().str());
    KTextEditor::Range range = (*base)->range().textRange();

    readLock.unlock();

    IDocument* doc = m_plugin->core()->documentController()->documentForUrl(url);

    if (!doc)
      doc = m_plugin->core()->documentController()->openDocument(url, range.start());

    doc->textDocument()->activeView()->setSelection(range);
  }
}

void ClassTree::openDeclaration()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  QAction* a = static_cast<QAction*>(sender());

  Q_ASSERT(a->data().canConvert<DUChainBasePointer>());
  
  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(a->data());
  if (base) {
    Declaration* dec = model()->declarationForObject(base);

    if (dec) {
      KUrl url( dec->url().str() );
      KTextEditor::Range range = dec->range().textRange();

      readLock.unlock();

      m_plugin->core()->documentController()->openDocument(url, range.start());

    } else {
      kDebug() << "No declaration for base object" << base;
    }

  } else {
    kDebug() << "Base object has disappeared from the duchain";
  }
}

void ClassTree::openDefinition()
{
  Q_ASSERT(qobject_cast<QAction*>(sender()));

  DUChainReadLocker readLock(DUChain::lock());

  DUChainBasePointer base = qvariant_cast<DUChainBasePointer>(static_cast<QAction*>(sender())->data());
  if (base) {
    Declaration* def = model()->definitionForObject(base);

    if (def) {
      KUrl url(def->url().str());
      KTextEditor::Range range = def->range().textRange();

      readLock.unlock();

      m_plugin->core()->documentController()->openDocument(url, range.start());
    }
  }
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "classtree.moc"

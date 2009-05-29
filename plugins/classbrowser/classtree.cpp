/*
 * KDevelop Class viewer
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
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
#include <QLabel>

#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>
#include <KMenu>
#include <KLineEdit>
#include <KHBox>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "interfaces/contextmenuextension.h"
#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/idocument.h"
#include "interfaces/iplugincontroller.h"

#include "language/interfaces/codecontext.h"

#include "language/duchain/duchainbase.h"
#include "language/duchain/duchain.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/declaration.h"
#include "language/duchain/indexedstring.h"

#include "classmodel.h"
#include "classbrowserplugin.h"

using namespace KDevelop;
 
ClassWidget::ClassWidget(QWidget* parent, ClassBrowserPlugin* plugin)
  : QWidget(parent)
  , m_plugin(plugin)
  , m_model(new ClassModel())
  , m_tree(new ClassTree(this, plugin))
  , m_searchLine(new KLineEdit(this))
{
  setObjectName("Class Browser Tree");
  setWindowTitle(i18n("Classes"));
  setWindowIcon(SmallIcon("class"));

  // Set tree in the plugin
  m_plugin->setActiveClassTree(m_tree);

  // Set model in the tree view
  m_tree->setModel(m_model);

  // We need notification in the model for the collapse/expansion of nodes.
  connect(m_tree, SIGNAL(collapsed(const QModelIndex&)),
          m_model, SLOT(collapsed(const QModelIndex&)));
  connect(m_tree, SIGNAL(expanded(const QModelIndex&)),
          m_model, SLOT(expanded(const QModelIndex&)));

  // Init search box
  m_searchLine->setClearButtonShown( true );
  connect(m_searchLine, SIGNAL(textChanged(QString)), m_model, SLOT(updateFilterString(QString)));
  QLabel *searchLabel = new QLabel( i18n("S&earch:"), this );
  searchLabel->setBuddy( m_searchLine );

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing( 5 );
  layout->setMargin( 0 );
  layout->addWidget(searchLabel);
  layout->addWidget(m_searchLine);

  setFocusProxy( m_searchLine );

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setMargin(0);
  vbox->addLayout(layout);
  vbox->addWidget(m_tree);
  setLayout( vbox );

  setWhatsThis( i18n( "Class Browser" ) );
}

ClassWidget::~ClassWidget()
{
  delete m_model;
}




ClassTree::ClassTree(QWidget* parent, ClassBrowserPlugin* plugin)
  : QTreeView(parent)
  , m_plugin(plugin)
{
  header()->hide();
  setIndentation(10);

  connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
}

ClassTree::~ClassTree()
{
}

static bool _populatingClassBrowserContextMenu = false;

bool ClassTree::populatingClassBrowserContextMenu()
{
  return _populatingClassBrowserContextMenu;
}

void ClassTree::contextMenuEvent(QContextMenuEvent* e)
{
  QMenu *menu = new QMenu(this);
  QModelIndex index = indexAt(e->pos());
  if (index.isValid())
  {
    Context* c;
    {
      DUChainReadLocker readLock(DUChain::lock());
      if(Declaration* decl = dynamic_cast<Declaration*>(model()->duObjectForIndex(index)))
        c = new DeclarationContext( decl );
      else
      {
        delete menu;
        return;
      }
    }
    _populatingClassBrowserContextMenu = true;
    
    QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( c );
    ContextMenuExtension::populateMenu(menu, extensions);
    
    _populatingClassBrowserContextMenu = false;
  }

  if (!menu->actions().isEmpty())
    menu->exec(QCursor::pos());
}

ClassModel* ClassTree::model()
{
  return static_cast<ClassModel*>(QTreeView::model());
}

void ClassTree::itemActivated(const QModelIndex& index)
{
  DUChainReadLocker readLock(DUChain::lock());

  DUChainBase* base = model()->duObjectForIndex(index);
  if (base)
  {
    KUrl url = KUrl(base->url().str());
    KTextEditor::Range range = base->range().textRange();

    readLock.unlock();

    m_plugin->core()->documentController()->openDocument(url, range.start());
  }
  
  if(isExpanded(index))
      collapse(index);
  else
      expand(index);
}

void ClassTree::highlightIdentifier(KDevelop::IndexedQualifiedIdentifier a_id)
{
  QModelIndex index = model()->getIndexForIdentifier(a_id);
  if ( !index.isValid() )
    return;

  // expand and select the item.
  selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  scrollTo(index, PositionAtCenter);
  expand(index);
}


// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on

#include "classtree.moc"

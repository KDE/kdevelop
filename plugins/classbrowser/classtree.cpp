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

#include <QMenu>
#include <QHeaderView>
#include <QContextMenuEvent>

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

ClassTree::ClassTree( QWidget* parent, ClassBrowserPlugin* plugin )
  : QTreeView( parent )
  , m_plugin( plugin ), m_tooltip( 0 )
{
  header()->hide();
  setIndentation( 10 );

  connect( this, SIGNAL( activated( QModelIndex ) ), SLOT( itemActivated( QModelIndex ) ) );
}

ClassTree::~ClassTree()
{
}

static bool _populatingClassBrowserContextMenu = false;

bool ClassTree::populatingClassBrowserContextMenu()
{
  return _populatingClassBrowserContextMenu;
}

void ClassTree::contextMenuEvent( QContextMenuEvent* e )
{
  QMenu *menu = new QMenu( this );
  QModelIndex index = indexAt( e->pos() );
  if ( index.isValid() )
  {
    Context* c;
    {
      DUChainReadLocker readLock( DUChain::lock() );
      if( Declaration* decl = dynamic_cast<Declaration*>( model()->duObjectForIndex( index ) ) )
        c = new DeclarationContext( decl );
      else
      {
        delete menu;
        return;
      }
    }
    _populatingClassBrowserContextMenu = true;

    QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( c );
    ContextMenuExtension::populateMenu( menu, extensions );

    _populatingClassBrowserContextMenu = false;
  }

  if ( !menu->actions().isEmpty() )
    menu->exec( QCursor::pos() );
}

bool ClassTree::event( QEvent* event )
{
  if ( event->type() == QEvent::ToolTip )
  {
    // if we request a tooltip over a duobject item, show a tooltip for it
    const QPoint &p = mapFromGlobal( QCursor::pos() );
    const QModelIndex &idxView = indexAt( p );

    DUChainReadLocker readLock( DUChain::lock() );
    if ( Declaration* decl = dynamic_cast<Declaration*>( model()->duObjectForIndex( idxView ) ) )
    {
      if ( m_tooltip ) {
        m_tooltip->close();
      }
      QWidget* navigationWidget = decl->topContext()->createNavigationWidget( decl );
      if ( navigationWidget )
      {
        m_tooltip = new KDevelop::NavigationToolTip( this, mapToGlobal( p ) + QPoint( 40, 0 ), navigationWidget );
        m_tooltip->resize( navigationWidget->sizeHint() + QSize( 10, 10 ) );
        ActiveToolTip::showToolTip( m_tooltip );
        return true;
      }
    }
  }

  return QAbstractItemView::event( event );
}

ClassModel* ClassTree::model()
{
  return static_cast<ClassModel*>( QTreeView::model() );
}

void ClassTree::itemActivated( const QModelIndex& index )
{
  DUChainReadLocker readLock( DUChain::lock() );

  DeclarationPointer decl = DeclarationPointer( dynamic_cast<Declaration*>( model()->duObjectForIndex( index ) ) );
  readLock.unlock();

// Delegate to plugin function
  m_plugin->showDefinition( decl );

  if( isExpanded( index ) )
    collapse( index );
  else
    expand( index );
}

void ClassTree::highlightIdentifier( KDevelop::IndexedQualifiedIdentifier a_id )
{
  QModelIndex index = model()->getIndexForIdentifier( a_id );
  if ( !index.isValid() )
    return;

  // expand and select the item.
  selectionModel()->select( index, QItemSelectionModel::ClearAndSelect );
  scrollTo( index, PositionAtCenter );
  expand( index );
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on


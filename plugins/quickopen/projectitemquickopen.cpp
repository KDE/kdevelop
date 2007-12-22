/* This file is part of the KDE libraries
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "projectitemquickopen.h"
#include <QIcon>
#include <QTextBrowser>
#include <klocale.h>
#include <iprojectcontroller.h>
#include <idocumentcontroller.h>
#include <iquickopen.h>
#include <iproject.h>
#include <icore.h>
#include <duchain/topducontext.h>
#include <duchain/duchain.h>
#include <duchain/duchainlock.h>
#include <duchain/declaration.h>
#include <duchain/typesystem.h>
#include <duchain/abstractfunctiondeclaration.h>
#include <project/projectmodel.h>

using namespace KDevelop;
typedef KDevelop::Filter<DUChainItem> Base;

//May return zero
TopDUContext* getTopContext(const HashedString& url) {
  KDevelop::TopDUContext* chosen = 0;
  QList<KDevelop::TopDUContext*> contexts = KDevelop::DUChain::self()->chainsForDocument(url);
  foreach( KDevelop::TopDUContext* ctx, contexts )
    if( !(ctx->flags() & KDevelop::TopDUContext::ProxyContextFlag) )
      chosen = ctx;
  return chosen;
}


DUChainItemData::DUChainItemData( const DUChainItem& file ) : m_item(file) {
}

QString DUChainItemData::text() const {
  return m_item.m_text;
}

QString DUChainItemData::htmlDescription() const {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  if(!m_item.m_item)
    return i18n("Not available any more");

  QString text;
  KSharedPtr<FunctionType> function = m_item.m_item->type<FunctionType>();
  if( function )
    text  = QString("%1 %2%3").arg(function->partToString( FunctionType::SignatureReturn)).arg(m_item.m_item->identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
  else
    text = m_item.m_item->toString();
  
  return "<small><small>" + text + ", " + i18n("Project") + " " + m_item.m_project + /*", " + i18n("path") + totalUrl().path() +*/ "</small></small>"; //Show only the path because of limited space
}

bool DUChainItemData::execute( QString& /*filterText*/ ) {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  if(!m_item.m_item)
    return false;
  
  ICore::self()->documentController()->openDocument( KUrl(m_item.m_item->url().str()), m_item.m_item->range().textRange().start() );
  return true;
}

bool DUChainItemData::isExpandable() const {
  return true;
}

QWidget* DUChainItemData::expandingWidget() const {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  if(!m_item.m_item)
    return 0;

  Declaration* decl = dynamic_cast<KDevelop::Declaration*>(m_item.m_item.data());
  if( !decl || !decl->context() )
    return 0;

  return decl->context()->createNavigationWidget( decl, decl->topContext(), "<small><small>" + i18n("Project") + " " + m_item.m_project + "<br>" + "</small></small>");
}

QIcon DUChainItemData::icon() const {
  return QIcon();
}

DUChainItemDataProvider::DUChainItemDataProvider( IQuickOpen* quickopen ) : m_quickopen(quickopen) {
  reset();
}

void DUChainItemDataProvider::setFilterText( const QString& text ) {
  Base::setFilter( text );
}

void fillItem( const QString& project, QList<DUChainItem>& items, Declaration* decl, DUChainItemDataProvider::ItemTypes itemTypes ) {
  if( ((itemTypes & DUChainItemDataProvider::Classes) && decl->kind() == Declaration::Type && dynamic_cast<StructureType*>(decl->abstractType().data()))
      || ((itemTypes & DUChainItemDataProvider::Functions) && dynamic_cast<AbstractFunctionDeclaration*>(decl)) ) {
    DUChainItem f;
    f.m_project = project;
        //KSharedPtr<FunctionType> function = decl->type<FunctionType>();
        //if( function )
      //f.m_text = QString("%1 %2%3").arg(function->partToString( FunctionType::SignatureReturn )).arg(decl->identifier().toString()).arg(function->partToString( FunctionType::SignatureArguments )
    f.m_text = decl->qualifiedIdentifier().toString();
    f.m_item = DeclarationPointer(decl);
    items << f;
  }
}

void fillItems( const QString& project, QList<DUChainItem>& items, DUContext* context, DUChainItemDataProvider::ItemTypes itemTypes ) {
  QList<DUContext*> contexts = context->childContexts();
  QList<Declaration*> declarations = context->localDeclarations();
  
  QList<DUContext*>::const_iterator contextIterator = contexts.begin();
  QList<Declaration*>::const_iterator declarationIterator = declarations.begin();

  QList<DUContext*>::const_iterator contextEnd = contexts.end();
  QList<Declaration*>::const_iterator declarationEnd = declarations.end();

  while( contextIterator != contextEnd || declarationIterator != declarationEnd ) {
    if( contextIterator != contextEnd && declarationIterator != declarationEnd ) {
      //Sort by position
      if( (*contextIterator)->range().textRange().start() < (*declarationIterator)->range().textRange().start() ) {
        fillItems( project, items, *contextIterator, itemTypes );
        ++contextIterator;
      } else {
        fillItem( project, items, *declarationIterator, itemTypes );
        ++declarationIterator;
      }
    } else if( contextIterator != contextEnd ) {
      fillItems( project, items, *contextIterator, itemTypes );
      ++contextIterator;
    } else if( declarationIterator != declarationEnd ) {
      fillItem( project, items, *declarationIterator, itemTypes );
      ++declarationIterator;
    }
  }
}

void DUChainItemDataProvider::reset() {
  Base::clearFilter();
  QList<DUChainItem> items;

  QSet<HashedString> enabledFiles = m_quickopen->fileSet();
  
  foreach( HashedString u, enabledFiles ) {
    KDevelop::DUChainReadLocker lock( DUChain::lock() );

    TopDUContext* ctx = getTopContext( u );
    if( ctx )
      fillItems( QString(), items, ctx, m_itemTypes );
  }

  setItems(items);
}

uint DUChainItemDataProvider::itemCount() const {
  return Base::filteredItems().count();
}

QList<KDevelop::QuickOpenDataPointer> DUChainItemDataProvider::data( uint start, uint end ) const {
  if( end > Base::filteredItems().count() )
    end = Base::filteredItems().count();

  QList<KDevelop::QuickOpenDataPointer> ret;
  
  for( int a = start; a < end; a++ ) {
    DUChainItem f( Base::filteredItems()[a] );
    ret << KDevelop::QuickOpenDataPointer( new DUChainItemData( Base::filteredItems()[a] ) );
  }

  return ret;
}

QStringList DUChainItemDataProvider::supportedItemTypes() {
  QStringList ret;
  ret << i18n("Classes");
  ret << i18n("Functions");
  return ret;
}

void DUChainItemDataProvider::enableData( const QStringList& items, const QStringList& scopes ) {
  if( scopes.contains(i18n("Project")) ) {
    m_itemTypes = NoItems;
    if( items.contains( i18n("Classes") ) )
      m_itemTypes = (ItemTypes)(m_itemTypes | Classes);
    if( items.contains( i18n("Functions") ) )
      m_itemTypes = (ItemTypes)(m_itemTypes | Functions);
      
  } else {
    m_itemTypes = NoItems;
  }
}

QString DUChainItemDataProvider::itemText( const DUChainItem& data ) const {
  return data.m_text;
}


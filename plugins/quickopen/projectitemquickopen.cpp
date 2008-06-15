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
#include <duchain/topducontext.h>
#include <iproject.h>
#include <icore.h>
#include <duchain/duchain.h>
#include <duchain/duchainlock.h>
#include <duchain/declaration.h>
#include <duchain/typesystem.h>
#include <duchain/abstractfunctiondeclaration.h>
#include <project/projectmodel.h>
#include <klocale.h>
#include <iquickopen.h>

using namespace KDevelop;

///@todo Use ILanguageSupport::standardContext(..)
//May return zero
KDevelop::TopDUContext* getTopContext(const HashedString& url) {
  KDevelop::TopDUContext* chosen = 0;
  QList<KDevelop::TopDUContext*> contexts = KDevelop::DUChain::self()->chainsForDocument(url);
  foreach( KDevelop::TopDUContext* ctx, contexts )
    if( !(ctx->flags() & KDevelop::TopDUContext::ProxyContextFlag) )
      chosen = ctx;
  return chosen;
}

void fillItem( const QString& project, QList<DUChainItem>& items, Declaration* decl, ProjectItemDataProvider::ItemTypes itemTypes ) {
  if( ((itemTypes & ProjectItemDataProvider::Classes) && decl->kind() == Declaration::Type && dynamic_cast<StructureType*>(decl->abstractType().data()))
      || ((itemTypes & ProjectItemDataProvider::Functions) && dynamic_cast<AbstractFunctionDeclaration*>(decl)) ) {
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

void fillItems( const QString& project, QList<DUChainItem>& items, DUContext* context, ProjectItemDataProvider::ItemTypes itemTypes ) {
  QVector<DUContext*> contexts = context->childContexts();
  QVector<Declaration*> declarations = context->localDeclarations();
  
  QVector<DUContext*>::const_iterator contextIterator = contexts.begin();
  QVector<Declaration*>::const_iterator declarationIterator = declarations.begin();

  QVector<DUContext*>::const_iterator contextEnd = contexts.end();
  QVector<Declaration*>::const_iterator declarationEnd = declarations.end();

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

ProjectItemDataProvider::ProjectItemDataProvider( KDevelop::IQuickOpen* quickopen ) : DUChainItemDataProvider(quickopen) {
}

void ProjectItemDataProvider::reset() {
  Base::clearFilter();
  QList<DUChainItem> items;

  QSet<HashedString> enabledFiles = m_quickopen->fileSet();
  
  foreach( HashedString u, enabledFiles ) {
    KDevelop::DUChainReadLocker lock( DUChain::lock() );

    TopDUContext* ctx = getTopContext( u );
    if( ctx ) ///@todo Get the project for this file and give it as first parameter
      fillItems( QString(), items, ctx, m_itemTypes );
  }

  setItems(items);
}

QStringList ProjectItemDataProvider::supportedItemTypes() {
  QStringList ret;
  ret << i18n("Classes");
  ret << i18n("Functions");
  return ret;
}

void ProjectItemDataProvider::enableData( const QStringList& items, const QStringList& scopes ) {
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

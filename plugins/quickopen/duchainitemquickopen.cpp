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
#include <icore.h>
#include <duchain/duchain.h>
#include <duchain/duchainlock.h>
#include <duchain/declaration.h>
#include <duchain/typesystem.h>

using namespace KDevelop;

DUChainItemData::DUChainItemData( const DUChainItem& file, bool openDefinition ) : m_item(file), m_openDefinition(openDefinition) {
}

QString DUChainItemData::text() const {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  if(!m_item.m_item)
    return i18n("Not available any more");
  QString text;
  KSharedPtr<FunctionType> function = m_item.m_item->type<FunctionType>();
  if( function )
    text  = QString("%1 %2%3").arg(function->partToString( FunctionType::SignatureReturn)).arg(m_item.m_item->qualifiedIdentifier().toString()).arg(function->partToString( FunctionType::SignatureArguments ));
  else
    text = m_item.m_text;
  
  return text;
}

QList<QVariant> DUChainItemData::highlighting() const {

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  KSharedPtr<FunctionType> function = m_item.m_item->type<FunctionType>();
  if(!function)
    return QList<QVariant>();

  QTextCharFormat boldFormat;
  boldFormat.setFontWeight(QFont::Bold);
  QTextCharFormat normalFormat;
   
  int prefixLength = function->partToString( FunctionType::SignatureReturn).length() + 1;

  //Only highlight the last part of the qualified identifier, so the scope doesn't distract too much
  QualifiedIdentifier id = m_item.m_item->qualifiedIdentifier();
  QString fullId = id.toString();
  QString lastId;
  if(!id.isEmpty())
      lastId = id.last().toString();
  
  prefixLength += fullId.length() - lastId.length();
  
  QList<QVariant> ret;
  ret << 0;
  ret << prefixLength;
  ret << QVariant(normalFormat);
  ret << prefixLength;
  ret << lastId.length();
  ret << QVariant(boldFormat);
    
  return ret;
    
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

  KUrl url = KUrl(m_item.m_item->url().str());
  KTextEditor::Cursor cursor = m_item.m_item->range().textRange().start();

  if(m_openDefinition && m_item.m_item->definition()) {
    url = KUrl(m_item.m_item->definition()->url().str());
    cursor = m_item.m_item->definition()->range().textRange().start();
  }
  
  lock.unlock();
  ICore::self()->documentController()->openDocument( url, cursor );
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

DUChainItemDataProvider::DUChainItemDataProvider( IQuickOpen* quickopen, bool openDefinitions ) : m_quickopen(quickopen), m_openDefinitions(openDefinitions) {
  reset();
}

void DUChainItemDataProvider::setFilterText( const QString& text ) {
  Base::setFilter( text );
}

uint DUChainItemDataProvider::itemCount() const {
  return Base::filteredItems().count();
}

QList<KDevelop::QuickOpenDataPointer> DUChainItemDataProvider::data( uint start, uint end ) const {
  if( end > (uint)Base::filteredItems().count() )
    end = Base::filteredItems().count();

  QList<KDevelop::QuickOpenDataPointer> ret;
  
  for( uint a = start; a < end; a++ ) {
    DUChainItem f( Base::filteredItems()[a] );
    ret << KDevelop::QuickOpenDataPointer( createData( Base::filteredItems()[a] ) );
  }

  return ret;
}

DUChainItemData* DUChainItemDataProvider::createData( const DUChainItem& item ) const {
    return new DUChainItemData( item, m_openDefinitions );
}

QString DUChainItemDataProvider::itemText( const DUChainItem& data ) const {
  return data.m_text;
}

void DUChainItemDataProvider::reset() {
}

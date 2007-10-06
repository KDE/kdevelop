/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "quickopen.h"

#include <QDir>

#include <klocale.h>
#include <idocumentcontroller.h>
#include <icore.h>
#include <idocument.h>
#include <ktexteditor/document.h>

#include <duchainlock.h>

#include "cppcodecompletionmodel.h"
#include "cpplanguagesupport.h"

using namespace KDevelop;
using namespace Cpp;


IncludeFileData::IncludeFileData( const IncludeItem& item ) : m_item(item) {
}

QString IncludeFileData::text() const
{
  return m_item.name;
}

bool IncludeFileData::execute( QString& filterText ) {
  if( m_item.isDirectory ) {
    //Change the filter-text to match the sub-directory
    KUrl u( filterText );
    u.setFileName( m_item.name );
    filterText = u.path( KUrl::AddTrailingSlash );
    return false;
  } else {
    KUrl u( m_item.basePath );
    u.addPath( m_item.name );
    CppLanguageSupport::self()->core()->documentController()->openDocument( u );

    return true;
  }
}


QString IncludeFileData::htmlDescription() const
{
  KUrl path = m_item.basePath;
  path.addPath( m_item.name );
  QString ret;
  if( m_item.isDirectory )
    ret = QString( i18n("Directory %1", path.prettyUrl()) );
  else
    ret = path.prettyUrl();
  
  ret += "</br>";
  ret += i18n( "Found in %1th include-path", m_item.pathNumber );
}

void IncludeFileDataProvider::setFilterText( const QString& text )
{
  QString filterText;
  if( text.contains( QDir::separator() ) )
  {
    ///If the text contains '/', list items under the given prefix,
    ///and filter them by the text behind the last '/'
    KUrl u( text );

    if( text.trimmed().endsWith( QDir::separator() ) )
      filterText = QString();
    else
      filterText = u.fileName();
    
    u.setFileName( QString() );
    QString prefixPath = u.path();

    if( prefixPath != m_lastSearchedPrefix )
    {
      kDebug(9007) << "extracted prefix " << prefixPath;

      setItems( CppLanguageSupport::self()->allFilesInIncludePath( m_baseUrl, true, prefixPath ) );

      m_lastSearchedPrefix = prefixPath;
    }
  }else{
    if( !m_lastSearchedPrefix.isEmpty() || text.isEmpty() ) {
      ///We were searching in a sub-path, but are not any more, or we are initializing the search with an empty text.
      m_lastSearchedPrefix = QString();
      
      setItems( CppLanguageSupport::self()->allFilesInIncludePath( m_baseUrl, true, QString() ) );
    }

    filterText = text;
  }

  setFilter( filterText );
}

void IncludeFileDataProvider::reset()
{
  m_lastSearchedPrefix = QString();
  m_duContext = DUContextPointer();
  m_baseUrl = KUrl();
  
  IDocument* doc = CppLanguageSupport::self()->core()->documentController()->activeDocument();

  if( doc )
  {
    m_baseUrl = doc->url();

    {
      DUChainReadLocker lock( DUChain::lock() );
      m_duContext = DUContextPointer( getCompletionContext( doc->url() ) );
    }
  }
  
  setFilterText(QString());
}

uint IncludeFileDataProvider::itemCount() const
{
  return filteredItems().count();
}

QList<QuickOpenDataPointer> IncludeFileDataProvider::data( uint start, uint end ) const
{
  QList<QuickOpenDataPointer> ret;

  const QList<Cpp::IncludeItem>& items( filteredItems() );
  
  if( end > items.count() )
    end = items.count();
  
  for( uint a = start; a < end; a++ )
    ret << QuickOpenDataPointer( new IncludeFileData( items[a] ) );
  
  return ret;
}

QString IncludeFileDataProvider::itemText( const Cpp::IncludeItem& data ) const
{
  return data.name;
}


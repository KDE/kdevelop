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
#include <QIcon>
#include <QSet>

#include <klocale.h>
#include <kiconloader.h>
#include <idocumentcontroller.h>
#include <icore.h>
#include <idocument.h>
#include <ktexteditor/document.h>

#include <duchainlock.h>

#include "navigationwidget.h"
#include "cppcodecompletionmodel.h"
#include "cpplanguagesupport.h"

using namespace KDevelop;
using namespace Cpp;

///Finds the shortest path through the imports to a given included file
QList<KUrl> getInclusionPath( const DUContext* context, const DUContext* import ) {

  QList<KUrl> ret;
  
  if( context == import ) {
    ret << import->url();
    return ret;
  }
  
  if( dynamic_cast<const TopDUContext*>(import) && dynamic_cast<const TopDUContext*>(context) && !static_cast<const TopDUContext*>(context)->imports( static_cast<const TopDUContext*>(import), context->textRange().end() ) )
    return ret;
  
  QList<DUContextPointer> imports = context->importedParentContexts();
  
  foreach( DUContextPointer i, imports ) {
    if( !i )
      continue;
    
    QList<KUrl> ret2 = getInclusionPath( i.data(), import );

    if( !ret2.isEmpty() && ( ret.isEmpty() || ret.count() > ret2.count() ) )
      ret = ret2;
  }

  if( !ret.isEmpty() )
    ret.push_front( context->url() );
  
  return ret;
}

void collectImporters( QSet<KUrl>& importers, DUContext* ctx )
{
  if( importers.contains( ctx->url() ) )
    return;
  
  importers.insert( ctx->url() );

  foreach( DUContext* ctx, ctx->importedChildContexts() )
    collectImporters( importers, ctx );
}

IncludeFileData::IncludeFileData( const IncludeItem& item, const TopDUContextPointer& includedFrom ) : m_item(item), m_includedFrom(includedFrom) {
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

QIcon IncludeFileData::icon() const {
  ///@todo Better icons?
  static QIcon standardIcon = KIconLoader::global()->loadIcon( "CTdisconnected_parents", KIconLoader::Small );
  static QIcon includedIcon = KIconLoader::global()->loadIcon( "CTparents", KIconLoader::Small );
  static QIcon importerIcon = KIconLoader::global()->loadIcon( "CTchildren", KIconLoader::Small );

  if( m_item.pathNumber == -1 )
    return importerIcon;
  else if( m_includedFrom )
    return includedIcon;
  else
    return standardIcon;
}

bool IncludeFileData::isExpandable() const {
  return true;
}

QWidget* IncludeFileData::expandingWidget() const {
  DUChainReadLocker lock( DUChain::lock() );
  
  QString htmlPrefix, htmlSuffix;
  
  QList<KUrl> inclusionPath; //Here, store the shortest way of intermediate includes to the included file.

  if( m_includedFrom && m_item.pathNumber != -1 )
  {
    //Find the trace from m_includedFrom to the this file
    KUrl u = m_item.basePath;
    u.addPath( m_item.name );

    QList<TopDUContext*> allChains = DUChain::self()->chainsForDocument(u);

    foreach( TopDUContext* t, allChains )
    {
      if( m_includedFrom.data()->imports( t, m_includedFrom->textRange().end() ) )
      {
        QList<KUrl> inclusion = getInclusionPath( m_includedFrom.data(), t );

        if( inclusionPath.isEmpty() || inclusionPath.count() > inclusion.count() )
          inclusionPath = inclusion;
      }
    }
  }else if( m_item.pathNumber == -1 && m_includedFrom )
  {
    //Find the trace from this file to m_includedFrom
    KUrl u = m_item.basePath;
    u.addPath( m_item.name );

    QList<TopDUContext*> allChains = DUChain::self()->chainsForDocument(u);

    foreach( TopDUContext* t, allChains )
    {
      if( t->imports( m_includedFrom.data(), m_includedFrom->textRange().end() ) )
      {
        QList<KUrl> inclusion = getInclusionPath( t, m_includedFrom.data() );

        if( inclusionPath.isEmpty() || inclusionPath.count() > inclusion.count() )
          inclusionPath = inclusion;
      }
    }
  }

  if( m_item.pathNumber == -1 ) {
    htmlPrefix = i18n("This file imports the current open document<br/>");
  } else {
    if( !inclusionPath.isEmpty() )
      inclusionPath.pop_back(); //Remove the file itself from the list
    
    htmlSuffix = "<br/>" + i18n( "Found in %1th include-path", m_item.pathNumber );
    
  }
  
  foreach( const KUrl& u, inclusionPath )
    htmlPrefix += i18n("Included through") + " " + QString("KDEV_FILE_LINK{%1}").arg(u.prettyUrl()) + "<br/>";
  
  return new NavigationWidget( m_item, htmlPrefix, htmlSuffix );
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
  
  return ret;
}

IncludeFileDataProvider::IncludeFileDataProvider() : m_allowImports(true), m_allowPossibleImports(true), m_allowImporters(true) {
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

      QList<IncludeItem> allIncludeItems;

      if( m_allowImports )
        allIncludeItems = CppLanguageSupport::self()->allFilesInIncludePath( m_baseUrl, true, QString() );

      foreach( KUrl u, m_importers ) {
        IncludeItem i;
        i.isDirectory = false;
        i.name = u.fileName();
        u.setFileName(QString());
        i.basePath = u;
        i.pathNumber = -1; //We mark this as an importer by putting pathNumber to -1
        allIncludeItems << i;
      }
      
      setItems( allIncludeItems );
    }

    filterText = text;
  }

  setFilter( filterText );
}

void IncludeFileDataProvider::reset()
{
  m_lastSearchedPrefix = QString();
  m_duContext = TopDUContextPointer();
  m_baseUrl = KUrl();
  m_importers.clear();
  
  IDocument* doc = CppLanguageSupport::self()->core()->documentController()->activeDocument();

  if( doc )
  {
    m_baseUrl = doc->url();

    {
      DUChainReadLocker lock( DUChain::lock() );
      m_duContext = TopDUContextPointer( getCompletionContext( doc->url() ) );

      if( m_allowImporters && m_duContext ) {
        QSet<KUrl> importers;

        collectImporters( importers, m_duContext.data() );

        foreach( const KUrl& u,  importers )
          m_importers << u;
      }
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
  
  if( end > (uint)items.count() )
    end = items.count();
  
  DUChainReadLocker lock( DUChain::lock() );
  
  for( uint a = start; a < end; a++ )
  {
    //Find out whether the url is included into the current file
    bool isIncluded = false;

    if( m_duContext )
    {
      KUrl u = items[a].basePath;
      u.addPath( items[a].name );

      QList<TopDUContext*> allChains = DUChain::self()->chainsForDocument(u);

      foreach( TopDUContext* t, allChains )
      {
        if( m_duContext.data()->imports( t, m_duContext->textRange().end() ) )
        {
          isIncluded = true;
          break;
        }
      }
    }

    //If it is an importer(marked by pathNumber -1), give m_duContext so we can search the inclusion-path later
    ret << QuickOpenDataPointer( new IncludeFileData( items[a], ( isIncluded || items[a].pathNumber == -1 ) ? m_duContext : TopDUContextPointer() ) );
  }
  
  return ret;
}

QString IncludeFileDataProvider::itemText( const Cpp::IncludeItem& data ) const
{
  return data.name;
}

QStringList IncludeFileDataProvider::scopes() {
  QStringList ret;
  ret << i18n("Imports");
/*  ret << i18n("Possible Imports");*/
  ret << i18n("Importers");
  return ret;
}

void IncludeFileDataProvider::enableScopes( const QStringList& scopes ) {
  m_allowImports = scopes.contains( i18n("Imports") );
/*  m_allowPossibleImports = scopes.contains( i18n("Possible Imports") );*/
  m_allowImporters = scopes.contains( i18n("Importers") );
}


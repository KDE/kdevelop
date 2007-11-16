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

#include "project_file_quickopen.h"
#include <QIcon>
#include <QTextBrowser>
#include <klocale.h>
#include <iprojectcontroller.h>
#include <idocumentcontroller.h>
#include <iproject.h>
#include <icore.h>
#include <duchain/topducontext.h>
#include <duchain/duchain.h>
#include <duchain/duchainlock.h>
#include <project/projectmodel.h>

using namespace KDevelop;

ProjectFileData::ProjectFileData( const ProjectFile& file ) : m_file(file) {
}

QString ProjectFileData::text() const {
  return m_file.m_relativePath;
}
KUrl ProjectFileData::totalUrl() const {
  KUrl totalUrl = m_file.m_projectUrl;
  totalUrl.addPath( m_file.m_relativePath );
  return totalUrl;
}

QString ProjectFileData::htmlDescription() const {
  return "<small><small>" + i18n("Project") + " " + m_file.m_project + /*", " + i18n("path") + totalUrl().path() +*/ "</small></small>"; //Show only the path because of limited space
}

bool ProjectFileData::execute( QString& filterText ) {
  m_file.m_core->documentController()->openDocument( totalUrl() );
  return true;
}

bool ProjectFileData::isExpandable() const {
  return true;
}

QWidget* ProjectFileData::expandingWidget() const {
  ///Find a du-chain for the document
  KUrl u( totalUrl() );

  ///Pick a non-proxy context
  KDevelop::TopDUContext* chosen = 0;
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  
  QList<KDevelop::TopDUContext*> contexts = KDevelop::DUChain::self()->chainsForDocument(u);
  foreach( KDevelop::TopDUContext* ctx, contexts )
    if( !(ctx->flags() & KDevelop::TopDUContext::ProxyContextFlag) )
      chosen = ctx;

  
  if( chosen )
    return chosen->createNavigationWidget(0, "<small><small>" + i18n("Project") + " " + m_file.m_project + "<br>" + "</small></small>");
  else {
    QTextBrowser* ret = new QTextBrowser();
    ret->resize(400, 100);
    ret->setText("<small><small>" + i18n("Project") + " " + m_file.m_project + "<br>" + /*i18n("Url") + " " + totalUrl().prettyUrl() + "<br>" +*/ i18n("Not parsed yet") + "</small></small>");
    return ret;
  }
  
  return 0;
}

QIcon ProjectFileData::icon() const {
  return QIcon();
}

ProjectFileDataProvider::ProjectFileDataProvider(ICore* core) : m_core(core) {
  reset();
}

void ProjectFileDataProvider::setFilterText( const QString& text ) {
  Base::setFilter( text );
}

void ProjectFileDataProvider::reset() {
  Base::clearFilter();
  QList<ProjectFile> projectFiles;
  
  foreach( IProject* project, m_core->projectController()->projects() ) {
    QList<ProjectFileItem*> files = project->files();
    foreach( ProjectFileItem* file, files ) {
      ProjectFile f;
      f.m_projectUrl = project->folder();
      f.m_projectUrl.adjustPath(KUrl::AddTrailingSlash);
      f.m_relativePath = KUrl::relativeUrl( f.m_projectUrl, file->url() );
      f.m_project = project->name();
      f.m_core = m_core;
      projectFiles << f;
    }
  }

  setItems(projectFiles);
}

uint ProjectFileDataProvider::itemCount() const {
  return Base::filteredItems().count();
}

QList<KDevelop::QuickOpenDataPointer> ProjectFileDataProvider::data( uint start, uint end ) const {
  if( end > Base::filteredItems().count() )
    end = Base::filteredItems().count();
  if( start < 0 )
    start = 0;

  QList<KDevelop::QuickOpenDataPointer> ret;
  
  for( int a = start; a < end; a++ ) {
    ProjectFile f( Base::filteredItems()[a] );
    ret << KDevelop::QuickOpenDataPointer( new ProjectFileData( Base::filteredItems()[a] ) );
  }

  return ret;
}

QString ProjectFileDataProvider::itemText( const ProjectFile& data ) const {
  return data.m_relativePath;
}


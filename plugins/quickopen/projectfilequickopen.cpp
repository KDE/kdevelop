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

#include "projectfilequickopen.h"
#include <QIcon>
#include <QTextBrowser>
#include <klocale.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/parsingenvironment.h>
#include <project/projectmodel.h>

#include "../openwith/iopenwith.h"

using namespace KDevelop;

ProjectFileData::ProjectFileData( const ProjectFile& file ) : m_file(file) {
}

QString ProjectFileData::text() const {
  KUrl u(m_file.m_projectUrl.toUrl());
  u.adjustPath(KUrl::AddTrailingSlash);
  return KUrl::relativeUrl( u, m_file.m_url.toUrl() );
}

KUrl ProjectFileData::totalUrl() const {
  return m_file.m_url.toUrl();
}

QString ProjectFileData::htmlDescription() const {
  return "<small><small>" + i18n("Project") + ' ' + m_file.m_project.str() + /*", " + i18n("path") + totalUrl().toLocalFile() +*/ "</small></small>"; //Show only the path because of limited space
}

bool ProjectFileData::execute( QString& /*filterText*/ ) {
  IOpenWith::openFiles(KUrl::List() << totalUrl());
  return true;
}

bool ProjectFileData::isExpandable() const {
  return true;
}

QList<QVariant> ProjectFileData::highlighting() const {
  QTextCharFormat boldFormat;
  boldFormat.setFontWeight(QFont::Bold);
  QTextCharFormat normalFormat;
  
  QString txt = text();
  
  QList<QVariant> ret;

  int fileNameLength = m_file.m_url.toUrl().fileName().length();
  
  ret << 0;
  ret << txt.length() - fileNameLength;
  ret << QVariant(normalFormat);
  ret << txt.length() - fileNameLength;
  ret << fileNameLength;
  ret << QVariant(boldFormat);
    
  return ret;
}
    
QWidget* ProjectFileData::expandingWidget() const {
  ///Find a du-chain for the document
  KUrl u( totalUrl() );

  ///Pick a non-proxy context
  KDevelop::TopDUContext* chosen = 0;
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  
  QList<KDevelop::TopDUContext*> contexts = KDevelop::DUChain::self()->chainsForDocument(u);
  foreach( KDevelop::TopDUContext* ctx, contexts )
    if( !(ctx->parsingEnvironmentFile() && ctx->parsingEnvironmentFile()->isProxyContext()) )
      chosen = ctx;

  
  if( chosen )
    return chosen->createNavigationWidget(0, 0, "<small><small>" + i18n("Project") + ' ' + m_file.m_project.str() + "<br>" + "</small></small>");
  else {
    QTextBrowser* ret = new QTextBrowser();
    ret->resize(400, 100);
    ret->setText("<small><small>" + i18n("Project") + ' ' + m_file.m_project.str() + "<br>" + /*i18n("Url") + " " + totalUrl().prettyUrl() + "<br>" +*/ i18n("Not parsed yet") + "</small></small>");
    return ret;
  }
  
  return 0;
}

QIcon ProjectFileData::icon() const {
  return m_file.m_icon;
}

ProjectFileDataProvider::ProjectFileDataProvider() {
  reset();
}

void ProjectFileDataProvider::setFilterText( const QString& text ) {
  QString filterText = text;
  if (filterText.startsWith(QLatin1String("./")) || filterText.startsWith(QLatin1String("../")) ) {
    // assume we want to filter relative to active document's url
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if (doc) {
      KUrl url = doc->url().upUrl();
      url.addPath(text);
      url.cleanPath();
      url.adjustPath(KUrl::RemoveTrailingSlash);
      filterText = url.pathOrUrl();
    }
  }
  Base::setFilter( filterText.split('/', QString::SkipEmptyParts), QChar('/') );
}

namespace
{
QSet<IndexedString> openFiles()
{
  QSet<IndexedString> openFiles;
  foreach( IDocument* doc, ICore::self()->documentController()->openDocuments() ) {
    openFiles << IndexedString(doc->url().pathOrUrl());
  }
  return openFiles;
}
}

bool sortProjectFiles(const ProjectFile& left, const ProjectFile& right)
{
    // arbitrarily sort projects (fast)
    if ( left.m_project != right.m_project ) {
        return left.m_project < right.m_project;
    }
    return left.m_url.byteArray() < right.m_url.byteArray();
}

void ProjectFileDataProvider::reset() {
  Base::clearFilter();
  QList<ProjectFile> projectFiles;
  QSet<IndexedString> openFiles_ = openFiles();

  foreach( IProject* project, ICore::self()->projectController()->projects() ) {
    QSet<IndexedString> allFiles = project->fileSet();
    allFiles -= openFiles_;
    IndexedString projectFolder(project->folder().pathOrUrl());
    IndexedString projectName(project->name());
    foreach(const IndexedString &file, allFiles) {
      ProjectFile f;
      f.m_projectUrl = projectFolder;
      f.m_url = file;
      f.m_project = projectName;
      projectFiles << f;
    }
  }

  qSort(projectFiles.begin(), projectFiles.end(), sortProjectFiles);

  setItems(projectFiles);
}

uint ProjectFileDataProvider::itemCount() const {
  return Base::filteredItems().count();
}

QSet<IndexedString> ProjectFileDataProvider::files() const {
  QSet<IndexedString> ret;

  foreach( IProject* project, ICore::self()->projectController()->projects() )
    ret += project->fileSet();

  return ret - openFiles();
}

QList<KDevelop::QuickOpenDataPointer> ProjectFileDataProvider::data( uint start, uint end ) const {
  if( end > (uint)Base::filteredItems().count() )
    end = Base::filteredItems().count();

  QList<KDevelop::QuickOpenDataPointer> ret;
  
  for( uint a = start; a < end; a++ ) {
    ProjectFile f( Base::filteredItems()[a] );
    ret << KDevelop::QuickOpenDataPointer( new ProjectFileData( Base::filteredItems()[a] ) );
  }

  return ret;
}

QString ProjectFileDataProvider::itemText( const ProjectFile& data ) const {
  return data.m_url.str();
}

void OpenFilesDataProvider::reset()
{
  Base::clearFilter();
  QList<ProjectFile> currentFiles;
  IProjectController* projCtrl = ICore::self()->projectController();
  IDocumentController* docCtrl = ICore::self()->documentController();

  KIcon icon("tab-duplicate");
  foreach( IDocument* doc, docCtrl->openDocuments() ) {
    ProjectFile f;
    f.m_icon = icon;
    f.m_url = IndexedString(doc->url().pathOrUrl());
    IProject* proj = projCtrl->findProjectForUrl(doc->url());
    if (proj) {
        f.m_projectUrl = IndexedString(proj->folder().pathOrUrl());
        f.m_project = IndexedString(proj->name());
    } else {
        f.m_project = IndexedString(i18n("none"));
    }
    currentFiles << f;
  }

  qSort(currentFiles.begin(), currentFiles.end(), sortProjectFiles);

  setItems(currentFiles);
}

QSet<KDevelop::IndexedString> OpenFilesDataProvider::files() const
{
  return openFiles();
}


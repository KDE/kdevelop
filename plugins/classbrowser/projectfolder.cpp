/*
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

#include "projectfolder.h"

#include <interfaces/iproject.h>
#include <serialization/indexedstring.h>
#include <KLocalizedString>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ProjectFolder::ProjectFolder( NodesModelInterface* a_model, IProject* project )
  : DocumentClassesFolder( i18n("Classes in project %1", project->name()), a_model )
  , m_project(project)
{
}

ProjectFolder::ProjectFolder( NodesModelInterface* a_model )
  : DocumentClassesFolder( "", a_model )
  , m_project( 0 )
{
}

void ProjectFolder::populateNode()
{
  foreach( const IndexedString &file, m_project->fileSet() ) {
    parseDocument(file);
  }

  recursiveSort();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FilteredProjectFolder::FilteredProjectFolder(NodesModelInterface* a_model, IProject* project)
  : ProjectFolder(a_model, project)
{
}

void FilteredProjectFolder::updateFilterString(QString a_newFilterString)
{
  m_filterString = a_newFilterString;

  if ( isPopulated() ) {
#if 1 // Choose speed over correctness.
    // Close the node and re-open it should be quicker than reload each document
    // and remove indevidual nodes (at the cost of loosing the current selection).
    performPopulateNode(true);
#else
    bool hadChanges = false;

    // Reload the documents.
    foreach( const IndexedString& file, getAllOpenDocuments() )
      hadChanges |= updateDocument(file);

    // Sort if we've updated documents.
    if ( hadChanges )
      recursiveSort();
    else
    {
      // If nothing changed, the title changed so mark the node as updated.
      m_model->nodesLayoutAboutToBeChanged(this);
      m_model->nodesLayoutChanged(this);
    }
#endif
  } else {
    // Displayed name changed only...
    m_model->nodesLayoutAboutToBeChanged(this);
    m_model->nodesLayoutChanged(this);
  }
}

bool FilteredProjectFolder::isClassFiltered(const KDevelop::QualifiedIdentifier& a_id)
{
  return !a_id.last().toString().contains(m_filterString, Qt::CaseInsensitive);
}


// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

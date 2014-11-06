/*
 * KDevelop Class Browser
 *
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

#include "allclassesfolder.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <KLocalizedString>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

AllClassesFolder::AllClassesFolder(NodesModelInterface* a_model)
  : DocumentClassesFolder(i18n("All projects classes"), a_model)
{
}

void AllClassesFolder::nodeCleared()
{
  DocumentClassesFolder::nodeCleared();
  
  disconnect(ICore::self()->projectController(), &IProjectController::projectOpened, this, &AllClassesFolder::projectOpened);
  disconnect(ICore::self()->projectController(), &IProjectController::projectClosing, this, &AllClassesFolder::projectClosing);
}

void AllClassesFolder::populateNode()
{
  DocumentClassesFolder::populateNode();

  // Get notification for future project addition / removal.
  connect (ICore::self()->projectController(), &IProjectController::projectOpened, this, &AllClassesFolder::projectOpened);
  connect (ICore::self()->projectController(), &IProjectController::projectClosing, this, &AllClassesFolder::projectClosing);

  // Parse each existing project file
  foreach(IProject* project, ICore::self()->projectController()->projects())
  {
    // Run over all the files in the project.
    foreach(const IndexedString& file, project->fileSet())
      parseDocument(file);
  }
}

void AllClassesFolder::projectClosing(KDevelop::IProject* project)
{
  // Run over all the files in the project.
  foreach(const IndexedString& file, project->fileSet())
    closeDocument(file);
}

void AllClassesFolder::projectOpened(KDevelop::IProject* project)
{
  // Run over all the files in the project.
  foreach(const IndexedString& file, project->fileSet())
    parseDocument(file);

  recursiveSort();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FilteredAllClassesFolder::FilteredAllClassesFolder(NodesModelInterface* a_model)
  : AllClassesFolder(a_model)
{
}

void FilteredAllClassesFolder::updateFilterString(QString a_newFilterString)
{
  m_filterString = a_newFilterString;

  if ( isPopulated() )
  {
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
  }
  else
  {
    // Displayed name changed only...
    m_model->nodesLayoutAboutToBeChanged(this);
    m_model->nodesLayoutChanged(this);
  }
}

bool FilteredAllClassesFolder::isClassFiltered(const KDevelop::QualifiedIdentifier& a_id)
{
  return !a_id.last().toString().contains(m_filterString, Qt::CaseInsensitive);
}




// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

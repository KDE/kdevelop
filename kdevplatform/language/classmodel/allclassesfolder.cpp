/*
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "allclassesfolder.h"

#include "../../interfaces/icore.h"
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

    disconnect(
        ICore::self()->projectController(), &IProjectController::projectOpened, this, &AllClassesFolder::projectOpened);
    disconnect(
        ICore::self()->projectController(), &IProjectController::projectClosing, this,
        &AllClassesFolder::projectClosing);
}

void AllClassesFolder::populateNode()
{
    DocumentClassesFolder::populateNode();

    // Get notification for future project addition / removal.
    connect(
        ICore::self()->projectController(), &IProjectController::projectOpened, this, &AllClassesFolder::projectOpened);
    connect(
        ICore::self()->projectController(), &IProjectController::projectClosing, this,
        &AllClassesFolder::projectClosing);

    // Parse each existing project file
    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* project : projects) {
        projectOpened(project);
    }
}

void AllClassesFolder::projectClosing(KDevelop::IProject* project)
{
    // Run over all the files in the project.
    const auto files = project->fileSet();
    for (const IndexedString& file : files) {
        closeDocument(file);
    }
}

void AllClassesFolder::projectOpened(KDevelop::IProject* project)
{
    // Run over all the files in the project.
    const auto files = project->fileSet();
    for (const IndexedString& file : files) {
        parseDocument(file);
    }

    recursiveSort();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FilteredAllClassesFolder::FilteredAllClassesFolder(NodesModelInterface* a_model)
    : AllClassesFolder(a_model)
{
}

void FilteredAllClassesFolder::updateFilterString(const QString& a_newFilterString)
{
    m_filterString = a_newFilterString;

    if (isPopulated()) {
#if 1 // Choose speed over correctness.
        // Close the node and re-open it should be quicker than reload each document
        // and remove individual nodes (at the cost of loosing the current selection).
        performPopulateNode(true);
#else
        bool hadChanges = false;

        // Reload the documents.
        foreach (const IndexedString& file, getAllOpenDocuments())
            hadChanges |= updateDocument(file);

        // Sort if we've updated documents.
        if (hadChanges)
            recursiveSort();
        else
        {
            // If nothing changed, the title changed so mark the node as updated.
            m_model->nodesLayoutAboutToBeChanged(this);
            m_model->nodesLayoutChanged(this);
        }
#endif
    } else
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

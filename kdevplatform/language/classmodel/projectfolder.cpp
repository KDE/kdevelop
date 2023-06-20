/*
    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "projectfolder.h"

#include "../../interfaces/iproject.h"
#include "../../serialization/indexedstring.h"
#include <KLocalizedString>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ProjectFolder::ProjectFolder(NodesModelInterface* a_model, IProject* project)
    : DocumentClassesFolder(i18n("Classes in project %1", project->name()), a_model)
    , m_project(project)
{
}

ProjectFolder::ProjectFolder(NodesModelInterface* a_model)
    : DocumentClassesFolder(QString(), a_model)
    , m_project(nullptr)
{
}

void ProjectFolder::populateNode()
{
    const auto files = m_project->fileSet();
    for (const IndexedString& file : files) {
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

void FilteredProjectFolder::updateFilterString(const QString& a_newFilterString)
{
    m_filterString = a_newFilterString;

    if (isPopulated()) {
#if 1 // Choose speed over correctness.
        // Close the node and re-open it should be quicker than reload each document
        // and remove individual nodes (at the cost of losing the current selection).
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

#include "moc_projectfolder.cpp"

/*
 * KDevelop Problem Reporter
 *
 * Copyright 2010 Dmitry Risenberg <dmitry.risenberg@gmail.com>
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


#include "watcheddocumentset.h"

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectfilemanager.h>

#include "problemreporterplugin.h"

using namespace KDevelop;

WatchedDocumentSet::WatchedDocumentSet(ProblemModel* parent)
    :QObject(parent)
{
}

void WatchedDocumentSet::setCurrentDocument(const KDevelop::IndexedString&)
{
}

WatchedDocumentSet::DocumentSet WatchedDocumentSet::get() const
{
    return m_documents;
}

ProblemModel* WatchedDocumentSet::model() const
{
    return static_cast<ProblemModel*>(parent());
}

CurrentDocumentSet::CurrentDocumentSet(const KDevelop::IndexedString& document, ProblemModel * parent)
    : WatchedDocumentSet(parent)
{
    m_documents.insert(document);
}

void CurrentDocumentSet::setCurrentDocument(const KDevelop::IndexedString& url)
{
    m_documents.clear();
    m_documents.insert(url);
    emit changed();
}

ProblemModel::Scope CurrentDocumentSet::getScope() const
{
    return ProblemModel::CurrentDocument;
}

OpenDocumentSet::OpenDocumentSet(ProblemModel* parent)
    : WatchedDocumentSet(parent)
{
    QList<KDevelop::IDocument*> docs = model()->plugin()->core()->documentController()->openDocuments();
    foreach (KDevelop::IDocument* doc, docs) {
        m_documents.insert(KDevelop::IndexedString(doc->url()));
    }
    connect(model()->plugin()->core()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), this, SLOT(documentClosed(KDevelop::IDocument*)));
    connect(model()->plugin()->core()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument*)), this, SLOT(documentCreated(KDevelop::IDocument*)));
}

void OpenDocumentSet::documentClosed(KDevelop::IDocument* doc)
{
    if (m_documents.remove(KDevelop::IndexedString(doc->url()))) {
        emit changed();
    }
}

void OpenDocumentSet::documentCreated(KDevelop::IDocument* doc)
{
    m_documents.insert(KDevelop::IndexedString(doc->url()));
    emit changed();
}

ProblemModel::Scope OpenDocumentSet::getScope() const
{
    return ProblemModel::OpenDocuments;
}

ProjectSet::ProjectSet(ProblemModel* parent)
    : WatchedDocumentSet(parent)
{
}

void ProjectSet::fileAdded(ProjectFileItem* file)
{
    m_documents.insert(file->indexedPath());
    emit changed();
}

void ProjectSet::fileRemoved(ProjectFileItem* file)
{
    if (m_documents.remove(file->indexedPath())) {
        emit changed();
    }
}

void ProjectSet::fileRenamed(const Path& oldFile, ProjectFileItem* newFile)
{
    if (m_documents.remove(IndexedString(oldFile.pathOrUrl()))) {
        m_documents.insert(newFile->indexedPath());
    }
}

void ProjectSet::trackProjectFiles(const IProject* project)
{
    if (project) {
        // The implementation should derive from QObject somehow
        QObject* fileManager = dynamic_cast<QObject*>(project->projectFileManager());
        if (fileManager) {
            connect(fileManager, SIGNAL(fileAdded(KDevelop::ProjectFileItem*)),
                    this, SLOT(fileAdded(KDevelop::ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRemoved(KDevelop::ProjectFileItem*)),
                    this, SLOT(fileRemoved(KDevelop::ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRenamed(KDevelop::Path,KDevelop::ProjectFileItem*)),
                    this, SLOT(fileRenamed(KDevelop::Path,KDevelop::ProjectFileItem*)));
        }
    }
}

CurrentProjectSet::CurrentProjectSet(const KDevelop::IndexedString& document, ProblemModel* parent)
    : ProjectSet(parent), m_currentProject(0)
{
    setCurrentDocumentInternal(document);
    trackProjectFiles(m_currentProject);
}

void CurrentProjectSet::setCurrentDocument(const KDevelop::IndexedString& url)
{
    setCurrentDocumentInternal(url);
}

void CurrentProjectSet::setCurrentDocumentInternal(const KDevelop::IndexedString& url)
{
    IProject* projectForUrl = model()->plugin()->core()->projectController()->findProjectForUrl(url.str());
    if (projectForUrl && projectForUrl != m_currentProject) {
        m_documents.clear();
        m_currentProject = projectForUrl;
        QList<ProjectFileItem*> files = m_currentProject->files();
        foreach (ProjectFileItem* file, files) {
            m_documents.insert(file->indexedPath());
        }
        emit changed();
    }
}

ProblemModel::Scope CurrentProjectSet::getScope() const
{
    return ProblemModel::CurrentProject;
}

AllProjectSet::AllProjectSet(ProblemModel* parent)
    : ProjectSet(parent)
{
    foreach(const IProject* project, model()->plugin()->core()->projectController()->projects()) {
        foreach (ProjectFileItem* file, project->files()) {
            m_documents.insert(file->indexedPath());
        }
        trackProjectFiles(project);
    }
}

ProblemModel::Scope AllProjectSet::getScope() const
{
    return ProblemModel::AllProjects;
}

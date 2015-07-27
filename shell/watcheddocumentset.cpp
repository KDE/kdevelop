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

namespace KDevelop
{

WatchedDocumentSet::WatchedDocumentSet(QObject* parent)
    :QObject(parent)
{
}

void WatchedDocumentSet::setCurrentDocument(const IndexedString&)
{
}

WatchedDocumentSet::DocumentSet WatchedDocumentSet::get() const
{
    return m_documents;
}

CurrentDocumentSet::CurrentDocumentSet(const IndexedString& document, QObject *parent)
    : WatchedDocumentSet(parent)
{
    m_documents.insert(document);
}

void CurrentDocumentSet::setCurrentDocument(const IndexedString& url)
{
    m_documents.clear();
    m_documents.insert(url);
    emit changed();
}

ProblemScope CurrentDocumentSet::getScope() const
{
    return CurrentDocument;
}

OpenDocumentSet::OpenDocumentSet(QObject *parent)
    : WatchedDocumentSet(parent)
{
    QList<IDocument*> docs = ICore::self()->documentController()->openDocuments();
    foreach (IDocument* doc, docs) {
        m_documents.insert(IndexedString(doc->url()));
    }
    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this, &OpenDocumentSet::documentClosed);
    connect(ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this, &OpenDocumentSet::documentCreated);
}

void OpenDocumentSet::documentClosed(IDocument* doc)
{
    if (m_documents.remove(IndexedString(doc->url()))) {
        emit changed();
    }
}

void OpenDocumentSet::documentCreated(IDocument* doc)
{
    m_documents.insert(IndexedString(doc->url()));
    emit changed();
}

ProblemScope OpenDocumentSet::getScope() const
{
    return OpenDocuments;
}

ProjectSet::ProjectSet(QObject *parent)
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
            // can't use new signal/slot syntax here, IProjectFileManager is no a QObject
            connect(fileManager, SIGNAL(fileAdded(ProjectFileItem*)),
                    this, SLOT(fileAdded(ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRemoved(ProjectFileItem*)),
                    this, SLOT(fileRemoved(ProjectFileItem*)));
            connect(fileManager, SIGNAL(fileRenamed(Path,ProjectFileItem*)),
                    this, SLOT(fileRenamed(Path,ProjectFileItem*)));
        }
    }
}

CurrentProjectSet::CurrentProjectSet(const IndexedString& document, QObject *parent)
    : ProjectSet(parent), m_currentProject(0)
{
    setCurrentDocumentInternal(document);
    trackProjectFiles(m_currentProject);
}

void CurrentProjectSet::setCurrentDocument(const IndexedString& url)
{
    setCurrentDocumentInternal(url);
}

void CurrentProjectSet::setCurrentDocumentInternal(const IndexedString& url)
{
    IProject* projectForUrl = ICore::self()->projectController()->findProjectForUrl(url.toUrl());
    if (projectForUrl && projectForUrl != m_currentProject) {
        m_documents.clear();
        m_currentProject = projectForUrl;

        foreach (const IndexedString &indexedString, m_currentProject->fileSet()) {
            m_documents.insert(indexedString);
        }
        emit changed();
    }
}

ProblemScope CurrentProjectSet::getScope() const
{
    return CurrentProject;
}

AllProjectSet::AllProjectSet(QObject *parent)
    : ProjectSet(parent)
{
    foreach(const IProject* project, ICore::self()->projectController()->projects()) {
        foreach (const IndexedString &indexedString, project->fileSet()) {
            m_documents.insert(indexedString);
        }
        trackProjectFiles(project);
    }
}

ProblemScope AllProjectSet::getScope() const
{
    return AllProjects;
}

}


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

#ifndef KDEVPLATFORM_PLUGIN_WATCHEDDOCUMENTSET_H
#define KDEVPLATFORM_PLUGIN_WATCHEDDOCUMENTSET_H

#include <QObject>
#include <QSet>

#include <serialization/indexedstring.h>
#include <shell/shellexport.h>

#include "problemconstants.h"

namespace KDevelop {
class IDocument;
class IProject;
class ProjectFileItem;
class Path;


/**
 * Helper class that tracks set of documents and notifies its owner whenever this set changes. Derived classes implement different tracking strategies.
 */
class KDEVPLATFORMSHELL_EXPORT WatchedDocumentSet : public QObject
{
    Q_OBJECT
public:
    typedef QSet<IndexedString> DocumentSet;
    explicit WatchedDocumentSet(QObject* parent);
    virtual DocumentSet get() const;
    virtual void setCurrentDocument(const IndexedString& url);
    virtual ProblemScope getScope() const = 0;
    ~WatchedDocumentSet() override {}

signals:
    void changed();

protected:
    DocumentSet m_documents;
};

/**
 * Tracks a document that is current at any given moment.
 * When a new file is activated, it becomes tracked instead of the old one.
 */
class CurrentDocumentSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit CurrentDocumentSet(const IndexedString& document, QObject* parent);
    void setCurrentDocument(const IndexedString& url) override;
    ProblemScope getScope() const override;
};

/**
 * Tracks all open documents.
 */
class OpenDocumentSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit OpenDocumentSet(QObject* parent);
    ProblemScope getScope() const override;

private slots:
    void documentClosed(IDocument* doc);
    void documentCreated(IDocument* doc);
};

/**
 * Tracks documents that are in the same project as the current file.
 * If current file is not in any project, none are tracked.
 */
class ProjectSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit ProjectSet(QObject* parent);

protected:
    void trackProjectFiles(const IProject* project);

protected slots:
    void fileAdded(ProjectFileItem*);
    void fileRemoved(ProjectFileItem* file);
    void fileRenamed(const Path& oldFile, ProjectFileItem* newFile);
};

/**
 * Tracks files in all open projects.
 */
class CurrentProjectSet : public ProjectSet
{
    Q_OBJECT
public:
    explicit CurrentProjectSet(const IndexedString& document, QObject* parent);
    void setCurrentDocument(const IndexedString& url) override;
    ProblemScope getScope() const override;

private:
    void setCurrentDocumentInternal(const IndexedString& url); // to avoid virtual in constructor
    IProject* m_currentProject;
};

class AllProjectSet : public ProjectSet
{
    Q_OBJECT
public:
    explicit AllProjectSet(QObject* parent);
    ProblemScope getScope() const override;
};

}

#endif // KDEVPLATFORM_PLUGIN_WATCHEDDOCUMENTSET_H

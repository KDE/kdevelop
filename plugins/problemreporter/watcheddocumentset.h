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

#include "problemmodel.h"

namespace KDevelop {
    class IDocument;
    class IProject;
    class ProjectFileItem;
class Path;
}

/**
 * Helper class for ProblemModel that tracks the set of documents from which errors are listed
 * and notifies its owner whenever this set changes. Derived classes implement different tracking strategies.
 */
class WatchedDocumentSet : public QObject
{
    Q_OBJECT
public:
    typedef QSet<KDevelop::IndexedString> DocumentSet;
    explicit WatchedDocumentSet(ProblemModel* parent);
    ProblemModel* model() const;
    virtual DocumentSet get() const;
    virtual void setCurrentDocument(const KDevelop::IndexedString& url);
    virtual ProblemModel::Scope getScope() const = 0;
    virtual ~WatchedDocumentSet() {}

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
    explicit CurrentDocumentSet(const KDevelop::IndexedString& document, ProblemModel * parent);
    virtual void setCurrentDocument(const KDevelop::IndexedString& url) override;
    virtual ProblemModel::Scope getScope() const override;
};

/**
 * Tracks all open documents.
 */
class OpenDocumentSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit OpenDocumentSet(ProblemModel* parent);
    virtual ProblemModel::Scope getScope() const override;

private slots:
    void documentClosed(KDevelop::IDocument* doc);
    void documentCreated(KDevelop::IDocument* doc);
};

/**
 * Tracks documents that are in the same project as the current file.
 * If current file is not in any project, none are tracked.
 */
class ProjectSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit ProjectSet(ProblemModel* parent);

protected:
    void trackProjectFiles(const KDevelop::IProject* project);

protected slots:
    void fileAdded(KDevelop::ProjectFileItem*);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);
};

/**
 * Tracks files in all open projects.
 */
class CurrentProjectSet : public ProjectSet
{
    Q_OBJECT
public:
    explicit CurrentProjectSet(const KDevelop::IndexedString& document, ProblemModel* parent);
    virtual void setCurrentDocument(const KDevelop::IndexedString& url) override;
    virtual ProblemModel::Scope getScope() const override;

private:
    void setCurrentDocumentInternal(const KDevelop::IndexedString& url); // to avoid virtual in constructor
    KDevelop::IProject* m_currentProject;
};

class AllProjectSet : public ProjectSet
{
    Q_OBJECT
public:
    explicit AllProjectSet(ProblemModel* parent);
    virtual ProblemModel::Scope getScope() const override;
};

#endif // KDEVPLATFORM_PLUGIN_WATCHEDDOCUMENTSET_H

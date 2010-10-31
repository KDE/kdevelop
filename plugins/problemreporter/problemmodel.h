/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef PROBLEMMODEL_H
#define PROBLEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QReadWriteLock>

#include <language/interfaces/iproblem.h>
#include <KUrl>

namespace KDevelop {
    class IDocument;
    class ParseJob;
    class TopDUContext;
}

class ProblemReporterPlugin;
class WatchedDocumentSet;

class ProblemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ProblemModel(ProblemReporterPlugin* parent);
    virtual ~ProblemModel();

    enum Columns {
        Error,
        Source,
        File,
        Line,
        Column,
        LastColumn
    };

    /**
     * Which set of files should the model track for errors. See @WatchedDocumentSet for more details.
     */
    enum Scope {
        CurrentDocument,
        OpenDocuments,
        CurrentProject,
        AllProjects
    };

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    KDevelop::ProblemPointer problemForIndex(const QModelIndex& index) const;
    /**
     * Get problems for @ref url.
     */
    QList<KDevelop::ProblemPointer> getProblems(KDevelop::IndexedString url, bool showImports);
    /**
     * Get merged list of problems for all @ref urls.
     */
    QList<KDevelop::ProblemPointer> getProblems(QSet<KDevelop::IndexedString> urls, bool showImports);
    /**
     * Update list of problems for file @ref url for @ref context.
     * Old problems for @ref url are thrown away.
     * Problems for imports are taken for context only if they are not already present.
     */
    void updateProblems(const KDevelop::IndexedString& url, KDevelop::TopDUContext * context);
    ProblemReporterPlugin* plugin();

public slots:
    void setShowImports(bool showImports);
    void setScope(int scope);   // Use int to be able to use QSignalMapper
    void forceFullUpdate();

private slots:
    void documentSetChanged();
    void setCurrentDocument(KDevelop::IDocument* doc);

private:
    void updateProblemsInternal(KDevelop::TopDUContext * context, const KDevelop::IndexedString& parentUrl);
    void getProblemsInternal(KDevelop::IndexedString url, bool showImports, QSet<KDevelop::IndexedString>& visitedUrls, QList<KDevelop::ProblemPointer>& result);
    void rebuildProblemList();

    ProblemReporterPlugin* m_plugin;

    QList<KDevelop::ProblemPointer> m_problems;

    // document -> list of problems
    typedef QHash<KDevelop::IndexedString, QList<KDevelop::ProblemPointer> > ProblemHash;
    ProblemHash m_topProblems;
    typedef QHash<KDevelop::IndexedString, QSet<KDevelop::IndexedString> > ImportHash;
    ImportHash m_imports;
    QReadWriteLock m_lock;

    KUrl m_currentDocument;  // current document
    bool m_showImports; // include problems from imported documents
    WatchedDocumentSet* m_documentSet;
};

#endif // PROBLEMMODEL_H

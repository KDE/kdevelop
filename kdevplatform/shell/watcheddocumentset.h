/*
    SPDX-FileCopyrightText: 2010 Dmitry Risenberg <dmitry.risenberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
class WatchedDocumentSetPrivate;

/**
 * Helper class that tracks set of documents and notifies its owner whenever this set changes. Derived classes implement different tracking strategies.
 */
class KDEVPLATFORMSHELL_EXPORT WatchedDocumentSet : public QObject
{
    Q_OBJECT
public:
    using DocumentSet = QSet<IndexedString>;

    explicit WatchedDocumentSet(QObject* parent);
    ~WatchedDocumentSet() override;

    bool showImports() const;
    void setShowImports(bool showImports);

    virtual DocumentSet get() const;
    virtual DocumentSet imports() const;

    virtual void setCurrentDocument(const IndexedString& url);
    virtual ProblemScope scope() const = 0;

Q_SIGNALS:
    void changed();

protected:
    const QScopedPointer<class WatchedDocumentSetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(WatchedDocumentSet)
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
    ProblemScope scope() const override;
};

/**
 * Tracks all open documents.
 */
class OpenDocumentSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit OpenDocumentSet(QObject* parent);
    ProblemScope scope() const override;

private Q_SLOTS:
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

protected Q_SLOTS:
    void fileAdded(KDevelop::ProjectFileItem*);
    void fileRemoved(KDevelop::ProjectFileItem* file);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);

protected:
    virtual bool include(const IndexedString& /*url*/) const { return true; }
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
    ProblemScope scope() const override;

private:
    void setCurrentDocumentInternal(const IndexedString& url); // to avoid virtual in constructor
    IProject* m_currentProject;
};

class AllProjectSet : public ProjectSet
{
    Q_OBJECT
public:
    explicit AllProjectSet(QObject* parent);
    ProblemScope scope() const override;

protected:
    enum class InitFlag {
        LoadOnInit,
        SkipLoadOnInit,
    };
    explicit AllProjectSet(InitFlag initFlag, QObject* parent);
    void reload();
};

class DocumentsInPathSet : public AllProjectSet
{
    Q_OBJECT
public:
    explicit DocumentsInPathSet(const QString& path, QObject* parent);
    ProblemScope scope() const override;
    void setPath(const QString& path);

private:
    bool include(const IndexedString& url) const override;

    QString m_path;
};

class DocumentsInCurrentPathSet : public DocumentsInPathSet
{
    Q_OBJECT
public:
    explicit DocumentsInCurrentPathSet(const IndexedString& document, QObject* parent);
    ProblemScope scope() const override;
    void setCurrentDocument(const IndexedString& url) override;
};

class BypassSet : public WatchedDocumentSet
{
    Q_OBJECT
public:
    explicit BypassSet(QObject* parent);

    ProblemScope scope() const override;
};

}

#endif // KDEVPLATFORM_PLUGIN_WATCHEDDOCUMENTSET_H

/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsdialect.h>

#include <serialization/indexedstring.h>
#include <language/duchain/problem.h>
#include <language/duchain/topducontext.h>

#include "duchainexport.h"

namespace KDevelop
{
class SimpleRange;
class RangeInRevision;
}

using SimpleUse = QPair<KDevelop::DUContextPointer, KDevelop::RangeInRevision>;

/**
 * This class wraps the qmljs parser and offers some helper functions
 * that make it simpler to use the parser in KDevelop.
 */
class KDEVQMLJSDUCHAIN_EXPORT ParseSession
{
public:
    /**
     * @return a unique identifier for QML/JS documents.
     */
    static KDevelop::IndexedString languageString();

    /**
     * @return the QML/JS language corresponding to a file suffix
     */
    static QmlJS::Dialect guessLanguageFromSuffix(const QString& path);

    /**
     * Parse the given @p contents.
     *
     * @param url The url for the document you want to parse.
     * @param contents The contents of the document you want to parse.
     */
    ParseSession(const KDevelop::IndexedString& url, const QString& contents, int priority);

    /**
     * @return the URL of this session
     */
    KDevelop::IndexedString url() const;

    /**
     * @return The module name of this file ("/foo/QtQuick.qml" yields "QtQuick")
     */
    QString moduleName() const;

    /**
     * @return true if the document was properly parsed, false otherwise.
     */
    bool isParsedCorrectly() const;

    /**
     * @return the root AST node or null if it failed to parse.
     */
    QmlJS::AST::Node* ast() const;

    /**
     * Add a problem concerning the given range
     */
    void addProblem(QmlJS::AST::Node* node,
                    const QString& message,
                    KDevelop::IProblem::Severity severity = KDevelop::IProblem::Warning);

    /**
     * @return the problems encountered during parsing.
     */
    QList<KDevelop::ProblemPointer> problems() const;

    /**
     * @return the string representation of @p location.
     */
    QString symbolAt(const QmlJS::SourceLocation& location) const;

    /**
     * @return the language of the parsed document.
     */
    QmlJS::Dialect language() const;

    /**
     * @return the comment related to the given source location or an empty string
     */
    QString commentForLocation(const QmlJS::SourceLocation& location) const;

    /**
     * Convert @p location to a KDevelop::RangeInRevision and return that.
     */
    KDevelop::RangeInRevision locationToRange(const QmlJS::SourceLocation& location) const;

    /**
     * Convert @p locationFrom and @p locationTo to a KDevelop::RangeInRevision and return that.
     */
    KDevelop::RangeInRevision locationsToRange(const QmlJS::SourceLocation& locationFrom,
                                               const QmlJS::SourceLocation& locationTo) const;

    /**
     * Range that starts at the end of the first token, and ends at the beginning of the second token
     */
    KDevelop::RangeInRevision locationsToInnerRange(const QmlJS::SourceLocation& locationFrom,
                                                    const QmlJS::SourceLocation& locationTo) const;

    /**
     * @return a range that spans @p fromNode and @p toNode.
     */
    KDevelop::RangeInRevision editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode) const;

    void setContextOnNode(QmlJS::AST::Node* node, KDevelop::DUContext* context);
    KDevelop::DUContext* contextFromNode(QmlJS::AST::Node* node) const;

    /**
     * Return whether all the files included by this file were already present in
     * the DUChain.
     */
    bool allDependenciesSatisfied() const;

    /**
     * Return the context of a given QML file, NULL if this file is not yet known
     * to the DUChain.
     *
     * When a file that exists is passed to this method and the file hasn't yet
     * been parsed, it is queued for parsing, and the current file will also be
     * re-parsed after it.
     */
    KDevelop::ReferencedTopDUContext contextOfFile(const QString &fileName);

    /**
     * Static version of contextOfFile. The @p url parameter is used to trigger
     * a reparse of @p url if @p fileName was not yet in the DUChain
     */
    static KDevelop::ReferencedTopDUContext contextOfFile(const QString& fileName,
                                                          const KDevelop::IndexedString& url,
                                                          int ownPriority);

    /**
     * Schedule for update all the files that depend on this file
     */
    void reparseImporters();

    /**
     * Schedule a document for update using the default flags of QML/JS
     */
    static void scheduleForParsing(const KDevelop::IndexedString& url, int priority);

    /**
     * Dump AST tree to stdout.
     */
    void dumpNode(QmlJS::AST::Node* node) const;

private:
    KDevelop::IndexedString m_url;
    QString m_baseName;
    QmlJS::Document::MutablePtr m_doc;
    int m_ownPriority;
    bool m_allDependenciesSatisfied;

    QList<KDevelop::ProblemPointer> m_problems;
    using NodeToContextHash = QHash<QmlJS::AST::Node*, KDevelop::DUContextPointer>;
    NodeToContextHash m_astToContext;
};

#endif // PARSESESSION_H

/*************************************************************************************
*  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <qmljs/qmljsdocument.h>

#include <serialization/indexedstring.h>
#include <language/duchain/problem.h>
#include <language/duchain/topducontext.h>

#include "duchainexport.h"

namespace KDevelop
{
class SimpleRange;
class RangeInRevision;
}

typedef QPair<KDevelop::DUContextPointer, KDevelop::RangeInRevision> SimpleUse;

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
    static QmlJS::Language::Enum guessLanguageFromSuffix(const QString& path);

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
     * @return The module name of this file ("/foo/QtQuick_1.0.qml" yields "QtQuick")
     */
    QString moduleName() const;

    /**
     * @return The version of this file, or QString if none ("QtQuick_1.0.qml" yields "1.0")
     */
    QString moduleVersion() const;

    /**
     * @return true if the document was properly parsed, false otherwise.
     */
    bool isParsedCorrectly() const;

    /**
     * @return the root AST node or null if it failed to parse.
     */
    QmlJS::AST::Node* ast() const;

    /**
     * @return the problems encountered during parsing.
     */
    QList<KDevelop::ProblemPointer> problems() const;

    /**
     * @return the string representation of @p location.
     */
    QString symbolAt(const QmlJS::AST::SourceLocation& location) const;

    /**
     * @return the language of the parsed document.
     */
    QmlJS::Language::Enum language() const;

    /**
     * @return the comment related to the given source location or an empty string
     */
    QString commentForLocation(const QmlJS::AST::SourceLocation& location) const;

    /**
     * Convert @p location to a KDevelop::RangeInRevision and return that.
     */
    KDevelop::RangeInRevision locationToRange(const QmlJS::AST::SourceLocation& location) const;

    /**
     * Convert @p locationFrom and @p locationTo to a KDevelop::RangeInRevision and return that.
     */
    KDevelop::RangeInRevision locationsToRange(const QmlJS::AST::SourceLocation& locationFrom,
                                               const QmlJS::AST::SourceLocation& locationTo) const;

    /**
     * Range that starts at the end of the first token, and ends at the beginning of the second token
     */
    KDevelop::RangeInRevision locationsToInnerRange(const QmlJS::AST::SourceLocation& locationFrom,
                                                    const QmlJS::AST::SourceLocation& locationTo) const;

    /**
     * @return a range that spans @p fromNode and @p toNode.
     */
    KDevelop::RangeInRevision editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode) const;

    /**
     * Implemented to make the AbstractUseBuilder happy.
     */
    void mapAstUse(QmlJS::AST::Node* node, const SimpleUse& use)
    {
        Q_UNUSED(node);
        Q_UNUSED(use);
    }

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
     *
     * @return The actual priority of the file being queued (this priority can
     *         only be lower then @p priority)
     */
    static void scheduleForParsing(const KDevelop::IndexedString& url, int priority);

    /**
     * Dump AST tree to stdout.
     */
    void dumpNode(QmlJS::AST::Node* node) const;

private:
    KDevelop::IndexedString m_url;
    QString m_baseNameWithoutVersion;
    QString m_version;
    QmlJS::Document::MutablePtr m_doc;
    int m_ownPriority;
    bool m_allDependenciesSatisfied;

    typedef QHash<QmlJS::AST::Node*, KDevelop::DUContextPointer> NodeToContextHash;
    NodeToContextHash m_astToContext;
};

#endif // PARSESESSION_H

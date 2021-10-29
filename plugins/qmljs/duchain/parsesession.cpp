/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parsesession.h"
#include "debugvisitor.h"
#include "cache.h"

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/parser/qmljsengine_p.h>

#include <language/duchain/stringhelpers.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/editor/documentrange.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icore.h>

using namespace KDevelop;

IndexedString ParseSession::languageString()
{
    static const IndexedString langString("QML/JS");
    return langString;
}

bool isSorted(const QList<QmlJS::AST::SourceLocation>& locations)
{
    if (locations.size() <= 1) {
        return true;
    }
    for(int i = 1; i < locations.size(); ++i) {
        if (locations.at(i).begin() <= locations.at(i-1).begin()) {
            return false;
        }
    }
    return true;
}

QmlJS::Dialect ParseSession::guessLanguageFromSuffix(const QString& path)
{
    if (path.endsWith(QLatin1String(".js"))) {
        return QmlJS::Dialect::JavaScript;
    } else if (path.endsWith(QLatin1String(".json"))) {
        return QmlJS::Dialect::Json;
    } else {
        return QmlJS::Dialect::Qml;
    }
}

ParseSession::ParseSession(const IndexedString& url, const QString& contents, int priority)
: m_url(url),
  m_ownPriority(priority),
  m_allDependenciesSatisfied(true)
{
    const QString path = m_url.str();
    m_doc = QmlJS::Document::create(path, guessLanguageFromSuffix(path));
    m_doc->setSource(contents);
    m_doc->parse();
    Q_ASSERT(isSorted(m_doc->engine()->comments()));

    // Parse the module name and the version of url (this is used only when the file
    // is a QML module, but doesn't break for JavaScript files)
    m_baseName = QString::fromUtf8(m_url.byteArray())
        .section(QLatin1Char('/'), -1, -1)                   // Base name
        .section(QLatin1Char('.'), 0, -2);                   // Without extension
}

bool ParseSession::isParsedCorrectly() const
{
    return m_doc->isParsedCorrectly();
}

QmlJS::AST::Node* ParseSession::ast() const
{
    return m_doc->ast();
}

IndexedString ParseSession::url() const
{
    return m_url;
}

QString ParseSession::moduleName() const
{
    return m_baseName;
}

void ParseSession::addProblem(QmlJS::AST::Node* node,
                              const QString& message,
                              IProblem::Severity severity)
{
    ProblemPointer p(new Problem);

    p->setDescription(message);
    p->setSeverity(severity);
    p->setSource(IProblem::SemanticAnalysis);
    p->setFinalLocation(DocumentRange(m_url, editorFindRange(node, node).castToSimpleRange()));

    m_problems << p;
}

QList<ProblemPointer> ParseSession::problems() const
{
    QList<ProblemPointer> problems = m_problems;

    const auto diagnosticMessages = m_doc->diagnosticMessages();
    problems.reserve(problems.size() + diagnosticMessages.size());
    for (const auto& msg : diagnosticMessages) {
        ProblemPointer p(new Problem);
        p->setDescription(msg.message);
        p->setSeverity(IProblem::Error);
        p->setSource(IProblem::Parser);
        p->setFinalLocation(DocumentRange(m_url, locationToRange(msg.loc).castToSimpleRange()));
        problems << p;
    }

    return problems;
}

QString ParseSession::symbolAt(const QmlJS::AST::SourceLocation& location) const
{
    return m_doc->source().mid(location.offset, location.length);
}

QmlJS::Dialect ParseSession::language() const
{
    return m_doc->language();
}

bool compareSourceLocation(const QmlJS::AST::SourceLocation& l,
                           const QmlJS::AST::SourceLocation& r)
{
    return l.begin() < r.begin();
}

QString ParseSession::commentForLocation(const QmlJS::AST::SourceLocation& location) const
{
    // find most recent comment in sorted list of comments
    const QList< QmlJS::AST::SourceLocation >& comments = m_doc->engine()->comments();
    auto it = std::lower_bound(
        comments.constBegin(),
        comments.constEnd(),
        location, compareSourceLocation
    );

    if (it == comments.constBegin()) {
        return QString();
    }

    // lower bound returns the place of insertion,
    // we want the comment before that
    it--;
    RangeInRevision input = locationToRange(location);
    RangeInRevision match = locationToRange(*it);
    if (match.end.line != input.start.line - 1 && match.end.line != input.start.line) {
        return QString();
    }

    ///TODO: merge consecutive //-style comments?
    return formatComment(symbolAt(*it));
}

RangeInRevision ParseSession::locationToRange(const QmlJS::AST::SourceLocation& location) const
{
    const int linesInLocation = m_doc->source().midRef(location.offset, location.length).count(QLatin1Char('\n'));
    return RangeInRevision(location.startLine - 1, location.startColumn - 1,
                           location.startLine - 1 + linesInLocation, location.startColumn - 1 + location.length);
}

RangeInRevision ParseSession::locationsToRange(const QmlJS::AST::SourceLocation& locationFrom,
                                               const QmlJS::AST::SourceLocation& locationTo) const
{
    return RangeInRevision(locationToRange(locationFrom).start,
                           locationToRange(locationTo).end);
}

RangeInRevision ParseSession::locationsToInnerRange(const QmlJS::AST::SourceLocation& locationFrom,
                                                    const QmlJS::AST::SourceLocation& locationTo) const
{
    return RangeInRevision(locationToRange(locationFrom).end,
                           locationToRange(locationTo).start);
}

RangeInRevision ParseSession::editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode) const
{
    return locationsToRange(fromNode->firstSourceLocation(), toNode->lastSourceLocation());
}

void ParseSession::setContextOnNode(QmlJS::AST::Node* node, DUContext* context)
{
    m_astToContext.insert(node, DUContextPointer(context));
}

DUContext* ParseSession::contextFromNode(QmlJS::AST::Node* node) const
{
    return m_astToContext.value(node, DUContextPointer()).data();
}

bool ParseSession::allDependenciesSatisfied() const
{
    return m_allDependenciesSatisfied;
}

ReferencedTopDUContext ParseSession::contextOfFile(const QString& fileName)
{
    ReferencedTopDUContext res = contextOfFile(fileName, m_url, m_ownPriority);

    if (!res) {
        // The file was not yet present in the DUChain, store this information.
        // This will prevent the second parsing pass from running (it would be
        // useless as the file will be re-parsed when res will become available)
        m_allDependenciesSatisfied = false;
    }

    return res;
}

ReferencedTopDUContext ParseSession::contextOfFile(const QString& fileName,
                                                   const KDevelop::IndexedString& url,
                                                   int ownPriority)
{
    if (fileName.isEmpty()) {
        return ReferencedTopDUContext();
    }

    // Get the top context of this module file
    DUChainReadLocker lock;
    IndexedString moduleFileString(fileName);
    ReferencedTopDUContext moduleContext = DUChain::self()->chainForDocument(moduleFileString);

    lock.unlock();
    QmlJS::Cache::instance().addDependency(url, moduleFileString);

    if (!moduleContext) {
        // Queue the file on which we depend with a lower priority than the one of this file
        scheduleForParsing(moduleFileString, ownPriority - 1);

        // Register a dependency between this file and the imported one
        return ReferencedTopDUContext();
    } else {
        return moduleContext;
    }
}

void ParseSession::reparseImporters()
{
    const auto& files = QmlJS::Cache::instance().filesThatDependOn(m_url);
    for (const KDevelop::IndexedString& file : files) {
        scheduleForParsing(file, m_ownPriority);
    }
}

void ParseSession::scheduleForParsing(const IndexedString& url, int priority)
{
    BackgroundParser* bgparser = KDevelop::ICore::self()->languageController()->backgroundParser();
    const auto features = TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsContextsAndUses;

    if (bgparser->isQueued(url)) {
        bgparser->removeDocument(url);
    }

    bgparser->addDocument(url, features, priority, nullptr, ParseJob::FullSequentialProcessing);
}

void ParseSession::dumpNode(QmlJS::AST::Node* node) const
{
    DebugVisitor v(this);
    v.startVisiting(node);
}

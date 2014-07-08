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

#include "parsesession.h"
#include "debugvisitor.h"
#include <qmljs/parser/qmljsast_p.h>

#include <language/editor/simplerange.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icore.h>

#include <kstandarddirs.h>

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

QmlJS::Language::Enum ParseSession::guessLanguageFromSuffix(const QString& path)
{
    if (path.endsWith(".js")) {
        return QmlJS::Language::JavaScript;
    } else if (path.endsWith(".json")) {
        return QmlJS::Language::Json;
    } else {
        return QmlJS::Language::Qml;
    }
}

ParseSession::ParseSession(const IndexedString& url, const QString& contents, int priority)
: m_url(url),
  m_ownPriority(priority)
{
    const QString path = m_url.str();
    m_doc = QmlJS::Document::create(path, guessLanguageFromSuffix(path));
    m_doc->setSource(contents);
    m_doc->parse();
    Q_ASSERT(isSorted(m_doc->engine()->comments()));

    // Parse the module name and the version of url (this is used only when the file
    // is a QML module, but doesn't break for JavaScript files)
    QString baseName = QString::fromUtf8(m_url.byteArray())
        .section('/', -1, -1)                   // Base name
        .section('.', 0, -2);                   // Without extension
    QStringList nameAndVersion = baseName.split('_');

    m_baseNameWithoutVersion = nameAndVersion.at(0);
    m_version = (nameAndVersion.count() > 1 ? nameAndVersion.at(1) : QLatin1String("1.0"));
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
    return m_baseNameWithoutVersion;
}

QString ParseSession::moduleVersion() const
{
    return m_version;
}

QList<ProblemPointer> ParseSession::problems() const
{
    QList<ProblemPointer> problems;

    foreach (const QmlJS::DiagnosticMessage& msg, m_doc->diagnosticMessages()) {
        ProblemPointer p(new Problem);
        p->setDescription(msg.message);
        p->setSeverity(ProblemData::Error);
        p->setSource(ProblemData::Parser);
        p->setFinalLocation(DocumentRange(m_url, locationToRange(msg.loc).castToSimpleRange()));
        problems << p;
    }

    return problems;
}

QString ParseSession::symbolAt(const QmlJS::AST::SourceLocation& location) const
{
    return m_doc->source().mid(location.offset, location.length);
}

QmlJS::Language::Enum ParseSession::language() const
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
    QList< QmlJS::AST::SourceLocation >::const_iterator it = qLowerBound(
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
    const int linesInLocation = m_doc->source().midRef(location.offset, location.length).count('\n');
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

ReferencedTopDUContext ParseSession::contextOfModule(const QString& module)
{
    if (m_moduleToContext.contains(module)) {
        // The TopDUContext of this module has been cached in the parse session,
        // return it immediately without calling potentially slow KStandardDirs methods
        return m_moduleToContext.value(module);
    }

    // Find the .qml file corresponding to the module
    QString moduleFile = KGlobal::dirs()->findResource("data",
        QString("kdevqmljssupport/qmlplugins/%1").arg(module)
    );

    if (moduleFile.isNull()) {
        return ReferencedTopDUContext();
    }

    // Get the top context of this module file
    ReferencedTopDUContext moduleContext = contextOfFile(moduleFile);

    if (moduleContext) {
        m_moduleToContext.insert(module, moduleContext);
    }

    return moduleContext;
}

ReferencedTopDUContext ParseSession::contextOfFile(const QString& fileName)
{
    // Get the top context of this module file
    DUChainReadLocker lock;
    IndexedString moduleFileString(fileName);
    ReferencedTopDUContext moduleContext = DUChain::self()->chainForDocument(moduleFileString);

    lock.unlock();

    if (!moduleContext) {
        // Ask KDevelop to parse the file
        scheduleForParsing(moduleFileString, m_ownPriority - 1);

        // Then reparse this file, the import will exist
        scheduleForParsing(m_url, m_ownPriority);

        return ReferencedTopDUContext();
    } else {
        return moduleContext;
    }
}

void ParseSession::reparseImporters(DUContext* context)
{
    DUChainReadLocker lock;

    for (DUContext* importer : context->importers()) {
        scheduleForParsing(importer->url(), m_ownPriority);
    }
}

void ParseSession::scheduleForParsing(const IndexedString& url, int priority)
{
    BackgroundParser* bgparser = KDevelop::ICore::self()->languageController()->backgroundParser();
    TopDUContext::Features features = (TopDUContext::Features)
        (TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsContextsAndUses);

    if (bgparser->isQueued(url)) {
        if (bgparser->priorityForDocument(url) > priority) {
            // Remove the document and re-queue it with a greater priority
            bgparser->removeDocument(url);
        } else {
            // Document already queued, do nothing
            return;
        }
    }

    bgparser->addDocument(url, features, priority, 0, ParseJob::FullSequentialProcessing);
}

void ParseSession::dumpNode(QmlJS::AST::Node* node) const
{
    DebugVisitor v(this);
    v.startVisiting(node);
}

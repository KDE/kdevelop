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
#include <qmljs/parser/qmljsast_p.h>

#include <language/editor/simplerange.h>
#include <language/interfaces/iproblem.h>

using namespace KDevelop;

SimpleRange ParseSession::locationToSimpleRange(const QmlJS::AST::SourceLocation& location)
{
    return SimpleRange(location.startLine - 1, location.startColumn - 1,
                       location.startLine - 1, location.startColumn - 1 + location.length);
}

RangeInRevision ParseSession::editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode)
{
    return RangeInRevision::castFromSimpleRange(SimpleRange(
        locationToSimpleRange(fromNode->firstSourceLocation()).start,
        locationToSimpleRange(toNode->lastSourceLocation()).end
    ));
}

IndexedString ParseSession::languageString()
{
    static const IndexedString langString("QML/JS");
    return langString;
}

ParseSession::ParseSession(const IndexedString& url, const QString& contents)
: m_url(url)
{
    const QString path = m_url.str();
    m_doc = QmlJS::Document::create(path, QmlJS::Document::guessLanguageFromSuffix(path));
    m_doc->setSource(contents);
    m_doc->parse();
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

QList<ProblemPointer> ParseSession::problems() const
{
    QList<ProblemPointer> problems;
    foreach(const QmlJS::DiagnosticMessage& msg, m_doc->diagnosticMessages()) {
        ProblemPointer p(new Problem);
        p->setDescription(msg.message);
        p->setSeverity(ProblemData::Error);
        p->setSource(ProblemData::Parser);
        p->setFinalLocation(DocumentRange(m_url, locationToSimpleRange(msg.loc)));
        problems << p;
    }
    return problems;
}

QString ParseSession::symbolAt(const QmlJS::AST::SourceLocation& location) const
{
    return m_doc->source().mid(location.offset, location.length);
}

QmlJS::Document::Language ParseSession::language() const
{
    return m_doc->language();
}

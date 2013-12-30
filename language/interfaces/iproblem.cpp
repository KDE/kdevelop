/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "iproblem.h"
#include <duchain/duchainregister.h>
#include <interfaces/iassistant.h>
#include <klocalizedstring.h>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(Problem);
}

using namespace KDevelop;

Problem::Problem()
    : DUChainBase(*new ProblemData)
{
    d_func_dynamic()->setClassId(this);
}

KDevelop::Problem::Problem(KDevelop::ProblemData& data) : DUChainBase(data) {
}

KDevelop::Problem::~Problem() {
}

KDevelop::IndexedString KDevelop::Problem::url() const
{
    return d_func()->url;
}

DocumentRange Problem::finalLocation() const
{
    return DocumentRange(d_func()->url, rangeInCurrentRevision());
}

void Problem::setFinalLocation(const DocumentRange & location)
{
    setRange(transformToLocalRevision(location));
    d_func_dynamic()->url = location.document;
}

QStack< DocumentCursor > Problem::locationStack() const
{
    return QStack< DocumentCursor >();
//     return d_func()->locationStack;
}

void Problem::addLocation(const DocumentCursor & cursor)
{
    Q_UNUSED(cursor);
//     d_func()->locationStack.push(DocumentCursor(cursor));
}

void Problem::clearLocationStack()
{
//     d_func()->locationStack.clear();
}

void Problem::setLocationStack(const QStack< DocumentCursor > & locationStack)
{
    Q_UNUSED(locationStack);
//     d_func()->locationStack = locationStack;
}

QString Problem::description() const
{
    return d_func()->description.str();
}

void Problem::setDescription(const QString & description)
{
    d_func_dynamic()->description = IndexedString(description);
}

QString Problem::explanation() const
{
    return d_func()->explanation.str();
}

void Problem::setExplanation(const QString & explanation)
{
    d_func_dynamic()->explanation = IndexedString(explanation);
}

ProblemData::Source Problem::source() const
{
    return d_func()->source;
}

void Problem::setSource(ProblemData::Source source)
{
    d_func_dynamic()->source = source;
}

KSharedPtr< KDevelop::IAssistant > KDevelop::Problem::solutionAssistant() const {
    return m_solution;
}

void KDevelop::Problem::setSolutionAssistant(KSharedPtr< KDevelop::IAssistant > assistant) {
    m_solution = assistant;
}

KDevelop::ProblemData::Severity KDevelop::Problem::severity() const {
    return d_func()->severity;
}

void KDevelop::Problem::setSeverity(ProblemData::Severity severity) {
    d_func_dynamic()->severity = severity;
}

QString Problem::severityString() const
{
    switch(severity()) {
        case ProblemData::Error:
            return i18n("Error");
        case ProblemData::Warning:
            return i18n("Warning");
        case ProblemData::Hint:
            return i18n("Hint");
    }
    return QString();
}

QString Problem::sourceString() const
{
    switch (source()) {
        case ProblemData::Disk:
            return i18n("Disk");
        case ProblemData::Preprocessor:
            return i18n("Preprocessor");
        case ProblemData::Lexer:
            return i18n("Lexer");
        case ProblemData::Parser:
            return i18n("Parser");
        case ProblemData::DUChainBuilder:
            return i18n("Definition-Use Chain");
        case ProblemData::SemanticAnalysis:
            return i18n("Semantic Analysis");
        case ProblemData::ToDo:
            return i18n("TODO");
        case ProblemData::Unknown:
        default:
            return i18n("Unknown");
    }
}

QString Problem::toString() const
{
    return QString("%9: %1:%2 in %3:[(%4,%5),(%6,%7)] %8")
        .arg(description())
        .arg(sourceString())
        .arg(url().str())
        .arg(range().start.line)
        .arg(range().start.column)
        .arg(range().end.line)
        .arg(range().end.column)
        .arg(explanation())
        .arg(severityString());
}

QDebug operator<<(QDebug s, const Problem& problem)
{
    s.nospace() << problem.toString();
    return s.space();
}

QDebug operator<<(QDebug s, const ProblemPointer& problem)
{
    s.nospace() << problem->toString();
    return s.space();
}

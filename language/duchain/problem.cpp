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

#include "problem.h"

#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "topducontext.h"
#include "topducontextdata.h"
#include "duchain.h"
#include "duchainlock.h"

#include <interfaces/iassistant.h>
#include <KLocalizedString>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(Problem);
DEFINE_LIST_MEMBER_HASH(ProblemData, diagnostics, LocalIndexedProblem)
}

using namespace KDevelop;

LocalIndexedProblem::LocalIndexedProblem(const ProblemPointer& problem, const TopDUContext* top)
  : m_index(problem->m_indexInTopContext)
{
    ENSURE_CHAIN_READ_LOCKED
    // ensure child problems are properly serialized before we serialize the parent problem
    if (static_cast<uint>(problem->m_diagnostics.size()) != problem->d_func()->diagnosticsSize()) {
        // see below, the diagnostic size is kept in sync by the mutable API of Problem
        Q_ASSERT(!problem->diagnostics().isEmpty());
        // the const cast is ugly but we don't really "change" the state as observed from the outside
        auto& serialized = const_cast<Problem*>(problem.data())->d_func_dynamic()->diagnosticsList();
        Q_ASSERT(serialized.isEmpty());
        foreach(const ProblemPointer& child, problem->m_diagnostics) {
            serialized << LocalIndexedProblem(child, top);
        }
    }

    if (!m_index) {
        m_index = top->m_dynamicData->allocateProblemIndex(problem);
    }
}

ProblemPointer LocalIndexedProblem::data(const TopDUContext* top) const
{
  if (!m_index) {
    return {};
  }
  return top->m_dynamicData->getProblemForIndex(m_index);
}

Problem::Problem()
    : DUChainBase(*new ProblemData)
    , m_topContext(nullptr)
    , m_indexInTopContext(0)
{
    d_func_dynamic()->setClassId(this);
}

Problem::Problem(ProblemData& data)
    : DUChainBase(data)
    , m_topContext(nullptr)
    , m_indexInTopContext(0)
{
}

Problem::~Problem()
{
}

TopDUContext* Problem::topContext() const
{
    return m_topContext.data();
}

IndexedString Problem::url() const
{
    return d_func()->url;
}

DocumentRange Problem::finalLocation() const
{
    return DocumentRange(d_func()->url, rangeInCurrentRevision());
}

void Problem::setFinalLocation(const DocumentRange& location)
{
    setRange(transformToLocalRevision(location));
    d_func_dynamic()->url = location.document;
}

QList<ProblemPointer> Problem::diagnostics() const
{
    return m_diagnostics;
}

void Problem::setDiagnostics(const QList<ProblemPointer>& diagnostics)
{
    m_diagnostics = diagnostics;
    // keep serialization in sync, see also LocalIndexedProblem ctor above
    d_func_dynamic()->diagnosticsList().clear();
}

void Problem::addDiagnostic(const ProblemPointer& diagnostic)
{
    m_diagnostics << diagnostic;
}

void Problem::clearDiagnostics()
{
    m_diagnostics.clear();
    // keep serialization in sync, see also LocalIndexedProblem ctor above
    d_func_dynamic()->diagnosticsList().clear();
}

QString Problem::description() const
{
    return d_func()->description.str();
}

void Problem::setDescription(const QString& description)
{
    d_func_dynamic()->description = IndexedString(description);
}

QString Problem::explanation() const
{
    return d_func()->explanation.str();
}

void Problem::setExplanation(const QString& explanation)
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

QExplicitlySharedDataPointer<IAssistant> Problem::solutionAssistant() const
{
    return {};
}

ProblemData::Severity Problem::severity() const
{
    return d_func()->severity;
}

void Problem::setSeverity(ProblemData::Severity severity)
{
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
    return QString("%1: %2 in %3:[(%4,%5),(%6,%7)]: %8 (found by %9)")
        .arg(severityString())
        .arg(description())
        .arg(url().str())
        .arg(range().start.line)
        .arg(range().start.column)
        .arg(range().end.line)
        .arg(range().end.column)
        .arg((explanation().isEmpty() ? QString("<no explanation>") : explanation()))
        .arg(sourceString());
}

void Problem::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
    auto top = dynamic_cast<TopDUContext*>(parent);
    Q_ASSERT(top);

    m_topContext = top;
    m_indexInTopContext = ownIndex;

    // deserialize child diagnostics here, as the top-context might get unloaded
    // but we still want to keep the child-diagnostics in-tact, as one would assume
    // a shared-ptr works.
    const auto data = d_func();
    m_diagnostics.reserve(data->diagnosticsSize());
    for (uint i = 0; i < data->diagnosticsSize(); ++i) {
        m_diagnostics << ProblemPointer(data->diagnostics()[i].data(top));
    }

    DUChainBase::rebuildDynamicData(parent, ownIndex);
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

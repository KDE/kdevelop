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

#include <interfaces/iassistant.h>
#include <KLocalizedString>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(Problem);
DEFINE_LIST_MEMBER_HASH(ProblemData, diagnostics, LocalIndexedProblem)
}

using namespace KDevelop;

LocalIndexedProblem::LocalIndexedProblem(const Problem* problem)
  : m_index(problem->m_indexInTopContext)
{
}

Problem* LocalIndexedProblem::data(const TopDUContext* top) const
{
  if (!m_index) {
    return nullptr;
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
  if (m_topContext) {
    m_topContext->m_dynamicData->clearProblemIndex(this);
  }
}

void Problem::setContext(TopDUContext* context)
{
    Q_ASSERT(!m_topContext);
    Q_ASSERT(!m_indexInTopContext);
    Q_ASSERT(context);
    m_topContext = context;
    m_indexInTopContext = m_topContext->m_dynamicData->allocateProblemIndex(this);
}

TopDUContext* Problem::topContext() const
{
    return m_topContext;
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
    if (!m_topContext || !m_diagnostics.isEmpty()) {
      // child data already deserialized
      return m_diagnostics;
    }

    const auto data = d_func();
    m_diagnostics.reserve(data->diagnosticsSize());
    for (uint i = 0; i < data->diagnosticsSize(); ++i) {
        m_diagnostics << ProblemPointer(data->diagnostics()[i].data(m_topContext));
    }
    return m_diagnostics;
}

void Problem::setDiagnostics(const QList<ProblemPointer>& diagnostics)
{
    clearDiagnostics();

    m_diagnostics = diagnostics;
}

void Problem::addDiagnostic(const ProblemPointer& diagnostic)
{
    m_diagnostics << diagnostic;
}

void Problem::clearDiagnostics()
{
    m_diagnostics.clear();
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

KSharedPtr<IAssistant> Problem::solutionAssistant() const
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
  m_topContext = dynamic_cast<TopDUContext*>(parent);
  Q_ASSERT(m_topContext);
  m_indexInTopContext = ownIndex;

  DUChainBase::rebuildDynamicData(parent, ownIndex);
}

ProblemPointer Problem::prepareStorage(TopDUContext* context)
{
    if (!m_topContext) {
        this->setContext(context);
    } else if (m_topContext != context) {
        // clone problem from another topcontext so we can store its data
        // NOTE: this is ugly and only required since ProblemPointer was used as a shared ptr
        // but the current DUChain serialization mechanism cannot cope with that as it will crash when you
        // try to delete mmapped data e.g. Copying the problem workarounds this limitation nicely and is
        // not bad from a performance POV as only a few problems exist per context usually.
        auto data = DUChainItemSystem::self().cloneData(*d_func());
        Q_ASSERT_X(data, Q_FUNC_INFO, "Failed to clone problem data.");
        ProblemPointer ret(dynamic_cast<Problem*>(DUChainItemSystem::self().create(data)));
        Q_ASSERT_X(ret, Q_FUNC_INFO, "Failed to clone problem.");
        return ret->prepareStorage(context);
    }

    auto data = d_func_dynamic();
    // prepare child diagnostics for storage
    data->diagnosticsList().clear();
    for (auto& child : m_diagnostics) {
        child = child->prepareStorage(context);
        data->diagnosticsList().append(LocalIndexedProblem(child.data()));
    }

    return ProblemPointer(this);
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

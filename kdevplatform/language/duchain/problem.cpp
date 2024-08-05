/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problem.h"

#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "topducontext.h"
#include "topducontextdata.h"
#include "duchain.h"
#include "duchainlock.h"

#include <editor/documentrange.h>

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
    // see below, the diagnostic size is kept in sync by the mutable API of Problem
    // the const cast is ugly but we don't really "change" the state as observed from the outside
    auto& serialized = const_cast<Problem*>(problem.data())->d_func_dynamic()->diagnosticsList();
    serialized.clear();
    serialized.reserve(problem->m_diagnostics.size());
    for (const ProblemPointer& child : std::as_const(problem->m_diagnostics)) {
        serialized << LocalIndexedProblem(child, top);
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
    return top->m_dynamicData->problemForIndex(m_index);
}

Problem::Problem()
    : DUChainBase(*new ProblemData)
{
    d_func_dynamic()->setClassId(this);
}

Problem::Problem(ProblemData& data)
    : DUChainBase(data)
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
    return DocumentRange(d_func()->url, d_func()->m_range.castToSimpleRange());
}

void Problem::setFinalLocation(const DocumentRange& location)
{
    setRange(RangeInRevision::castFromSimpleRange(location));
    d_func_dynamic()->url = location.document;
}

IProblem::FinalLocationMode Problem::finalLocationMode() const
{
    return d_func()->finalLocationMode;
}

void Problem::setFinalLocationMode(IProblem::FinalLocationMode mode)
{
    d_func_dynamic()->finalLocationMode = mode;
}

void Problem::clearDiagnostics()
{
    m_diagnostics.clear();
    // keep serialization in sync, see also LocalIndexedProblem ctor above
    d_func_dynamic()->diagnosticsList().clear();
}

QVector<IProblem::Ptr> Problem::diagnostics() const
{
    QVector<IProblem::Ptr> vector;

    for (const auto& ptr : std::as_const(m_diagnostics)) {
        vector.push_back(ptr);
    }

    return vector;
}

void Problem::setDiagnostics(const QVector<IProblem::Ptr>& diagnostics)
{
    clearDiagnostics();

    for (const IProblem::Ptr& problem : diagnostics) {
        addDiagnostic(problem);
    }
}

void Problem::addDiagnostic(const IProblem::Ptr& diagnostic)
{
    auto* problem = dynamic_cast<Problem*>(diagnostic.data());
    Q_ASSERT(problem != nullptr);

    ProblemPointer ptr(problem);

    m_diagnostics << ptr;
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

IProblem::Source Problem::source() const
{
    return d_func()->source;
}

void Problem::setSource(IProblem::Source source)
{
    d_func_dynamic()->source = source;
}

QExplicitlySharedDataPointer<IAssistant> Problem::solutionAssistant() const
{
    return {};
}

IProblem::Severity Problem::severity() const
{
    return d_func()->severity;
}

void Problem::setSeverity(Severity severity)
{
    d_func_dynamic()->severity = severity;
}

QString Problem::severityString() const
{
    switch (severity()) {
    case IProblem::NoSeverity:
        return {};
    case IProblem::Error:
        return i18n("Error");
    case IProblem::Warning:
        return i18n("Warning");
    case IProblem::Hint:
        return i18n("Hint");
    }
    return QString();
}

QString Problem::sourceString() const
{
    switch (source()) {
    case IProblem::Disk:
        return i18n("Disk");
    case IProblem::Preprocessor:
        return i18n("Preprocessor");
    case IProblem::Lexer:
        return i18n("Lexer");
    case IProblem::Parser:
        return i18n("Parser");
    case IProblem::DUChainBuilder:
        return i18n("Definition-Use Chain");
    case IProblem::SemanticAnalysis:
        return i18n("Semantic analysis");
    case IProblem::ToDo:
        return i18n("To-do");
    case IProblem::Unknown:
    default:
        return i18n("Unknown");
    }
}

QString Problem::toString() const
{
    return i18nc("<severity>: <description> in <url>:[<range>]: <explanation> (found by <source>)",
                 "%1: %2 in %3:[(%4,%5),(%6,%7)]: %8 (found by %9)",
                 severityString(),
                 description(),
                 url().str(),
                 range().start.line,
                 range().start.column,
                 range().end.line,
                 range().end.column,
                 (explanation().isEmpty() ? i18n("<no explanation>") : explanation()),
                 sourceString());
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
    if (!problem) {
        s.nospace() << "<invalid problem>";
    } else {
        s.nospace() << problem->toString();
    }
    return s.space();
}

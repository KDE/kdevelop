/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "clangproblem.h"

#include "util/clangtypes.h"
#include "../debug.h"

#include <language/duchain/duchainlock.h>
#include <language/codegen/documentchangeset.h>

#include <KLocale>

using namespace KDevelop;

namespace {

ProblemData::Severity diagnosticSeverityToSeverity(CXDiagnosticSeverity severity)
{
    switch (severity) {
    case CXDiagnostic_Fatal:
    case CXDiagnostic_Error:
        return ProblemData::Error;
    case CXDiagnostic_Warning:
        return ProblemData::Warning;
        break;
    default:
        return ProblemData::Hint;
    }
}

/**
 * Clang diagnostic messages always start with a lowercase character
 *
 * @return Prettified version, starting with uppercase character
 */
inline QString prettyDiagnosticSpelling(const QString& str)
{
    QString ret = str;
    if (ret.isEmpty()) {
      return {};
    }
    ret[0] = ret[0].toUpper();
    return ret;
}

ClangFixits fixitsForDiagnostic(CXDiagnostic diagnostic)
{
    ClangFixits fixits;
    auto numFixits = clang_getDiagnosticNumFixIts(diagnostic);
    for (uint i = 0; i < numFixits; ++i) {
        CXSourceRange range;
        const QString replacementText = ClangString(clang_getDiagnosticFixIt(diagnostic, i, &range)).toString();


        // TODO: Apparently there's no way to find out the raw text via the C API given a source range
        // Could be useful to pass that into ClangFixit to be sure to replace the correct text
        // cf. DocumentChangeSet.m_oldText
        fixits << ClangFixit{replacementText, ClangRange(range).toDocumentRange(), QString()};
    }
    return fixits;
}

}

QDebug operator<<(QDebug debug, const ClangFixit& fixit)
{
    debug.nospace() << "ClangFixit["
        << "replacementText=" << fixit.replacementText
        << ", range=" << fixit.range
        << ", description=" << fixit.description
        << "]";
    return debug;
}

ClangProblem::ClangProblem(CXDiagnostic diagnostic)
{
    auto severity = diagnosticSeverityToSeverity(clang_getDiagnosticSeverity(diagnostic));
    setSeverity(severity);

    QString description(ClangString(clang_getDiagnosticSpelling(diagnostic)));
    const QString diagnosticOption(ClangString(clang_getDiagnosticOption(diagnostic, nullptr)));
    if (!diagnosticOption.isEmpty()) {
        description.append(QString(" [%1]").arg(diagnosticOption));
    }
    setDescription(prettyDiagnosticSpelling(description));

    ClangLocation location(clang_getDiagnosticLocation(diagnostic));
    CXFile diagnosticFile;
    clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
    const ClangString fileName(clang_getFileName(diagnosticFile));
    DocumentRange docRange(IndexedString(fileName), KTextEditor::Range(location, location));
    const uint numRanges = clang_getDiagnosticNumRanges(diagnostic);
    for (uint i = 0; i < numRanges; ++i) {
        auto range = ClangRange(clang_getDiagnosticRange(diagnostic, i)).toRange();
        if (range.start().line() == docRange.start().line()) {
            docRange.start().setColumn(qMin(range.start().column(), docRange.start().column()));
            docRange.end().setColumn(qMax(range.end().column(), docRange.end().column()));
        }
    }

    setFixits(fixitsForDiagnostic(diagnostic));
    setFinalLocation(docRange);
    setSource(ProblemData::SemanticAnalysis);

    QList<ProblemPointer> diagnostics;
    auto childDiagnostics = clang_getChildDiagnostics(diagnostic);
    auto numChildDiagnostics = clang_getNumDiagnosticsInSet(childDiagnostics);
    for (uint j = 0; j < numChildDiagnostics; ++j) {
        auto childDiagnostic = clang_getDiagnosticInSet(childDiagnostics, j);
        ClangProblem::Ptr problem(new ClangProblem(childDiagnostic));
        diagnostics << ProblemPointer(problem.data());
    }
    setDiagnostics(diagnostics);
}

IAssistant::Ptr ClangProblem::solutionAssistant() const
{
    if (allFixits().isEmpty()) {
        return {};
    }

    return IAssistant::Ptr(new ClangFixitAssistant(allFixits()));
}

ClangFixits ClangProblem::fixits() const
{
    return m_fixits;
}

void ClangProblem::setFixits(const ClangFixits& fixits)
{
    m_fixits = fixits;
}

ClangFixits ClangProblem::allFixits() const
{
    ClangFixits result;
    result << m_fixits;

    for (const ProblemPointer& diagnostic : diagnostics()) {
        const Ptr problem(dynamic_cast<ClangProblem*>(diagnostic.data()));
        Q_ASSERT(problem);
        result << problem->allFixits();
    }
    return result;
}

ClangFixitAssistant::ClangFixitAssistant(const ClangFixits& fixits)
    : m_title(tr("Fix-it Hints"))
    , m_fixits(fixits)
{
}

ClangFixitAssistant::ClangFixitAssistant(const QString& title, const ClangFixits& fixits)
    : m_title(title)
    , m_fixits(fixits)
{
}

QString ClangFixitAssistant::title() const
{
    return m_title;
}

void ClangFixitAssistant::createActions()
{
    KDevelop::IAssistant::createActions();

    for (const ClangFixit& fixit : m_fixits) {
        addAction(IAssistantAction::Ptr(new ClangFixitAction(fixit)));
    }
}

ClangFixits ClangFixitAssistant::fixits() const
{
    return m_fixits;
}

ClangFixitAction::ClangFixitAction(const ClangFixit& fixit)
    : m_fixit(fixit)
{
}

QString ClangFixitAction::description() const
{
    if (!m_fixit.description.isEmpty())
        return m_fixit.description;

    // fallback in case there's no hint for the diagnostic
    // Make sure we don't break on a replacementText such as '#include <foobar>'
    auto formattedReplacement = m_fixit.replacementText;
    formattedReplacement.replace("<", "&amp;lt;").replace(">", "&amp;gt;");

    const auto range = m_fixit.range;
    if (range.start() == range.end()) {
        return i18n("Insert \"%1\" at line: %2, column: %3",
                    formattedReplacement, range.start().line()+1, range.start().column()+1);
    } else if (range.start().line() == range.end().line()) {
        return i18n("Replace text at line: %1, column: %2 with: \"%3\"",
                    range.start().line()+1, range.start().column()+1, formattedReplacement);
    } else {
        return i18n("Replace multiple lines starting at line: %1, column: %2 with: \"%3\"",
                    range.start().line()+1, range.start().column()+1, formattedReplacement);
    }
}

void ClangFixitAction::execute()
{
    DocumentChangeSet changes;
    {
        DUChainReadLocker lock;

        DocumentChange change(m_fixit.range.document, m_fixit.range,
                    QString(), m_fixit.replacementText);
        // TODO: We probably don't want this
        change.m_ignoreOldText = true;
        changes.addChange(change);
    }

    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    changes.applyAllChanges();
    emit executed(this);
}

#include "clangproblem.moc"

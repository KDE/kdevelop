/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangproblem.h"
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

#include "util/clangtypes.h"
#include "util/clangdebug.h"
#include "util/clangutils.h"

#include <language/duchain/duchainlock.h>
#include <language/codegen/documentchangeset.h>

#include <KLocalizedString>

using namespace KDevelop;

namespace {

IProblem::Severity diagnosticSeverityToSeverity(CXDiagnosticSeverity severity, const QString& optionName)
{
    switch (severity) {
    case CXDiagnostic_Fatal:
    case CXDiagnostic_Error:
        return IProblem::Error;
    case CXDiagnostic_Warning:
        if (optionName.startsWith(QLatin1String("-Wunused-"))) {
            return IProblem::Hint;
        }
        return IProblem::Warning;
    default:
        return IProblem::Hint;
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

ClangFixits fixitsForDiagnostic(CXDiagnostic diagnostic, CXTranslationUnit unit)
{
    ClangFixits fixits;
    auto numFixits = clang_getDiagnosticNumFixIts(diagnostic);
    fixits.reserve(numFixits);
    for (uint i = 0; i < numFixits; ++i) {
        CXSourceRange range;
        const QString replacementText = ClangString(clang_getDiagnosticFixIt(diagnostic, i, &range)).toString();
        const auto original = ClangUtils::getRawContents(unit, range);
        fixits << ClangFixit{replacementText, ClangRange(range).toDocumentRange(), QString(), original};
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
        << ", currentText=" << fixit.currentText
        << "]";
    return debug;
}

ClangProblem::ClangProblem() = default;

ClangProblem::ClangProblem(const ClangProblem& other)
    : Problem(),
      m_fixits(other.m_fixits)
{
    setSource(other.source());
    setFinalLocation(other.finalLocation());
    setFinalLocationMode(other.finalLocationMode());
    setDescription(other.description());
    setExplanation(other.explanation());
    setSeverity(other.severity());

    auto diagnostics = other.diagnostics();
    for (auto& diagnostic : diagnostics) {
        auto* clangDiagnostic = dynamic_cast<ClangProblem*>(diagnostic.data());
        Q_ASSERT(clangDiagnostic);
        diagnostic = ClangProblem::Ptr(new ClangProblem(*clangDiagnostic));
    }
    setDiagnostics(diagnostics);
}

ClangProblem::ClangProblem(CXDiagnostic diagnostic, CXTranslationUnit unit)
{
    const QString diagnosticOption = ClangString(clang_getDiagnosticOption(diagnostic, nullptr)).toString();

    auto severity = diagnosticSeverityToSeverity(clang_getDiagnosticSeverity(diagnostic), diagnosticOption);
    setSeverity(severity);

    QString description = ClangString(clang_getDiagnosticSpelling(diagnostic)).toString();
    if (!diagnosticOption.isEmpty()) {
        description.append(QLatin1String(" [") + diagnosticOption + QLatin1Char(']'));
    }
    setDescription(prettyDiagnosticSpelling(description));

    ClangLocation location(clang_getDiagnosticLocation(diagnostic));
    CXFile diagnosticFile;
    clang_getFileLocation(location, &diagnosticFile, nullptr, nullptr, nullptr);
    const ClangString fileName(clang_getFileName(diagnosticFile));
    DocumentRange docRange(IndexedString(QUrl::fromLocalFile(fileName.toString()).adjusted(QUrl::NormalizePathSegments)), KTextEditor::Range(location, location));
    const uint numRanges = clang_getDiagnosticNumRanges(diagnostic);
    for (uint i = 0; i < numRanges; ++i) {
        auto range = ClangRange(clang_getDiagnosticRange(diagnostic, i)).toRange();
        // Note that the second condition is a workaround for seemingly wrong ranges that
        // were observed sometimes. In principle, such a range should be valid.
        if(!range.isValid() || (range.isEmpty() && range.start().line() == 0 && range.start().column() == 0)){
            continue;
        }

        if (range.start() < docRange.start()) {
            docRange.setStart(range.start());
        }
        if (range.end() > docRange.end()) {
            docRange.setEnd(range.end());
        }
    }
    if (docRange.isEmpty()) {
        // try to find a bigger range for the given location by using the token at the given location
        CXFile file = nullptr;
        unsigned line = 0;
        unsigned column = 0;
        clang_getExpansionLocation(location, &file, &line, &column, nullptr);
        // just skip ahead some characters, hoping that it's sufficient to encompass
        // a token we can use for building the range
        auto nextLocation = clang_getLocation(unit, file, line, column + 100);
        auto rangeToTokenize = clang_getRange(location, nextLocation);
        const ClangTokens tokens(unit, rangeToTokenize);
        if (tokens.size()) {
            docRange.setRange(ClangRange(clang_getTokenExtent(unit, tokens.at(0))).toRange());
        }
    }

    setFixits(fixitsForDiagnostic(diagnostic, unit));
    setFinalLocation(docRange);
    setSource(IProblem::SemanticAnalysis);

    QVector<IProblem::Ptr> diagnostics;
    auto childDiagnostics = clang_getChildDiagnostics(diagnostic);
    auto numChildDiagnostics = clang_getNumDiagnosticsInSet(childDiagnostics);
    diagnostics.reserve(numChildDiagnostics);
    for (uint j = 0; j < numChildDiagnostics; ++j) {
        auto childDiagnostic = clang_getDiagnosticInSet(childDiagnostics, j);
        ClangProblem::Ptr problem(new ClangProblem(childDiagnostic, unit));
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

    const auto& diagnostics = this->diagnostics();
    for (const IProblem::Ptr& diagnostic : diagnostics) {
        const Ptr problem(dynamic_cast<ClangProblem*>(diagnostic.data()));
        Q_ASSERT(problem);
        result << problem->allFixits();
    }
    return result;
}

ClangFixitAssistant::ClangFixitAssistant(const ClangFixits& fixits)
    : m_title(i18n("Fix-it Hints"))
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

    for (const ClangFixit& fixit : std::as_const(m_fixits)) {
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

    const auto range = m_fixit.range;
    if (range.start() == range.end()) {
        return i18n("Insert \"%1\" at line: %2, column: %3",
                    m_fixit.replacementText, range.start().line()+1, range.start().column()+1);
    } else if (range.start().line() == range.end().line()) {
        if (m_fixit.currentText.isEmpty()) {
            return i18n("Replace text at line: %1, column: %2 with: \"%3\"",
                    range.start().line()+1, range.start().column()+1, m_fixit.replacementText);
        } else
            return i18n("Replace \"%1\" with: \"%2\"",
                    m_fixit.currentText, m_fixit.replacementText);
    } else {
        return i18n("Replace multiple lines starting at line: %1, column: %2 with: \"%3\"",
                    range.start().line()+1, range.start().column()+1, m_fixit.replacementText);
    }
}

void ClangFixitAction::execute()
{
    DocumentChangeSet changes;
    {
        DUChainReadLocker lock;

        DocumentChange change(m_fixit.range.document, m_fixit.range,
                    m_fixit.currentText, m_fixit.replacementText);
        change.m_ignoreOldText = !m_fixit.currentText.isEmpty();
        changes.addChange(change);
    }

    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    changes.applyAllChanges();
    emit executed(this);
}

#include "moc_clangproblem.cpp"

/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "problemhighlighter.h"

#include <serialization/indexedstring.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/navigation/problemnavigationcontext.h>
#include <language/editor/documentrange.h>

#include <shell/problem.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KColorScheme>

using namespace KTextEditor;
using namespace KDevelop;

namespace
{

QColor colorForSeverity(IProblem::Severity severity)
{
    KColorScheme scheme(QPalette::Active);
    switch (severity) {
    case IProblem::Error:
        return scheme.foreground(KColorScheme::NegativeText).color();
    case IProblem::Warning:
        return scheme.foreground(KColorScheme::NeutralText).color();
    case IProblem::Hint:
    default:
        return scheme.foreground(KColorScheme::PositiveText).color();
    }
}
}

ProblemHighlighter::ProblemHighlighter(KTextEditor::Document* document)
    : m_document(document)
{
    Q_ASSERT(m_document);

    connect(ICore::self()->languageController()->completionSettings(), &ICompletionSettings::settingsChanged, this,
            &ProblemHighlighter::settingsChanged);
    connect(m_document.data(), &Document::aboutToReload, this, &ProblemHighlighter::clearProblems);
    connect(m_document, &Document::aboutToInvalidateMovingInterfaceContent, this, &ProblemHighlighter::clearProblems);
    // This can't use new style connect syntax since aboutToRemoveText is only part of KTextEditor::DocumentPrivate
    connect(m_document, SIGNAL(aboutToRemoveText(KTextEditor::Range)), this,
            SLOT(aboutToRemoveText(KTextEditor::Range)));
}

bool ProblemHighlighter::Settings::operator==(const Settings& other) const noexcept
{
    return highlightSemanticProblems == other.highlightSemanticProblems
        && highlightProblematicLines == other.highlightProblematicLines;
}

auto ProblemHighlighter::readSettings() -> Settings
{
    const auto* const completionSettings = ICore::self()->languageController()->completionSettings();
    return {completionSettings->highlightSemanticProblems(), completionSettings->highlightProblematicLines()};
}

void ProblemHighlighter::settingsChanged()
{
    const auto newSettings = readSettings();
    if (newSettings == m_currentSettings) {
        return; // nothing of interest has changed
    }
    m_currentSettings = newSettings;
    forceSetProblems(m_problems);
}

ProblemHighlighter::~ProblemHighlighter()
{
    if (m_topHLRanges.isEmpty() || !m_document)
        return;

    qDeleteAll(m_topHLRanges);
}

void ProblemHighlighter::setProblems(const QVector<IProblem::Ptr>& problems)
{
    if (problems == m_problems) {
        return;
    }
    forceSetProblems(problems);
}

void ProblemHighlighter::forceSetProblems(const QList<IProblem::Ptr>& problems)
{
    if (!m_document) {
        return;
    }

    const bool hadProblems = !m_problems.isEmpty();
    m_problems = problems;

    qDeleteAll(m_topHLRanges);
    m_topHLRanges.clear();

    /// TODO: create a better MarkInterface that makes it possible to add the marks to the scrollbar
    ///      but having no background.
    ///      also make it nicer together with other plugins, this would currently fail with
    ///      this method...
    const uint errorMarkType = KTextEditor::Document::MarkTypes::Error;
    const uint warningMarkType = KTextEditor::Document::MarkTypes::Warning;
    if (hadProblems) {
        // clear previously added marks
        const auto oldMarks = m_document->marks();
        for (KTextEditor::Mark* mark : oldMarks) {
            if (mark->type & (errorMarkType | warningMarkType)) {
                m_document->removeMark(mark->line, errorMarkType | warningMarkType);
            }
        }
    }

    if (problems.isEmpty()) {
        return;
    }

    const IndexedString url(m_document->url());

    DUChainReadLocker lock;

    TopDUContext* top = DUChainUtils::standardContextForUrl(m_document->url());

    for (const IProblem::Ptr& problem : problems) {
        auto problemFinalLocation = problem->finalLocation();

        if (problemFinalLocation.document != url || !problemFinalLocation.isValid())
            continue;

        KTextEditor::Range range;
        if (top)
            range = top->transformFromLocalRevision(RangeInRevision::castFromSimpleRange(problemFinalLocation));
        else
            range = problemFinalLocation;

        // Fix problem's location range if necessary
        if (problem->finalLocationMode() != IProblem::Range && range.onSingleLine()) {
            int line = range.start().line();
            const QString lineString = m_document->line(line);

            int startColumn = 0;
            int endColumn = lineString.length();

            // If the line contains only space-characters then
            // we will highlight it "as is", without trimming.
            if (problem->finalLocationMode() == IProblem::TrimmedLine && !lineString.trimmed().isEmpty()) {
                while (lineString.at(startColumn++).isSpace()) {}
                --startColumn;

                while (lineString.at(--endColumn).isSpace()) {}
                ++endColumn;
            }

            range.setStart(Cursor(line, startColumn));
            range.setEnd(Cursor(line, endColumn));

            problemFinalLocation.setRange(range);
            problem->setFinalLocation(problemFinalLocation);
            problem->setFinalLocationMode(IProblem::Range);
        }

        if (range.end().line() >= m_document->lines())
            range.end() = KTextEditor::Cursor(m_document->endOfLine(m_document->lines() - 1));

        if (range.isEmpty()) {
            range.setEnd(range.end() + KTextEditor::Cursor(0, 1));
        }

        auto* const problemRange = m_document->newMovingRange(range);
        m_topHLRanges.append(problemRange);

        if (problem->source() != IProblem::ToDo
            && (problem->severity() != IProblem::Hint || m_currentSettings.highlightSemanticProblems)) {
            KTextEditor::Attribute::Ptr attribute(new KTextEditor::Attribute());
            attribute->setUnderlineStyle(QTextCharFormat::WaveUnderline);
            attribute->setUnderlineColor(colorForSeverity(problem->severity()));
            problemRange->setAttribute(attribute);
        }

        if (m_currentSettings.highlightProblematicLines) {
            uint mark;
            if (problem->severity() == IProblem::Error) {
                mark = errorMarkType;
            } else if (problem->severity() == IProblem::Warning) {
                mark = warningMarkType;
            } else {
                continue;
            }
            m_document->addMark(problemFinalLocation.start().line(), mark);
        }
    }
}

void ProblemHighlighter::aboutToRemoveText(const KTextEditor::Range& range)
{
    if (range.onSingleLine()) { // no need to optimize this
        return;
    }

    QList<MovingRange*>::iterator it = m_topHLRanges.begin();
    while (it != m_topHLRanges.end()) {
        if (range.contains((*it)->toRange())) {
            delete (*it);
            it = m_topHLRanges.erase(it);
        } else {
            ++it;
        }
    }
}

void ProblemHighlighter::clearProblems()
{
    setProblems({});
}

#include "moc_problemhighlighter.cpp"

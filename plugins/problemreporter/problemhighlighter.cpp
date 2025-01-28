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

#include <algorithm>

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

void ProblemHighlighter::settingsChanged()
{
    // Re-highlight
    setProblems(m_problems);
}

ProblemHighlighter::~ProblemHighlighter()
{
#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 9, 0)
    // KTextEditor::Document no longer participates in the ownership of its moving ranges
    // since https://commits.kde.org/ktexteditor/3991a497c16373cbb798c22c6a84cdd85486e468
    // first included in KTextEditor version 6.9.
    if (!m_document) {
        // m_document's destructor has already destroyed its moving ranges.
        // Therefore all elements of m_topHLRanges are dangling pointers. Release them.
        for (auto& movingRange : m_topHLRanges) {
            movingRange.release();
        }
    }
#endif
}

void ProblemHighlighter::setProblems(const QVector<IProblem::Ptr>& problems)
{
    if (!m_document)
        return;

    if (m_problems == problems)
        return;

    const bool hadProblems = !m_problems.isEmpty();
    m_problems = problems;

    m_topHLRanges.clear();

    IndexedString url(m_document->url());

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

    DUChainReadLocker lock;

    TopDUContext* top = DUChainUtils::standardContextForUrl(m_document->url());

    for (const IProblem::Ptr& problem : problems) {
        if (problem->finalLocation().document != url || !problem->finalLocation().isValid())
            continue;

        KTextEditor::Range range;
        if (top)
            range = top->transformFromLocalRevision(RangeInRevision::castFromSimpleRange(problem->finalLocation()));
        else
            range = problem->finalLocation();

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

            problem->setFinalLocation(DocumentRange(problem->finalLocation().document, range));
            problem->setFinalLocationMode(IProblem::Range);
        }

        if (range.end().line() >= m_document->lines())
            range.end() = KTextEditor::Cursor(m_document->endOfLine(m_document->lines() - 1));

        if (range.isEmpty()) {
            range.setEnd(range.end() + KTextEditor::Cursor(0, 1));
        }

        auto* const problemRange = m_document->newMovingRange(range);
        m_topHLRanges.push_back(MovingRangePtr{problemRange});

        if (problem->source() != IProblem::ToDo
            && (problem->severity() != IProblem::Hint
                || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems())) {
            KTextEditor::Attribute::Ptr attribute(new KTextEditor::Attribute());
            attribute->setUnderlineStyle(QTextCharFormat::WaveUnderline);
            attribute->setUnderlineColor(colorForSeverity(problem->severity()));
            problemRange->setAttribute(attribute);
        }

        if (ICore::self()->languageController()->completionSettings()->highlightProblematicLines()) {
            uint mark;
            if (problem->severity() == IProblem::Error) {
                mark = errorMarkType;
            } else if (problem->severity() == IProblem::Warning) {
                mark = warningMarkType;
            } else {
                continue;
            }
            m_document->addMark(problem->finalLocation().start().line(), mark);
        }
    }
}

void ProblemHighlighter::aboutToRemoveText(const KTextEditor::Range& range)
{
    if (range.onSingleLine()) { // no need to optimize this
        return;
    }

    const auto newEnd = std::remove_if(m_topHLRanges.begin(), m_topHLRanges.end(), [range](const MovingRangePtr& r) {
        return range.contains(r->toRange());
    });
    m_topHLRanges.erase(newEnd, m_topHLRanges.end());
}

void ProblemHighlighter::clearProblems()
{
    setProblems({});
}

#include "moc_problemhighlighter.cpp"

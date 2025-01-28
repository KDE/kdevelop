/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "probleminlinenoteprovider.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/editor/documentrange.h>

#include <KColorScheme>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QIcon>
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>

using namespace KDevelop;

namespace {
static constexpr int marginColumns = 2;

struct SeverityColors
{
    QColor foreground;
    QColor background;
};

static SeverityColors severityColors(IProblem::Severity severity)
{
    const KColorScheme::ForegroundRole foregroundRole =
        (severity == IProblem::Error) ?   KColorScheme::NegativeText :
        (severity == IProblem::Warning) ? KColorScheme::NeutralText :
        /* IProblem::Hint, default */     KColorScheme::PositiveText;
    const KColorScheme::BackgroundRole backgroundRole =
        (severity == IProblem::Error) ?   KColorScheme::NegativeBackground :
        (severity == IProblem::Warning) ? KColorScheme::NeutralBackground :
        /* IProblem::Hint, default */     KColorScheme::PositiveBackground;

    KColorScheme scheme(QPalette::Active);
    return {
        scheme.foreground(foregroundRole).color(),
        scheme.background(backgroundRole).color()
    };
}

[[nodiscard]] ICompletionSettings::ProblemInlineNotesLevel readProblemInlineNotesLevel()
{
    return ICore::self()->languageController()->completionSettings()->problemInlineNotesLevel();
}

} // unnamed namespace

ProblemInlineNoteProvider::ProblemInlineNoteProvider(KTextEditor::Document* document)
    : m_document{document}
    , m_currentLevel{readProblemInlineNotesLevel()}
{
    auto registerProvider = [this] (KTextEditor::Document*, KTextEditor::View* view) {
        view->registerInlineNoteProvider(this);
    };
    for (auto* view : m_document->views()) {
       registerProvider(m_document, view);
    }
    connect(m_document, &KTextEditor::Document::viewCreated, this,  registerProvider);
    connect(ICore::self()->languageController()->completionSettings(), &ICompletionSettings::settingsChanged,
            this, &ProblemInlineNoteProvider::completionSettingsChanged);
}

ProblemInlineNoteProvider::~ProblemInlineNoteProvider()
{
    if (!m_document) {
        return;
    }
    for (auto* view : m_document->views()) {
        view->unregisterInlineNoteProvider(this);
    }
}

void ProblemInlineNoteProvider::completionSettingsChanged()
{
    const auto newLevel = readProblemInlineNotesLevel();
    if (newLevel == m_currentLevel) {
        return;
    }
    m_currentLevel = newLevel;
    setProblems(m_problems);
}

void ProblemInlineNoteProvider::setProblems(const QVector<IProblem::Ptr>& problems)
{
    if (!m_document) {
        return;
    }
    m_problemForLine.clear();
    m_problems = problems;
    if (problems.isEmpty()) {
        emit inlineNotesReset();
        return;
    }

    if (m_currentLevel == ICompletionSettings::NoProblemsInlineNotesLevel) {
        return;
    }
    for (const IProblem::Ptr& problem : problems) {
        if (problem->finalLocation().document.toUrl() != m_document->url() || !problem->finalLocation().isValid()) {
            continue;
        }
        switch (problem->severity()) {
        case IProblem::NoSeverity:
        case IProblem::Hint:
            if (m_currentLevel != ICompletionSettings::AllProblemsInlineNotesLevel) {
                continue;
            }
            break;
        case IProblem::Warning: 
            if (m_currentLevel == ICompletionSettings::ErrorsProblemInlineNotesLevel) {
                continue;
            }
            break;
        case IProblem::Error:
            break;
        }
        const int line = problem->finalLocation().start().line();
        // Only render the problem with the highest severity in each line.
        if (m_problemForLine.contains(line)) {
            const IProblem::Ptr currentProblem = m_problemForLine.value(line);
            if (problem->severity() == currentProblem->severity()) {
                if (problem->finalLocation().start().column() < currentProblem->finalLocation().start().column()) {
                    m_problemForLine[line] = problem;
                }
            // No Severity has the lowest value
            } else if (problem->severity() < currentProblem->severity() && problem->severity() != IProblem::NoSeverity) {
                m_problemForLine[line] = problem;
            } else if (currentProblem->severity() == IProblem::NoSeverity)  {
                m_problemForLine[line] = problem;
            }
        } else {
             m_problemForLine[line] = problem;
        }
    }
    emit inlineNotesReset();
}

QVector<int> ProblemInlineNoteProvider::inlineNotes(int line) const
{
    return m_problemForLine.contains(line) ? QVector<int>(1, m_document->endOfLine(line).column() + marginColumns)
                                           : QVector<int>();
}

// matching logic of KateIconBorder's icon height calculation for a line
static constexpr int iconTopBottomMargin = 1;

static int iconHeight(const KTextEditor::InlineNote& note)
{
    QFontMetrics fontMetrics(note.font());

    return qMin(fontMetrics.height(), note.lineHeight()) - 2 * iconTopBottomMargin;
}

static constexpr int noteBorderWidth = 2;

struct InlineNoteLayout
{
    int iconSize;
    int iconX;
    int descriptionX;
    int rightMargin;
};


// Design of note:
// following basically the message widget design, but reduced to only a colored border on the left side
//
// |_O_description_
// <vertical borderline><left margin><icon><spacing><description text><right margin>
static void doInlineNoteLayout(const KTextEditor::InlineNote& note,
                               InlineNoteLayout* layout)
{
    const auto* view = note.view();
    const auto* style = view->style();
    const int leftMargin = style->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, view);
    layout->rightMargin = style->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr, view);
    const int noteSpacing = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr, view);

    layout->iconSize = iconHeight(note);

    layout->iconX = noteBorderWidth + leftMargin;
    layout->descriptionX = layout->iconX + layout->iconSize + noteSpacing;
}

QSize ProblemInlineNoteProvider::inlineNoteSize(const KTextEditor::InlineNote& note) const
{
    InlineNoteLayout layout;
    doInlineNoteLayout(note, &layout);

    const auto prob = m_problemForLine[note.position().line()];
    QFont font = note.font();
    font.setItalic(true);
    const QFontMetrics metric(font);
    const QRect boundingRect = metric.boundingRect(prob->description());
    return {layout.descriptionX + boundingRect.width() + layout.rightMargin, note.lineHeight()};
}

void ProblemInlineNoteProvider::paintInlineNote(const KTextEditor::InlineNote& note, QPainter& painter,
                                                Qt::LayoutDirection direction) const
{
    // TODO: https://commits.kde.org/ktexteditor/1ee470de9aea37979836dd3e381cce7bc26344d7 added the third parameter -
    // direction - to KTextEditor::InlineNoteProvider::paintInlineNote(). Should the new argument be used here somehow?
    Q_UNUSED(direction);

    InlineNoteLayout layout;
    doInlineNoteLayout(note, &layout);

    const int line = note.position().line();
    const auto prob = m_problemForLine[note.position().line()];
    QFont font = note.font();
    font.setItalic(true);
    painter.setFont(font);
    const KTextEditor::View* view = note.view();
    // NOTE cursorToCoordinate is relative to (0,0) of the view widget so we have to subtract the x
    // value of the start of the line from it. However it returns also -1 for cursors that have no
    // actual text so we subtract the width of the margin column(s) from the available width
    const int viewWidth = view->textAreaRect().width();
    const int textAreaStart = view->cursorToCoordinate(KTextEditor::Cursor(line, 0)).x();
    const int marginWidth = view->cursorToCoordinate(KTextEditor::Cursor(line, marginColumns)).x() - textAreaStart;
    const int nonTextSize = layout.descriptionX + layout.rightMargin;
    const int lineEnd = view->cursorToCoordinate(KTextEditor::Cursor(line, note.position().column() - marginColumns)).x() - textAreaStart;
    const int availableTextWidth = viewWidth - marginWidth - lineEnd - nonTextSize;
    QString text = painter.fontMetrics().elidedText(prob->description(), Qt::ElideRight, availableTextWidth);
    QIcon icon = IProblem::iconForSeverity(prob->severity());
    // QFontMetrics doesn't provide results that are good enough for painting so we use QPainter::boundingRect here
    QRect boundingRect = painter.boundingRect(QRect(layout.descriptionX, 0, 0, 0), Qt::AlignLeft, text);
    const auto colors = severityColors(prob->severity());
    // background
    painter.setBrush(colors.background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(boundingRect.adjusted(-(layout.descriptionX), 0, layout.rightMargin, 0));
    // borderline
    painter.setBrush(colors.foreground);
    painter.drawRect(QRect(0, 0, noteBorderWidth, note.lineHeight()));
    // icpn
    icon.paint(&painter, layout.iconX, iconTopBottomMargin, layout.iconSize, layout.iconSize, Qt::AlignCenter);
    // text
    painter.setPen(colors.foreground);
    painter.drawText(boundingRect, Qt::AlignLeft, text);
}

#include "moc_probleminlinenoteprovider.cpp"

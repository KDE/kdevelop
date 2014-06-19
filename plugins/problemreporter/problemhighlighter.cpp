/*
 * KDevelop Problem Reporter
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "problemhighlighter.h"

#include <KTextEditor/Document>
#include <KTextEditor/MarkInterface>

#include <language/duchain/indexedstring.h>
#include <ktexteditor/texthintinterface.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/navigation/problemnavigationcontext.h>
#include <language/util/navigationtooltip.h>
#include <ktexteditor/view.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/topducontext.h>
#include <ktexteditor/movinginterface.h>

using namespace KTextEditor;
using namespace KDevelop;

ProblemHighlighter::ProblemHighlighter(KTextEditor::Document* document)
    : m_document(document)
{
    Q_ASSERT(m_document);

    foreach(KTextEditor::View* view, m_document->views())
        viewCreated(document, view);

    connect(m_document, SIGNAL(viewCreated(KTextEditor::Document*,KTextEditor::View*)), this, SLOT(viewCreated(KTextEditor::Document*,KTextEditor::View*)));
    connect(ICore::self()->languageController()->completionSettings(), SIGNAL(settingsChanged(ICompletionSettings*)), this, SLOT(settingsChanged()));
    connect(m_document, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)),
            this, SLOT(aboutToInvalidateMovingInterfaceContent()));
    connect(m_document, SIGNAL(aboutToRemoveText(KTextEditor::Range)),
            this, SLOT(aboutToRemoveText(KTextEditor::Range)));
    connect(m_document, SIGNAL(reloaded(KTextEditor::Document*)),
            this, SLOT(documentReloaded()));
}

void ProblemHighlighter::settingsChanged()
{
    //Re-highlight
    setProblems(m_problems);
}

void ProblemHighlighter::viewCreated(Document* , View* view)
{
    KTextEditor::TextHintInterface* iface = dynamic_cast<KTextEditor::TextHintInterface*>(view);
    if( !iface )
        return;

    connect(view, SIGNAL(needTextHint(KTextEditor::Cursor,QString&)), this, SLOT(textHintRequested(KTextEditor::Cursor,QString&)));
}

void ProblemHighlighter::textHintRequested(const KTextEditor::Cursor& pos, QString& )
{
    KTextEditor::View* view = qobject_cast<KTextEditor::View*>(sender());
    Q_ASSERT(view);

    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>(m_document.data());
    if(moving) {
        ///@todo Sort the ranges when writing them, and do binary search instead of linear
        foreach(MovingRange* range, m_topHLRanges) {
            if(m_problemsForRanges.contains(range) && range->contains(pos))
            {
                //There is a problem which's range contains the cursor
                ProblemPointer problem = m_problemsForRanges[range];
                if (problem->source() == ProblemData::ToDo) {
                    continue;
                }

                KDevelop::AbstractNavigationWidget* widget = new KDevelop::AbstractNavigationWidget;
                widget->setContext(NavigationContextPointer(new ProblemNavigationContext(problem)));

                KDevelop::NavigationToolTip* tooltip = new KDevelop::NavigationToolTip(view, QCursor::pos() + QPoint(20, 40), widget);

                tooltip->resize( widget->sizeHint() + QSize(10, 10) );
                ActiveToolTip::showToolTip(tooltip, 99, "problem-tooltip");
                return;
            }
        }
    }
}

ProblemHighlighter::~ProblemHighlighter()
{
    if(m_topHLRanges.isEmpty() || !m_document)
        return;

    qDeleteAll(m_topHLRanges);
}

void ProblemHighlighter::setProblems(const QList<KDevelop::ProblemPointer>& problems)
{
    if(!m_document)
        return;

    KTextEditor::MovingInterface* iface = dynamic_cast<KTextEditor::MovingInterface*>(m_document.data());
    Q_ASSERT(iface);

    const bool hadProblems = !m_problems.isEmpty();
    m_problems = problems;

    qDeleteAll(m_topHLRanges);
    m_topHLRanges.clear();
    m_problemsForRanges.clear();

    IndexedString url( m_document->url() );

    ///TODO: create a better MarkInterface that makes it possible to add the marks to the scrollbar
    ///      but having no background.
    ///      also make it nicer together with other plugins, this would currently fail with
    ///      this method...
    const uint errorMarkType = KTextEditor::MarkInterface::Error;
    const uint warningMarkType = KTextEditor::MarkInterface::Warning;
    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>(m_document.data());
    if (markIface && hadProblems) {
        // clear previously added marks
        foreach(KTextEditor::Mark* mark, markIface->marks().values()) {
            if (mark->type == errorMarkType || mark->type == warningMarkType) {
                markIface->removeMark(mark->line, mark->type);
            }
        }
    }

    DUChainReadLocker lock;

    TopDUContext* top = DUChainUtils::standardContextForUrl(m_document->url());

    foreach (const KDevelop::ProblemPointer& problem, problems) {
        if (problem->finalLocation().document != url || !problem->finalLocation().isValid())
            continue;

        SimpleRange range;
        if(top)
            range = top->transformFromLocalRevision(RangeInRevision::castFromSimpleRange(problem->finalLocation()));
        else
            range = problem->finalLocation();

        if(range.end.line >= m_document->lines())
            range.end = SimpleCursor(m_document->endOfLine(m_document->lines()-1));

        if(range.isEmpty())
            range.end.column += 1;

        KTextEditor::MovingRange* problemRange = iface->newMovingRange(range.textRange());

        m_problemsForRanges.insert(problemRange, problem);
        m_topHLRanges.append(problemRange);

        if(problem->source() != ProblemData::ToDo && (problem->severity() != ProblemData::Hint
            || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems()))
        {
            KTextEditor::Attribute::Ptr error(new KTextEditor::Attribute());
            if(problem->severity() == ProblemData::Error)
                error->setUnderlineColor(Qt::red);
            else if(problem->severity() == ProblemData::Warning)
                error->setUnderlineColor(Qt::darkYellow);
            else if(problem->severity() == ProblemData::Hint)
                error->setUnderlineColor(Qt::yellow);

            error->setUnderlineStyle(QTextCharFormat::WaveUnderline);

#if 0
            KTextEditor::Attribute::Ptr dyn(new KTextEditor::Attribute());
            QColor col(Qt::red);
            col.setAlpha(40);
            dyn->setBackground(col);
            error->setDynamicAttribute(Attribute::ActivateMouseIn, dyn);
            error->setDynamicAttribute(Attribute::ActivateCaretIn, dyn);
#endif

            problemRange->setAttribute(error);
        }

        if (markIface && ICore::self()->languageController()->completionSettings()->highlightProblematicLines()) {
            uint mark;
            if (problem->severity() == ProblemData::Error) {
                mark = errorMarkType;
            } else if (problem->severity() == ProblemData::Warning) {
                mark = warningMarkType;
            } else {
                continue;
            }
            markIface->addMark(problem->finalLocation().start.line, mark);
        }
    }
}

void ProblemHighlighter::aboutToInvalidateMovingInterfaceContent()
{
    qDeleteAll(m_topHLRanges);
    m_topHLRanges.clear();
    m_problemsForRanges.clear();
}

void ProblemHighlighter::aboutToRemoveText( const KTextEditor::Range& range )
{
    if (range.onSingleLine()) { // no need to optimize this
        return;
    }

    QList< MovingRange* >::iterator it = m_topHLRanges.begin();
    while(it != m_topHLRanges.end()) {
        if (range.contains((*it)->toRange())) {
            m_problemsForRanges.remove(*it);
            delete (*it);
            it = m_topHLRanges.erase(it);
        } else {
            ++it;
        }
    }
}

void ProblemHighlighter::documentReloaded()
{
    setProblems(m_problems);
}

#include "problemhighlighter.moc"

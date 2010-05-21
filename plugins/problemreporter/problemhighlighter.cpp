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
#include <language/editor/hashedstring.h>

#include <language/editor/editorintegrator.h>
#include <ktexteditor/texthintinterface.h>
#include <ktexteditor/smartinterface.h>
#include <qwidget.h>
#include <ktextbrowser.h>
#include <qboxlayout.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/navigation/problemnavigationcontext.h>
#include <language/util/navigationtooltip.h>
#include <ktexteditor/view.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

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

    connect(view, SIGNAL(needTextHint(const KTextEditor::Cursor&, QString&)), this, SLOT(textHintRequested(const KTextEditor::Cursor&, QString&)));
}

void ProblemHighlighter::textHintRequested(const KTextEditor::Cursor& pos, QString& )
{
    KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(m_document.data());
    if(smart) {
        QMutexLocker lock(smart->smartMutex());
        foreach(SmartRange* range, m_topHLRanges) {
            SmartRange* deepestRange = range->deepestRangeContaining(pos);
            if(m_problemsForRanges.contains(deepestRange))
            {
                ProblemPointer problem = m_problemsForRanges[deepestRange];
                
                lock.unlock();
                
                KDevelop::AbstractNavigationWidget* widget = new KDevelop::AbstractNavigationWidget;
                widget->setContext(NavigationContextPointer(new ProblemNavigationContext(problem)));
                
                KDevelop::NavigationToolTip* tooltip = new KDevelop::NavigationToolTip(0, QCursor::pos() + QPoint(20, 40), widget);
                
                tooltip->resize( widget->sizeHint() + QSize(10, 10) );
                ActiveToolTip::showToolTip(tooltip, 99, "problem-tooltip");
                
                //There is a problem that contains the smart-range
                return;
            }
        }
    }
}

ProblemHighlighter::~ProblemHighlighter()
{
    if(m_topHLRanges.isEmpty() || !m_document)
        return;

    KDevelop::EditorIntegrator editor;
    editor.setCurrentUrl(IndexedString(m_document->url()), true);

    LockedSmartInterface iface = editor.smart();
    if (iface)
        qDeleteAll(m_topHLRanges);
}

void ProblemHighlighter::setProblems(const QList<KDevelop::ProblemPointer>& problems)
{
    if(!m_document)
        return;
    
    KDevelop::EditorIntegrator editor;
    IndexedString url(m_document->url());
    editor.setCurrentUrl(url, true);

    LockedSmartInterface iface = editor.smart();
    if (!iface)
        return;
    
    const bool hadProblems = !m_problems.isEmpty();
    m_problems = problems;

    qDeleteAll(m_topHLRanges);
    m_topHLRanges.clear();
    m_problemsForRanges.clear();

    KTextEditor::SmartRange* topRange = editor.topRange(iface, EditorIntegrator::Highlighting);
    m_topHLRanges.append(topRange);

    HashedString hashedUrl = url.str();

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

    foreach (const KDevelop::ProblemPointer& problem, problems) {
        if (problem->finalLocation().document() != hashedUrl || !problem->finalLocation().isValid())
            continue;

        DocumentRange range = problem->finalLocation();
        
        if(range.end().line() >= m_document->lines())
            range.end() = m_document->endOfLine(m_document->lines()-1);
        
        KTextEditor::SmartRange* problemRange = editor.createRange(iface, range);
        
        m_problemsForRanges.insert(problemRange, problem);
        
//         *range = problem->finalLocation();
        if (problemRange->isEmpty())
            problemRange->smartEnd().advance(1);

        if(problem->severity() != ProblemData::Hint || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems()) {
        
            KTextEditor::Attribute::Ptr error(new KTextEditor::Attribute());
            if(problem->severity() == ProblemData::Error)
                error->setUnderlineColor(Qt::red);
            else if(problem->severity() == ProblemData::Warning)
                error->setUnderlineColor(Qt::magenta);
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
        problemRange->addWatcher(this);
        editor.exitCurrentRange(iface);
        
        if (markIface && ICore::self()->languageController()->completionSettings()->highlightProblematicLines()) {
            uint mark;
            if (problem->severity() == ProblemData::Error) {
                mark = errorMarkType;
            } else if (problem->severity() == ProblemData::Warning) {
                mark = warningMarkType;
            } else {
                continue;
            }
            markIface->addMark(problem->finalLocation().start().line(), mark);
        }
    }
    
    editor.exitCurrentRange(iface);
}

void ProblemHighlighter::rangeDeleted(KTextEditor::SmartRange *range)
{
    m_topHLRanges.removeAll(range);
}

void ProblemHighlighter::rangeContentsChanged(KTextEditor::SmartRange* range)
{
    range->setAttribute(KTextEditor::Attribute::Ptr());
}

#include "problemhighlighter.moc"

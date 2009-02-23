/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "breakpointcontroller.h"

#include "newbreakpoint.h"
#include "breakpoints.h"
#include "gdbcontroller.h"
#include "gdbcommand.h"
#include "breakpoint.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/document.h>
#include <KParts/PartManager>
#include <KDebug>

#include "util/treeitem.h"
#include "util/treemodel.h"

// #include "modeltest.h"

using namespace GDBDebugger;
using namespace GDBMI;
using namespace KTextEditor;

static int m_activeFlag = 0;

BreakpointController::BreakpointController(GDBController* parent)
    : IBreakpointController(parent, new Breakpoints(this, parent))
{
    connect(KDevelop::ICore::self()->partController(),
            SIGNAL(partAdded(KParts::Part*)),
            this,
            SLOT(slotUpdateBreakpointMarks(KParts::Part*)));

    // FIXME: this, presumably, should be in generic breakpoint controller.
    connect (KDevelop::ICore::self()->documentController(), 
	     SIGNAL( textDocumentCreated( KDevelop::IDocument* ) ), 
	     this, 
	     SLOT( textDocumentCreated( KDevelop::IDocument* ) ) );


    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(parent,     SIGNAL(event(event_t)),
            this,       SLOT(slotEvent(event_t)));
}

GDBController * BreakpointController::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

void BreakpointController::slotEvent(event_t e)
{
    if (e == program_running ||
        e == program_exited ||
        e == debugger_exited)
    {
        clearExecutionPoint();
    }

    switch(e)
    {
    case program_state_changed:
        {
            breakpointsItem()->update();
        }

    case connected_to_program:
        {
            kDebug(9012) << "connected to program";
            dynamic_cast<Breakpoints*>(breakpointsItem())->sendToGDB();
            #if 0
            foreach (Breakpoint* bp, breakpoints())
            {
                if ( (bp->dbgId() == -1 ||  bp->isPending())
                        && !bp->isDbgProcessing()
                        && bp->isValid())
                {
                    bp->sendToGdb();
                }
            }
            #endif
            break;
        }
    case program_exited:
        {
            #if 0
            foreach (Breakpoint* b, breakpoints())
            {
                b->applicationExited();
            }
            #endif
        }
    case debugger_exited:
        breakpointsItem()->markOut();
        break;
    default:
        ;
    }
}

void BreakpointController::handleBreakpointList(const GDBMI::ResultRecord& r)
{
#if 0
    m_activeFlag++;

    const GDBMI::Value& blist = r["BreakpointTable"]["body"];

    for(int i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& b = blist[i];

        int id = b["number"].literal().toInt();
        Breakpoint* bp = findBreakpointById(id);
        if (bp)
        {
            bp->setActive(m_activeFlag, id);
            bp->setHits(b["times"].toInt());
            if (b.hasField("ignore"))
                bp->setIgnoreCount(b["ignore"].toInt());
            else
                bp->setIgnoreCount(0);
            if (b.hasField("cond"))
                bp->setConditional(b["cond"].literal());
            else
                bp->setConditional(QString::null);

            // TODO: make the above functions do this instead
            bp->notifyModified();
        }
        else
        {
            // It's a breakpoint added outside, most probably
            // via gdb console. Add it now.
            QString type = b["type"].literal();

            if (type == "breakpoint" || type == "hw breakpoint")
            {
                if (b.hasField("fullname") && b.hasField("line"))
                {
                    Breakpoint* bp = new FilePosBreakpoint(this,
                        b["fullname"].literal(),
                        b["line"].literal().toInt());

                    bp->setActive(m_activeFlag, id);
                    bp->setActionAdd(false);
                    bp->setPending(false);

                    addBreakpoint(bp);
                }
            }

        }
    }

    // Remove any inactive breakpoints.
    foreach (Breakpoint* breakpoint, breakpoints())
    {
        if (!(breakpoint->isActive(m_activeFlag)))
        {
            // FIXME: need to review is this happens for
            // as-yet unset breakpoint.
            breakpoint->removedInGdb();
        }
    }
#endif
}

#if 0
void BreakpointController::savePartialProjectSession(QDomElement* el)
{
    /*QDomDocument domDoc = el->ownerDocument();
    if (domDoc.isNull())
        return;

    QDomElement breakpointListEl = domDoc.createElement("breakpointList");
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr =
            (BreakpointTableRow *) m_table->item(row, Control);
        Breakpoint* bp = btr->breakpoint();

        QDomElement breakpointEl =
            domDoc.createElement("breakpoint"+QString::number(row));

        breakpointEl.setAttribute("type", bp->type());
        breakpointEl.setAttribute("location", bp->location(false));
        breakpointEl.setAttribute("enabled", bp->isEnabled());
        breakpointEl.setAttribute("condition", bp->conditional());
        breakpointEl.setAttribute("tracingEnabled",
                                  QString::number(bp->tracingEnabled()));
        breakpointEl.setAttribute("traceFormatStringEnabled",
                                  QString::number(bp->traceFormatStringEnabled()));
        breakpointEl.setAttribute("tracingFormatString",
                                  bp->traceFormatString());

        QDomElement tracedExpressions =
            domDoc.createElement("tracedExpressions");

        QStringList::const_iterator i, e;
        for(i = bp->tracedExpressions().begin(),
                e = bp->tracedExpressions().end();
            i != e; ++i)
        {
            QDomElement expr = domDoc.createElement("expression");
            expr.setAttribute("value", *i);
            tracedExpressions.appendChild(expr);
        }

        breakpointEl.appendChild(tracedExpressions);

        breakpointListEl.appendChild(breakpointEl);
    }

    if (!breakpointListEl.isNull())
        el->appendChild(breakpointListEl);*/
}

/***************************************************************************/

void BreakpointController::restorePartialProjectSession(const QDomElement* el)
{
    /** Eventually, would be best to make each breakpoint type handle loading/
        saving it's data. The only problem is that on load, we need to allocate
        with new different objects, depending on type, and that requires some
        kind of global registry. Gotta find out if a solution for that exists in
        KDE (Boost.Serialization is too much dependency, and rolling my own is
        boring).
    */
    /*QDomElement breakpointListEl = el->namedItem("breakpointList").toElement();
    if (!breakpointListEl.isNull())
    {
        QDomElement breakpointEl;
        for (breakpointEl = breakpointListEl.firstChild().toElement();
                !breakpointEl.isNull();
                breakpointEl = breakpointEl.nextSibling().toElement())
        {
            Breakpoint* bp=0;
            BP_TYPES type = (BP_TYPES) breakpointEl.attribute( "type", "0").toInt();
            switch (type)
            {
            case BP_TYPE_FilePos:
            {
                bp = new FilePosBreakpoint();
                break;
            }
            case BP_TYPE_Watchpoint:
            {
                bp = new Watchpoint("");
                break;
            }
            default:
                break;
            }

            // Common settings for any type of breakpoint
            if (bp)
            {
                bp->setLocation(breakpointEl.attribute( "location", ""));
                if (type == BP_TYPE_Watchpoint)
                {
                    bp->setEnabled(false);
                }
                else
                {
                    bp->setEnabled(
                        breakpointEl.attribute( "enabled", "1").toInt());
                }
                bp->setConditional(breakpointEl.attribute( "condition", ""));

                bp->setTracingEnabled(
                    breakpointEl.attribute("tracingEnabled", "0").toInt());
                bp->setTraceFormatString(
                    breakpointEl.attribute("tracingFormatString", ""));
                bp->setTraceFormatStringEnabled(
                    breakpointEl.attribute("traceFormatStringEnabled", "0")
                    .toInt());

                QDomNode tracedExpr =
                    breakpointEl.namedItem("tracedExpressions");

                if (!tracedExpr.isNull())
                {
                    QStringList l;

                    for(QDomNode c = tracedExpr.firstChild(); !c.isNull();
                        c = c.nextSibling())
                    {
                        QDomElement el = c.toElement();
                        l.push_back(el.attribute("value", ""));
                    }
                    bp->setTracedExpressions(l);
                }

                // Now add the breakpoint. Don't try to check if
                // breakpoint already exists.
                // It's easy to check that breakpoint on the same
                // line already exists, but it might have different condition,
                // and checking conditions for equality is too complex thing.
                // And anyway, it's will be suprising of realoading a project
                // changes the set of breakpoints.
                addBreakpoint(bp);
            }
        }
    }*/
}
#endif

FilePosBreakpoint * BreakpointController::findBreakpoint(const QString & file, int line) const
{
    foreach (Breakpoint* bp, breakpoints())
        if (FilePosBreakpoint* fbp = qobject_cast<FilePosBreakpoint*>(bp))
            if (fbp->fileName() == file && fbp->lineNum() == line)
                return fbp;

    return 0;
}

Watchpoint * BreakpointController::findWatchpoint(const QString & variableName) const
{
    foreach (Breakpoint* bp, breakpoints())
        if (Watchpoint* wp = qobject_cast<Watchpoint*>(bp))
            if (wp->varName() == variableName)
                return wp;

    return 0;
}

Breakpoint* BreakpointController::findBreakpointById(int id) const
{
    foreach (Breakpoint* bp, breakpoints())
        if (bp->dbgId() == id)
            return bp;

    return 0;
}

Watchpoint* BreakpointController::findWatchpointByAddress(quint64 address) const
{
    foreach (Breakpoint* breakpoint, breakpoints())
        if (Watchpoint* w = qobject_cast<Watchpoint*>(breakpoint))
            if (w->address() == address)
                return w;

    return false;
}

void BreakpointController::slotToggleBreakpoint(const KUrl& url, const KTextEditor::Cursor& cursor)
{
    slotToggleBreakpoint(url.path(), cursor.line() + 1);
}

void BreakpointController::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = findBreakpoint(fileName, lineNum);

    if (fpBP)
        removeBreakpoint(fpBP);
    else
        addBreakpoint(new FilePosBreakpoint(this, fileName, lineNum));
}


void BreakpointController::slotBreakpointModified(Breakpoint* b)
{
    if (b->isActionDie())
    {
        // Breakpoint was deleted, kill the table row.
        removeBreakpoint(b);
    }
    else
    {
#if 0
        emit dataChanged(indexForBreakpoint(b, 0), indexForBreakpoint(b, Last));
#endif
    }
}


void BreakpointController::removeAllBreakpoints()
{
    foreach (Breakpoint* breakpoint, breakpoints())
        breakpoint->remove();

    m_breakpoints.clear();
    reset();
}


void BreakpointController::adjustMark(Breakpoint* bp, bool add)
{
    if (FilePosBreakpoint* fbp = qobject_cast<FilePosBreakpoint*>(bp)) {
        if (KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->documentForUrl(KUrl(fbp->fileName()))) {
            if (MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument())) {
                // Temporarily block so we don't get confused by receiving extra
                // marksChanged signals
                document->textDocument()->blockSignals(true);

                iface->removeMark( fbp->lineNum() - 1, BreakpointMark | ActiveBreakpointMark | ReachedBreakpointMark | DisabledBreakpointMark );

                if (add) {
                    uint markType = BreakpointMark;
                    if( !bp->isPending() )
                        markType |= ActiveBreakpointMark;
                    if( !bp->isEnabled() )
                        markType |= DisabledBreakpointMark;
                    iface->addMark( fbp->lineNum() - 1, markType );
                }

                document->textDocument()->blockSignals(false);
            }
        }
    }
}

void BreakpointController::removeBreakpoint(Breakpoint* bp)
{
    if (!bp)
        return;

    int row = m_breakpoints.indexOf(bp);
    Q_ASSERT(row != -1);
    if (row == -1)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_breakpoints.removeAt(row);
    endRemoveRows();

    adjustMark(bp, false);

    bp->remove();
}

Breakpoint* BreakpointController::addBreakpoint(Breakpoint *bp)
{
    beginInsertRows(QModelIndex(), m_breakpoints.count(), m_breakpoints.count());

    m_breakpoints.append(bp);

    endInsertRows();

    connect(bp, SIGNAL(modified(Breakpoint*)),
            this, SLOT(slotBreakpointModified(Breakpoint*)));
    connect(bp, SIGNAL(enabledChanged(Breakpoint*)),
            this, SLOT(slotBreakpointEnabledChanged(Breakpoint*)));

    bp->sendToGdb();

    adjustMark(bp, true);

    return bp;
}

void BreakpointController::slotUpdateBreakpointMarks(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part))
    {
        if( !dynamic_cast<MarkInterface*>(doc))
            return;

        // When a file is loaded then we need to tell the editor (display window)
        // which lines contain a breakpoint.
        foreach (Breakpoint* breakpoint, breakpoints())
            adjustMark(breakpoint, true);
    }
}

void BreakpointController::slotBreakpointEnabledChanged(Breakpoint * b)
{
    adjustMark(b, true);
}

void BreakpointController::textDocumentCreated(KDevelop::IDocument* doc)
{
    KTextEditor::MarkInterface *iface =
        qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());

    if( iface ) {
        connect (doc->textDocument(), SIGNAL(
                     markChanged(KTextEditor::Document*, 
                                 KTextEditor::Mark, 
                                 KTextEditor::MarkInterface::MarkChangeAction)),
                 this,
                 SLOT(markChanged(KTextEditor::Document*, 
                                 KTextEditor::Mark, 
                                  KTextEditor::MarkInterface::MarkChangeAction)));
    }
}

void BreakpointController::markChanged(
    KTextEditor::Document* doc, 
    KTextEditor::Mark mark, 
    KTextEditor::MarkInterface::MarkChangeAction action)
{
    if (action == KTextEditor::MarkInterface::MarkAdded)
    {
        // FIXME: check that there's no breakpoint at this line already?
        universe_->addCodeBreakpoint(doc->url().path(KUrl::RemoveTrailingSlash) 
                                     + ":" + QString::number(mark.line));
    }
    else
    {
        kDebug() << "It'd remove a breakpoint, but it's not implemented yet\n";
    }
}


#include "breakpointcontroller.moc"

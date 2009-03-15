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

#include "ibreakpointcontroller.h"
#include "inewbreakpoint.h"
#include "ibreakpoints.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <ktexteditor/markinterface.h>

#include <QPixmap>
#include <KIcon>
#include <KParts/PartManager>
#include <KDebug>
#include <KLocale>

#include <ktexteditor/document.h>

using namespace KDevelop;
using namespace KTextEditor;

IBreakpointController::IBreakpointController(QObject* parent, IBreakpoints* universe)
    : TreeModel(QVector<QString>() << "" << "" << "Type" << "Location" << "Condition", parent), universe_(universe)
{
    setRootItem(universe_);
    universe_->load();
    universe_->createHelperBreakpoint();

    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            universe_, SLOT(save()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            universe_, SLOT(save()));
    connect(this, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            universe_, SLOT(save()));

    //new ModelTest(this, this);

    if (KDevelop::ICore::self()) {
        foreach(KParts::Part* p, KDevelop::ICore::self()->partController()->parts())
            slotPartAdded(p);
        connect(KDevelop::ICore::self()->partController(),
                SIGNAL(partAdded(KParts::Part*)),
                this,
                SLOT(slotPartAdded(KParts::Part*)));
    }
}

void IBreakpointController::slotPartAdded(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part))
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(doc);
        if( !iface )
            return;
        
        iface->setMarkDescription((MarkInterface::MarkTypes)BreakpointMark, i18n("Breakpoint"));
        iface->setMarkPixmap((MarkInterface::MarkTypes)BreakpointMark, *inactiveBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ActiveBreakpointMark, *activeBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ReachedBreakpointMark, *reachedBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)DisabledBreakpointMark, *disabledBreakpointPixmap());
        iface->setMarkPixmap((MarkInterface::MarkTypes)ExecutionPointMark, *executionPointPixmap());
        iface->setEditableMarks( BookmarkMark | BreakpointMark );
#if 0
        connect( doc, 
                 SIGNAL(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)), this, SLOT(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)) );
#endif
    }
}

QVariant 
IBreakpointController::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{ 
    if (orientation == Qt::Horizontal && role == Qt::DecorationRole
        && section == 0)
        return KIcon("dialog-ok-apply");
    else if (orientation == Qt::Horizontal && role == Qt::DecorationRole
             && section == 1)
        return KIcon("system-switch-user");

    return TreeModel::headerData(section, orientation, role);
}

Qt::ItemFlags IBreakpointController::flags(const QModelIndex &index) const
{
    /* FIXME: all this logic must be in item */
    if (!index.isValid())
        return 0;

    if (index.column() == 0)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable 
            | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

    if (index.column() == INewBreakpoint::location_column 
        || index.column() == INewBreakpoint::condition_column)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    return static_cast<Qt::ItemFlags>(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void IBreakpointController::clearExecutionPoint()
{
    kDebug(9012) << "clearExecutionPoint";
    foreach (KDevelop::IDocument* document, KDevelop::ICore::self()->documentController()->openDocuments())
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
        if (!iface)
            continue;

        QHashIterator<int, KTextEditor::Mark*> it = iface->marks();
        while (it.hasNext())
        {
            Mark* mark = it.next().value();
            if( mark->type & ExecutionPointMark )
                iface->removeMark( mark->line, ExecutionPointMark );
        }
    }
}

void IBreakpointController::gotoExecutionPoint(const KUrl &url, int lineNum)
{
    clearExecutionPoint();
    kDebug(9012) << "gotoExecutionPoint";
    KDevelop::IDocument* document = KDevelop::ICore::self()
        ->documentController()
        ->openDocument(url, KTextEditor::Cursor(lineNum, 0));

    if( !document )
        return;

    MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
    if( !iface )
        return;

    document->textDocument()->blockSignals(true);
    iface->addMark( lineNum, ExecutionPointMark );
    document->textDocument()->blockSignals(false);
}

void IBreakpointController::markChanged(
    KTextEditor::Document *document, 
    KTextEditor::Mark mark, 
    KTextEditor::MarkInterface::MarkChangeAction action)
{
#if 0
    int type = mark.type;
    /* Is this a breakpoint mark, to begin with? */
    if (type != (MarkInterface::MarkTypes)BreakpointMark
        && type != (MarkInterface::MarkTypes)ActiveBreakpointMark
        && type != (MarkInterface::MarkTypes)ReachedBreakpointMark
        && type != (MarkInterface::MarkTypes)DisabledBreakpointMark)
        return;

    switch (action) {
        case KTextEditor::MarkInterface::MarkAdded: {
            IFilePosBreakpoint* fileBreakpoint = new IFilePosBreakpoint(this, document->url().path(), mark.line);
            addBreakpoint(fileBreakpoint);
        }
            break;

        case KTextEditor::MarkInterface::MarkRemoved:
            // Find this breakpoint and delete it
            foreach (IBreakpoint* breakpoint, m_breakpoints)
                if (IFilePosBreakpoint* fileBreakpoint = qobject_cast<IFilePosBreakpoint*>(breakpoint))
                    if (mark.line == fileBreakpoint->lineNum())
                        if (document->url().path() == fileBreakpoint->fileName()) {
                            fileBreakpoint->remove();
                            removeBreakpoint(fileBreakpoint);
                        }
            break;
    }

    if ( KDevelop::ICore::self()->documentController()->activeDocument() && KDevelop::ICore::self()->documentController()->activeDocument()->textDocument() == document )
    {
        //bring focus back to the editor
        // TODO probably want a different command here
        KDevelop::ICore::self()->documentController()->activateDocument(KDevelop::ICore::self()->documentController()->activeDocument());
    }
#endif
}

const QPixmap* IBreakpointController::inactiveBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}

const QPixmap* IBreakpointController::activeBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Active, QIcon::Off);
  return &pixmap;
}

const QPixmap* IBreakpointController::reachedBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Selected, QIcon::Off);
  return &pixmap;
}

const QPixmap* IBreakpointController::disabledBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Disabled, QIcon::Off);
  return &pixmap;
}

const QPixmap* IBreakpointController::executionPointPixmap()
{
  static QPixmap pixmap=KIcon("go-next").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}


//NOTE: Isn't that done by TreeModel? why don't we remove this?
#if 0
int IBreakpointController::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return Last + 1;
}

QVariant IBreakpointController::data(const QModelIndex & index, int role) const
{
    Breakpoint* breakpoint = breakpointForIndex(index);
    if (!breakpoint)
        return QVariant();

    switch (index.column()) {
        case Enable:
            switch (role) {
                case Qt::CheckStateRole:
                case Qt::EditRole:
                    return breakpoint->isEnabled();
            }
            break;

        case Type:
            switch (role) {
                case Qt::DisplayRole: {
                    QString displayType = breakpoint->displayType();
                    if (breakpoint->isTemporary())
                        displayType = i18n(" temporary");
                    if (breakpoint->isHardwareBP())
                        displayType += i18n(" hw");
                    return displayType;
                }
            }
            break;

        case Status:
            switch (role) {
                case Qt::DisplayRole:
                    return breakpoint->statusDisplay(m_activeFlag);
            }
            break;

        case Location:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return breakpoint->location();
            }
            break;

        case Condition:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return breakpoint->conditional();
            }
            break;

        case IgnoreCount:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return breakpoint->ignoreCount();
            }
            break;

        case Hits:
            switch (role) {
                case Qt::DisplayRole:
                    return breakpoint->hits();
            }
            break;

        case Tracing:
            switch (role) {
                case Qt::DisplayRole:
                    return breakpoint->tracingEnabled() ? i18n("Enabled") : i18n("Disabled");
            }
            break;
    }

    return QVariant();
}

Qt::ItemFlags IBreakpointController::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    flags |= Qt::ItemIsEnabled;

    if (index.column() == Enable ||
        index.column() == Location ||
        index.column() == Condition ||
        index.column() == IgnoreCount)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant IBreakpointController::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (section) {
        case Enable:
            break;//return i18n("Enabled");
        case Type:
            return i18n("Type");
        case Status:
            return i18n("Status");
        case Location:
            return i18n("Location");
        case Condition:
            return i18n("Condition");
        case IgnoreCount:
            return i18n("Ignore Count");
        case Hits:
            return i18n("Hits");
        case Tracing:
            return i18n("Tracing");
    }

    return QVariant();
}

QModelIndex IBreakpointController::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > Last)
        return QModelIndex();

    if (row >= m_breakpoints.count())
        return QModelIndex();

    return createIndex(row, column, m_breakpoints.at(row));
}

QModelIndex IBreakpointController::parent(const QModelIndex & index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

int IBreakpointController::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_breakpoints.count();

    return 0;
}

bool IBreakpointController::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Breakpoint* bp = breakpointForIndex(index);
    if (!bp)
        return false;

    bool changed = false;

    switch (role) {
        case Qt::EditRole:
            switch (index.column()) {
                case Location:
                    if (bp->location() != value.toString())
                    {
                        // GDB does not allow to change location of
                        // an existing breakpoint. So, need to remove old
                        // breakpoint and add another.

                        // Announce to editor that breakpoit at its
                        // current location is dying.
                        bp->setActionDie();
                        adjustMark(bp, false);

                        // However, we don't want the line in breakpoint
                        // widget to disappear and appear again.

                        // Emit delete command. This won't resync breakpoint
                        // table (unlike clearBreakpoint), so we won't have
                        // nasty effect where line in the table first disappears
                        // and then appears again, and won't have internal issues
                        // as well.
                        if (!controller()->stateIsOn(s_dbgNotStarted))
                            controller()->addCommand(BreakDelete, bp->dbgRemoveCommand().toLatin1());

                        // Now add new breakpoint in gdb. It will correspond to
                        // the same 'Breakpoint' and 'BreakpointRow' objects in
                        // KDevelop is the previous, deleted, breakpoint.

                        // Note: clears 'actionDie' implicitly.
                        bp->setActionAdd(true);
                        bp->setLocation(value.toString());
                        adjustMark(bp, true);
                        changed = true;
                    }
                    break;

                case Condition:
                    bp->setConditional(value.toString());
                    changed = true;
                    break;

                case IgnoreCount:
                    bp->setIgnoreCount(value.toInt());
                    changed = true;
                    break;

                case Enable:
                    bp->setEnabled(value.toBool());
                    changed = true;
                    break;
            }
            break;
    }

    if (changed) {
        bp->setActionModify(true);
        bp->sendToGdb();

        emit dataChanged(index, index);
    }

    return changed;
}

Breakpoint * IBreakpointController::breakpointForIndex(const QModelIndex & index) const
{
    if (!index.isValid())
        return 0;

    return static_cast<Breakpoint*>(index.internalPointer());
}

#endif

IBreakpoints* IBreakpointController::breakpointsItem() { return universe_; }

#if 0
QModelIndex IBreakpointController::indexForBreakpoint(IBreakpoint * breakpoint, int column) const
{
    if (!breakpoint)
        return QModelIndex();

    int row = m_breakpoints.indexOf(breakpoint);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, column, breakpoint);
}
#endif

#include "ibreakpointcontroller.moc"

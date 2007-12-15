/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
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

#ifndef BREAKPOINTCONTROLLER_H
#define BREAKPOINTCONTROLLER_H

#include <QAbstractItemModel>

#include <KUrl>

#include <ktexteditor/markinterface.h>

#include "mi/gdbmi.h"
#include "gdbglobal.h"

namespace KDevelop { class IDocument; }

namespace GDBDebugger
{

class GDBController;
class Breakpoint;
class FilePosBreakpoint;
class Watchpoint;

/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class BreakpointController : public QAbstractItemModel
{
    Q_OBJECT

public:
    BreakpointController(GDBController* parent);
    ~BreakpointController();

    GDBController* controller() const;

    const QList<Breakpoint*>& breakpoints() const;

    /**
    * Displays an icon in the file at the line that the debugger has stoped
    * at.
    * @param url        The file the debugger has stopped at.
    * @param lineNum    The line number to display. Note: We may not know it.
    */
    void gotoExecutionPoint(const KUrl &url, int lineNum=-1);

    /**
    * Remove the executution point being displayed.
    */
    void clearExecutionPoint();

    enum Columns {
        Enable,
        Type,
        Status,
        Location,
        Condition,
        IgnoreCount,
        Hits,
        Tracing,
        Last = Tracing
    };

    FilePosBreakpoint* findBreakpoint(const QString& file, int line) const;
    Watchpoint* findWatchpoint(const QString& variableName) const;
    Watchpoint* findWatchpointByAddress(quint64 address) const;
    Breakpoint* findBreakpointById(int id) const;

    Breakpoint* addBreakpoint(Breakpoint *bp);
    void removeBreakpoint(Breakpoint *bp);
    void removeAllBreakpoints();

    // Item model reimplementations
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    //virtual bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    Breakpoint* breakpointForIndex(const QModelIndex& index) const;
    QModelIndex indexForBreakpoint(Breakpoint* breakpoint, int column = 0) const;

signals:

    /**
    * The user has toggled a breakpoint.
    */
    void toggledBreakpoint(const QString &fileName, int lineNum);

    /**
    * The user wants to edit the properties of a breakpoint.
    */
    void editedBreakpoint(const QString &fileName, int lineNum);

    /**
    * The user wants to enable/disable a breakpoint.
    */
    void toggledBreakpointEnabled(const QString &fileName, int lineNum);

private slots:
    /**
    * Whenever a new document is added this slot gets triggered and we then
    * look for a MarkInterfaceExtension part. When it is a
    * MarkInterfaceExtension part we set the various pixmaps of the
    * breakpoint icons.
    */
    void documentLoaded( KDevelop::IDocument* document );

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);

    void slotEvent(event_t);

    void slotBreakpointModified(Breakpoint* b);
    void slotBreakpointEnabledChanged(Breakpoint* b);

private:
    void handleBreakpointList(const GDBMI::ResultRecord& r);

    void adjustMark(Breakpoint* bp, bool add);

    static const QPixmap* inactiveBreakpointPixmap();
    static const QPixmap* activeBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();
    static const QPixmap* executionPointPixmap();

    enum MarkType {
        BookmarkMark           = KTextEditor::MarkInterface::markType01,
        BreakpointMark         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpointMark   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpointMark  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpointMark = KTextEditor::MarkInterface::markType05,
        ExecutionPointMark     = KTextEditor::MarkInterface::markType06
    };

    QList<Breakpoint*> m_breakpoints;
};

}

#endif

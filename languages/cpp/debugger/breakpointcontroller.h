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
#include <Qt>
#include <QSet>

#include <KUrl>
#include <KConfig>
#include <KConfigGroup>
#include <KParts/Part>

#include <ktexteditor/markinterface.h>

#include "mi/gdbmi.h"
#include "gdbglobal.h"
#include "util/treemodel.h"
#include "util/treeitem.h"

namespace KDevelop { class IDocument; }

namespace GDBDebugger
{

class GDBController;
class Breakpoint;
class FilePosBreakpoint;
class Watchpoint;
class Breakpoints;

class NewBreakpoint : public TreeItem
{
public:
    enum kind_t { code_breakpoint = 0, write_breakpoint, read_breakpoint,
                  access_breakpoint, last_breakpoint_kind };

    NewBreakpoint(TreeModel *model, TreeItem *parent, GDBController* controller,
                  kind_t kind);
    NewBreakpoint(TreeModel *model, TreeItem *parent, GDBController* controller,
                  const KConfigGroup& config);

    /* This constructor creates a "please enter location" item, that will
       turn into real breakpoint when user types something.  */
    NewBreakpoint(TreeModel *model, TreeItem *parent,
                  GDBController* controller);

    int id() const { return id_; }
    void update(const GDBMI::Value &b);
    void fetchMoreChildren() {}

    /* Mark this breakpoint as no longer inserted, due to GDB
       no longer running.  */
    void markOut();

    void setColumn(int index, const QVariant& value);
    void setDeleted();

    void sendToGDBMaybe();
    int hitCount() const;

    QVariant data(int column, int role) const;

    bool pending() const { return pending_; }
    bool dirty() const { return !dirty_.empty(); }

    void save(KConfigGroup& config);

    static const int enable_column = 0;
    static const int state_column = 1;
    static const int type_column = 2;
    static const int location_column = 3;
    static const int condition_column = 4;
        
private:
    void handleDeleted(const GDBMI::ResultRecord &v);
    void handleInserted(const GDBMI::ResultRecord &r);
    void handleEnabledOrDisabled(const GDBMI::ResultRecord &r);
    void handleConditionChanged(const GDBMI::ResultRecord &r);
    void handleAddressComputed(const GDBMI::ResultRecord &r);

    int id_;
    bool enabled_;
    QSet<int> dirty_;
    QSet<int> errors_;
    GDBController* controller_;
    bool deleted_;
    int hitCount_;
    kind_t kind_;
    /* The GDB 'pending' flag.  */
    bool pending_;
    bool pleaseEnterLocation_;

    static const char *string_kinds[last_breakpoint_kind];
};

class Breakpoints : public TreeItem
{
    Q_OBJECT
public:
    Breakpoints(TreeModel *model, GDBController *controller);

    void sendToGDB();
    void markOut();

    void update();

    void fetchMoreChildren() {}

    NewBreakpoint* addCodeBreakpoint();
    NewBreakpoint* addWatchpoint();
    NewBreakpoint* addReadWatchpoint();
    void remove(const QModelIndex &index);

    NewBreakpoint *breakpointById(int id);

    void createHelperBreakpoint();

Q_SIGNALS:
    void error(NewBreakpoint *b, const QString& message, int column);
    friend class NewBreakpoint;                                                        

public slots:
    void save();
    void load();

private:

    void handleBreakpointList(const GDBMI::ResultRecord &r);

    GDBController *controller_;
};


/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class BreakpointController : public TreeModel
{
    Q_OBJECT

public:
    BreakpointController(GDBController* parent);
    ~BreakpointController();

    GDBController* controller() const;

    Breakpoints* breakpointsItem();

    const QList<Breakpoint*>& breakpoints() const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

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

    void slotPartAdded(KParts::Part* part);

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
    Breakpoints* universe_;


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

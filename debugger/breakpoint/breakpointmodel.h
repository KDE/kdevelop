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
   along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KDEVPLATFORM_BREAKPOINTMODEL_H
#define KDEVPLATFORM_BREAKPOINTMODEL_H

#include <QtCore/QAbstractTableModel>

#include <KTextEditor/MarkInterface>
#include "breakpoint.h"

class QUrl;

namespace KParts { class Part; }
namespace KTextEditor {
class Cursor;
}

namespace KDevelop
{
class IDebugController;
class IDocument;
class Breakpoint;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        /**
         * Whether the breakpoint is active or not (settable by user): value is Qt::Checked
         * or Qt::Unchecked.
         */
        EnableColumn,

        /**
         * Synchronization state of the breakpoint (not settable by user): value is one of the
         * BreakpointState enum values.
         */
        StateColumn,

        /**
         * Kind/type of breakpoint (never changes): value is one of the BreakpointKind
         * enum values.
         */
        KindColumn,

        /**
         * Location of the breakpoint (modifiable by user); value is a string describing the
         * location; note that the formatting of retrieved data can be affected by a custom
         * BreakpointRole.
         */
        LocationColumn,

        /**
         * Condition for conditional breakpoints (modifiable by user).
         */
        ConditionColumn,

        /**
         * The number of times this breakpoint has been hit (cannot be modified by the user).
         */
        HitCountColumn,

        /**
         * How many hits of the breakpoint will be ignored before the breakpoint actually stops
         * the program (can be modified by the user and is updated by the debugger backend).
         */
        IgnoreHitsColumn,

        NumColumns
    };

    enum ColumnFlag {
        EnableColumnFlag = 1 << EnableColumn,
        StateColumnFlag = 1 << StateColumn,
        KindColumnFlag = 1 << KindColumn,
        LocationColumnFlag = 1 << LocationColumn,
        ConditionColumnFlag = 1 << ConditionColumn,
        HitCountColumnFlag = 1 << HitCountColumn,
        IgnoreHitsColumnFlag = 1 << IgnoreHitsColumn
    };
    Q_DECLARE_FLAGS(ColumnFlags, ColumnFlag)

    BreakpointModel(QObject* parent);
    virtual ~BreakpointModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex breakpointIndex(Breakpoint *b, int column);
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

     ///Note: to retrieve the full path use Breakpoint::LocationRole, Qt::DisplayRole returns only a file's name
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
 
    void toggleBreakpoint(const QUrl &url, const KTextEditor::Cursor& cursor);


    KDevelop::Breakpoint* addCodeBreakpoint();
    KDevelop::Breakpoint* addCodeBreakpoint(const QUrl& location, int line);
    KDevelop::Breakpoint* addCodeBreakpoint(const QString& expression);
    KDevelop::Breakpoint* addWatchpoint();
    KDevelop::Breakpoint* addWatchpoint(const QString& expression);
    KDevelop::Breakpoint* addReadWatchpoint();
    KDevelop::Breakpoint* addReadWatchpoint(const QString& expression);
    KDevelop::Breakpoint* addAccessWatchpoint();
    KDevelop::Breakpoint* addAccessWatchpoint(const QString& expression);

    Breakpoint *breakpoint(int row);
    QList<Breakpoint*> breakpoints() const;

    void errorEmit(Breakpoint *b, const QString& message, int column) { emit error(b, message, column); }
    void hitEmit(Breakpoint *b) { emit hit(b); }

Q_SIGNALS:
    void error(KDevelop::Breakpoint *b, const QString& message, int column);
    void hit(KDevelop::Breakpoint *b);

public Q_SLOTS:
    void save();
    void load();

private:
    enum MarkType {
        BreakpointMark = KTextEditor::MarkInterface::BreakpointActive,
        ReachedBreakpointMark  = KTextEditor::MarkInterface::BreakpointReached,
        DisabledBreakpointMark = KTextEditor::MarkInterface::BreakpointDisabled,
        PendingBreakpointMark   = KTextEditor::MarkInterface::markType08,

        AllBreakpointMarks = BreakpointMark | ReachedBreakpointMark | DisabledBreakpointMark | PendingBreakpointMark
    };

    IDebugController * debugController();

private Q_SLOTS:

    void updateMarks();

    void slotPartAdded(KParts::Part* part);

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);
    void textDocumentCreated(KDevelop::IDocument*);
    void documentSaved(KDevelop::IDocument*);
    void aboutToDeleteMovingInterfaceContent(KTextEditor::Document *document);

    void markContextMenuRequested( KTextEditor::Document* document, KTextEditor::Mark mark,
                               const QPoint &pos, bool& handled );

private:
    static const QPixmap* breakpointPixmap();
    static const QPixmap* pendingBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();

private:
    friend class Breakpoint;
    friend class IBreakpointController;

    void updateHitCount(int row, int hitCount);

    void registerBreakpoint(Breakpoint* breakpoint);

    void reportChange(Breakpoint *breakpoint, Breakpoint::Column column);
    uint breakpointType(Breakpoint *breakpoint);
    Breakpoint *breakpoint(const QUrl& url, int line);

    bool m_dontUpdateMarks;
    QList<Breakpoint*> m_breakpoints;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::BreakpointModel::ColumnFlags)

#endif

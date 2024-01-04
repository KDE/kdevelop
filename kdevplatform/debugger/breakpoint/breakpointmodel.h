/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#ifndef KDEVPLATFORM_BREAKPOINTMODEL_H
#define KDEVPLATFORM_BREAKPOINTMODEL_H

#include <QAbstractTableModel>

#include <KTextEditor/Document>
#include "breakpoint.h"

class QUrl;
class TestBreakpointModel;

namespace KParts { class Part; }
namespace KTextEditor {
class Cursor;
}

namespace KDevelop
{
class IDocument;
class Breakpoint;
class BreakpointModelPrivate;

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

    explicit BreakpointModel(QObject* parent);
    ~BreakpointModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex breakpointIndex(Breakpoint *b, int column);
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

     ///Note: to retrieve the full path use Breakpoint::LocationRole, Qt::DisplayRole returns only a file's name
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * Delete a given registered breakpoint.
     * @param breakpoint a non-null pointer
     */
    void removeBreakpoint(Breakpoint* breakpoint);

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

    Breakpoint* breakpoint(int row) const;
    QList<Breakpoint*> breakpoints() const;

Q_SIGNALS:
    void error(int row, const QString& errorText);
    void hit(int row);

public Q_SLOTS:
    void save();
    void load();

private:
    enum MarkType {
        BreakpointMark = KTextEditor::Document::BreakpointActive,
        ReachedBreakpointMark  = KTextEditor::Document::BreakpointReached,
        DisabledBreakpointMark = KTextEditor::Document::BreakpointDisabled,
        PendingBreakpointMark   = KTextEditor::Document::markType08,

        AllBreakpointMarks = BreakpointMark | ReachedBreakpointMark | DisabledBreakpointMark | PendingBreakpointMark
    };

private Q_SLOTS:

    void updateMarks();

    void slotPartAdded(KParts::Part* part);

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::Document::MarkChangeAction action);
    void textDocumentCreated(KDevelop::IDocument*);
    void documentSaved(KDevelop::IDocument*);
    void aboutToDeleteMovingInterfaceContent(KTextEditor::Document *document);

    void markContextMenuRequested(KTextEditor::Document *document, KTextEditor::Mark mark, QPoint pos, bool &handled);

private:
    static const QPixmap* breakpointPixmap();
    static const QPixmap* pendingBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();

private:
    friend class Breakpoint;
    friend class IBreakpointController;
    friend class ::TestBreakpointModel;

    void updateState(int row, Breakpoint::BreakpointState state);
    void updateHitCount(int row, int hitCount);
    void updateErrorText(int row, const QString& errorText);
    void notifyHit(int row);

    void registerBreakpoint(Breakpoint* breakpoint);
    void scheduleSave();

    void reportChange(Breakpoint *breakpoint, Breakpoint::Column column);
    uint breakpointType(Breakpoint *breakpoint) const;
    Breakpoint *breakpoint(const QUrl& url, int line) const;

    void setupMovingCursor(KTextEditor::Document* document, Breakpoint* breakpoint) const;

private:
    const QScopedPointer<class BreakpointModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(BreakpointModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BreakpointModel::ColumnFlags)
}

#endif

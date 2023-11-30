/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BREAKPOINT_H
#define KDEVPLATFORM_BREAKPOINT_H

#include <QSet>
#include <QUrl>

#include <debugger/debuggerexport.h>

class QVariant;
class KConfigGroup;
namespace KTextEditor {
class MovingCursor;
}
namespace KDevelop
{
class BreakpointModel;

class KDEVPLATFORMDEBUGGER_EXPORT Breakpoint
{
public:
    enum BreakpointKind {
        CodeBreakpoint = 0,
        WriteBreakpoint,
        ReadBreakpoint,
        AccessBreakpoint,
        LastBreakpointKind
    };
    enum BreakpointState {
        NotStartedState,
        DirtyState,
        PendingState,
        CleanState
    };
    ///Custom roles for retrieving data from breakpoint model.
    enum BreakpointRole{
        LocationRole = Qt::UserRole + 1 ///< Retrieves breakpoint's full path unlike Qt::DisplayRole. Note: it's only applicable to LocationColumn.
    };

    Q_DISABLE_COPY_MOVE(Breakpoint)

    Breakpoint(BreakpointModel *model, BreakpointKind kind);
    Breakpoint(BreakpointModel *model, const KConfigGroup& config);

    bool setData(int index, const QVariant& value);

    ///Note: to retrieve the full path use LocationRole, Qt::DisplayRole return only a file's name
    QVariant data(int column, int role) const;

    void save(KConfigGroup& config);

    // Moved to BreakpointModel; temporarily left here to ease API transition
    enum Column {
        EnableColumn,
        StateColumn,
        TypeColumn,
        LocationColumn,
        ConditionColumn,
        HitCountColumn,
        IgnoreHitsColumn
    };

    void setUrl(const QUrl &url);
    QUrl url() const;

    /**
     * Assign a line number to the breakpoint.
     */
    void setLine(int line);

    /**
     * Get the current breakpoint line number.
     * @note If movingCursor() is non-null, returns the cursor's line number,
     *       which may be different than savedLine().
     */
    int line() const;

    /**
     * Get the breakpoint saved line number.
     */
    int savedLine() const;

    void setLocation(const QUrl& url, int line);
    QString location();

    BreakpointKind kind() const;

    void setAddress(const QString& address);
    QString address() const;

    int hitCount() const;

    /**
     * Check if the breakpoint is deleted.
     * @note This method exists to ease the API transition in IBreakpointController;
     *       it should be removed eventually, since check for already freed memory does not work.
     */
    bool deleted() const;
    
    bool enabled() const;
    
    void setMovingCursor(KTextEditor::MovingCursor *cursor);
    KTextEditor::MovingCursor *movingCursor() const;

    void setIgnoreHits(int c);
    int ignoreHits() const;

    void setCondition(const QString &c);
    QString condition() const;

    void setExpression(const QString &c);
    QString expression() const;

    BreakpointState state() const;
    QString errorText() const;

protected:
    friend class BreakpointModel;
    friend class IBreakpointController;
    
    /**
     * Return the model this breakpoint is attached to
     *
     * @note This might be null, e.g. after the breakpoint has been marked as deleted
     */
    BreakpointModel *breakpointModel();

    BreakpointModel *m_model;
    bool m_enabled;
    bool m_deleted;
    BreakpointState m_state;
    BreakpointKind m_kind;
    /* For watchpoints, the address it is set at.  */
    QString m_address;
    QUrl m_url;
    int m_line;
    QString m_condition;
    KTextEditor::MovingCursor *m_movingCursor;
    int m_hitCount;
    int m_ignoreHits;
    QString m_expression;
    QString m_errorText;

    void saveMovingCursorLine();
    void reportChange(Column c);
};

}
#endif

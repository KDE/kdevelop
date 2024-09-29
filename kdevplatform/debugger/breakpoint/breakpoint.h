/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BREAKPOINT_H
#define KDEVPLATFORM_BREAKPOINT_H

#include <QUrl>

#include <debugger/debuggerexport.h>

class TestBreakpointModel;
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

    void setHitCount(int hits);
    int hitCount() const;

    /**
     * Check if the breakpoint is deleted.
     * @note This method exists to ease the API transition in IBreakpointController;
     *       it should be removed eventually, since check for already freed memory does not work.
     */
    bool deleted() const;
    
    bool enabled() const;

    // NOTE: A Breakpoint is represented in the UI by a row in the Breakpoints tool view table and a mark on its
    // document border. KTextEditor::Mark is a plain class that contains only two non-identifying data members: the
    // mark's line number and type. A document mark tracks the text on its line. So when lines are inserted or
    // removed above the mark's line in the document, the mark's line number is automatically adjusted to remain
    // attached to the tracked text. A Breakpoint needs to remain associated with its document mark. For this
    // purpose, it also tracks the mark's line number via a dedicated moving cursor. restartDocumentLineTrackingAt()
    // assigns and stopDocumentLineTracking() destroys this moving cursor. By tracking the same document text, the
    // breakpoint mark and moving cursor are associated. A Breakpoint uses the associated moving cursor's line
    // number to identify and locate its mark.
    // Unfortunately, this association between the breakpoint mark and moving cursor can be broken. When the user
    // removes a breakpoint's line, the breakpoint mark on it is deleted, but the moving cursor remains and keeps
    // tracking a line next to the removed one. Furthermore, when the user removes the end-of-line character between
    // two breakpoint-marked document lines (i.e. joins the two lines into one), the mark types on those lines are
    // bitwise-OR-ed (the last updated breakpoint mark is visible on the document border), and the two associated
    // moving cursors end up tracking the single joined document line (though their column numbers differ, which can
    // potentially help improve the current behavior).
    // If the user types some text on an empty line with a breakpoint and presses Enter, the breakpoint mark stays
    // on its now nonempty line, but the associated moving cursor tracks the newly inserted next line. Thus the
    // breakpoint mark's line gets out of sync with its associated moving cursor's tracked line.

    void stopDocumentLineTracking();
    void restartDocumentLineTrackingAt(KTextEditor::MovingCursor* cursor);

    KTextEditor::MovingCursor *movingCursor() const;

    void setIgnoreHits(int c);
    int ignoreHits() const;

    void setCondition(const QString &c);
    QString condition() const;

    void setExpression(const QString &c);
    QString expression() const;

    void setState(BreakpointState state);
    BreakpointState state() const;
    QString errorText() const;

protected:
    friend class BreakpointModel;
    friend class IBreakpointController;
    friend class ::TestBreakpointModel;

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

    uint markType() const;
    void updateMarkType() const;

    /**
     * Assign the document url that the breakpoint is associated with.
     * @pre If the breakpoint has a moving cursor, then movingCursor()->document()->url()
     *      must already be equal to @p url.
     * @note The calling code is responsible for updating the breakpoint's LocationColumn afterwards.
     */
    void assignUrl(const QUrl& url);

    /**
     * Destroy the associated moving cursor.
     * @pre Document line tracking is active, i.e. the moving cursor is valid.
     * @note The associated breakpoint mark is not touched. The calling code is
     *       responsible for keeping the mark in sync with the breakpoint.
     */
    void removeMovingCursor();
    void saveMovingCursorLine();
    void reportChange(Column c);
    /**
     * Update or replace the breakpoint mark and moving cursor for the specified new location.
     * @note Calls stopDocumentLineTracking() if the new location is invalid or if
     *       the breakpoint cannot have a moving cursor.
     */
    void updateMovingCursor(const QUrl& url, int line);
};

}
#endif

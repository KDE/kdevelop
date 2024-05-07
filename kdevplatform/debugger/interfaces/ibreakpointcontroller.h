/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IBREAKPOINTCONTROLLER_H
#define KDEVPLATFORM_IBREAKPOINTCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QSet>

#include <debugger/debuggerexport.h>

#include "idebugsession.h"
#include "../breakpoint/breakpoint.h"
#include "../breakpoint/breakpointmodel.h"

namespace KDevelop {
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpointController : public QObject
{
    Q_OBJECT
public:
    explicit IBreakpointController(IDebugSession* parent);

    /**
     * Called just after a breakpoint is added in the given row.
     * The breakpoint which was previously at the given row and all later breakpoints have
     * been moved.
     *
     * Implementations may implement this function to maintain their internal data structures.
     * Note, however, that breakpoints are typically still empty (i.e. without a useful location)
     * when this method is called.
     */
    virtual void breakpointAdded(int row);

    /**
     * Implementors must handle changes to the breakpoint model, which are signaled via
     * this method, by forwarding the changes to the backend debugger.
     */
    virtual void breakpointModelChanged(int row, BreakpointModel::ColumnFlags columns);

    /**
     * Called when a breakpoint is about to be deleted from the model.
     */
    virtual void breakpointAboutToBeDeleted(int row);

    /**
     * Called when the debugger state changed.
     *
     * Note: this method exists to ease the API transition; it should probably go away eventually,
     * since controller implementations that do want to listen to debugger state changes probably
     * have better ways to do so.
     */
    virtual void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState);

protected:
    IDebugSession *debugSession() const;
    BreakpointModel *breakpointModel() const;

    void updateState(int row, Breakpoint::BreakpointState state);
    void updateHitCount(int row, int hitCount);
    void updateErrorText(int row, const QString& errorText);
    void notifyHit(int row, const QString & msg);

    // The API below is obsolete and will be removed soon
protected:
    void breakpointStateChanged(Breakpoint* breakpoint);
    void setHitCount(Breakpoint* breakpoint, int count);

    void error(Breakpoint* breakpoint, const QString& msg, Breakpoint::Column column);
    void hit(Breakpoint* breakpoint, const QString& msg = QString());

    void sendMaybeAll();
    virtual void sendMaybe(Breakpoint *breakpoint) = 0;

    QMap<const Breakpoint*, QSet<Breakpoint::Column> > m_dirty;
    QSet<const Breakpoint*> m_pending;
    int m_dontSendChanges;
};

}

#endif // KDEVPLATFORM_IBREAKPOINTCONTROLLER_H

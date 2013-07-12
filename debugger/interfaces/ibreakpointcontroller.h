/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2009 Niko Sams <niko.sams@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_IBREAKPOINTCONTROLLER_H
#define KDEVPLATFORM_IBREAKPOINTCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include "../debuggerexport.h"
#include "idebugsession.h"
#include "../breakpoint/breakpoint.h"

class QModelIndex;
namespace KDevelop {
class BreakpointModel;
class Breakpoint;
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpointController : public QObject
{
    Q_OBJECT
public:
    IBreakpointController(IDebugSession* parent);
    
    Breakpoint::BreakpointState breakpointState(const Breakpoint *breakpoint) const;
    int breakpointHitCount(const Breakpoint *breakpoint) const;
    QSet<Breakpoint::Column> breakpointErrors(const Breakpoint *breakpoint) const;
    QString breakpointErrorText(const Breakpoint *breakpoint) const;

protected:
    IDebugSession *debugSession() const;
    BreakpointModel *breakpointModel() const;

    void breakpointStateChanged(Breakpoint* breakpoint);
    void setHitCount(Breakpoint* breakpoint, int count);

    void error(Breakpoint* breakpoint, const QString& msg, Breakpoint::Column column);
    void hit(Breakpoint* breakpoint, const QString& msg = QString());

    void sendMaybeAll();
    virtual void sendMaybe(Breakpoint *breakpoint) = 0;

    QMap<const Breakpoint*, QSet<Breakpoint::Column> > m_dirty;
    QSet<const Breakpoint*> m_pending;
    QMap<const Breakpoint*, int> m_hitCount;
    QMap<const Breakpoint*, QSet<Breakpoint::Column> > m_errors;
    QMap<const Breakpoint*, QString > m_errorText; ///< message of the last error
    int m_dontSendChanges;

private Q_SLOTS:
    void breakpointChanged(KDevelop::Breakpoint* breakpoint, KDevelop::Breakpoint::Column column);
    void breakpointDeleted(KDevelop::Breakpoint *breakpoint);
    void debuggerStateChanged(KDevelop::IDebugSession::DebuggerState);
};

}

#endif // KDEVPLATFORM_IBREAKPOINTCONTROLLER_H

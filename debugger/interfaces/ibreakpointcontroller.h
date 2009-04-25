class QModelIndex;
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KDEVELOP_IBREAKPOINTCONTROLLER_H
#define KDEVELOP_IBREAKPOINTCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "../debuggerexport.h"

namespace KDevelop {
class BreakpointModel;
class Breakpoint;
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpointController : public QObject
{
    Q_OBJECT
public:
    IBreakpointController(IDebugSession* parent);

protected:
    IDebugSession *debugSession() const;
    BreakpointModel *breakpointModel() const;

    virtual void sendMaybe(Breakpoint *breakpoint) = 0;

    QMap<KDevelop::Breakpoint*, QSet<int> > m_dirty;
    bool m_dontSendChanges;

private Q_SLOTS:
    void dataChanged(const QModelIndex &topRight, const QModelIndex &bottomLeft);
    void breakpointDeleted(KDevelop::Breakpoint *breakpoint);
};

}

#endif // KDEVELOP_IBREAKPOINTCONTROLLER_H

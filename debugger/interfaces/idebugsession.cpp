/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "idebugsession.h"
#include "iframestackmodel.h"
#include "ivariablecontroller.h"
#include "util/debug.h"

#include <QApplication>
#include <QWidget>
#include <QFileInfo>


namespace KDevelop {


IDebugSession::IDebugSession()
    : m_breakpointController(0), m_variableController(0), m_frameStackModel(0)
{
    connect(this, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)), SLOT(slotStateChanged(KDevelop::IDebugSession::DebuggerState)));
}

IDebugSession::~IDebugSession()
{
}

bool IDebugSession::isRunning() const
{
    DebuggerState s = state();
    return (s == ActiveState || s == PausedState);
}

IBreakpointController *IDebugSession::breakpointController() const
{
    return m_breakpointController;
}


IVariableController *IDebugSession::variableController() const
{
    return m_variableController;
}

IFrameStackModel* IDebugSession::frameStackModel() const
{
    /* The delayed initialization is used so that derived
       class can override createFrameStackModel and have
       it called. If we tried to call virtual function
       from a constructor, it would not work.  */
    if (m_frameStackModel == 0) {
        m_frameStackModel = const_cast<IDebugSession*>(this)->createFrameStackModel();
        Q_ASSERT(m_frameStackModel);
    }
    return m_frameStackModel;
}

void IDebugSession::raiseEvent(event_t e)
{
    if (frameStackModel()) frameStackModel()->handleEvent(e);
    if (m_variableController) m_variableController->handleEvent(e);
    // FIXME: consider if we actually need signals
    emit event(e);
}

QPair<QUrl, int> IDebugSession::convertToLocalUrl(const QPair<QUrl, int> &remoteUrl) const
{
    return remoteUrl;
}

QPair<QUrl, int> IDebugSession::convertToRemoteUrl(const QPair<QUrl, int>& localUrl) const
{
    return localUrl;
}

void IDebugSession::clearCurrentPosition()
{
    qCDebug(DEBUGGER);
    m_url.clear();
    m_addr = "";
    m_line = -1;
    emit clearExecutionPoint();
}

void IDebugSession::setCurrentPosition(const QUrl& url, int line, const QString& addr)
{
    qCDebug(DEBUGGER) << url << line << addr;

    if (url.isEmpty() || !QFileInfo(convertToLocalUrl(qMakePair(url,line)).first.path()).exists()) {
        clearCurrentPosition();
        m_addr = addr;
        emit showStepInDisassemble(addr);
    } else {
        m_url = url;
        m_line = line;
        m_addr = addr;
        emit showStepInSource(url, line, addr);
    }
}

QUrl IDebugSession::currentUrl() const
{
    return m_url;
}

int IDebugSession::currentLine() const
{
    return m_line;
}

QString IDebugSession::currentAddr() const
{
    return m_addr;
}

void IDebugSession::slotStateChanged(IDebugSession::DebuggerState state)
{
    if (state != PausedState) {
        clearCurrentPosition();
    }
}


}


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
{
    connect(this, &IDebugSession::stateChanged, this, &IDebugSession::slotStateChanged);
}

IDebugSession::~IDebugSession()
{
}

bool IDebugSession::isRunning() const
{
    DebuggerState s = state();
    return (s == ActiveState || s == PausedState);
}

void IDebugSession::raiseEvent(event_t e)
{
    if (IFrameStackModel* model = frameStackModel()) {
        model->handleEvent(e);
    }
    if (IVariableController* variables = variableController()) {
        variables->handleEvent(e);
    }
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


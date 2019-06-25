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
#include <debug.h>

#include <QFileInfo>


namespace KDevelop {

class IDebugSessionPrivate
{
public:
    explicit IDebugSessionPrivate(IDebugSession* q) : q(q) {}

    void slotStateChanged(IDebugSession::DebuggerState state);

    IDebugSession* q;

    /// Current position in debugged program, gets set when the state changes
    QUrl m_url;
    int m_line;
    QString m_addr;
};

IDebugSession::IDebugSession()
    : d_ptr(new IDebugSessionPrivate(this))
{
    connect(this, &IDebugSession::stateChanged,
            this, [this](IDebugSession::DebuggerState state) { Q_D(IDebugSession); d->slotStateChanged(state); });
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
    Q_D(IDebugSession);

    qCDebug(DEBUGGER);
    d->m_url.clear();
    d->m_addr.clear();
    d->m_line = -1;
    emit clearExecutionPoint();
}

void IDebugSession::setCurrentPosition(const QUrl& url, int line, const QString& addr)
{
    Q_D(IDebugSession);

    qCDebug(DEBUGGER) << url << line << addr;

    if (url.isEmpty() || !QFileInfo::exists(convertToLocalUrl(qMakePair(url,line)).first.path())) {
        clearCurrentPosition();
        d->m_addr = addr;
        emit showStepInDisassemble(addr);
    } else {
        d->m_url = url;
        d->m_line = line;
        d->m_addr = addr;
        emit showStepInSource(url, line, addr);
    }
}

QUrl IDebugSession::currentUrl() const
{
    Q_D(const IDebugSession);

    return d->m_url;
}

int IDebugSession::currentLine() const
{
    Q_D(const IDebugSession);

    return d->m_line;
}

QString IDebugSession::currentAddr() const
{
    Q_D(const IDebugSession);

    return d->m_addr;
}

void IDebugSessionPrivate::slotStateChanged(IDebugSession::DebuggerState state)
{
    if (state != IDebugSession::PausedState) {
        q->clearCurrentPosition();
    }
}

}

#include "moc_idebugsession.cpp"

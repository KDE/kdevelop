/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idebugsession.h"
#include "iframestackmodel.h"
#include "ivariablecontroller.h"
#include <debug.h>

#include <QFileInfo>

namespace {
struct PrintPosition
{
    const QUrl& url;
    const int line;
    const QString& addr;
};

QDebug operator<<(QDebug debug, const PrintPosition& p)
{
    const QDebugStateSaver saver(debug);
    // IDebugSession::currentLine() is zero-based, so add 1 to print the conventional one-based line number.
    debug.noquote().nospace() << p.url.toString(QUrl::PreferLocalFile) << ':' << p.line + 1 << ", addr: " << p.addr;
    return debug;
}
} // unnamed namespace

namespace KDevelop {

class IDebugSessionPrivate
{
public:
    explicit IDebugSessionPrivate(IDebugSession* q) : q(q) {}

    void slotStateChanged(IDebugSession::DebuggerState state);

    IDebugSession* q;

    /// Current position in debugged program, gets set when the state changes
    QUrl m_url;
    int m_line = -1;
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

auto IDebugSession::toolViewToRaiseAtEnd() const -> ToolView
{
    return ToolView::Debug;
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

    // Pad output with spaces to align the printed position and facilitate comparison with setCurrentPosition().
    qCDebug(DEBUGGER) << "clearing current position:  " << PrintPosition{d->m_url, d->m_line, d->m_addr};

    d->m_url.clear();
    d->m_addr.clear();
    d->m_line = -1;
    emit clearExecutionPoint();
}

void IDebugSession::setCurrentPosition(const QUrl& url, int line, const QString& addr)
{
    Q_D(IDebugSession);

    qCDebug(DEBUGGER) << "setting current position to:" << PrintPosition{url, line, addr};

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

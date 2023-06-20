/*
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevsignalspy.h"

#include <QEventLoop>
#include <QTimer>

namespace KDevelop {
KDevSignalSpy::KDevSignalSpy(QObject* obj, const char* signal,
                             Qt::ConnectionType ct)
    : QObject(nullptr)
    , m_obj(obj)
    , m_emitted(false)
{
    m_timer = new QTimer(this);
    m_loop = new QEventLoop(this);
    connect(obj, signal, this, SLOT(signalEmitted()), ct);
}

bool KDevSignalSpy::wait(int timeout)
{
    Q_ASSERT(!m_loop->isRunning()); Q_ASSERT(!m_timer->isActive());

    m_emitted = false;
    if (timeout > 0) {
        connect(m_timer, &QTimer::timeout, m_loop, &QEventLoop::quit);
        m_timer->setSingleShot(true);
        m_timer->start(timeout);
    }
    m_loop->exec();

    return m_emitted;
}

void KDevSignalSpy::signalEmitted()
{
    m_emitted = true;
    disconnect(m_obj, nullptr, this, nullptr);
    m_timer->stop();
    m_loop->quit();
}
} // KDevelop

#include "moc_kdevsignalspy.cpp"

/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "kdevsignalspy.h"
#include <qeventloop.h>
#include <qtimer.h>

namespace KDevelop
{

KDevSignalSpy::KDevSignalSpy(QObject *obj, const char *signal,
              Qt::ConnectionType ct )
  : QObject(0), m_obj(obj), m_emitted(false) 
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
        connect(m_timer, SIGNAL(timeout()), m_loop, SLOT(quit()));
        m_timer->setSingleShot(true);
        m_timer->start(timeout);
    }
    m_loop->exec();

    return m_emitted;
}

void KDevSignalSpy::signalEmitted()
{
    m_emitted = true;
    disconnect(m_obj, 0, this, 0);
    m_timer->stop();
    m_loop->quit();
}

} // KDevelop



/*
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

#include "qxtsignalwaiter.h"
#include <QCoreApplication>
#include <QTimerEvent>

QxtSignalWaiter::QxtSignalWaiter(const QObject* sender, const char* signal) : QObject(0)
{
    Q_ASSERT(sender && signal);
    connect(sender, signal, this, SLOT(signalCaught()));
}

// Returns true if the signal was caught, returns false if the wait timed out
bool QxtSignalWaiter::wait(const QObject* sender, const char* signal, int msec)
{
    QxtSignalWaiter w(sender, signal);
    return w.wait(msec);
}

// Returns true if the signal was caught, returns false if the wait timed out
bool QxtSignalWaiter::wait(int msec)
{
    // Check input parameters
    if (msec < -1) return false;

    // activate the timeout
    if (msec != -1) timerID = startTimer(msec);

    // Begin waiting
    ready = timeout = false;
    while (!ready && !timeout)
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);

    // Clean up and return status
    killTimer(timerID);
    return ready || !timeout;
}

void QxtSignalWaiter::signalCaught()
{
    ready = true;
}

void QxtSignalWaiter::timerEvent(QTimerEvent* event)
{
    killTimer(timerID);
    timeout = true;
}

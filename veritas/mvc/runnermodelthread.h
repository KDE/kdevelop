/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
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

/*!
 * \file  runnermodelthread.h
 *
 * \brief Declares class RunnerModelThread.
 */

#ifndef VERITAS_RUNNERMODELTHREAD_H
#define VERITAS_RUNNERMODELTHREAD_H

#include <QThread>

namespace Veritas
{

class RunnerModel;

/*!
 * \brief The RunnerModelThread class enables asynchronous execution
 *        of runner items.
 *
 * The RunnerModelThread class enables RunnerModel to asynchronously
 * execute Test objects. When the thread is started it calls the
 * private \c threadCode() method of RunnerModel where the 'real'
 * threaded code is executed. Therefore RunnerModelThread is a friend
 * of RunnerModel.
 */
class RunnerModelThread : public QThread
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a runner model thread with the given \a parent.
     */
    RunnerModelThread(RunnerModel* parent);

    /*!
     * Destroys this runner model thread.
     */
    ~RunnerModelThread();

    /*!
     * Causes the current thread to sleep for \a msecs milliseconds.
     */
    void msleep(unsigned long msecs) const;

private: // Operations

    /*!
     * Reimplemented from QThread. Starts the thread.
     */
    void run();

    // Copy and assignment not supported.
    RunnerModelThread(const RunnerModelThread&);
    RunnerModelThread& operator=(const RunnerModelThread&);
};

} // namespace

#endif // VERITAS_RUNNERMODELTHREAD_H

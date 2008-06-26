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
 * \file  runnermodelthread.cpp
 *
 * \brief Implements class RunnerModelThread.
 */

#include "runnermodelthread.h"
#include "runnermodel.h"

namespace Veritas
{

RunnerModelThread::RunnerModelThread(RunnerModel* parent)
        : QThread(parent)
{

}

RunnerModelThread::~RunnerModelThread()
{

}

void RunnerModelThread::msleep(unsigned long msecs) const
{
    QThread::msleep(msecs);
}

void RunnerModelThread::run()
{
    setTerminationEnabled();

    RunnerModel* model = static_cast<RunnerModel*>(parent());

    if (model) {
        model->threadCode();
    }
}

} // namespace

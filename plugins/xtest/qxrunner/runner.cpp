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
 * \file  runner.cpp
 *
 * \brief Implements class Runner.
 */

#include "runner.h"
#include "runnerwindow.h"

#include <QIcon>

namespace QxRunner
{

Runner::Runner(RunnerModel* model) : m_model(model)
{
    m_icon = 0;
}

Runner::~Runner()
{
    delete m_icon;
}

void Runner::run()
{
    RunnerWindow runnerWindow;

    if (m_icon) {
        // Application specific icon.
        runnerWindow.setWindowIcon(*m_icon);
    }

    runnerWindow.setModel(m_model);
    runnerWindow.show();

    qApp->connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    qApp->exec();
}

void Runner::setWindowIcon(const QIcon& icon)
{
    m_icon = new QIcon(icon);
}

} // namespace

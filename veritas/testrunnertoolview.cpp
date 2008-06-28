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

#include "testrunnertoolview.h"
#include "runnermodel.h"
#include "runnerwindow.h"
#include <KAboutData>
#include <QWidget>
#include <KDebug>
#include <itest.h>
#include "test.h"

using KDevelop::IPlugin;
using Veritas::ITest;
using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;
using Veritas::TestRunnerToolView;

TestRunnerToolView::TestRunnerToolView(const KComponentData& about, QObject* parent)
    : IPlugin(about, parent), m_window(0)
{}

TestRunnerToolView::~TestRunnerToolView()
{}

QWidget* TestRunnerToolView::spawnWindow()
{
    m_window = new RunnerWindow;
    connect(m_window->ui().actionReload, SIGNAL(triggered(bool)), this, SLOT(reload()));
    reload();
    return m_window;
}

void TestRunnerToolView::reload()
{
    ITest* root = registerTests(); // implemented by concrete plugins
    RunnerModel* model = new RunnerModel;
    model->setRootItem(qobject_cast<Test*>(root));
    model->setExpectedResults(Veritas::RunError);
    m_window->setModel(model);
    m_window->show();
}

#include "testrunnertoolview.moc"

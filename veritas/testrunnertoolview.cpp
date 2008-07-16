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

#include "veritas/testrunnertoolview.h"

#include <KAboutData>
#include <KDebug>
#include <KSelectAction>
#include <QWidget>

#include "interfaces/iproject.h"
#include "interfaces/iprojectcontroller.h"
#include "shell/core.h"

#include "veritas/test.h"
#include "veritas/mvc/runnermodel.h"
#include "veritas/mvc/runnerwindow.h"

Q_DECLARE_METATYPE(KDevelop::IProject*)

using KDevelop::IPlugin;
using KDevelop::IProject;
using KDevelop::IProjectController;

using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;
using Veritas::TestRunnerToolView;

namespace Veritas
{
class TestRunnerToolViewPrivate
{
public:
    TestRunnerToolViewPrivate() : window(0), selected(0) {}
    RunnerWindow* window;
    IProject* selected;
};
}

using Veritas::TestRunnerToolViewPrivate;

TestRunnerToolView::TestRunnerToolView(const KComponentData& about, QObject* parent)
        : IPlugin(about, parent), d(new TestRunnerToolViewPrivate)
{}

TestRunnerToolView::~TestRunnerToolView()
{
    delete d;
}

QWidget* TestRunnerToolView::spawnWindow()
{
    d->window = new RunnerWindow;
    IProjectController* ipc = core()->projectController();
    foreach(IProject* proj, ipc->projects()) {
        d->window->addProjectToPopup(proj);
    }
    connect(ipc, SIGNAL(projectOpened(KDevelop::IProject*)),
            d->window, SLOT(addProjectToPopup(KDevelop::IProject*)));
    connect(ipc, SIGNAL(projectClosed(KDevelop::IProject*)),
            d->window, SLOT(rmProjectFromPopup(KDevelop::IProject*)));

    connect(d->window->projectPopup(), SIGNAL(triggered(QAction*)),
            this, SLOT(setSelected(QAction*)));
    connect(d->window->ui().actionReload, SIGNAL(triggered(bool)),
            this, SLOT(reload()));

    reload();
    return d->window;
}

void TestRunnerToolView::reload()
{
    Test* root = registerTests(); // implemented by concrete plugins
    RunnerModel* model = new RunnerModel;
    model->setRootItem(qobject_cast<Test*>(root));
    model->setExpectedResults(Veritas::RunError);
    d->window->setModel(model);
    d->window->show();
    d->window->runnerView()->setRootIsDecorated(false);
}

IProject* TestRunnerToolView::project() const
{
    return d->selected;
}

void TestRunnerToolView::setSelected(QAction* action)
{
    d->selected = action->data().value<IProject*>();
}

#include "testrunnertoolview.moc"

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
#include "interfaces/iuicontroller.h"
#include "shell/core.h"
#include "shell/uicontroller.h"
#include "sublime/tooldocument.h"
#include "sublime/area.h"
#include "sublime/view.h"
#include "veritas/test.h"
#include "veritas/mvc/runnermodel.h"
#include "veritas/mvc/runnerwindow.h"
#include "veritas/mvc/verbosemanager.h"
#include "ui_runnerwindow.h"

Q_DECLARE_METATYPE(KDevelop::IProject*)

using Sublime::Area;
using Sublime::Document;
using Sublime::ToolDocument;
using Sublime::View;

using KDevelop::Core;
using KDevelop::IPlugin;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::IToolViewFactory;
using KDevelop::IUiController;
using KDevelop::UiController;

using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;
using Veritas::VerboseManager;
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

QWidget* TestRunnerToolView::resultsWidget()
{
    return d->window->resultsWidget();
}

TestRunnerToolView::TestRunnerToolView(const KComponentData& about, QObject* parent)
        : IPlugin(about, parent),
        d(new TestRunnerToolViewPrivate),
        resultsVisible(false),
        m_resultsView(0),
        m_resultsArea(0)
{}

TestRunnerToolView::~TestRunnerToolView()
{
    //removeResultsView();
    delete d;
}

void TestRunnerToolView::removeResultsView()
{
    if (m_resultsView && m_resultsArea) {
        m_resultsArea->removeToolView(m_resultsView);
    }
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
    connect(d->window->ui()->actionReload, SIGNAL(triggered(bool)),
            this, SLOT(reload()));

    connect(d->window->verboseManager(), SIGNAL(openVerbose(Test*)),
            this, SLOT(openVerbose(Test*)));
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
    d->window->runnerView()->setRootIsDecorated(false);
    spawnResultsView();
}

IProject* TestRunnerToolView::project() const
{
    return d->selected;
}

void TestRunnerToolView::setSelected(QAction* action)
{
    d->selected = action->data().value<IProject*>();
}

class ResultsViewFactory: public KDevelop::IToolViewFactory
{
    public:
        ResultsViewFactory(const QString& id, TestRunnerToolView *runner): m_runner(runner), m_id(id) {}

        virtual QWidget* create(QWidget *parent = 0) {
            Q_UNUSED(parent);
            return m_runner->resultsWidget(); // TODO this is bad. Currently only a single
            // resultsWidget is ever created, which means
            // that multiple resultsviews will segfault kdevelop.
            // Introducing multiple resultViews requires
            // changes in RunnerWindow.
        }
        virtual Qt::DockWidgetArea defaultPosition() {
            return Qt::BottomDockWidgetArea;
        }
        virtual QString id() const {
            return m_id;
        }

    private:
        TestRunnerToolView *m_runner;
        QString m_id;
};

namespace
{
class UiToolViewFactory: public Sublime::ToolFactory
{
    public:
        UiToolViewFactory(IToolViewFactory *factory): m_factory(factory) {}
        ~UiToolViewFactory() { delete m_factory; }
        virtual QWidget* create(Sublime::ToolDocument *doc, QWidget *parent = 0) {
            Q_UNUSED(doc);
            return m_factory->create(parent);
        }
        QList<QAction*> toolBarActions(QWidget* viewWidget) const {
            return m_factory->toolBarActions(viewWidget);
        }
        QString id() const { return m_factory->id(); }

    private:
        IToolViewFactory *m_factory;
};
}

class ResultsViewFinder
{
    public:
        ResultsViewFinder(const QString& id) : m_id(id), found(false) {}
        Area::WalkerMode operator()(View *view, Sublime::Position position) {
            Document* doc = view->document();
            if (m_id == doc->documentSpecifier()) {
                found = true;
                return Area::StopWalker;
            } else {
                return Area::ContinueWalker;
            }
        }
        QString m_id;
        bool found;
};

void TestRunnerToolView::spawnResultsView()
{
    // only allow a single view.
    UiController* uic = Core::self()->uiControllerInternal();
    Area* a = uic->activeArea(); // should iterate over all areas instead
    ResultsViewFinder rvf(this->resultsViewId());
    a->walkToolViews(rvf, Sublime::AllPositions);
    if (rvf.found) {
        return; // dont add twice.
    }

    // these tool views should not show up in the 'Add Tool View' dialog.
    // either use the ugly code below or fix this in UiController.
    //uic->addToolView("Test Results", new ResultsViewFactory(resultsViewId(), this));

    // this is bad. might want to make this available in IUiController
    ResultsViewFactory* fac = new ResultsViewFactory(resultsViewId(), this);
    m_resultsArea = uic->activeArea();
    ToolDocument *doc = new ToolDocument(QString("Test Results"), uic, new UiToolViewFactory(fac));
    m_resultsView = doc->createView();
    Sublime::Position pos = Sublime::dockAreaToPosition(fac->defaultPosition());
    m_resultsArea->addToolView(m_resultsView, pos);
    connect(m_resultsView, SIGNAL(raise(Sublime::View*)),
            uic, SLOT(raiseToolView(Sublime::View*)));
    fac->viewCreated(m_resultsView);
}

#include "testrunnertoolview.moc"

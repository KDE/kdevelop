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
#include <QDockWidget>
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
#include "veritas/mvc/resultsmodel.h"
#include "veritas/mvc/runnerwindow.h"
#include "veritas/mvc/verbosemanager.h"
#include "ui_runnerwindow.h"

Q_DECLARE_METATYPE(KDevelop::IProject*)

using Sublime::Area;
using Sublime::Document;
using Sublime::ToolDocument;
using Sublime::View;

using KDevelop::Core;
using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::IToolViewFactory;
using KDevelop::IUiController;
using KDevelop::UiController;

using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;
using Veritas::ResultsModel;
using Veritas::VerboseManager;
using Veritas::TestViewData;

namespace Veritas
{

class TestViewDataPrivate
{
public:
    TestViewDataPrivate();
    RunnerWindow* window;
    IProject* selectedProject;
    Sublime::View *resultsView;
    Sublime::Area *resultsArea;
    ResultsModel *resultsModel;
};

TestViewDataPrivate::TestViewDataPrivate()
        : window(0),
        selectedProject(0),
        resultsView(0),
        resultsArea(0)
{}

} // namespace

using Veritas::TestViewDataPrivate;

TestViewData::TestViewData(QObject* parent)
        : QObject(parent),
        d(new TestViewDataPrivate)
{
    QStringList resultHeaders;
    resultHeaders << i18n("Test Name") << i18n("Result") << i18n("Message")
    << i18n("File Name") << i18n("Line Number");
    d->resultsModel = new ResultsModel(resultHeaders, this);
    d->window = new RunnerWindow(d->resultsModel);

    connect(this, SIGNAL(registerFinished(Veritas::Test*)),
            this, SLOT(setupToolView(Veritas::Test*)));
}

void TestViewData::setupToolView(Veritas::Test* root)
{
    if (!root) { kDebug() << "root null"; return; }
    RunnerModel* model = new RunnerModel;
    model->setRootItem(root);
    model->setExpectedResults(Veritas::RunError);
    d->window->setModel(model);
    d->window->runnerView()->setRootIsDecorated(false);
    d->window->resetProgressBar();
    spawnResultsView();
}

TestViewData::~TestViewData()
{
    delete d;
}

QWidget* TestViewData::resultsWidget()
{
    return d->window->resultsWidget();
}

void TestViewData::removeResultsView()
{
    if (d->resultsView && d->resultsArea) {
        d->resultsArea->removeToolView(d->resultsView);
    }
}

QWidget* TestViewData::runnerWidget()
{
    IProjectController* ipc = ICore::self()->projectController();
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

    connect(d->window->verboseManager(), SIGNAL(openVerbose(Veritas::Test*)),
            this, SIGNAL(openVerbose(Veritas::Test*)));
    reload();
    return d->window;

}

/*! TODO rename this a bit. just make reload() pure virtual instead of registerTests */
void TestViewData::reload()
{
    registerTests(); // implemented by concrete plugins
}

IProject* TestViewData::project() const
{
    return d->selectedProject;
}

void TestViewData::setSelected(QAction* action)
{
    kDebug() << action->data().value<IProject*>();
    d->selectedProject = action->data().value<IProject*>();
}

class ResultsViewFactory: public KDevelop::IToolViewFactory
{
public:
    ResultsViewFactory(const QString& id, TestViewData *runner): m_runner(runner), m_id(id) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        return m_runner->resultsWidget();
        // TODO this is bad. Currently only a single
        // resultsWidget is ever created, which means
        // that multiple resultsviews for a particular
        // runnertoolview will segfault kdevelop.
        // Introducing multiple resultViews requires
        // changes in RunnerWindow. Luckily a user
        // cannot close nor create multiple ones, so
        // the situation above is impossible.
    }
    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::BottomDockWidgetArea;
    }
    virtual QString id() const {
        return m_id;
    }
    virtual void viewCreated(Sublime::View* view) {
        QWidget* w = view->widget();
        if (not w) return;
        QObject* p = w->parent();
        if (not p) return;
        QDockWidget* d = dynamic_cast<QDockWidget*>(p);
        if (not d) return;
        d->setFeatures(d->features() & 0xfffe); // no close for you.
    }

private:
    TestViewData *m_runner;
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
        //return m_factory->toolBarActions(viewWidget);
        return QList<QAction*>();
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

void TestViewData::spawnResultsView()
{
    // only allow a single view.
    UiController* uic = Core::self()->uiControllerInternal();
    Area* a = uic->activeArea(); // should iterate over all areas instead
    ResultsViewFinder rvf(this->resultsViewId());
    a->walkToolViews(rvf, Sublime::AllPositions);
    if (rvf.found) {
        return; // do not add twice.
    }

    // these tool views should not show up in the 'Add Tool View' dialog.
    // either use the ugly code below or fix this in UiController.
    //uic->addToolView("Test Results", new ResultsViewFactory(resultsViewId(), this));

    // this is bad. might want to make this available in IUiController
    ResultsViewFactory* fac = new ResultsViewFactory(resultsViewId(), this);
    d->resultsArea = uic->activeArea();
    ToolDocument *doc = new ToolDocument(QString("Test Results"), uic, new UiToolViewFactory(fac));
    d->resultsView = doc->createView();
    Sublime::Position pos = Sublime::dockAreaToPosition(fac->defaultPosition());
    d->resultsArea->addToolView(d->resultsView, pos);
    connect(d->resultsView, SIGNAL(raise(Sublime::View*)),
            uic, SLOT(raiseToolView(Sublime::View*)));
    fac->viewCreated(d->resultsView);
}

#include "testrunnertoolview.moc"

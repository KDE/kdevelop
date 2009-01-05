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

// veritas
#include "testrunner.h"
#include "itestframework.h"
#include "itesttreebuilder.h"
#include "internal/toolviewdata.h"
#include "test.h"
#include "internal/selectionstore.h"
#include "ui_runnerwindow.h"

#include "internal/runnermodel.h"
#include "internal/resultsmodel.h"
#include "internal/runnerwindow.h"

// Qt - KDE
#include <KAboutData>
#include <KDebug>
#include <KSelectAction>
#include <QDockWidget>
#include <QWidget>

// kdevelop
#include "interfaces/icore.h"
#include "interfaces/iproject.h"
#include "interfaces/iprojectcontroller.h"
#include "interfaces/iuicontroller.h"
#include "sublime/area.h"
#include "sublime/controller.h"
#include "sublime/mainwindow.h"
#include "sublime/tooldocument.h"
#include "sublime/view.h"

Q_DECLARE_METATYPE(KDevelop::IProject*)

using Sublime::Area;
using Sublime::Document;
using Sublime::ToolDocument;
using Sublime::View;

using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::IToolViewFactory;
using KDevelop::IUiController;

using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::RunnerWindow;
using Veritas::ResultsModel;
using Veritas::TestRunner;
using Veritas::ITestTreeBuilder;
using Veritas::ITestFramework;

class TestRunner::Private
{
public:
    Private() :
        window(0),
        resultsView(0),
        resultsArea(0),
        previousRoot(0)
    {}

    ~Private() {
        delete resultsModel;
    }

    RunnerWindow* window;
    Sublime::View *resultsView;
    Sublime::Area *resultsArea;
    ResultsModel *resultsModel;
    Test* previousRoot;
    ITestFramework* framework;
    TestRunner* self;
    ITestTreeBuilder* treeBuilder;

    QString resultsViewId() const {
        Q_ASSERT(g_toolViewStore.contains(framework));
        ToolViewData& tvd = g_toolViewStore[framework];
        int id = tvd.runner2id[self];
        return QString("org.kdevelop.%1ResultsView%2").arg(framework->name()).arg(id);
    }

};

TestRunner::TestRunner(ITestFramework* framework, ITestTreeBuilder* builder)
    : QObject(0), d(new TestRunner::Private)
{
    d->self = this;
    d->framework = framework;
    d->treeBuilder = builder;
    Q_ASSERT(g_toolViewStore.contains(framework));
    ToolViewData& tvd = g_toolViewStore[framework];
    tvd.runner2id[this] = tvd.runnerToolCounter;
    tvd.runnerToolCounter++;

    QStringList resultHeaders;
    resultHeaders << i18n("Test") << i18n("Message")
                  << i18n("File") << i18n("Line");
    d->resultsModel = new ResultsModel(resultHeaders, this);
    d->window = new RunnerWindow(d->resultsModel);

    connect(d->treeBuilder, SIGNAL(reloadFinished(Veritas::Test*)),
            this, SLOT(setupToolView(Veritas::Test*)));
    connect(d->window->ui()->actionReload, SIGNAL(triggered(bool)),
            SLOT(reloadTree()));
    connect(d->window, SIGNAL(requestReload()), 
            SLOT(reloadTree()));
}

void TestRunner::setupToolView(Veritas::Test* root)
{
    if (!root) { kDebug() << "root null"; return; }
    if (d->previousRoot) {
        SelectionStore ss;
        ss.saveTree(d->previousRoot);
        ss.restoreTree(root);
    }
    d->previousRoot = root;
    RunnerModel* model = new RunnerModel;
    model->setRootItem(root);
    model->setExpectedResults(Veritas::RunError);
    d->window->setModel(model);
    d->window->runnerView()->setRootIsDecorated(false);
    d->window->resetProgressBar();
    if (!d->resultsView) spawnResultsView();
}

TestRunner::~TestRunner()
{
    delete d;
}

QWidget* TestRunner::resultsWidget()
{
    return d->window->resultsWidget();
}

void TestRunner::removeResultsView()
{
    if (d->resultsView && d->resultsArea) {
        d->resultsArea->removeToolView(d->resultsView);
    }
}

QWidget* TestRunner::runnerWidget()
{
    IProjectController* ipc = ICore::self()->projectController();
    foreach(IProject* proj, ipc->projects()) {
        d->window->addProjectToPopup(proj);
    }
    connect(ipc, SIGNAL(projectOpened(KDevelop::IProject*)),
            d->window, SLOT(addProjectToPopup(KDevelop::IProject*)));
    connect(ipc, SIGNAL(projectClosed(KDevelop::IProject*)),
            d->window, SLOT(rmProjectFromPopup(KDevelop::IProject*)));

    return d->window;
}

void TestRunner::reloadTree()
{
    d->treeBuilder->reload(project());
}

IProject* TestRunner::project() const
{
    return d->window->selectedProject();
}

namespace {
class Filter : public QObject
{
public:
    Filter(QObject* parent) : QObject(parent), m_parent(parent) {}
    virtual ~Filter() {}
    bool eventFilter(QObject* obj, QEvent* event) {
        if (event->type() == QEvent::ContextMenu) return true;
        if (m_parent) return m_parent->eventFilter(obj, event);
        return false;
    }
    QObject* m_parent;
};}

class ResultsViewFactory: public KDevelop::IToolViewFactory
{
public:
    ResultsViewFactory(const QString& id, TestRunner *runner): m_runner(runner), m_id(id) {}

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
        if (!w) return;
        QObject* p = w->parent();
        if (!p) return;
        QDockWidget* d = qobject_cast<QDockWidget*>(p);
        if (!d) return;
        d->setFeatures(d->features() & 0xfffc);
        Filter* f = new Filter(d);
        d->installEventFilter(f);
    }

private:
    TestRunner *m_runner;
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
        Q_UNUSED(viewWidget);
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
        Q_UNUSED(position);
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

void TestRunner::spawnResultsView()
{
    // only allow a single view.
    IUiController* uic = ICore::self()->uiController();
    Sublime::Controller* ctr = uic->controller();
    foreach(Area* a, ctr->allAreas()) {
        ResultsViewFinder rvf(d->resultsViewId());
        a->walkToolViews(rvf, Sublime::AllPositions);
        if (rvf.found) {
            kDebug() << "Not adding twice, found a resultsview for this runner.";
            return;
        }
    }

    // these tool views should not show up in the 'Add Tool View' dialog.
    // either use the ugly code below or fix this in UiController.
    //uic->addToolView("Test Results", new ResultsViewFactory(resultsViewId(), this));

    // this is bad. might want to make this available in IUiController
    ResultsViewFactory* fac = new ResultsViewFactory(d->resultsViewId(), this);
    QList<Sublime::MainWindow*> mws = ctr->mainWindows();
    if (mws.isEmpty() || !mws[0]) {
        kDebug() << "No mainwindow not adding results view.";
        return;
    }

    Sublime::MainWindow* mw = mws[0]; // just take the first one.
    d->resultsArea = mw->area();
    ToolDocument *doc = new ToolDocument(QString("Test Results"), ctr, new UiToolViewFactory(fac));
    d->resultsView = doc->createView();
    Sublime::Position pos = Sublime::dockAreaToPosition(fac->defaultPosition());
    d->resultsArea->addToolView(d->resultsView, pos);
    connect(d->resultsView, SIGNAL(raise(Sublime::View*)),
            ctr, SLOT(raiseToolView(Sublime::View*)));
    fac->viewCreated(d->resultsView);
}

#include "testrunner.moc"

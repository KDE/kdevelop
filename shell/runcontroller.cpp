/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "runcontroller.h"

#include "core.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "projectcontroller.h"
#include "iproject.h"
#include "mainwindow.h"
#include "ioutputview.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QItemDelegate>

#include <KSelectAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

using namespace KDevelop;

class RunController::RunControllerPrivate
{
public:
    QItemDelegate* delegate;

    IRunController::State state;

    QHash<KJob*, KAction*> jobs;
    KActionMenu* stopAction;
    KSelectAction* defaultTargetAction;
};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    // TODO: need to implement compile only if needed before execute
    // TODO: need to implement abort all running programs when project closed

    d->state = Idle;
    d->delegate = new QItemDelegate(this);

    setupActions();
}

KJob* RunController::execute(const IRun & run)
{
    RunJob* job = new RunJob(this, run);
    registerJob(job);
    return job;
}

RunController::~ RunController()
{
    delete d;
}

void RunController::setupActions()
{
    KAction *action;

    // TODO not multi-window friendly, FIXME
    KActionCollection* ac = Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    action = new KAction (i18n("Configure Launches"), this);
    ac->addAction("configure_launches", action);

    action = new KAction( KIcon("system-run"), i18n("Execute Program"), this);
    action->setShortcut(Qt::SHIFT + Qt::Key_F9);
    action->setToolTip(i18n("Execute program"));
    action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the currently active target or the main program specified in project settings, <b>Run Options</b> tab."));
    ac->addAction("run_execute", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotExecute()));

    action = d->stopAction = new KActionMenu( KIcon("dialog-close"), i18n("Stop Jobs"), this);
    action->setShortcut(Qt::Key_Escape);
    action->setToolTip(i18n("Stop all currently running jobs"));
    action->setWhatsThis(i18n("<b>Stop Jobs</b><p>Requests that all running jobs are stopped."));
    action->setEnabled(false);
    ac->addAction("run_stop", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stopAllProcesses()));

    d->defaultTargetAction = new KSelectAction( i18n("Default Run Target"), this);
    d->defaultTargetAction->setToolTip(i18n("Current Run Target"));
    d->defaultTargetAction->setWhatsThis(i18n("<b>Run Target</b><p>Select which target to run when run is invoked."));
    ac->addAction("run_default_target", d->defaultTargetAction);

    bool first = true;
    foreach (IProject* project, Core::self()->projectController()->projects()) {
        QAction* action = d->defaultTargetAction->addAction(i18n("Default for project '%1'", project->name()));
        action->setData(QVariant::fromValue(static_cast<void*>(project)));

        if (first) {
            // TODO save this setting
            action->setChecked(true);
            first = false;
        }
    }

    connect(Core::self()->projectController(), SIGNAL(projectOpened( KDevelop::IProject* )), this, SLOT(slotProjectOpened(KDevelop::IProject*)));
    connect(Core::self()->projectController(), SIGNAL(projectClosing( KDevelop::IProject* )), this, SLOT(slotProjectClosing(KDevelop::IProject*)));
}

void KDevelop::RunController::slotProjectOpened(KDevelop::IProject * project)
{
    QAction* action = d->defaultTargetAction->addAction(i18n("Default for project '%1'", project->name()));
    action->setData(QVariant::fromValue(static_cast<void*>(project)));

    if (!d->defaultTargetAction->currentAction())
        action->setChecked(true);
}

void KDevelop::RunController::slotProjectClosing(KDevelop::IProject * project)
{
    foreach (QAction* action, d->defaultTargetAction->actions()) {
        if (project == qvariant_cast<void*>(action->data())) {
            bool wasSelected = action->isChecked();
            delete action;
            if (wasSelected)
                if (!d->defaultTargetAction->actions().isEmpty())
                    d->defaultTargetAction->actions().first()->setChecked(true);
        }
    }
}

void RunController::slotExecute()
{
    execute(defaultRun());
}

IRun KDevelop::RunController::defaultRun() const
{
    IProject* project = 0;
    QAction* projectAction = d->defaultTargetAction->currentAction();
    if (projectAction)
        project = static_cast<IProject*>(qvariant_cast<void*>(projectAction->data()));

    IRun run;

    if (!project)
        return run;

    KConfigGroup group(project->projectConfiguration(), "Run Options" );

    run.setExecutable(group.readEntry( "Executable", "" ));
    run.setWorkingDirectory(group.readEntry( "Working Directory", "" ));
    QString arg=group.readEntry( "Arguments", QString() );
    if(!arg.isEmpty())
        run.setArguments(QStringList(arg));
    run.setInstrumentor("default");

    if (group.readEntry("Compile Before Execution", false))
        if (group.readEntry("Install Before Execution", false))
            if (group.readEntry("Super User Install", false))
                ;// TODO: sudo make install
            else
                ;// TODO: make install
        else
            ;// TODO: make

    if (group.readEntry("Start In Terminal", false))
        ;// TODO: start in terminal rather than output view

    return run;
}

IRunProvider * KDevelop::RunController::findProvider(const QString & instrumentor)
{
    foreach (IPlugin* i, Core::self()->pluginController()->allPluginsForExtension("org.kdevelop.IRunProvider", QStringList()))
        if (KDevelop::IRunProvider* provider = i->extension<KDevelop::IRunProvider>())
            if (provider->instrumentorsProvided().contains(instrumentor))
                return provider;

    return 0;
}

void KDevelop::RunController::registerJob(KJob * job)
{
    if (!job)
        return;

    if (!d->jobs.contains(job)) {
        KAction* stopJobAction = new KAction(job->objectName().isEmpty() ? i18n("Unnamed job") : job->objectName(), this);
        stopJobAction->setData(QVariant::fromValue(static_cast<void*>(job)));
        d->stopAction->addAction(stopJobAction);
        connect (stopJobAction, SIGNAL(triggered(bool)), SLOT(slotKillJob()));

        d->jobs.insert(job, stopJobAction);

        IRunController::registerJob(job);

        emit jobRegistered(job);
    }

    job->start();

    checkState();
}

void KDevelop::RunController::unregisterJob(KJob * job)
{
    IRunController::unregisterJob(job);

    Q_ASSERT(d->jobs.contains(job));

    // Delete the stop job action
    delete d->jobs.take(job);

    checkState();

    emit jobUnregistered(job);
}

void KDevelop::RunController::checkState()
{
    bool running = false;

    foreach (KJob* job, d->jobs.keys()) {
        if (!job->isSuspended()) {
            running = true;
            break;
        }
    }

    if (d->state != running ? Running : Idle) {
        d->state = running ? Running : Idle;
        emit runStateChanged(d->state);
    }

    d->stopAction->setEnabled(running);
}

void KDevelop::RunController::stopAllProcesses()
{
    foreach (KJob* job, d->jobs.keys()) {
        if (job->capabilities() & KJob::Killable)
            job->kill(KJob::EmitResult);
    }
}

void KDevelop::RunController::slotKillJob()
{
    KAction* action = dynamic_cast<KAction*>(sender());
    Q_ASSERT(action);

    KJob* job = static_cast<KJob*>(qvariant_cast<void*>(action->data()));
    if (job->capabilities() & KJob::Killable)
        job->kill();
}

void KDevelop::RunController::finished(KJob * job)
{
    switch (job->error()) {
        case KJob::NoError:
        case KJob::KilledJobError:
            break;

        default:
            KMessageBox::error(qApp->activeWindow(), job->errorString(), i18n("Process Error"));
    }

    unregisterJob(job);
}

void KDevelop::RunController::suspended(KJob * job)
{
    Q_UNUSED(job);

    checkState();
}

void KDevelop::RunController::resumed(KJob * job)
{
    Q_UNUSED(job);

    checkState();
}

KDevelop::RunJob::RunJob(RunController* controller, const IRun & run)
    : m_controller(controller)
    , m_provider(0)
    , m_run(run)
{
    setCapabilities(Killable);

    QString instrumentorName = i18n("Run");
    if (!m_run.instrumentor().isEmpty()) {
        m_provider = m_controller->findProvider(m_run.instrumentor());
        if (m_provider) {
            instrumentorName = m_provider->translatedInstrumentor(run.instrumentor());
        }
    }
    setObjectName(i18n("%1: %2", instrumentorName, run.executable().path()));
}

void KDevelop::RunJob::start()
{
    if (m_run.instrumentor().isEmpty()) {
        setErrorText(i18n("No run target was selected. Please select a default run target in the Run menu."));
        setError(ErrorInvalidTarget);
        emitResult();
        return;
    }

    if (!m_provider) {
        setErrorText(i18n("Execution failed: no plugin found for requested instrumentor \"%1\"", m_run.instrumentor()));
        setError(ErrorNoProvider);
        emitResult();
        return;
    }

    QObject* m_providerObject = dynamic_cast<QObject*>(m_provider);
    Q_ASSERT(m_providerObject);

    connect(m_providerObject, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
    connect(m_providerObject, SIGNAL(output(KJob*, const QString&, KDevelop::IRunProvider::OutputTypes)), this, SLOT(slotOutput(KJob*, const QString&, KDevelop::IRunProvider::OutputTypes)));

    m_provider->execute(m_run, this);

    setStandardToolView(IOutputView::RunView);
    setDelegate(m_controller->delegate());
    setTitle(m_run.executable().path());
    setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    startOutput();
}

QList< KJob * > KDevelop::RunController::currentJobs() const
{
    return d->jobs.keys();
}

void RunJob::slotOutput(KJob * job, const QString & line, KDevelop::IRunProvider::OutputTypes type)
{
    if (job != this)
        return;

    if (!model())
        return;

    QStandardItem* item = new QStandardItem(line);

    switch (type) {
        case IRunProvider::StandardError:
            item->setForeground(Qt::red);
            break;
        case IRunProvider::RunProvider:
            item->setForeground(Qt::blue);
            break;
        default:
            break;
    }

    model()->appendRow(item);
}

void KDevelop::RunJob::slotFinished(KJob * job)
{
    if (job == this)
        emitResult();
}

bool KDevelop::RunJob::doKill()
{
    m_provider->abort(this);

    return true;
}

QItemDelegate * KDevelop::RunController::delegate() const
{
    return d->delegate;
}

#include "runcontroller.moc"

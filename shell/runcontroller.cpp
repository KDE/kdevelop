/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>
Copyright 2008 Aleix Pol <aleixpol@gmail.com>

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

#include <QApplication>
#include <QStandardItemModel>
#include <QPalette>

#include <KDE/KSelectAction>
#include <KDE/KActionMenu>
#include <KDE/KActionCollection>
#include <KDE/KMessageBox>
#include <KDE/KLocale>
#include <KDE/KDebug>
#include <KDE/KColorScheme>
#include <KDE/KCompositeJob>
#include <kdialogjobuidelegate.h>

#include <interfaces/iproject.h>
#include <interfaces/idocumentcontroller.h>
#include <outputview/ioutputview.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "core.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "projectcontroller.h"
#include "mainwindow.h"

using namespace KDevelop;

typedef QPair<QString, IProject*> Target;
Q_DECLARE_METATYPE(Target)

class RunController::RunControllerPrivate
{
public:
    QItemDelegate* delegate;

    IRunController::State state;

    QHash<KJob*, KAction*> jobs;
    KActionMenu* stopAction;
    KSelectAction* currentTargetAction;
};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    setObjectName("RunController");
    // TODO: need to implement compile only if needed before execute
    // TODO: need to implement abort all running programs when project closed

    d->state = Idle;
    d->delegate = new RunDelegate(this);

    if(!(Core::self()->setupFlags() & Core::NoUi)) setupActions();
}

void RunController::initialize()
{
}

class ExecuteCompositeJob : public KCompositeJob
{
    public:
        ExecuteCompositeJob(QObject* parent, const QList<KJob*>& jobs)
            : KCompositeJob(parent)
            , m_killing(false)
        {
            setCapabilities(Killable);

            qDebug() << "execute composite" << jobs;
            foreach(KJob* job, jobs) {
                addSubjob(job);
            }
        }

    public slots:
        virtual void start()
        {
            if(hasSubjobs())
                subjobs().first()->start();
            else
                emitResult();
        }

        void slotResult(KJob* job)
        {
            kDebug() << "finished: "<< job << job->error() << error();
            KCompositeJob::slotResult(job);

            if(hasSubjobs() && !error() && !m_killing)
            {
                kDebug() << "remaining: " << subjobs().count() << subjobs();
                KJob* nextJob=subjobs().first();
                nextJob->start();
            } else {
                emitResult();
            }
        }

    protected:
        virtual bool doKill()
        {
            m_killing = true;
            if(hasSubjobs()) {
                KJob* j = subjobs().first();
                if (j->kill()) {
                    removeSubjob(j);
                    return true;
                } else {
                    return false;
                }
            }
            return true;
        }

    private:
        bool m_killing;
};

KJob* RunController::execute(const IRun & run)
{
    if(!run.dependencies().isEmpty())
        ICore::self()->documentController()->saveAllDocuments(IDocument::Silent);

    QList<KJob*> jobs;
    foreach(KJob* job, run.dependencies())
    {
        jobs.append(job);
    }

    jobs.append(new RunJob(this, run));
    ExecuteCompositeJob* ecj=new ExecuteCompositeJob(this, jobs);
    ecj->setObjectName(jobs.last()->objectName());
    registerJob(ecj);
    return ecj;
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

    d->currentTargetAction = new KSelectAction( i18n("Current Run Target"), this);
    d->currentTargetAction->setToolTip(i18n("Current Run Target"));
    d->currentTargetAction->setWhatsThis(i18n("<b>Run Target</b><p>Select which target to run when run is invoked."));
    ac->addAction("run_default_target", d->currentTargetAction);

    foreach (IProject* project, Core::self()->projectController()->projects()) {
        slotProjectOpened(project);
    }

    if(!d->currentTargetAction->actions().isEmpty())
        d->currentTargetAction->actions().first()->setChecked(true);
    connect(Core::self()->projectController(), SIGNAL(projectOpened( KDevelop::IProject* )),
            this, SLOT(slotProjectOpened(KDevelop::IProject*)));
    connect(Core::self()->projectController(), SIGNAL(projectClosing( KDevelop::IProject* )),
            this, SLOT(slotProjectClosing(KDevelop::IProject*)));
    connect(Core::self()->projectController(), SIGNAL(projectConfigurationChanged(KDevelop::IProject*)),
             this, SLOT(slotRefreshProject(KDevelop::IProject*)));
}

QAction* KDevelop::RunController::addTarget(KDevelop::IProject * project, const QString& targetName)
{
    // First check weather we already have the action, if there's a large number
    // we might need to use a map/hash for storing the data instead of
    // QAction::data()
    bool found = false;
    foreach( const QAction* a, d->currentTargetAction->actions() ) {
        Target t = qVariantValue<Target>( a->data() );
        if( t.first == targetName && t.second == project )
        {
            found = true;
            break;
        }
    }
    QAction* action = 0;
    if( !found )
    {
        action = d->currentTargetAction->addAction(i18n("%1 : %2", project->name(), targetName));
        action->setData(qVariantFromValue<Target>(Target(targetName, project)));
    }
    return action;
}

void KDevelop::RunController::slotProjectOpened(KDevelop::IProject * project)
{
    KConfigGroup group(project->projectConfiguration(), "Run Options");
    QStringList runTargets = group.readEntry("Run Targets", QStringList());

    QAction* a=0;
    foreach(const QString& target, runTargets) {
        a=addTarget(project, target);
    }

    if(a)
        a->setChecked(true);
}

void KDevelop::RunController::slotProjectClosing(KDevelop::IProject * project)
{
    foreach (QAction* action, d->currentTargetAction->actions()) {
        if (project == qvariant_cast<Target>(action->data()).second) {
            bool wasSelected = action->isChecked();
            delete action;
            if (wasSelected)
                if (!d->currentTargetAction->actions().isEmpty())
                    d->currentTargetAction->actions().first()->setChecked(true);
        }
    }
}

void KDevelop::RunController::slotRefreshProject(KDevelop::IProject* project)
{
    slotProjectClosing(project);
    slotProjectOpened(project);
}

void RunController::slotExecute()
{
    execute(defaultRun());
}

QStringList splitArguments(const QString& args)
{
    QStringList ret;
    bool inQuotes=false, scaping=false;
    for(int i=0; i<args.size(); i++)
    {
        if(i==0) ret += QString();

        if(scaping)
        {
            ret.last() += args[i];
            scaping=false;
        }
        else switch(args[i].toAscii())
        {
            case '\\':
                scaping=true;
                break;
            case '\"':
                inQuotes=!inQuotes;
                break;
            case ' ':
                if(inQuotes)
                    ret.last() += args[i];
                else
                    ret += QString();

                break;
            default:
                ret.last() += args[i];
                break;
        }

    }
    return ret;
}

//Copied from projectitemlineedit.cpp
QModelIndex pathToIndex(const QAbstractItemModel* model, const QStringList& tofetch)
{
    if(tofetch.isEmpty())
        return QModelIndex();

    QModelIndex current=model->index(0,0, QModelIndex());

    QModelIndex ret;
    foreach(const QString& currentName, tofetch)
    {
        QModelIndexList l = model->match(current, Qt::EditRole, currentName, 1, Qt::MatchExactly);
        if(l.count()>0) {
            ret=l.first();
            current = model->index(0,0, ret);
        } else
            current = QModelIndex();
    }
    Q_ASSERT(model->data(ret).toString()==tofetch.last());
    return ret;
}

IRun KDevelop::RunController::defaultRun() const
{
    IProject* project = 0;
    IRun run;

    QAction* projectAction = d->currentTargetAction->currentAction();

    Target data;
    if (projectAction) {
        data=qvariant_cast<Target>(projectAction->data());
        project = data.second;
    }

    if (!project)
        return run;

    QString targetName=data.first;

    KConfigGroup group(project->projectConfiguration(), targetName+"-Run Options" );

    QString exec=group.readEntry("Executable", QString());
    ProjectModel *model=ICore::self()->projectController()->projectModel();
    if(exec.isEmpty())
    {
        QString target=group.readEntry("Run Item", QString());
        QModelIndex idx=pathToIndex(model, target.split('/'));
        if( idx.isValid() )
        {
            ProjectBaseItem *it=model->item(idx);
            // This should never happen, pathToIndex asks
            // the model for indexes that match a target
            // so the indexes should always be convertable to an item
            Q_ASSERT(it);
            if(it->executable())
            {
                exec=it->executable()->builtUrl().toLocalFile();
            } else
            {
                KMessageBox::error(0, i18n("Target '%1' is not executable.", target));
            }
        } else
        {
            KMessageBox::error(0, i18n("Target '%1' could not be found.", target));
        }
    }
    //FIXME: throw error
    run.setExecutable(exec);
    run.setWorkingDirectory(group.readEntry("Working Directory", QString()));
    run.setArguments(splitArguments(group.readEntry("Arguments", QString())));
    if (group.readEntry("Start In Terminal", false))
        run.setInstrumentor("konsole");
    else
        run.setInstrumentor("default");
    run.setRunAsUser(group.readEntry("Run As User", QString()));

    QStringList compileItems=group.readEntry("Compile Items", QStringList());
    int actionDeps=group.readEntry("BeforeExecute", 1);

    QList<KJob*> comp;
    if(actionDeps!=0)
    {
        foreach(const QString& it, compileItems)
        {
            QModelIndex idx=pathToIndex(model, it.split('/'));
            ProjectBaseItem *pit=model->item(idx);

            if(!pit)
            {
                KMessageBox::error(0, i18n("Could not find %1", it));
                continue;
            }

            IProject* project = pit->project();
            if (!project)
                continue;

            IPlugin* fmgr = project->managerPlugin();
            IBuildSystemManager* mgr = fmgr->extension<IBuildSystemManager>();
            IProjectBuilder* builder;
            if( mgr )
            {
                builder=mgr->builder( project->projectItem() );
                KJob* buildJob;
                switch(actionDeps)
                {
                    case 1:
                        buildJob=builder->build(pit);
                        break;
                    case 2:
                        buildJob=builder->install(pit);
                        break;
                    case 3:
#ifdef __GNUC__
                        #warning make it install as superuser.
#endif
                        break;
                }
                comp+=buildJob;
            }
            else
            {
                kDebug() << "Failed to compile";
            }
        }
        run.setDependencies(comp);
    }

    return run;
}

IRunProvider * KDevelop::RunController::findProvider(const QString & instrumentor)
{
    foreach (IPlugin* i, Core::self()->pluginController()->allPluginsForExtension("org.kdevelop.IRunProvider", QStringList())) {
        KDevelop::IRunProvider* provider = i->extension<KDevelop::IRunProvider>();
        if (provider && provider->instrumentorsProvided().contains(instrumentor))
                return provider;
    }

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

        job->setUiDelegate( new KDialogJobUiDelegate() );

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

    if ( ( d->state != Running ? false : true ) == running ) {
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
    unregisterJob(job);

    switch (job->error()) {
        case KJob::NoError:
        case KJob::KilledJobError:
            break;

        default:
            KMessageBox::error(qApp->activeWindow(), job->errorString(), i18n("Process Error"));
    }
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
        setErrorText(i18n("No run target was selected. Please select a run target in the Run menu."));
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

    // TODO should this really be hard-coded? or should the provider get a say...
    // Don't show the run output if the process is being run in konsole
    if (m_run.instrumentor() != "konsole") {
        setStandardToolView(IOutputView::RunView);
        setDelegate(m_controller->delegate());
        setTitle(m_run.executable().path());
        setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
        startOutput();
    }
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

    if( model()->columnCount() == 0 )
    {
        model()->insertColumns( 0, 1 );
    }

    int rowCount = model()->rowCount();
    model()->insertRows( rowCount, 1 );
    QModelIndex row_idx = model()->index( rowCount, 0 );
    model()->setData( row_idx, QVariant( line ) );
    model()->setData( row_idx, QVariant::fromValue(type), Qt::UserRole+1 );
}

void KDevelop::RunJob::slotFinished(KJob * job)
{
    if (job == this)
        emitResult();
}

bool KDevelop::RunJob::doKill()
{
    setError(KJob::KilledJobError);
    m_provider->abort(this);

    return true;
}

QItemDelegate * KDevelop::RunController::delegate() const
{
    return d->delegate;
}

RunDelegate::RunDelegate( QObject* parent )
: QItemDelegate(parent), runProviderBrush( KColorScheme::View, KColorScheme::PositiveText ),
errorBrush( KColorScheme::View, KColorScheme::NegativeText )
{
}

void RunDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(Qt::UserRole+1);
    if( status.isValid() && status.canConvert<KDevelop::IRunProvider::OutputTypes>() )
    {
        IRunProvider::OutputTypes type = status.value<KDevelop::IRunProvider::OutputTypes>();
        if( type == IRunProvider::RunProvider )
        {
            opt.palette.setBrush( QPalette::Text, runProviderBrush.brush( option.palette ) );
        } else if( type == IRunProvider::StandardError )
        {
            opt.palette.setBrush( QPalette::Text, errorBrush.brush( option.palette ) );
        }
    }
    QItemDelegate::paint(painter, opt, index);
}


#include "runcontroller.moc"

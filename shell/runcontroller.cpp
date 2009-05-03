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
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>
#include <outputview/ioutputview.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "core.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "projectcontroller.h"
#include "mainwindow.h"
#include "launchconfiguration.h"
#include "launchconfigurationdialog.h"
#include <interfaces/isession.h>
#include <QSignalMapper>

using namespace KDevelop;

QString RunController::LaunchConfigurationsGroup = "Launch";
QString RunController::LaunchConfigurationsListEntry = "Launch Configurations";
static QString CurrentLaunchConfigProjectEntry = "Current Launch Config Project";
static QString CurrentLaunchConfigNameEntry = "Current Launch Config GroupName";

typedef QPair<QString, IProject*> Target;
Q_DECLARE_METATYPE(Target)


//TODO: Doesn't handle add/remove of launch configs in the dialog or renaming of configs
//TODO: Doesn't auto-select launch configs opened from projects

class DebugMode : public ILaunchMode
{
public:
    DebugMode() {}
    virtual KIcon icon() const { return KIcon(); }
    virtual QString id() const { return "debug"; }
    virtual QString name() const { return i18n("Debug"); }
};

class ProfileMode : public ILaunchMode
{
public:
    ProfileMode() {}
    virtual KIcon icon() const { return KIcon(); }
    virtual QString id() const { return "profile"; }
    virtual QString name() const { return i18n("Profile"); }
};

class ExecuteMode : public ILaunchMode
{
public:
    ExecuteMode() {}
    virtual KIcon icon() const { return KIcon("system-run"); }
    virtual QString id() const { return "execute"; }
    virtual QString name() const { return i18n("Execute"); }
};

class RunController::RunControllerPrivate
{
public:
    QItemDelegate* delegate;

    IRunController::State state;

    RunController* q;

    QHash<KJob*, KAction*> jobs;
    KActionMenu* stopAction;
    KSelectAction* currentTargetAction;
    QMap<QString,LaunchConfigurationType*> launchConfigurationTypes;
    QList<LaunchConfiguration*> launchConfigurations;
    QMap<QString,ILaunchMode*> launchModes;
    QSignalMapper* launchChangeMapper;
    bool hasLaunchConfigType( const QString& typeId ) 
    {
        return launchConfigurationTypes.contains( typeId );
    }
    void saveCurrentLaunchAction()
    {
        if( currentTargetAction->currentAction() )
        {
            KConfigGroup grp = Core::self()->activeSession()->config()->group( RunController::LaunchConfigurationsGroup );
            LaunchConfiguration* l = static_cast<LaunchConfiguration*>( qVariantValue<void*>( currentTargetAction->currentAction()->data() ) );
            grp.writeEntry( CurrentLaunchConfigProjectEntry, l->project() ? l->project()->name() : "" );
            grp.writeEntry( CurrentLaunchConfigNameEntry, l->configGroupName() );
            grp.sync();
        }
    }
    void configureLaunches()
    {
        LaunchConfigurationDialog dlg;
        dlg.exec();
    }
    
    QString launchActionText( LaunchConfiguration* l )
    {
        QString label;
        if( l->project() ) 
        {
            label = QString("%1 : %2").arg( l->project()->name()).arg(l->name());
        } else
        {
            label = QString("%1" ).arg(l->name());
        }
        return label;
    }
    
    void updateCurrentLaunchAction()
    {
        KConfigGroup launchGrp = Core::self()->activeSession()->config()->group( RunController::LaunchConfigurationsGroup );
        QString currentLaunchProject = launchGrp.readEntry( CurrentLaunchConfigProjectEntry, "" );
        QString currentLaunchName = launchGrp.readEntry( CurrentLaunchConfigNameEntry, "" );
        
        LaunchConfiguration* l = 0;
        if( currentTargetAction->currentAction() )
        {
            l = static_cast<LaunchConfiguration*>( qVariantValue<void*>( currentTargetAction->currentAction()->data() ) );
        } else if( !launchConfigurations.isEmpty() )
        {
            l = launchConfigurations.at( 0 );
        }

        if( l && ( ( !currentLaunchProject.isEmpty() && ( !l->project() || l->project()->name() != currentLaunchProject ) ) || l->configGroupName() != currentLaunchName ) )
        {
            foreach( QAction* a, currentTargetAction->actions() )
            {
                LaunchConfiguration* l = static_cast<LaunchConfiguration*>( qvariant_cast<void*>( a->data() ) );
                if( currentLaunchName == l->configGroupName() 
                    && ( ( currentLaunchProject.isEmpty() && !l->project() ) 
                         || ( l->project() && l->project()->name() == currentLaunchProject ) ) )
                {
                    a->setChecked( true );
                    break;
                }
            }
        }
        if( !currentTargetAction->currentAction() )
        {
            kDebug() << "oops no current action, using first if list is non-empty";
            if( !currentTargetAction->actions().isEmpty() )
            {
                currentTargetAction->actions().first()->setChecked( true );
            }
        }
    }

    void addLaunchAction( LaunchConfiguration* l )
    {
        KAction* action = currentTargetAction->addAction(launchActionText( l ));
        action->setData(qVariantFromValue<void*>(l));
    }
    void readLaunchConfigs( KSharedConfigPtr cfg, IProject* prj )
    {
        KConfigGroup group(cfg, RunController::LaunchConfigurationsGroup);
        QStringList configs = group.readEntry( RunController::LaunchConfigurationsListEntry, QStringList() );

        foreach( const QString& cfg, configs )
        {
            KConfigGroup grp = group.group( cfg );
            if( launchConfigurationTypeForId( grp.readEntry( LaunchConfiguration::LaunchConfigurationTypeEntry, "" ) ) )
            {
                q->addLaunchConfiguration( new LaunchConfiguration( grp, prj ) );
            }
        }
    }
    LaunchConfigurationType* launchConfigurationTypeForId( const QString& id )
    {
        QMap<QString, LaunchConfigurationType*>::iterator it = launchConfigurationTypes.find( id );
        if( it != launchConfigurationTypes.end() ) 
        {
            return it.value();
        } else
        {
            kWarning() << "couldn't find type for id:" << id << ". Known types:" << launchConfigurationTypes.keys();
        }
        return 0;
        
    }

};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    setObjectName("RunController");
    // TODO: need to implement compile only if needed before execute
    // TODO: need to implement abort all running programs when project closed

    d->state = Idle;
    d->q = this;
    d->delegate = new RunDelegate(this);
    d->launchChangeMapper = new QSignalMapper( this );
    connect(d->launchChangeMapper, SIGNAL(mapped(int)), SLOT(launchChanged(int)) );

    if(!(Core::self()->setupFlags() & Core::NoUi)) {
        // Note that things like registerJob() do not work without the actions, it'll simply crash.
        setupActions();
    }
}

void KDevelop::RunController::launchChanged( int i )
{
    LaunchConfiguration* l = d->launchConfigurations.at( i );
    foreach( QAction* a, d->currentTargetAction->actions() )
    {
        if( static_cast<LaunchConfiguration*>( qVariantValue<void*>( a->data() ) ) == l )
        {
            a->setText( d->launchActionText( l ) );
            break;
        }
    }
}

void RunController::cleanup()
{
    
}

void RunController::initialize()
{
    addLaunchMode( new ExecuteMode() );
    addLaunchMode( new ProfileMode() );
    addLaunchMode( new DebugMode() );
    d->readLaunchConfigs( Core::self()->activeSession()->config(), 0 );

    foreach (IProject* project, Core::self()->projectController()->projects()) {
        slotProjectOpened(project);
    }
    connect(Core::self()->projectController(), SIGNAL(projectOpened( KDevelop::IProject* )),
            this, SLOT(slotProjectOpened(KDevelop::IProject*)));
    connect(Core::self()->projectController(), SIGNAL(projectClosing( KDevelop::IProject* )),
            this, SLOT(slotProjectClosing(KDevelop::IProject*)));
    connect(Core::self()->projectController(), SIGNAL(projectConfigurationChanged(KDevelop::IProject*)),
             this, SLOT(slotRefreshProject(KDevelop::IProject*)));

    if((Core::self()->setupFlags() & Core::NoUi)) return;

    d->updateCurrentLaunchAction();
}

KJob* RunController::execute(const QString& runMode, LaunchConfiguration* run)
{
    if( !run )
    {
        kDebug() << "execute called without launch config!";
        return 0;
    }
    //TODO: Port to launch framework, probably needs to be part of the launcher
    //if(!run.dependencies().isEmpty())
    //    ICore::self()->documentController()->saveAllDocuments(IDocument::Silent);

    //foreach(KJob* job, run.dependencies())
    //{
    //    jobs.append(job);
    //}

    kDebug() << "mode:" << runMode;
    QString launcherId = run->launcherForMode( runMode );
    kDebug() << "launcher id:" << launcherId;
    
    ILauncher* launcher = run->type()->launcherForId( launcherId );
    
    if( !launcher ) {
        kWarning() << i18n("Launcher could not be found for the name '%1'. Check the launch configuration.", launcherId );
        return 0;
    }

    KJob* launch = launcher->start(runMode, run);
    registerJob(launch);
    return launch;
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
    action->setStatusTip(i18n("Open Launch Configuration Dialog"));
    action->setToolTip(i18n("Open Launch Configuration Dialog"));
    action->setWhatsThis(i18n("<p>Opens a dialog to setup new launch configurations, or to change the existing ones.</p>"));
    connect(action, SIGNAL(triggered(bool)), SLOT(configureLaunches()));

    action = new KAction( KIcon("system-run"), i18n("Execute Launch"), this);
    action->setShortcut(Qt::SHIFT + Qt::Key_F9);
    action->setToolTip(i18n("Execute current Launch"));
    action->setStatusTip(i18n("Execute current Launch"));
    action->setWhatsThis(i18n("<b>Execute Launch</b><p>Executes the target or the program specified in currently active launch configuration.</p>"));
    ac->addAction("run_execute", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotExecute()));
    
    action = new KAction( KIcon("dbgrun"), i18n("Debug Launch"), this);
    action->setShortcut(Qt::Key_F9);
    action->setToolTip(i18n("Debug current Launch"));
    action->setStatusTip(i18n("Debug current Launch"));
    action->setWhatsThis(i18n("<b>Debug Launch</b><p>Executes the target or the program specified in currently active launch configuration inside a Debugger.</p>"));
    ac->addAction("run_debug", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotDebug()));

    action = d->stopAction = new KActionMenu( KIcon("dialog-close"), i18n("Stop Jobs"), this);
    action->setShortcut(Qt::Key_Escape);
    action->setToolTip(i18n("Stop all currently running jobs"));
    action->setWhatsThis(i18n("<b>Stop Jobs</b><p>Requests that all running jobs are stopped.</p>"));
    action->setEnabled(false);
    ac->addAction("run_stop", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stopAllProcesses()));

    d->currentTargetAction = new KSelectAction( i18n("Current Launch Configuration"), this);
    d->currentTargetAction->setToolTip(i18n("Current Launch Configuration"));
    d->currentTargetAction->setStatusTip(i18n("Current Launch Configuration"));
    d->currentTargetAction->setWhatsThis(i18n("<p>Select which launch configuration to run when run is invoked.</p>"));
    ac->addAction("run_default_target", d->currentTargetAction);
}

LaunchConfigurationType* RunController::launchConfigurationTypeForId( const QString& id )
{
    return d->launchConfigurationTypeForId( id );
}

void KDevelop::RunController::slotProjectOpened(KDevelop::IProject * project)
{
    d->readLaunchConfigs( project->projectConfiguration(), project );
    d->updateCurrentLaunchAction();
}

void KDevelop::RunController::slotProjectClosing(KDevelop::IProject * project)
{
    foreach (QAction* action, d->currentTargetAction->actions()) {
        LaunchConfiguration* l = static_cast<LaunchConfiguration*>(qvariant_cast<void*>(action->data()));
        if ( project == l->project() ) {
            l->save();
            d->launchConfigurations.removeAll(l);
            delete l;
            bool wasSelected = action->isChecked();
            delete action;
            if (wasSelected && !d->currentTargetAction->actions().isEmpty())
                d->currentTargetAction->actions().first()->setChecked(true);
        }
    }
}

void KDevelop::RunController::slotRefreshProject(KDevelop::IProject* project)
{
    slotProjectClosing(project);
    slotProjectOpened(project);
}

void RunController::slotDebug()
{
    executeDefaultLaunch( "debug" );
}

void RunController::slotExecute()
{
    executeDefaultLaunch( "execute" );
}

LaunchConfiguration* KDevelop::RunController::defaultLaunch() const
{
    QAction* projectAction = d->currentTargetAction->currentAction();
    if( projectAction )
        return static_cast<LaunchConfiguration*>(qvariant_cast<void*>(projectAction->data()));
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
        //TODO: This produces nasty messageboxes atm, so re-activate only after
        //moving to a nicer job delegate
        // connect( job, SIGNAL(finished(KJob*)), SLOT(finished(KJob*)) );
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

QList< KJob * > KDevelop::RunController::currentJobs() const
{
    return d->jobs.keys();
}

QList<LaunchConfiguration*> RunController::launchConfigurations() const
{
    return d->launchConfigurations;
}

QList<LaunchConfigurationType*> RunController::launchConfigurationTypes() const
{
    return d->launchConfigurationTypes.values();
}

void RunController::addConfigurationType( LaunchConfigurationType* type )
{
    if( !d->launchConfigurationTypes.contains( type->id() ) )
    {
        d->launchConfigurationTypes.insert( type->id(), type );
    }
}

void RunController::removeConfigurationType( LaunchConfigurationType* type )
{
    foreach( LaunchConfiguration* l, d->launchConfigurations )
    {
        if( l->type() == type ) 
        {
            d->launchConfigurations.removeAll( l );
            delete l;
        }
    }
    d->launchConfigurationTypes.remove( type->id() );
}

void KDevelop::RunController::addLaunchMode(KDevelop::ILaunchMode* mode) 
{
    if( !d->launchModes.contains( mode->id() ) )
    {
        d->launchModes.insert( mode->id(), mode );
    }
}

QList< KDevelop::ILaunchMode* > KDevelop::RunController::launchModes() const
{
    return d->launchModes.values();
}

void KDevelop::RunController::removeLaunchMode(KDevelop::ILaunchMode* mode)
{
    d->launchModes.remove( mode->id() );
}

KDevelop::ILaunchMode* KDevelop::RunController::launchModeForId(const QString& id) const 
{
    QMap<QString,ILaunchMode*>::iterator it = d->launchModes.find( id );
    if( it != d->launchModes.end() ) 
    {
        return it.value();
    }
    return 0;
}

void KDevelop::RunController::addLaunchConfiguration(KDevelop::LaunchConfiguration* l)
{
    if( !d->launchConfigurations.contains( l ) ) 
    {
        d->addLaunchAction( l );
        d->launchConfigurations << l;
        if( !d->currentTargetAction->currentAction() )
        {
            if( !d->currentTargetAction->actions().isEmpty() )
            {
                d->currentTargetAction->actions().first()->setChecked( true );
            }
        }
        d->launchChangeMapper->setMapping( l, d->launchConfigurations.count() -1 );
        connect( l, SIGNAL(nameChanged(QString)), d->launchChangeMapper, SLOT(map()) );
    }    
}

void KDevelop::RunController::removeLaunchConfiguration(KDevelop::LaunchConfiguration* l)
{
    KConfigGroup launcherGroup;
    if( l->project() ) {
        launcherGroup = l->project()->projectConfiguration()->group( LaunchConfigurationsGroup );
    } else {
        launcherGroup = Core::self()->activeSession()->config()->group( LaunchConfigurationsGroup );
    }
    QStringList configs = launcherGroup.readEntry( RunController::LaunchConfigurationsListEntry, QStringList() );
    configs.removeAll( l->configGroupName() );
    launcherGroup.deleteGroup( l->configGroupName() );
    launcherGroup.writeEntry( RunController::LaunchConfigurationsListEntry, configs );
    launcherGroup.sync();

    foreach( QAction* a, d->currentTargetAction->actions() )
    {
        if( static_cast<LaunchConfiguration*>( qVariantValue<void*>( a->data() ) ) == l )
        {
            bool wasSelected = a->isChecked();
            d->currentTargetAction->removeAction( a );
            if( wasSelected && !d->currentTargetAction->actions().isEmpty() )
            {
                d->currentTargetAction->actions().first()->setChecked( true );
            }
            break;
        }
    }

    d->launchConfigurations.removeAll( l );
    
    delete l;
}

void KDevelop::RunController::executeDefaultLaunch(const QString& runMode)
{
    if( !defaultLaunch() )
    {
        kWarning() << "no default launch!";
        return;
    }
    execute( runMode, defaultLaunch() );
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
//     if( status.isValid() && status.canConvert<KDevelop::IRunProvider::OutputTypes>() )
//     {
//         IRunProvider::OutputTypes type = status.value<KDevelop::IRunProvider::OutputTypes>();
//         if( type == IRunProvider::RunProvider )
//         {
//             opt.palette.setBrush( QPalette::Text, runProviderBrush.brush( option.palette ) );
//         } else if( type == IRunProvider::StandardError )
//         {
//             opt.palette.setBrush( QPalette::Text, errorBrush.brush( option.palette ) );
//         }
//     }
    QItemDelegate::paint(painter, opt, index);
}


#include "runcontroller.moc"

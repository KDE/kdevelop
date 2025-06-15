/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "runcontroller.h"

#include <QDBusConnection>
#include <QPalette>
#include <QPointer>

#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KDialogJobUiDelegate>
#include <KLocalizedString>
#include <KSelectAction>

#include <interfaces/iproject.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>
#include <interfaces/launchconfigurationtype.h>
#include <outputview/outputjob.h>
#include <project/builderjob.h>
#include <project/projectmodel.h>
#include <sublime/message.h>

#include "core.h"
#include "uicontroller.h"
#include "projectcontroller.h"
#include "mainwindow.h"
#include "launchconfiguration.h"
#include "launchconfigurationdialog.h"
#include "unitylauncher.h"
#include "debug.h"
#include <interfaces/isession.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <sublime/area.h>

using namespace KDevelop;

namespace {
namespace Strings {
QString LaunchConfigurationsGroup()
{
    return QStringLiteral("Launch");
}

QString LaunchConfigurationsListEntry()
{
    return QStringLiteral("Launch Configurations");
}

QString CurrentLaunchConfigProjectEntry()
{
    return QStringLiteral("Current Launch Config Project");
}

QString CurrentLaunchConfigNameEntry()
{
    return QStringLiteral("Current Launch Config GroupName");
}

QString ConfiguredFromProjectItemEntry()
{
    return QStringLiteral("Configured from ProjectItem");
}
}
}

using Target = QPair<QString, IProject*>;
Q_DECLARE_METATYPE(Target)


//TODO: Doesn't handle add/remove of launch configs in the dialog or renaming of configs
//TODO: Doesn't auto-select launch configs opened from projects

class DebugMode : public ILaunchMode
{
public:
    DebugMode() {}
    QIcon icon() const override { return QIcon::fromTheme(QStringLiteral("debug-run")); }
    QString id() const override { return QStringLiteral("debug"); }
    QString name() const override { return i18nc("launch mode", "Debug"); }
};

class ProfileMode : public ILaunchMode
{
public:
    ProfileMode() {}
    QIcon icon() const override { return QIcon::fromTheme(QStringLiteral("office-chart-area")); }
    QString id() const override { return QStringLiteral("profile"); }
    QString name() const override { return i18nc("launch mode", "Profile"); }
};

class ExecuteMode : public ILaunchMode
{
public:
    ExecuteMode() {}
    QIcon icon() const override { return QIcon::fromTheme(QStringLiteral("system-run")); }
    QString id() const override { return QStringLiteral("execute"); }
    QString name() const override { return i18nc("launch mode", "Execute"); }
};

class KDevelop::RunControllerPrivate
{
public:
    QItemDelegate* delegate;

    IRunController::State state;

    RunController* q;

    QHash<KJob*, QAction*> jobs;
    QAction* stopAction;
    KActionMenu* stopJobsMenu;
    QAction* runAction;
    QAction* dbgAction;
    KSelectAction* currentTargetAction;
    QMap<QString,LaunchConfigurationType*> launchConfigurationTypes;
    QList<LaunchConfiguration*> launchConfigurations;
    QMap<QString,ILaunchMode*> launchModes;
    QMap<int,QPair<QString,QString> > launchAsInfo;
    KDevelop::ProjectBaseItem* contextItem;
    DebugMode* debugMode;
    ExecuteMode* executeMode;
    ProfileMode* profileMode;
    UnityLauncher* unityLauncher;

    bool hasLaunchConfigType( const QString& typeId )
    {
        return launchConfigurationTypes.contains( typeId );
    }
    void saveCurrentLaunchAction()
    {
        if (!currentTargetAction) return;

        if( currentTargetAction->currentAction() )
        {
            KConfigGroup grp = Core::self()->activeSession()->config()->group( Strings::LaunchConfigurationsGroup() );
            LaunchConfiguration* l = static_cast<LaunchConfiguration*>( currentTargetAction->currentAction()->data().value<void*>() );
            grp.writeEntry( Strings::CurrentLaunchConfigProjectEntry(), l->project() ? l->project()->name() : QString() );
            grp.writeEntry( Strings::CurrentLaunchConfigNameEntry(), l->configGroupName() );
            grp.sync();
        }
    }

    QString launchActionText( LaunchConfiguration* l )
    {
        QString label;
        if( l->project() )
        {
            label = QStringLiteral("%1 : %2").arg( l->project()->name(), l->name());
        } else
        {
            label = l->name();
        }
        return label;
    }

    void launchAs( int id )
    {
        //qCDebug(SHELL) << "Launching id:" << id;
        QPair<QString,QString> info = launchAsInfo[id];
        //qCDebug(SHELL) << "fetching type and mode:" << info.first << info.second;
        LaunchConfigurationType* type = launchConfigurationTypeForId( info.first );
        ILaunchMode* mode = q->launchModeForId( info.second );

        //qCDebug(SHELL) << "got mode and type:" << type << type->id() << mode << mode->id();
        if( type && mode )
        {
            const auto launchers = type->launchers();
            auto it = std::find_if(launchers.begin(), launchers.end(), [&](ILauncher* l) {
                //qCDebug(SHELL) << "available launcher" << l << l->id() << l->supportedModes();
                return (l->supportedModes().contains(mode->id()));
            });
            if (it != launchers.end()) {
                ILauncher* launcher = *it;

                QStringList itemPath = Core::self()->projectController()->projectModel()->pathFromIndex(contextItem->index());
                auto it = std::find_if(launchConfigurations.constBegin(), launchConfigurations.constEnd(),
                                       [&] (LaunchConfiguration* l) {
                    QStringList path = l->config().readEntry(Strings::ConfiguredFromProjectItemEntry(), QStringList());
                    if (l->type() == type && path == itemPath) {
                        qCDebug(SHELL) << "already generated ilaunch" << path;
                        return true;
                    }
                    return false;
                });
                ILaunchConfiguration* ilaunch = (it != launchConfigurations.constEnd()) ? *it : nullptr;

                if (!ilaunch) {
                    ilaunch = q->createLaunchConfiguration( type,
                                                            qMakePair( mode->id(), launcher->id() ),
                                                            contextItem->project(),
                                                            contextItem->text() );
                    auto* launch = static_cast<LaunchConfiguration*>(ilaunch);
                    type->configureLaunchFromItem( launch->config(), contextItem );
                    launch->config().writeEntry(Strings::ConfiguredFromProjectItemEntry(), itemPath);
                    //qCDebug(SHELL) << "created config, launching";
                } else {
                    //qCDebug(SHELL) << "reusing generated config, launching";
                }
                q->setDefaultLaunch(ilaunch);
                q->execute( mode->id(), ilaunch );
            }
        }
    }

    void updateCurrentLaunchAction()
    {
        if (!currentTargetAction) return;

        KConfigGroup launchGrp = Core::self()->activeSession()->config()->group( Strings::LaunchConfigurationsGroup() );
        QString currentLaunchProject = launchGrp.readEntry( Strings::CurrentLaunchConfigProjectEntry(), "" );
        QString currentLaunchName = launchGrp.readEntry( Strings::CurrentLaunchConfigNameEntry(), "" );

        LaunchConfiguration* l = nullptr;
        if( currentTargetAction->currentAction() )
        {
            l = static_cast<LaunchConfiguration*>( currentTargetAction->currentAction()->data().value<void*>() );
        } else if( !launchConfigurations.isEmpty() )
        {
            l = launchConfigurations.at( 0 );
        }

        if( l && ( ( !currentLaunchProject.isEmpty() && ( !l->project() || l->project()->name() != currentLaunchProject ) ) || l->configGroupName() != currentLaunchName ) )
        {
            const auto actions = currentTargetAction->actions();
            for (QAction* a : actions) {
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
            qCDebug(SHELL) << "oops no current action, using first if list is non-empty";
            if( !currentTargetAction->actions().isEmpty() )
            {
                currentTargetAction->actions().at(0)->setChecked( true );
            }
        }
    }

    void addLaunchAction( LaunchConfiguration* l )
    {
        if (!currentTargetAction) return;

        QAction* action = currentTargetAction->addAction(launchActionText( l ));
        action->setData(QVariant::fromValue<void*>(l));
    }

    /**
     * Read launch configurations from the session config, all open project configs,
     * and add them to the list of all available launch configurations.
     *
     * @param launchConfigurationTypeId a registered configuration type ID to add only launch configurations of
     *        this type, or an empty string to add launch configurations of all registered configuration types
     */
    void readAllLaunchConfigurations(const QString& launchConfigurationTypeId = {})
    {
        readLaunchConfigurations(Core::self()->activeSession()->config(), nullptr, launchConfigurationTypeId);
        const auto projects = Core::self()->projectController()->projects();
        for (auto* const project : projects) {
            readProjectLaunchConfigurations(project, launchConfigurationTypeId);
        }
    }
    /**
     * Read launch configurations from the config of a given project
     * and add them to the list of all available launch configurations.
     *
     * @copydetails readAllLaunchConfigurations()
     */
    void readProjectLaunchConfigurations(IProject* project, const QString& launchConfigurationTypeId = {})
    {
        readLaunchConfigurations(project->projectConfiguration(), project, launchConfigurationTypeId);
    }
    /**
     * Read launch configurations from a given config and add them to the list of all available launch configurations.
     *
     * @param cfg a config to read launch configurations from
     * @param project the project of @p cfg or @c nullptr if @p cfg is a global (session) config
     * @copydetails readAllLaunchConfigurations()
     */
    void readLaunchConfigurations(const KSharedConfigPtr& cfg, IProject* project,
                                  const QString& launchConfigurationTypeId = {})
    {
        Q_ASSERT(launchConfigurationTypeId.isEmpty() || launchConfigurationTypes.contains(launchConfigurationTypeId));
        const auto shouldAddLaunchConfiguration = [&launchConfigurationTypeId, this](const QString& typeId) {
            return launchConfigurationTypeId.isEmpty() ? (launchConfigurationTypeForId(typeId) != nullptr)
                                                       : (typeId == launchConfigurationTypeId);
        };

        KConfigGroup group(cfg, Strings::LaunchConfigurationsGroup());
        const QStringList configs = group.readEntry(Strings::LaunchConfigurationsListEntry(), QStringList());

        for (const QString& cfg : configs) {
            KConfigGroup grp = group.group( cfg );
            if (shouldAddLaunchConfiguration(grp.readEntry(LaunchConfiguration::LaunchConfigurationTypeEntry(), ""))) {
                q->addLaunchConfiguration(new LaunchConfiguration(grp, project));
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
            qCWarning(SHELL) << "couldn't find type for id:" << id << ". Known types:" << launchConfigurationTypes.keys();
        }
        return nullptr;

    }

};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d_ptr(new RunControllerPrivate)
{
    Q_D(RunController);

    setObjectName(QStringLiteral("RunController"));

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kdevelop/RunController"),
        this, QDBusConnection::ExportScriptableSlots);

    // TODO: need to implement compile only if needed before execute
    // TODO: need to implement abort all running programs when project closed

    d->currentTargetAction = nullptr;
    d->state = Idle;
    d->q = this;
    d->delegate = new RunDelegate(this);
    d->contextItem = nullptr;
    d->executeMode = nullptr;
    d->debugMode = nullptr;
    d->profileMode = nullptr;

    d->unityLauncher = new UnityLauncher(this);
    d->unityLauncher->setLauncherId(KAboutData::applicationData().desktopFileName());

    if(!(Core::self()->setupFlags() & Core::NoUi)) {
        // Note that things like registerJob() do not work without the actions, it'll simply crash.
        setupActions();
    }
}

RunController::~RunController() = default;

void KDevelop::RunController::launchChanged( LaunchConfiguration* l )
{
    Q_D(RunController);

    const auto actions = d->currentTargetAction->actions();
    for (QAction* a : actions) {
        if( static_cast<LaunchConfiguration*>( a->data().value<void*>() ) == l )
        {
            a->setText( d->launchActionText( l ) );
            break;
        }
    }
}

void RunController::cleanup()
{
    Q_D(RunController);

    delete d->executeMode;
    d->executeMode = nullptr;
    delete d->profileMode;
    d->profileMode = nullptr;
    delete d->debugMode;
    d->debugMode = nullptr;

    stopAllProcesses();
    d->saveCurrentLaunchAction();
}

void RunController::initialize()
{
    Q_D(RunController);

    d->executeMode = new ExecuteMode();
    addLaunchMode( d->executeMode );
    d->profileMode = new ProfileMode();
    addLaunchMode( d->profileMode );
    d->debugMode = new DebugMode;
    addLaunchMode( d->debugMode );

    connect(Core::self()->projectController(), &IProjectController::projectOpened,
            this, &RunController::slotProjectOpened);
    connect(Core::self()->projectController(), &IProjectController::projectClosing,
            this, &RunController::slotProjectClosing);

    d->readAllLaunchConfigurations();
    d->updateCurrentLaunchAction();
}

KJob* RunController::execute(const QString& runMode, ILaunchConfiguration* launch)
{
    if( !launch )
    {
        qCDebug(SHELL) << "execute called without launch config!";
        return nullptr;
    }
    auto* run = static_cast<LaunchConfiguration*>(launch);
    //TODO: Port to launch framework, probably needs to be part of the launcher
    //if(!run.dependencies().isEmpty())
    //    ICore::self()->documentController()->saveAllDocuments(IDocument::Silent);

    //foreach(KJob* job, run.dependencies())
    //{
    //    jobs.append(job);
    //}

    qCDebug(SHELL) << "mode:" << runMode;
    QString launcherId = run->launcherForMode( runMode );
    qCDebug(SHELL) << "launcher id:" << launcherId;

    ILauncher* launcher = run->type()->launcherForId( launcherId );

    if( !launcher ) {
        const QString messageText = i18n("The current launch configuration does not support the '%1' mode.", runMode);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
        return nullptr;
    }

    KJob* launchJob = launcher->start(runMode, run);
    registerJob(launchJob);
    return launchJob;
}

void RunController::setupActions()
{
    Q_D(RunController);

    QAction* action;

    // TODO not multi-window friendly, FIXME
    KActionCollection* ac = Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18nc("@action", "Configure Launches..."), this);
    ac->addAction(QStringLiteral("configure_launches"), action);
    action->setMenuRole(QAction::NoRole); // OSX: Be explicit about role, prevent hiding due to conflict with "Preferences..." menu item
    action->setToolTip(i18nc("@info:tooltip", "Open Launch Configuration Dialog"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Opens a dialog to setup new launch configurations, or to change the existing ones."));
    connect(action, &QAction::triggered, this, &RunController::showConfigurationDialog);

    d->runAction = new QAction( QIcon::fromTheme(QStringLiteral("system-run")), i18nc("@action", "Execute Launch"), this);
    d->runAction->setIconText( i18nc("@action Short text for 'Execute Launch' used in the toolbar", "Execute") );
    ac->setDefaultShortcut(d->runAction, Qt::SHIFT | Qt::Key_F9);
    d->runAction->setToolTip(i18nc("@info:tooltip", "Execute current launch"));
    d->runAction->setWhatsThis(i18nc("@info:whatsthis", "Executes the target or the program specified in currently active launch configuration."));
    ac->addAction(QStringLiteral("run_execute"), d->runAction);
    connect(d->runAction, &QAction::triggered, this, &RunController::slotExecute);

    d->dbgAction = new QAction( QIcon::fromTheme(QStringLiteral("debug-run")), i18nc("@action", "Debug Launch"), this);
    ac->setDefaultShortcut(d->dbgAction, Qt::ALT | Qt::Key_F9);
    d->dbgAction->setIconText( i18nc("@action Short text for 'Debug Launch' used in the toolbar", "Debug") );
    d->dbgAction->setToolTip(i18nc("@info:tooltip", "Debug current launch"));
    d->dbgAction->setWhatsThis(i18nc("@info:whatsthis", "Executes the target or the program specified in currently active launch configuration inside a Debugger."));
    ac->addAction(QStringLiteral("run_debug"), d->dbgAction);
    connect(d->dbgAction, &QAction::triggered, this, &RunController::slotDebug);
    Core::self()->uiControllerInternal()->area(0, QStringLiteral("code"))->addAction(d->dbgAction);

//     TODO: at least get a profile target, it's sad to have the menu entry without a profiler
//     QAction* profileAction = new QAction( QIcon::fromTheme(""), i18n("Profile Launch"), this);
//     profileAction->setToolTip(i18nc("@info:tooltip", "Profile current launch"));
//     profileAction->setWhatsThis(i18nc("@info:whatsthis", "Executes the target or the program specified in currently active launch configuration inside a Profiler."));
//     ac->addAction("run_profile", profileAction);
//     connect(profileAction, SIGNAL(triggered(bool)), this, SLOT(slotProfile()));

    action = d->stopAction = new QAction( QIcon::fromTheme(QStringLiteral("process-stop")), i18nc("@action", "Stop All Jobs"), this);
    action->setIconText(i18nc("@action Short text for 'Stop All Jobs' used in the toolbar", "Stop All"));
    // Ctrl+Escape would be nicer, but that is taken by the ksysguard desktop shortcut
    ac->setDefaultShortcut( action, QKeySequence(QStringLiteral("Ctrl+Shift+Escape")));
    action->setToolTip(i18nc("@info:tooltip", "Stop all currently running jobs"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Requests that all running jobs are stopped."));
    action->setEnabled(false);
    ac->addAction(QStringLiteral("run_stop_all"), action);
    connect(action, &QAction::triggered, this, &RunController::stopAllProcesses);
    Core::self()->uiControllerInternal()->area(0, QStringLiteral("debug"))->addAction(action);

    action = d->stopJobsMenu = new KActionMenu( QIcon::fromTheme(QStringLiteral("process-stop")), i18nc("@action", "Stop"), this);
    d->stopJobsMenu->setPopupMode(QToolButton::InstantPopup);
    action->setIconText(i18nc("@action Short text for 'Stop' used in the toolbar", "Stop"));
    action->setToolTip(i18nc("@info:tooltip", "Menu allowing to stop individual jobs"));
    action->setWhatsThis(i18nc("@info:whatsthis", "List of jobs that can be stopped individually."));
    action->setEnabled(false);
    ac->addAction(QStringLiteral("run_stop_menu"), action);

    d->currentTargetAction = new KSelectAction( i18nc("@title:menu", "Current Launch Configuration"), this);
    d->currentTargetAction->setToolTip(i18nc("@info:tooltip", "Current launch configuration"));
    d->currentTargetAction->setWhatsThis(i18nc("@info:whatsthis", "Select which launch configuration to run when run is invoked."));
    connect(d->currentTargetAction, &KSelectAction::actionTriggered, this, [d] {
        d->saveCurrentLaunchAction();
    });
    ac->addAction(QStringLiteral("run_default_target"), d->currentTargetAction);
}

LaunchConfigurationType* RunController::launchConfigurationTypeForId( const QString& id )
{
    Q_D(RunController);

    return d->launchConfigurationTypeForId( id );
}

void KDevelop::RunController::slotProjectOpened(KDevelop::IProject * project)
{
    Q_D(RunController);

    d->readProjectLaunchConfigurations(project);
    d->updateCurrentLaunchAction();
}

void KDevelop::RunController::slotProjectClosing(KDevelop::IProject * project)
{
    Q_D(RunController);

    if (!d->currentTargetAction) return;

    const auto actions = d->currentTargetAction->actions();
    for (QAction* action : actions) {
        LaunchConfiguration* l = static_cast<LaunchConfiguration*>(qvariant_cast<void*>(action->data()));
        if ( project == l->project() ) {
            l->save();
            d->launchConfigurations.removeAll(l);
            delete l;
            bool wasSelected = action->isChecked();
            delete action;
            if (wasSelected && !d->currentTargetAction->actions().isEmpty())
                d->currentTargetAction->actions().at(0)->setChecked(true);
        }
    }
}

void RunController::slotDebug()
{
    Q_D(RunController);

    if (d->launchConfigurations.isEmpty()) {
        showConfigurationDialog();
    }

    if (!d->launchConfigurations.isEmpty()) {
        executeDefaultLaunch( QStringLiteral("debug") );
    }
}

void RunController::slotProfile()
{
    Q_D(RunController);

    if (d->launchConfigurations.isEmpty()) {
        showConfigurationDialog();
    }

    if (!d->launchConfigurations.isEmpty()) {
        executeDefaultLaunch( QStringLiteral("profile") );
    }
}

void RunController::slotExecute()
{
    Q_D(RunController);

    if (d->launchConfigurations.isEmpty()) {
        showConfigurationDialog();
    }

    if (!d->launchConfigurations.isEmpty()) {
        executeDefaultLaunch( QStringLiteral("execute") );
    }
}

void KDevelop::RunController::showConfigurationDialog() const
{
    LaunchConfigurationDialog dlg;
    dlg.exec();
}

LaunchConfiguration* KDevelop::RunController::defaultLaunch() const
{
    Q_D(const RunController);

    QAction* projectAction = d->currentTargetAction->currentAction();
    if( projectAction )
        return static_cast<LaunchConfiguration*>(qvariant_cast<void*>(projectAction->data()));
    return nullptr;
}

void KDevelop::RunController::registerJob(KJob * job)
{
    Q_D(RunController);

    if (!job)
        return;

    if (!(job->capabilities() & KJob::Killable)) {
        // see e.g. https://bugs.kde.org/show_bug.cgi?id=314187
        qCWarning(SHELL) << "non-killable job" << job << "registered - this might lead to crashes on shutdown.";
    }

    if (!d->jobs.contains(job)) {
        QAction* stopJobAction = nullptr;
        if (Core::self()->setupFlags() != Core::NoUi) {
            stopJobAction = new QAction(job->objectName().isEmpty() ? i18nc("@item:inmenu", "<%1> Unnamed job", QString::fromUtf8(job->staticMetaObject.className())) : job->objectName(), this);
            stopJobAction->setData(QVariant::fromValue(static_cast<void*>(job)));
            d->stopJobsMenu->addAction(stopJobAction);
            connect (stopJobAction, &QAction::triggered, this, &RunController::slotKillJob);

            job->setUiDelegate( new KDialogJobUiDelegate() );
        }

        d->jobs.insert(job, stopJobAction);

        connect( job, &KJob::finished, this, &RunController::finished );
        connect(job, &KJob::percentChanged, this, &RunController::jobPercentChanged);

        IRunController::registerJob(job);

        emit jobRegistered(job);
    }

    const QPointer thisGuard(this);
    job->start();
    if (!thisGuard) {
        return; // already destroyed (KDevelop is probably exiting now)
    }

    checkState();
}

void KDevelop::RunController::unregisterJob(KJob * job)
{
    Q_D(RunController);

    IRunController::unregisterJob(job);

    Q_ASSERT(d->jobs.contains(job));

    // Delete the stop job action
    QAction *action = d->jobs.take(job);
    if (action)
        action->deleteLater();

    checkState();

    emit jobUnregistered(job);
}

void KDevelop::RunController::checkState()
{
    Q_D(RunController);

    bool running = false;

    int jobCount = 0;
    int totalProgress = 0;

    for (auto it = d->jobs.constBegin(), end = d->jobs.constEnd(); it != end; ++it) {
        KJob *job = it.key();

        if (!job->isSuspended()) {
            running = true;

            ++jobCount;
            totalProgress += job->percent();
        }
    }

    d->unityLauncher->setProgressVisible(running);
    if (jobCount > 0) {
        d->unityLauncher->setProgress((totalProgress + 1) / jobCount);
    } else {
        d->unityLauncher->setProgress(0);
    }

    if ( ( d->state != Running ? false : true ) == running ) {
        d->state = running ? Running : Idle;
        emit runStateChanged(d->state);
    }

    if (Core::self()->setupFlags() != Core::NoUi) {
        d->stopAction->setEnabled(running);
        d->stopJobsMenu->setEnabled(running);
    }
}

void KDevelop::RunController::stopAllProcesses()
{
    Q_D(RunController);

    // composite jobs might remove child jobs, see also:
    // https://bugs.kde.org/show_bug.cgi?id=258904
    const auto jobs = d->jobs.keys();
    for (KJob* job : jobs) {
        // now we check the real list whether it was deleted
        if (!d->jobs.contains(job))
            continue;
        if (job->capabilities() & KJob::Killable) {
            job->kill(KJob::EmitResult);
        } else {
            qCWarning(SHELL) << "cannot stop non-killable job: " << job;
        }
    }
}

void KDevelop::RunController::slotKillJob()
{
    auto* action = qobject_cast<QAction*>(sender());
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
        case OutputJob::FailedShownError:
        // either the user canceled the saving or an error message box was displayed
        case BuilderJob::CouldNotSaveAllDocumentsError:
            break;

        default:
        {
            auto* message = new Sublime::Message(job->errorString(), Sublime::Message::Error);
            Core::self()->uiController()->postMessage(message);
        }
    }
}

void RunController::jobPercentChanged()
{
    checkState();
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
    Q_D(const RunController);

    return d->jobs.keys();
}

QList<ILaunchConfiguration*> RunController::launchConfigurations() const
{
    QList<ILaunchConfiguration*> configs;
    const auto configsInternal = launchConfigurationsInternal();
    configs.reserve(configsInternal.size());
    for (LaunchConfiguration* config : configsInternal) {
        configs << config;
    }
    return configs;
}

QList<LaunchConfiguration*> RunController::launchConfigurationsInternal() const
{
    Q_D(const RunController);

    return d->launchConfigurations;
}

QList<LaunchConfigurationType*> RunController::launchConfigurationTypes() const
{
    Q_D(const RunController);

    return d->launchConfigurationTypes.values();
}

void RunController::addConfigurationType( LaunchConfigurationType* type )
{
    Q_D(RunController);

    const auto typeId = type->id();
    if (d->launchConfigurationTypes.contains(typeId)) {
        qCWarning(SHELL) << "cannot add configuration type with an already registered ID" << typeId;
        return;
    }
    d->launchConfigurationTypes.insert(typeId, type);

    if (!d->executeMode) {
        qCDebug(SHELL) << "added configuration type" << typeId
                       << "before initialization or after cleanup of the RunController, "
                          "so will not read launch configurations now";
        return;
    }
    qCDebug(SHELL).nospace() << "added configuration type " << typeId << ", reading launch configurations of this type";
    d->readAllLaunchConfigurations(typeId);
    d->updateCurrentLaunchAction();
}

void RunController::removeConfigurationType( LaunchConfigurationType* type )
{
    Q_D(RunController);

    const auto oldLaunchConfigurations = d->launchConfigurations;
    for (LaunchConfiguration* l : oldLaunchConfigurations) {
        if( l->type() == type )
        {
            removeLaunchConfigurationInternal( l );
        }
    }
    d->launchConfigurationTypes.remove( type->id() );
}

void KDevelop::RunController::addLaunchMode(KDevelop::ILaunchMode* mode)
{
    Q_D(RunController);

    if( !d->launchModes.contains( mode->id() ) )
    {
        d->launchModes.insert( mode->id(), mode );
    }
}

QList< KDevelop::ILaunchMode* > KDevelop::RunController::launchModes() const
{
    Q_D(const RunController);

    return d->launchModes.values();
}

void KDevelop::RunController::removeLaunchMode(KDevelop::ILaunchMode* mode)
{
    Q_D(RunController);

    d->launchModes.remove( mode->id() );
}

KDevelop::ILaunchMode* KDevelop::RunController::launchModeForId(const QString& id) const
{
    Q_D(const RunController);

    auto it = d->launchModes.find( id );
    if( it != d->launchModes.end() )
    {
        return it.value();
    }
    return nullptr;
}

void KDevelop::RunController::addLaunchConfiguration(KDevelop::LaunchConfiguration* l)
{
    Q_D(RunController);

    if( !d->launchConfigurations.contains( l ) )
    {
        d->addLaunchAction( l );
        d->launchConfigurations << l;
        if( !d->currentTargetAction->currentAction() )
        {
            if( !d->currentTargetAction->actions().isEmpty() )
            {
                d->currentTargetAction->actions().at(0)->setChecked( true );
            }
        }
        connect( l, &LaunchConfiguration::nameChanged, this, &RunController::launchChanged );
    }
}

void KDevelop::RunController::removeLaunchConfiguration(KDevelop::LaunchConfiguration* l)
{
    KConfigGroup launcherGroup;
    if( l->project() ) {
        launcherGroup = l->project()->projectConfiguration()->group( Strings::LaunchConfigurationsGroup() );
    } else {
        launcherGroup = Core::self()->activeSession()->config()->group( Strings::LaunchConfigurationsGroup() );
    }
    QStringList configs = launcherGroup.readEntry( Strings::LaunchConfigurationsListEntry(), QStringList() );
    configs.removeAll( l->configGroupName() );
    launcherGroup.deleteGroup( l->configGroupName() );
    launcherGroup.writeEntry( Strings::LaunchConfigurationsListEntry(), configs );
    launcherGroup.sync();

    removeLaunchConfigurationInternal( l );
}

void RunController::removeLaunchConfigurationInternal(LaunchConfiguration *l)
{
    Q_D(RunController);

    const auto actions = d->currentTargetAction->actions();
    for (QAction* a : actions) {
        if( static_cast<LaunchConfiguration*>( a->data().value<void*>() ) == l ) {
            bool wasSelected = a->isChecked();
            d->currentTargetAction->removeAction( a );
            if( wasSelected && !d->currentTargetAction->actions().isEmpty() ) {
                d->currentTargetAction->actions().at(0)->setChecked( true );
            }
            break;
        }
    }

    d->launchConfigurations.removeAll( l );

    delete l;
}

void KDevelop::RunController::executeDefaultLaunch(const QString& runMode)
{
    if (auto dl = defaultLaunch()) {
        execute(runMode, dl);
    } else {
        qCWarning(SHELL) << "no default launch!";
    }
}

void RunController::setDefaultLaunch(ILaunchConfiguration* l)
{
    Q_D(RunController);

    const auto actions = d->currentTargetAction->actions();
    for (QAction* a : actions) {
        if( static_cast<ILaunchConfiguration*>( a->data().value<void*>() ) == l )
        {
            a->setChecked(true);
            break;
        }
    }
}

bool launcherNameExists(const QString& name)
{
    const auto configs = Core::self()->runControllerInternal()->launchConfigurations();

    return std::any_of(configs.begin(), configs.end(), [&](ILaunchConfiguration* config) {
        return (config->name() == name);
    });
}

QString makeUnique(const QString& name)
{
    if(launcherNameExists(name)) {
        for(int i=2; ; i++) {
            QString proposed = QStringLiteral("%1 (%2)").arg(name).arg(i);
            if(!launcherNameExists(proposed)) {
                return proposed;
            }
        }
    }
    return name;
}

ILaunchConfiguration* RunController::createLaunchConfiguration ( LaunchConfigurationType* type,
                                                                 const QPair<QString,QString>& launcher,
                                                                 IProject* project, const QString& name )
{
    KConfigGroup launchGroup;
    if( project )
    {
        launchGroup = project->projectConfiguration()->group( Strings::LaunchConfigurationsGroup() );

    } else
    {
        launchGroup = Core::self()->activeSession()->config()->group( Strings::LaunchConfigurationsGroup() );

    }
    QStringList configs = launchGroup.readEntry( Strings::LaunchConfigurationsListEntry(), QStringList() );
    uint num = 0;
    QString baseName = QStringLiteral("Launch Configuration");
    while( configs.contains( QStringLiteral( "%1 %2" ).arg( baseName ).arg( num ) ) )
    {
        num++;
    }
    QString groupName = QStringLiteral( "%1 %2" ).arg( baseName ).arg( num );
    KConfigGroup launchConfigGroup = launchGroup.group( groupName );
    QString cfgName = name;
    if( name.isEmpty() )
    {
        cfgName = i18n("New %1 Launcher", type->name() );
        cfgName = makeUnique(cfgName);
    }

    launchConfigGroup.writeEntry(LaunchConfiguration::LaunchConfigurationNameEntry(), cfgName );
    launchConfigGroup.writeEntry(LaunchConfiguration::LaunchConfigurationTypeEntry(), type->id() );
    launchConfigGroup.sync();
    configs << groupName;
    launchGroup.writeEntry( Strings::LaunchConfigurationsListEntry(), configs );
    launchGroup.sync();
    auto* l = new LaunchConfiguration( launchConfigGroup, project );
    l->setLauncherForMode( launcher.first, launcher.second );
    Core::self()->runControllerInternal()->addLaunchConfiguration( l );
    return l;
}

QItemDelegate * KDevelop::RunController::delegate() const
{
    Q_D(const RunController);

    return d->delegate;
}

ContextMenuExtension RunController::contextMenuExtension(Context* ctx, QWidget* parent)
{
    Q_D(RunController);

    d->launchAsInfo.clear();
    d->contextItem = nullptr;
    ContextMenuExtension ext;
    if( ctx->type() == Context::ProjectItemContext ) {
        auto* prjctx = static_cast<KDevelop::ProjectItemContext*>(ctx);
        if( prjctx->items().count() == 1 )
        {
            ProjectBaseItem* itm = prjctx->items().at( 0 );
            int i = 0;
            for (ILaunchMode* mode : std::as_const(d->launchModes)) {
                auto* menu = new KActionMenu(i18nc("@title:menu", "%1 As...", mode->name() ), parent);
                const auto types = launchConfigurationTypes();
                for (LaunchConfigurationType* type : types) {
                    bool hasLauncher = false;
                    const auto launchers = type->launchers();
                    for (ILauncher* launcher : launchers) {
                        if( launcher->supportedModes().contains( mode->id() ) )
                        {
                            hasLauncher = true;
                        }
                    }
                    if( hasLauncher && type->canLaunch(itm) )
                    {
                        d->launchAsInfo[i] = qMakePair( type->id(), mode->id() );
                        auto* act = new QAction(menu);
                        act->setText( type->name() );
                        qCDebug(SHELL) << "Connect " << i << "for action" << act->text() << "in mode" << mode->name();
                        connect(act, &QAction::triggered,
                                this, [this, i] () { Q_D(RunController); d->launchAs(i); } );
                        menu->addAction(act);
                        i++;
                    }
                }
                if( menu->menu()->actions().count() > 0 )
                {
                    ext.addAction( ContextMenuExtension::RunGroup, menu);
                } else {
                    delete menu;
                }
            }
            if( ext.actions( ContextMenuExtension::RunGroup ).count() > 0 )
            {
                d->contextItem = itm;
            }
        }
    }
    return ext;
}



RunDelegate::RunDelegate( QObject* parent )
: QItemDelegate(parent), runProviderBrush( KColorScheme::View, KColorScheme::PositiveText ),
errorBrush( KColorScheme::View, KColorScheme::NegativeText )
{
}

void RunDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt = option;
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


#include "moc_runcontroller.cpp"

/*
* GDB Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "gdblaunchconfig.h"

#include <kconfiggroup.h>
#include <kicon.h>
#include <klocale.h>
#include <kshell.h>
#include <kmessagebox.h>
#include <kparts/mainwindow.h>

#include <outputview/outputmodel.h>
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentgrouplist.h>
#include <execute/executepluginconstants.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/iuicontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <util/executecompositejob.h>

#include "debugsession.h"
#include "debuggerplugin.h"

#include "ui_debuggerconfigwidget.h"

GdbConfigPage::GdbConfigPage( QWidget* parent )
    : LaunchConfigurationPage(parent), ui( new Ui::DebuggerConfigWidget )
{
    ui->setupUi( this );
    ui->kcfg_gdbPath->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
}

KIcon GdbConfigPage::icon() const
{
    return KIcon();
}

void GdbConfigPage::loadFromConfiguration( const KConfigGroup& cfg )
{
    ui->kcfg_gdbPath->setUrl( cfg.readEntry( GDBDebugger::gdbPathEntry, KUrl() ) );
    ui->kcfg_debuggingShell->setUrl( cfg.readEntry( GDBDebugger::debuggerShellEntry, KUrl() ) );
    ui->kcfg_configGdbScript->setUrl( cfg.readEntry( GDBDebugger::remoteGdbConfigEntry, KUrl() ) );
    ui->kcfg_runShellScript->setUrl( cfg.readEntry( GDBDebugger::remoteGdbShellEntry, KUrl() ) );
    ui->kcfg_runGdbScript->setUrl( cfg.readEntry( GDBDebugger::remoteGdbRunEntry, KUrl() ) );
    ui->kcfg_displayStaticMembers->setChecked( cfg.readEntry(GDBDebugger::staticMembersEntry, false) );
    ui->kcfg_asmDemangle->setChecked( cfg.readEntry( GDBDebugger::demangleNamesEntry, true) );
    //TODO: add ui for this
    //ui->kcfg_allowForceBP->setChecked( cfg.readEtnry( GDBDebugger::allowForcedBPEntry, true ) );
    ui->kcfg_breakOnLoadingLibrary->setChecked( cfg.readEntry( GDBDebugger::breakOnLibLoadEntry, true) );
    ui->kcfg_dbgTerminal->setChecked( cfg.readEntry( GDBDebugger::separateTerminalEntry, false) );
    ui->kcfg_enableFloatingToolBar->setChecked( cfg.readEntry(GDBDebugger::floatingToolbarEntry, false) );
}

void GdbConfigPage::saveToConfiguration( KConfigGroup cfg ) const
{
    cfg.writeEntry(GDBDebugger::gdbPathEntry, ui->kcfg_gdbPath->url() );
    cfg.writeEntry(GDBDebugger::debuggerShellEntry, ui->kcfg_debuggingShell->url() );
    cfg.writeEntry(GDBDebugger::remoteGdbConfigEntry, ui->kcfg_configGdbScript->url() );
    cfg.writeEntry(GDBDebugger::remoteGdbShellEntry, ui->kcfg_runShellScript->url() );
    cfg.writeEntry(GDBDebugger::remoteGdbRunEntry, ui->kcfg_runGdbScript->url() );
    cfg.writeEntry(GDBDebugger::staticMembersEntry, ui->kcfg_displayStaticMembers->isChecked() );
    cfg.writeEntry(GDBDebugger::demangleNamesEntry, ui->kcfg_asmDemangle->isChecked() );
    cfg.writeEntry(GDBDebugger::breakOnLibLoadEntry, ui->kcfg_breakOnLoadingLibrary->isChecked() );
    cfg.writeEntry(GDBDebugger::separateTerminalEntry, ui->kcfg_dbgTerminal->isChecked() );
    cfg.writeEntry(GDBDebugger::floatingToolbarEntry, ui->kcfg_enableFloatingToolBar->isChecked() );
}

QString GdbConfigPage::title() const
{
    return i18n( "GDB Configuration" );
}


GdbLauncher::GdbLauncher( GDBDebugger::CppDebuggerPlugin* p ) : m_plugin( p )
{
    factoryList << new GdbConfigPageFactory();
}

QList< KDevelop::LaunchConfigurationPageFactory* > GdbLauncher::configPages() const
{
    return factoryList;
}

QString GdbLauncher::id()
{
    return "gdb";
}

QString GdbLauncher::name() const
{
    return i18n("GDB");
}

KJob* GdbLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }
    if( launchMode == "debug" )
    {
        QStringList deps = cfg->config().readEntry( ExecutePlugin::dependencyEntry, QStringList() );
        QString depAction = cfg->config().readEntry( ExecutePlugin::dependencyActionEntry, "Nothing" );
        if( depAction != "Nothing" && !deps.isEmpty() )
        {
            QList<KJob*> l;
            KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
            foreach( const QString& dep, deps )
            {
                KDevelop::ProjectBaseItem* item = model->item( model->pathToIndex( dep.split('/') ) );
                if( item )
                {
                    KDevelop::ProjectBaseItem* folder = item;
                    while( folder && !folder->folder() )
                    {
                        folder = dynamic_cast<KDevelop::ProjectBaseItem*>( folder->parent() );
                    }
                    if( folder && item->project()->buildSystemManager()->builder( folder->folder() ) )
                    {
                        KDevelop::IProjectBuilder* builder = item->project()->buildSystemManager()->builder( folder->folder() );
                        if( depAction == "Build" )
                        {
                            l << builder->build( item );
                        } else if( depAction == "Install" )
                        {
                            l << builder->install( item );
                        } else if( depAction == "SudoInstall" )
                        {
                            KMessageBox::information( KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                                    i18n("Installing via sudo is not yet implemented"),
                                                    i18n("Not implemented") );
                        }
                    }
                }
            }
            l << new GdbJob( m_plugin, cfg );
            return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), l );
        }else
        {
            return new GdbJob( m_plugin, cfg );
        }
    }
    kWarning() << "Unknown launch mode" << launchMode << "for config:" << cfg->name();
    return 0;
}

QStringList GdbLauncher::supportedModes() const
{
    return QStringList() << "debug";
}

QString GdbLauncher::description() const
{
    return i18n("Executes a Native application in GDB");
}

KDevelop::LaunchConfigurationPage* GdbConfigPageFactory::createWidget( QWidget* parent )
{
    return new GdbConfigPage( parent );
}

GdbJob::GdbJob( GDBDebugger::CppDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg, QObject* parent) 
    : KDevelop::OutputJob(parent), m_launchcfg( launchcfg )
{
    m_session = p->createSession();
    connect(m_session, SIGNAL(applicationStandardOutputLines(QStringList)), SLOT(stderrReceived(QStringList)));
    connect(m_session, SIGNAL(applicationStandardErrorLines(QStringList)), SLOT(stdoutReceived(QStringList)));
    connect(m_session, SIGNAL(finished()), SLOT(done()) );
}

void GdbJob::start()
{
    KConfigGroup grp = m_launchcfg->config();
    KDevelop::EnvironmentGroupList l(KGlobal::config());
    
    QString executable = ExecutePlugin::executableFromConfig( grp );
    
    QString envgrp = grp.readEntry( ExecutePlugin::environmentGroupEntry, "" );
    
    if( executable.isEmpty() )
    {
        setError(-1);
        setErrorText( i18n("No executable specified") );
        kWarning() << "no executable set";
    } else
    {
        KShell::Errors err;
        if( KShell::splitArgs( executable, KShell::TildeExpand | KShell::AbortOnMeta, &err ).isEmpty() || err != KShell::NoError )
        {
            
            setError( -1 );
            if( err == KShell::BadQuoting ) 
            {
                setErrorText( i18n("There is a quoting error in the executable "
                "for the launch configuration '%1'. "
                "Aborting start.", m_launchcfg->name() ) );
            } else 
            {   
                setErrorText( i18n("A shell meta character was included in the "
                "executable for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", m_launchcfg->name() ) );
            }
            kWarning() << "executable has meta characters";
        }
    }
    
    if( envgrp.isEmpty() )
    {
        kWarning() << i18n("No environment group specified, looks like a broken "
        "configuration, please check run configuration '%1'. "
        "Using default environment group.", m_launchcfg->name() );
        envgrp = l.defaultGroup();
    }
    
    KShell::Errors err;
    QStringList arguments = KShell::splitArgs( grp.readEntry( ExecutePlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
    if( err != KShell::NoError )
    {
        
        setError( -1 );
        if( err == KShell::BadQuoting ) 
        {
            setErrorText( i18n("There is a quoting error in the arguments for "
            "the launch configuration '%1'. Aborting start.", m_launchcfg->name() ) );
        } else 
        {   
            setErrorText( i18n("A shell meta character was included in the "
            "arguments for the launch configuration '%1', "
            "this is not supported currently. Aborting start.", m_launchcfg->name() ) );
        }
        kDebug() << "arguments have meta characters";
    }
    if( error() != 0 )
    {
        emitResult();
        return;
    }
    
    setStandardToolView(KDevelop::IOutputView::DebugView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setModel( new KDevelop::OutputModel(), KDevelop::IOutputView::TakeOwnership );
    
    startOutput();
    
    m_session->startProgram( m_launchcfg, this );
}

void GdbJob::stderrReceived(const QStringList& l )
{
    model()->appendLines( l );
}

void GdbJob::stdoutReceived(const QStringList& l )
{
    model()->appendLines( l );
}

KDevelop::OutputModel* GdbJob::model()
{
    return dynamic_cast<KDevelop::OutputModel*>( KDevelop::OutputJob::model() );
}


void GdbJob::done()
{
    emitResult();
}


#include "gdblaunchconfig.moc"

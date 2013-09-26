/*
  * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#include "executeplugin.h"

#include <QApplication>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <kjob.h>
#include <kparts/mainwindow.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/environmentgrouplist.h>

#include "nativeappconfig.h"
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <kshell.h>
#include <util/kdevstringhandler.h>

QString ExecutePlugin::_nativeAppConfigTypeId = "Native Application";
QString ExecutePlugin::workingDirEntry = "Working Directory";
QString ExecutePlugin::executableEntry = "Executable";
QString ExecutePlugin::argumentsEntry = "Arguments";
QString ExecutePlugin::isExecutableEntry = "isExecutable";
QString ExecutePlugin::dependencyEntry = "Dependencies";
QString ExecutePlugin::environmentGroupEntry = "EnvironmentGroup";
QString ExecutePlugin::useTerminalEntry = "Use External Terminal";
QString ExecutePlugin::terminalEntry = "External Terminal";
QString ExecutePlugin::userIdToRunEntry = "User Id to Run";
QString ExecutePlugin::dependencyActionEntry = "Dependency Action";
QString ExecutePlugin::projectTargetEntry = "Project Target";

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevExecuteFactory, registerPlugin<ExecutePlugin>(); )
K_EXPORT_PLUGIN(KDevExecuteFactory(KAboutData("kdevexecute", "kdevexecute", ki18n("Execute support"), "0.1", ki18n("Allows running of native apps"), KAboutData::License_GPL)))

ExecutePlugin::ExecutePlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevExecuteFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IExecutePlugin )
    m_configType = new NativeAppConfigType();
    m_configType->addLauncher( new NativeAppLauncher() );
    kDebug() << "adding native app launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecutePlugin::~ExecutePlugin()
{
}

void ExecutePlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = 0;
}

QStringList ExecutePlugin::arguments( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{

    if( !cfg )
    {
        return QStringList();
    }
    
    KShell::Errors err;
    QStringList args = KShell::splitArgs( cfg->config().readEntry( ExecutePlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
    if( err != KShell::NoError )
    {
        
        if( err == KShell::BadQuoting ) 
        {
            err_ = i18n("There is a quoting error in the arguments for "
            "the launch configuration '%1'. Aborting start.", cfg->name() );
        } else 
        {   
            err_ = i18n("A shell meta character was included in the "
            "arguments for the launch configuration '%1', "
            "this is not supported currently. Aborting start.", cfg->name() );
        }
        args = QStringList();
        kWarning() << "Launch Configuration:" << cfg->name() << "arguments have meta characters";
    }
    return args;
}


KJob* ExecutePlugin::dependecyJob( KDevelop::ILaunchConfiguration* cfg ) const
{
    QVariantList deps = KDevelop::stringToQVariant( cfg->config().readEntry( dependencyEntry, QString() ) ).toList();
    QString depAction = cfg->config().readEntry( dependencyActionEntry, "Nothing" );
    if( depAction != "Nothing" && !deps.isEmpty() ) 
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QList<KDevelop::ProjectBaseItem*> items;
        foreach( const QVariant& dep, deps )
        {
            KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex( dep.toStringList() ) );
            if( item )
            {
                items << item;
            }
            else
            {
                KMessageBox::error(core()->uiController()->activeMainWindow(),
                                   i18n("Couldn't resolve the dependency: %1", dep.toString()));
            }
        }
        KDevelop::BuilderJob* job = new KDevelop::BuilderJob();
        if( depAction == "Build" )
        {
            job->addItems( KDevelop::BuilderJob::Build, items );
        } else if( depAction == "Install" )
        {
            job->addItems( KDevelop::BuilderJob::Install, items );
        }
        job->updateJobName();
        return job;
    }
    return 0;
}


QString ExecutePlugin::environmentGroup( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return "";
    }
    
    return cfg->config().readEntry( ExecutePlugin::environmentGroupEntry, "" );
}


KUrl ExecutePlugin::executable( KDevelop::ILaunchConfiguration* cfg, QString& err ) const
{
    KUrl executable;
    if( !cfg ) 
    {
        return executable;
    }
    KConfigGroup grp = cfg->config();
    if( grp.readEntry(ExecutePlugin::isExecutableEntry, false ) )
    {
        executable = grp.readEntry( ExecutePlugin::executableEntry, KUrl("") );
    } else 
    {
        QStringList prjitem = grp.readEntry( ExecutePlugin::projectTargetEntry, QStringList() );
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex(prjitem) );
        if( item && item->executable() )
        {
            // TODO: Need an option in the gui to choose between installed and builddir url here, currently cmake only supports builddir url
            executable = item->executable()->builtUrl();
        }
    }
    if( executable.isEmpty() )
    {
        err = i18n("No valid executable specified");
        kWarning() << "Launch Configuration:" << cfg->name() << "no valid executable set";
    } else
    {
        KShell::Errors err_;
        if( KShell::splitArgs( executable.toLocalFile(), KShell::TildeExpand | KShell::AbortOnMeta, &err_ ).isEmpty() || err_ != KShell::NoError )
        {
            executable = KUrl();
            if( err_ == KShell::BadQuoting ) 
            {
                err = i18n("There is a quoting error in the executable "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else 
            {   
                err = i18n("A shell meta character was included in the "
                "executable for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            kWarning() << "Launch Configuration:" << cfg->name() << "executable has meta characters";
        }
    }
    return executable;
}


bool ExecutePlugin::useTerminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return false;
    }
    
    return cfg->config().readEntry( ExecutePlugin::useTerminalEntry, false );
}


QString ExecutePlugin::terminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QString();
    }
    
    return cfg->config().readEntry( ExecutePlugin::terminalEntry, QString() );
}


KUrl ExecutePlugin::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return KUrl();
    }
    
    return cfg->config().readEntry( ExecutePlugin::workingDirEntry, KUrl() );
}


QString ExecutePlugin::nativeAppConfigTypeId() const
{
    return _nativeAppConfigTypeId;
}


#include "executeplugin.moc"

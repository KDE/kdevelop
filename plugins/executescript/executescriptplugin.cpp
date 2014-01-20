/*
  * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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

#include "executescriptplugin.h"

#include <QApplication>

#include <kconfiggroup.h>
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

#include "scriptappconfig.h"
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <kshell.h>
#include <util/kdevstringhandler.h>

QString ExecuteScriptPlugin::_scriptAppConfigTypeId = "Script Application";
QString ExecuteScriptPlugin::interpreterEntry = "Interpreter";
QString ExecuteScriptPlugin::workingDirEntry = "Working Directory";
QString ExecuteScriptPlugin::executableEntry = "Executable";
QString ExecuteScriptPlugin::executeOnRemoteHostEntry = "Execute on Remote Host";
QString ExecuteScriptPlugin::runCurrentFileEntry = "Run current file";
QString ExecuteScriptPlugin::remoteHostEntry = "Remote Host";
QString ExecuteScriptPlugin::argumentsEntry = "Arguments";
QString ExecuteScriptPlugin::isExecutableEntry = "isExecutable";
QString ExecuteScriptPlugin::environmentGroupEntry = "EnvironmentGroup";
//QString ExecuteScriptPlugin::useTerminalEntry = "Use External Terminal";
QString ExecuteScriptPlugin::userIdToRunEntry = "User Id to Run";
QString ExecuteScriptPlugin::projectTargetEntry = "Project Target";
QString ExecuteScriptPlugin::outputFilteringEntry = "Output Filtering Mode";

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevExecuteFactory, registerPlugin<ExecuteScriptPlugin>(); )
// K_EXPORT_PLUGIN(KDevExecuteFactory(KAboutData("kdevexecutescript", "kdevexecutescript", ki18n("Execute script support"), "1.0 Beta 1", ki18n("Allows running of scripts"), KAboutData::License_GPL)
//     .addAuthor(ki18n("Niko Sams"), ki18n("Author"), "niko.sams@gmail.com", "http://nikosams.blogspot.com")
// ))

ExecuteScriptPlugin::ExecuteScriptPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin("kdevexecutescript", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IExecuteScriptPlugin )
    m_configType = new ScriptAppConfigType();
    m_configType->addLauncher( new ScriptAppLauncher( this ) );
    kDebug() << "adding script launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecuteScriptPlugin::~ExecuteScriptPlugin()
{
}


void ExecuteScriptPlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = 0;
}

KUrl ExecuteScriptPlugin::script( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{
    KUrl script;

    if( !cfg )
    {
        return script;
    }
    KConfigGroup grp = cfg->config();

    script = grp.readEntry( ExecuteScriptPlugin::executableEntry, QUrl() );
    if( !script.isLocalFile() || script.isEmpty() )
    {
        err_ = i18n("No valid executable specified");
        kWarning() << "Launch Configuration:" << cfg->name() << "no valid script set";
    } else
    {
        KShell::Errors err;
        if( KShell::splitArgs( script.toLocalFile(), KShell::TildeExpand | KShell::AbortOnMeta, &err ).isEmpty() || err != KShell::NoError )
        {
            script = KUrl();
            if( err == KShell::BadQuoting )
            {
                err_ = i18n("There is a quoting error in the script "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else
            {
                err_ = i18n("A shell meta character was included in the "
                "script for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            kWarning() << "Launch Configuration:" << cfg->name() << "script has meta characters";
        }
    }
    return script;
}

QString ExecuteScriptPlugin::remoteHost(ILaunchConfiguration* cfg, QString& err) const
{
    if (!cfg) return QString();
    KConfigGroup grp = cfg->config();
    if(grp.readEntry(ExecuteScriptPlugin::executeOnRemoteHostEntry, false)) {
        QString host = grp.readEntry(ExecuteScriptPlugin::remoteHostEntry, "");
        if (host.isEmpty()) {
            err = i18n("No remote host set for launch configuration '%1'. "
            "Aborting start.", cfg->name() );
            kWarning() << "Launch Configuration:" << cfg->name() << "no remote host set";
        }
        return host;
    }
    return QString();
}

QStringList ExecuteScriptPlugin::arguments( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{
    if( !cfg )
    {
        return QStringList();
    }

    KShell::Errors err;
    QStringList args = KShell::splitArgs( cfg->config().readEntry( ExecuteScriptPlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
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

QString ExecuteScriptPlugin::environmentGroup( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return "";
    }
    
    return cfg->config().readEntry( ExecuteScriptPlugin::environmentGroupEntry, "" );
}

int ExecuteScriptPlugin::outputFilterModeId( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return 0;
    }

    return cfg->config().readEntry( ExecuteScriptPlugin::outputFilteringEntry, 0 );
}

bool ExecuteScriptPlugin::runCurrentFile(ILaunchConfiguration* cfg) const
{
    if( !cfg )
    {
        return false;
    }

    return cfg->config().readEntry( ExecuteScriptPlugin::runCurrentFileEntry, true );
}

QString ExecuteScriptPlugin::interpreter( KDevelop::ILaunchConfiguration* cfg, QString& err ) const
{
    QString interpreter;
    if( !cfg ) 
    {
        return interpreter;
    }
    KConfigGroup grp = cfg->config();

    interpreter = grp.readEntry( ExecuteScriptPlugin::interpreterEntry, QString("") );

    if( interpreter.isEmpty() )
    {
        err = i18n("No valid interpreter specified");
        kWarning() << "Launch Configuration:" << cfg->name() << "no valid interpreter set";
    } else
    {
        KShell::Errors err_;
        if( KShell::splitArgs( interpreter, KShell::TildeExpand | KShell::AbortOnMeta, &err_ ).isEmpty() || err_ != KShell::NoError )
        {
            interpreter.clear();
            if( err_ == KShell::BadQuoting ) 
            {
                err = i18n("There is a quoting error in the interpreter "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else 
            {   
                err = i18n("A shell meta character was included in the "
                "interpreter for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            kWarning() << "Launch Configuration:" << cfg->name() << "interpreter has meta characters";
        }
    }
    return interpreter;
}

/*
bool ExecuteScriptPlugin::useTerminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return false;
    }
    
    return cfg->config().readEntry( ExecuteScriptPlugin::useTerminalEntry, false );
}
*/

KUrl ExecuteScriptPlugin::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return KUrl();
    }
    
    return cfg->config().readEntry( ExecuteScriptPlugin::workingDirEntry, QUrl() );
}


QString ExecuteScriptPlugin::scriptAppConfigTypeId() const
{
    return _scriptAppConfigTypeId;
}


#include "executescriptplugin.moc"

/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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
#include "scriptappconfig.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "scriptappjob.h"
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/builderjob.h>
#include <kmessagebox.h>
#include <interfaces/iuicontroller.h>
#include <util/executecompositejob.h>
#include <kparts/mainwindow.h>
#include <interfaces/iplugincontroller.h>

#include "executescriptplugin.h"
#include <util/kdevstringhandler.h>
#include <util/environmentgrouplist.h>
#include <project/projectitemlineedit.h>

KIcon ScriptAppConfigPage::icon() const
{
    return KIcon("system-run");
}

void ScriptAppConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project )
{
    bool b = blockSignals( true );
    interpreter->setText( cfg.readEntry( ExecuteScriptPlugin::interpreterEntry, "" ) );
    if( cfg.readEntry( ExecuteScriptPlugin::isExecutableEntry, false ) ) 
    {
        executableRadio->setChecked( true );
        executablePath->setUrl( cfg.readEntry( ExecuteScriptPlugin::executableEntry, KUrl() ) );
    } else 
    {
        if( project )
        {
            projectTarget->setBaseItem( project->projectItem() );
        } else {
            projectTarget->setBaseItem( 0 );
        }
        projectTargetRadio->setChecked( true );
        projectTarget->setItemPath( cfg.readEntry( ExecuteScriptPlugin::projectTargetEntry, QStringList() ) );
    }
    if( cfg.readEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, false ) ) {
        remoteHostRadio->setChecked( true );
    } else {
        localHostRadio->setChecked( true );
    }
    remoteHost->setText( cfg.readEntry( ExecuteScriptPlugin::remoteHostEntry, "" ) );
    arguments->setText( cfg.readEntry( ExecuteScriptPlugin::argumentsEntry, "" ) );
    workingDirectory->setUrl( cfg.readEntry( ExecuteScriptPlugin::workingDirEntry, KUrl() ) );
    environment->setCurrentProfile( cfg.readEntry( ExecuteScriptPlugin::environmentGroupEntry, "default" ) );
    outputFilteringMode->setCurrentIndex( cfg.readEntry( ExecuteScriptPlugin::outputFilteringEntry, 0u ));
    //runInTerminal->setChecked( cfg.readEntry( ExecuteScriptPlugin::useTerminalEntry, false ) );
    blockSignals( b );
}

ScriptAppConfigPage::ScriptAppConfigPage( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);

    //Set workingdirectory widget to ask for directories rather than files
    workingDirectory->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    KDevelop::EnvironmentGroupList env( KGlobal::config() );
    environment->addItems( env.groups() );


    //connect signals to changed signal
    connect( interpreter, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( projectTarget, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( projectTargetRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executableRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executablePath->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( executablePath, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( arguments, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( workingDirectory, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( workingDirectory->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( environment, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    //connect( runInTerminal, SIGNAL(toggled(bool)), SIGNAL(changed()) );
}

void ScriptAppConfigPage::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry( ExecuteScriptPlugin::interpreterEntry, interpreter->text() );
    cfg.writeEntry( ExecuteScriptPlugin::isExecutableEntry, executableRadio->isChecked() );
    if( executableRadio-> isChecked() )
    {
        cfg.writeEntry( ExecuteScriptPlugin::executableEntry, executablePath->url() );
        cfg.deleteEntry( ExecuteScriptPlugin::projectTargetEntry );
    } else
    {
        cfg.writeEntry( ExecuteScriptPlugin::projectTargetEntry, projectTarget->itemPath() );
        cfg.deleteEntry( ExecuteScriptPlugin::executableEntry );
    }
    cfg.writeEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, remoteHostRadio->isChecked() );
    cfg.writeEntry( ExecuteScriptPlugin::remoteHostEntry, remoteHost->text() );
    cfg.writeEntry( ExecuteScriptPlugin::argumentsEntry, arguments->text() );
    cfg.writeEntry( ExecuteScriptPlugin::workingDirEntry, workingDirectory->url() );
    cfg.writeEntry( ExecuteScriptPlugin::environmentGroupEntry, environment->currentProfile() );
    cfg.writeEntry( ExecuteScriptPlugin::outputFilteringEntry, outputFilteringMode->currentIndex() );
    //cfg.writeEntry( ExecuteScriptPlugin::useTerminalEntry, runInTerminal->isChecked() );
}

QString ScriptAppConfigPage::title() const
{
    return i18n("Configure Native Application");
}

QList< KDevelop::LaunchConfigurationPageFactory* > ScriptAppLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString ScriptAppLauncher::description() const
{
    return "Executes Script Applications";
}

QString ScriptAppLauncher::id()
{
    return "scriptAppLauncher";
}

QString ScriptAppLauncher::name() const
{
    return i18n("Script Application");
}

ScriptAppLauncher::ScriptAppLauncher(ExecuteScriptPlugin* plugin)
: m_plugin( plugin )
{
}

KJob* ScriptAppLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }
    if( launchMode == "execute" )
    {
        return new ScriptAppJob( m_plugin, cfg);
    }
    kWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

QStringList ScriptAppLauncher::supportedModes() const
{
    return QStringList() << "execute";
}

KDevelop::LaunchConfigurationPage* ScriptAppPageFactory::createWidget(QWidget* parent)
{
    return new ScriptAppConfigPage( parent );
}

ScriptAppPageFactory::ScriptAppPageFactory()
{
}

ScriptAppConfigType::ScriptAppConfigType()
{
    factoryList.append( new ScriptAppPageFactory() );
}

ScriptAppConfigType::~ScriptAppConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString ScriptAppConfigType::name() const
{
    return i18n("Script Application");
}


QList<KDevelop::LaunchConfigurationPageFactory*> ScriptAppConfigType::configPages() const
{
    return factoryList;
}

QString ScriptAppConfigType::id() const
{
    return ExecuteScriptPlugin::_scriptAppConfigTypeId;
}

KIcon ScriptAppConfigType::icon() const
{
    return KIcon("application-x-executable-script");
}

bool ScriptAppConfigType::canLaunch(const KUrl& /*file*/) const
{
    return false;
}

bool ScriptAppConfigType::canLaunch(KDevelop::ProjectBaseItem* /*item*/) const
{
    return false;
}

void ScriptAppConfigType::configureLaunchFromItem(KConfigGroup /*config*/, KDevelop::ProjectBaseItem* /*item*/) const
{
}

void ScriptAppConfigType::configureLaunchFromCmdLineArguments(KConfigGroup /*config*/, const QStringList &/*args*/) const
{
}

void ScriptAppConfigType::createEmptyLauncher()
{
    QPair<QString,QString> launcher = qMakePair( launchers().at( 0 )->supportedModes().at(0), launchers().at( 0 )->id() );
    KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher);
    KConfigGroup cfg = config->config();
    cfg.sync();
    
    emit signalAddLaunchConfiguration(config);
}


#include "scriptappconfig.moc"

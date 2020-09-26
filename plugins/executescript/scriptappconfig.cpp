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

#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KShell>

#include <QIcon>
#include <QMimeDatabase>

#include <interfaces/icore.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "scriptappjob.h"
#include <interfaces/iproject.h>
#include <interfaces/idocumentcontroller.h>

#include "executescriptplugin.h"
#include <util/kdevstringhandler.h>
#include <util/path.h>
#include <debug.h>

using namespace KDevelop;

static const QString interpreterForUrl(const QUrl& url) {
    auto mimetype = QMimeDatabase().mimeTypeForUrl(url);
    static QHash<QString, QString> knownMimetypes;
    if ( knownMimetypes.isEmpty() ) {
        knownMimetypes[QStringLiteral("text/x-python")] = QStringLiteral("python3");
        knownMimetypes[QStringLiteral("application/x-php")] = QStringLiteral("php");
        knownMimetypes[QStringLiteral("application/x-ruby")] = QStringLiteral("ruby");
        knownMimetypes[QStringLiteral("application/x-shellscript")] = QStringLiteral("bash");
        knownMimetypes[QStringLiteral("application/x-perl")] = QStringLiteral("perl -e");
    }
    const QString& interp = knownMimetypes.value(mimetype.name());
    return interp;
}

QIcon ScriptAppConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("system-run"));
}

void ScriptAppConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project )
{
    QSignalBlocker blocker(this);
    if( project )
    {
        executablePath->setStartDir( project->path().toUrl() );
    }

    auto doc = KDevelop::ICore::self()->documentController()->activeDocument();
    interpreter->lineEdit()->setText( cfg.readEntry( ExecuteScriptPlugin::interpreterEntry,
                                                     doc ? interpreterForUrl(doc->url()) : QString() ) );
    executablePath->setUrl( QUrl::fromLocalFile(cfg.readEntry( ExecuteScriptPlugin::executableEntry, QString() )) );
    remoteHostCheckbox->setChecked( cfg.readEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, false ) );
    remoteHost->setText( cfg.readEntry( ExecuteScriptPlugin::remoteHostEntry, "" ) );
    bool runCurrent = cfg.readEntry( ExecuteScriptPlugin::runCurrentFileEntry, true );
    if ( runCurrent ) {
        runCurrentFile->setChecked( true );
    } else {
        runFixedFile->setChecked( true );
    }
    arguments->setText( cfg.readEntry( ExecuteScriptPlugin::argumentsEntry, "" ) );
    workingDirectory->setUrl( cfg.readEntry( ExecuteScriptPlugin::workingDirEntry, QUrl() ) );
    environment->setCurrentProfile(cfg.readEntry(ExecuteScriptPlugin::environmentProfileEntry, QString()));
    outputFilteringMode->setCurrentIndex( cfg.readEntry( ExecuteScriptPlugin::outputFilteringEntry, 2u ));
    //runInTerminal->setChecked( cfg.readEntry( ExecuteScriptPlugin::useTerminalEntry, false ) );
}

ScriptAppConfigPage::ScriptAppConfigPage( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    interpreter->lineEdit()->setPlaceholderText(i18nc("@info:placeholder", "Type or select an interpreter..."));

    //Set workingdirectory widget to ask for directories rather than files
    workingDirectory->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    configureEnvironment->setSelectionWidget(environment);

    //connect signals to changed signal
    connect( interpreter->lineEdit(), &QLineEdit::textEdited, this, &ScriptAppConfigPage::changed );
    connect( executablePath->lineEdit(), &KLineEdit::textEdited, this, &ScriptAppConfigPage::changed );
    connect( executablePath, &KUrlRequester::urlSelected, this, &ScriptAppConfigPage::changed );
    connect( arguments, &QLineEdit::textEdited, this, &ScriptAppConfigPage::changed );
    connect( workingDirectory, &KUrlRequester::urlSelected, this, &ScriptAppConfigPage::changed );
    connect( workingDirectory->lineEdit(), &KLineEdit::textEdited, this, &ScriptAppConfigPage::changed );
    connect( environment, &EnvironmentSelectionWidget::currentProfileChanged, this, &ScriptAppConfigPage::changed );
    //connect( runInTerminal, SIGNAL(toggled(bool)), SIGNAL(changed()) );
}

void ScriptAppConfigPage::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry( ExecuteScriptPlugin::interpreterEntry, interpreter->lineEdit()->text() );
    cfg.writeEntry( ExecuteScriptPlugin::executableEntry, executablePath->url() );
    cfg.writeEntry( ExecuteScriptPlugin::executeOnRemoteHostEntry, remoteHostCheckbox->isChecked() );
    cfg.writeEntry( ExecuteScriptPlugin::remoteHostEntry, remoteHost->text() );
    cfg.writeEntry( ExecuteScriptPlugin::runCurrentFileEntry, runCurrentFile->isChecked() );
    cfg.writeEntry( ExecuteScriptPlugin::argumentsEntry, arguments->text() );
    cfg.writeEntry( ExecuteScriptPlugin::workingDirEntry, workingDirectory->url() );
    cfg.writeEntry( ExecuteScriptPlugin::environmentProfileEntry, environment->currentProfile() );
    cfg.writeEntry( ExecuteScriptPlugin::outputFilteringEntry, outputFilteringMode->currentIndex() );
    //cfg.writeEntry( ExecuteScriptPlugin::useTerminalEntry, runInTerminal->isChecked() );
}

QString ScriptAppConfigPage::title() const
{
    return i18nc("@title:tab", "Configure Script Application");
}

QList< KDevelop::LaunchConfigurationPageFactory* > ScriptAppLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString ScriptAppLauncher::description() const
{
    return i18n("Executes Script Applications");
}

QString ScriptAppLauncher::id()
{
    return QStringLiteral("scriptAppLauncher");
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
        return nullptr;
    }
    if( launchMode == QLatin1String("execute") )
    {
        return new ScriptAppJob( m_plugin, cfg);
    }
    qCWarning(PLUGIN_EXECUTESCRIPT) << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return nullptr;
}

QStringList ScriptAppLauncher::supportedModes() const
{
    return QStringList() << QStringLiteral("execute");
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

QString ScriptAppConfigType::sharedId()
{
    return QStringLiteral("Script Application");
}

QString ScriptAppConfigType::id() const
{
    return sharedId();
}

QIcon ScriptAppConfigType::icon() const
{
    return QIcon::fromTheme(QStringLiteral("preferences-plugin-script"));
}

bool ScriptAppConfigType::canLaunch(const QUrl& file) const
{
    return ! interpreterForUrl(file).isEmpty();
}

bool ScriptAppConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    return ! interpreterForUrl(item->path().toUrl()).isEmpty();
}

void ScriptAppConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry(ExecuteScriptPlugin::executableEntry, item->path().toUrl());
    config.writeEntry(ExecuteScriptPlugin::interpreterEntry, interpreterForUrl(item->path().toUrl()));
    config.writeEntry(ExecuteScriptPlugin::outputFilteringEntry, 2u);
    config.writeEntry(ExecuteScriptPlugin::runCurrentFileEntry, false);
    config.sync();
}

void ScriptAppConfigType::configureLaunchFromCmdLineArguments(KConfigGroup cfg, const QStringList &args) const
{
    QStringList a(args);
    cfg.writeEntry( ExecuteScriptPlugin::interpreterEntry, a.takeFirst() );
    cfg.writeEntry( ExecuteScriptPlugin::executableEntry, a.takeFirst() );
    cfg.writeEntry( ExecuteScriptPlugin::argumentsEntry, KShell::joinArgs(a) );
    cfg.writeEntry( ExecuteScriptPlugin::runCurrentFileEntry, false );
    cfg.sync();
}


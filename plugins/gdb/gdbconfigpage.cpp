/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gdbconfigpage.h"

#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iproject.h>
#include <util/executecompositejob.h>
#include <execute/iexecuteplugin.h>

#include "dbgglobal.h"
#include "debuggerplugin.h"
#include "debuglog.h"
#include "midebugjobs.h"

#include "ui_gdbconfigpage.h"
#include <interfaces/iruncontroller.h>
#include <interfaces/icore.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;
namespace Config = KDevMI::GDB::Config;

GdbConfigPage::GdbConfigPage( QWidget* parent )
    : LaunchConfigurationPage(parent), ui( new Ui::GdbConfigPage )
{
    ui->setupUi( this );
    ui->kcfg_gdbPath->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    connect(ui->kcfg_asmDemangle, &QCheckBox::toggled, this, &GdbConfigPage::changed);
    connect(ui->kcfg_configGdbScript, &KUrlRequester::textChanged, this, &GdbConfigPage::changed);
    //connect(ui->kcfg_dbgTerminal, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(ui->kcfg_debuggingShell, &KUrlRequester::textChanged, this, &GdbConfigPage::changed);
    connect(ui->kcfg_displayStaticMembers, &QCheckBox::toggled, this, &GdbConfigPage::changed);
    connect(ui->kcfg_gdbPath, &KUrlRequester::textChanged, this, &GdbConfigPage::changed);
    connect(ui->kcfg_runGdbScript, &KUrlRequester::textChanged, this, &GdbConfigPage::changed);
    connect(ui->kcfg_runShellScript, &KUrlRequester::textChanged, this, &GdbConfigPage::changed);
    connect(ui->kcfg_startWith, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GdbConfigPage::changed);

    //Setup data info for combobox
    ui->kcfg_startWith->setItemData(0, QStringLiteral("ApplicationOutput"));
    ui->kcfg_startWith->setItemData(1, QStringLiteral("GdbConsole"));
    ui->kcfg_startWith->setItemData(2, QStringLiteral("FrameStack"));
}

GdbConfigPage::~GdbConfigPage()
{
    delete ui;
}

QIcon GdbConfigPage::icon() const
{
    return QIcon();
}

void GdbConfigPage::loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject*  )
{
    bool block = blockSignals( true );
    ui->kcfg_gdbPath->setUrl( cfg.readEntry( Config::GdbPathEntry, QUrl() ) );
    ui->kcfg_debuggingShell->setUrl( cfg.readEntry( Config::DebuggerShellEntry, QUrl() ) );
    ui->kcfg_configGdbScript->setUrl( cfg.readEntry( Config::RemoteGdbConfigEntry, QUrl() ) );
    ui->kcfg_runShellScript->setUrl( cfg.readEntry( Config::RemoteGdbShellEntry, QUrl() ) );
    ui->kcfg_runGdbScript->setUrl( cfg.readEntry( Config::RemoteGdbRunEntry, QUrl() ) );
    ui->kcfg_displayStaticMembers->setChecked( cfg.readEntry( Config::StaticMembersEntry, false ) );
    ui->kcfg_asmDemangle->setChecked( cfg.readEntry( Config::DemangleNamesEntry, true) );
    ui->kcfg_startWith->setCurrentIndex( ui->kcfg_startWith->findData( cfg.readEntry( KDevMI::Config::StartWithEntry, "ApplicationOutput" ) ) );
    blockSignals( block );
}

void GdbConfigPage::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* ) const
{
    cfg.writeEntry(Config::GdbPathEntry, ui->kcfg_gdbPath->url() );
    cfg.writeEntry(Config::DebuggerShellEntry, ui->kcfg_debuggingShell->url() );
    cfg.writeEntry(Config::RemoteGdbConfigEntry, ui->kcfg_configGdbScript->url() );
    cfg.writeEntry(Config::RemoteGdbShellEntry, ui->kcfg_runShellScript->url() );
    cfg.writeEntry(Config::RemoteGdbRunEntry, ui->kcfg_runGdbScript->url() );
    cfg.writeEntry(Config::StaticMembersEntry, ui->kcfg_displayStaticMembers->isChecked() );
    cfg.writeEntry(Config::DemangleNamesEntry, ui->kcfg_asmDemangle->isChecked() );
    cfg.writeEntry(KDevMI::Config::StartWithEntry, ui->kcfg_startWith->itemData( ui->kcfg_startWith->currentIndex() ).toString() );
}

QString GdbConfigPage::title() const
{
    return i18nc("@title:tab", "GDB Configuration");
}


GdbLauncher::GdbLauncher( KDevMI::GDB::CppDebuggerPlugin* p, IExecutePlugin* execute )
    : m_plugin( p )
    , m_execute( execute )
{
    factoryList << new GdbConfigPageFactory();
}

GdbLauncher::~GdbLauncher()
{
    qDeleteAll(factoryList);
}

QList< KDevelop::LaunchConfigurationPageFactory* > GdbLauncher::configPages() const
{
    return factoryList;
}

QString GdbLauncher::id()
{
    return QStringLiteral("gdb");
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
        return nullptr;
    }
    if( launchMode == QLatin1String("debug") )
    {
        Q_ASSERT(m_execute);
        Q_ASSERT(m_plugin);

        if (KDevelop::ICore::self()->debugController()->currentSession() != nullptr) {
            KMessageBox::ButtonCode answer = KMessageBox::warningTwoActions(
                nullptr,
                i18n("A program is already being debugged. Do you want to abort the "
                     "currently running debug session and continue with the launch?"),
                {}, KGuiItem(i18nc("@action:button", "Abort Current Session"), QStringLiteral("application-exit")),
                KStandardGuiItem::cancel());
            if (answer == KMessageBox::SecondaryAction)
                return nullptr;
        }

        QList<KJob*> l;
        KJob* depjob = m_execute->dependencyJob(cfg);
        if( depjob )
        {
            l << depjob;
        }
        l << new KDevMI::MIDebugJob( m_plugin, cfg, m_execute );
        return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), l );
    }
    qCWarning(DEBUGGERGDB) << "Unknown launch mode" << launchMode << "for config:" << cfg->name();
    return nullptr;
}

QStringList GdbLauncher::supportedModes() const
{
    return QStringList() << QStringLiteral("debug");
}

QString GdbLauncher::description() const
{
    return i18n("Executes a native application in GDB");
}

KDevelop::LaunchConfigurationPage* GdbConfigPageFactory::createWidget( QWidget* parent )
{
    return new GdbConfigPage( parent );
}

#include "moc_gdbconfigpage.cpp"

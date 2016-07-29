/*
 * LLDB Debugger Support
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lldbconfigpage.h"
#include "ui_lldbconfigpage.h"

#include "dbgglobal.h"

#include <util/environmentselectionwidget.h>

#include <KConfigGroup>
#include <KUrlRequester>

#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace KDevelop;
namespace Config = KDevMI::LLDB::Config;

LldbConfigPage::LldbConfigPage(QWidget* parent)
    : LaunchConfigurationPage(parent)
    , ui(new Ui::LldbConfigPage)
{
    ui->setupUi(this);
    ui->lineDebuggerExec->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    ui->lineConfigScript->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);

    QRegularExpression rx(R"([^:]+:\d+)");
    ui->lineRemoteServer->setValidator(new QRegularExpressionValidator(rx, this));

    ui->comboStartWith->setItemData(0, "ApplicationOutput");
    ui->comboStartWith->setItemData(1, "GdbConsole");
    ui->comboStartWith->setItemData(2, "FrameStack");

    connect(ui->lineDebuggerExec, &KUrlRequester::textChanged, this, &LldbConfigPage::changed);
    connect(ui->lineDebuggerArgs, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
    connect(ui->comboEnv, &EnvironmentSelectionWidget::currentProfileChanged,
            this, &LldbConfigPage::changed);

    connect(ui->lineConfigScript, &KUrlRequester::textChanged, this, &LldbConfigPage::changed);
    connect(ui->comboStartWith, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LldbConfigPage::changed);

    connect(ui->groupRemote, &QGroupBox::clicked, this, &LldbConfigPage::changed);
    connect(ui->lineRemoteServer, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
    connect(ui->lineOnDevPath, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
}

LldbConfigPage::~LldbConfigPage()
{
    delete ui;
}

QIcon LldbConfigPage::icon() const
{
    return {};
}

QString LldbConfigPage::title() const
{
    return i18n("LLDB Configuration");
}

void LldbConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject *)
{
    bool block = blockSignals(true);
    ui->lineDebuggerExec->setUrl(cfg.readEntry(Config::LldbExecutableEntry, QUrl()));
    ui->lineDebuggerArgs->setText(cfg.readEntry(Config::LldbArgumentsEntry, QString()));
    ui->comboEnv->setCurrentProfile(cfg.readEntry(Config::LldbEnvironmentEntry, QString()));
    ui->checkInheritSystem->setChecked(cfg.readEntry(Config::LldbInheritSystemEnvEntry, true));
    ui->lineConfigScript->setUrl(cfg.readEntry(Config::LldbConfigScriptEntry, QUrl()));
    ui->checkBreakOnStart->setChecked(cfg.readEntry(KDevMI::Config::BreakOnStartEntry, false));
    ui->comboStartWith->setCurrentIndex(ui->comboStartWith->findData(
        cfg.readEntry(KDevMI::Config::StartWithEntry, "ApplicationOutput")));
    ui->groupRemote->setChecked(cfg.readEntry(Config::LldbRemoteDebuggingEntry, false));
    ui->lineRemoteServer->setText(cfg.readEntry(Config::LldbRemoteServerEntry, QString()));
    ui->lineOnDevPath->setText(cfg.readEntry(Config::LldbRemotePathEntry, QString()));
    blockSignals(block);
}

void LldbConfigPage::saveToConfiguration(KConfigGroup cfg, KDevelop::IProject *) const
{
    cfg.writeEntry(Config::LldbExecutableEntry, ui->lineDebuggerExec->url());
    cfg.writeEntry(Config::LldbArgumentsEntry, ui->lineDebuggerArgs->text());
    cfg.writeEntry(Config::LldbEnvironmentEntry, ui->comboEnv->currentProfile());
    cfg.writeEntry(Config::LldbInheritSystemEnvEntry, ui->checkInheritSystem->isChecked());
    cfg.writeEntry(Config::LldbConfigScriptEntry, ui->lineConfigScript->url());
    cfg.writeEntry(KDevMI::Config::BreakOnStartEntry, ui->checkBreakOnStart->isChecked());
    cfg.writeEntry(KDevMI::Config::StartWithEntry, ui->comboStartWith->currentData().toString());
    cfg.writeEntry(Config::LldbRemoteDebuggingEntry, ui->groupRemote->isChecked());
    cfg.writeEntry(Config::LldbRemoteServerEntry, ui->lineRemoteServer->text());
    cfg.writeEntry(Config::LldbRemotePathEntry, ui->lineOnDevPath->text());
}

KDevelop::LaunchConfigurationPage * LldbConfigPageFactory::createWidget(QWidget* parent)
{
    return new LldbConfigPage(parent);
}

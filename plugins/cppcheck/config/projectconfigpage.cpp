/*
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projectconfigpage.h"
#include "ui_projectconfigpage.h"

#include "projectsettings.h"

#include <interfaces/iproject.h>

#include <QFontDatabase>

namespace cppcheck
{

ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent)
    : ConfigPage(plugin, new ProjectSettings, parent)
    , ui(new Ui::ProjectConfigPage)
    , m_parameters(new Parameters(project))
{
    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    ui->setupUi(this);

    ui->commandLine->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(this, &ProjectConfigPage::changed, this, &ProjectConfigPage::updateCommandLine);
    connect(ui->commandLineFilter, &QLineEdit::textChanged, this, &ProjectConfigPage::updateCommandLine);
    connect(ui->commandLineBreaks, &QCheckBox::stateChanged, this, &ProjectConfigPage::updateCommandLine);
}

ProjectConfigPage::~ProjectConfigPage()
{
}

QIcon ProjectConfigPage::icon() const
{
   return QIcon::fromTheme(QStringLiteral("cppcheck"));
}

void ProjectConfigPage::defaults()
{
    ConfigPage::defaults();
    updateCommandLine();
}

void ProjectConfigPage::reset()
{
    ConfigPage::reset();
    updateCommandLine();
}

QString ProjectConfigPage::name() const
{
    return i18nc("@title:tab", "Cppcheck");
}

void ProjectConfigPage::updateCommandLine()
{
    m_parameters->checkStyle = ui->kcfg_checkStyle->isChecked();
    m_parameters->checkPerformance = ui->kcfg_checkPerformance->isChecked();
    m_parameters->checkPortability = ui->kcfg_checkPortability->isChecked();
    m_parameters->checkInformation = ui->kcfg_checkInformation->isChecked();
    m_parameters->checkUnusedFunction = ui->kcfg_checkUnusedFunction->isChecked();
    m_parameters->checkMissingInclude = ui->kcfg_checkMissingInclude->isChecked();
    m_parameters->inconclusiveAnalysis = ui->kcfg_inconclusiveAnalysis->isChecked();
    m_parameters->forceCheck = ui->kcfg_forceCheck->isChecked();
    m_parameters->checkConfig = ui->kcfg_checkConfig->isChecked();
    m_parameters->useProjectIncludes = ui->kcfg_useProjectIncludes->isChecked();
    m_parameters->useSystemIncludes = ui->kcfg_useSystemIncludes->isChecked();
    m_parameters->ignoredIncludes = ui->kcfg_ignoredIncludes->text();
    m_parameters->extraParameters = ui->kcfg_extraParameters->text().trimmed();

    QString message;
    QString commandLine = m_parameters->commandLine(message).join(QLatin1Char(' '));

    if (message.isEmpty()) {
        ui->messageWidget->hide();
    } else {
        ui->messageWidget->setText(message);
        ui->messageWidget->show();
    }

    if (!ui->commandLineBreaks->isChecked()) {
        ui->commandLine->setPlainText(commandLine);
        return;
    }

    commandLine.replace(QLatin1String(" -"), QLatin1String("\n-"));
    QString filterText = ui->commandLineFilter->text();
    if (filterText.isEmpty()) {
        ui->commandLine->setPlainText(commandLine);
        ui->commandLineBreaks->setEnabled(true);
        return;
    }

    QStringList lines = commandLine.split(QLatin1Char('\n'));
    QMutableStringListIterator i(lines);

    while (i.hasNext()) {
        if (!i.next().contains(filterText)) {
            i.remove();
        }
    }

    ui->commandLine->setPlainText(lines.join(QLatin1Char('\n')));
    ui->commandLineBreaks->setEnabled(false);
}

}

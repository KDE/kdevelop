/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "projectconfigpage.h"
#include "ui_projectconfigpage.h"

#include "checksdb.h"
#include "checkswidget.h"
#include "plugin.h"
#include "projectsettings.h"

#include <interfaces/iproject.h>

#include <KLocalizedString>

namespace Clazy
{

ProjectConfigPage::ProjectConfigPage(Plugin* plugin, KDevelop::IProject* project, QWidget* parent)
    : ConfigPage(plugin, new ProjectSettings, parent)
    , m_parameters(new JobParameters(project))
{
    Q_ASSERT(plugin);

    Ui::ProjectConfigPage ui;
    ui.setupUi(this);

    if (plugin->checksDB()->isValid()) {
        ui.dbError->setVisible(false);
    } else {
        ui.dbError->setText(plugin->checksDB()->error());
        ui.dbError->setVisible(true);

        ui.tabWidget->setVisible(false);
        ui.commandLineWidget->setVisible(false);
        return;
    }

    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    // =============================================================================================

    auto checksWidget = new ChecksWidget(plugin->checksDB());
    checksWidget->setObjectName(QStringLiteral("kcfg_checks"));
    connect(checksWidget, &ChecksWidget::checksChanged, m_parameters.data(), &JobParameters::setChecks);

    auto checksLayout = new QVBoxLayout(ui.checksTab);
    checksLayout->addWidget(checksWidget);

    // =============================================================================================

    connect(ui.kcfg_onlyQt, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setOnlyQt(state != Qt::Unchecked);
    });

    connect(ui.kcfg_qtDeveloper, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setQtDeveloper(state != Qt::Unchecked);
    });

    connect(ui.kcfg_qt4Compat, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setQt4Compat(state != Qt::Unchecked);
    });

    connect(ui.kcfg_visitImplicitCode, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setVisitImplicitCode(state != Qt::Unchecked);
    });

    connect(ui.kcfg_ignoreIncludedFiles, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setIgnoreIncludedFiles(state != Qt::Unchecked);
    });

    ui.kcfg_headerFilter->setPlaceholderText(ui.kcfg_headerFilter->toolTip());
    connect(ui.kcfg_headerFilter, &QLineEdit::textChanged,
            m_parameters.data(), &JobParameters::setHeaderFilter);

    connect(ui.kcfg_enableAllFixits, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setEnableAllFixits(state != Qt::Unchecked);
    });

    connect(ui.kcfg_noInplaceFixits, &QCheckBox::stateChanged, this, [this](int state) {
            m_parameters->setNoInplaceFixits(state != Qt::Unchecked);
    });

    // =============================================================================================

    ui.kcfg_extraAppend->setPlaceholderText(ui.kcfg_extraAppend->toolTip());
    connect(ui.kcfg_extraAppend, &QLineEdit::textChanged,
            m_parameters.data(), &JobParameters::setExtraAppend);

    ui.kcfg_extraPrepend->setPlaceholderText(ui.kcfg_extraPrepend->toolTip());
    connect(ui.kcfg_extraPrepend, &QLineEdit::textChanged,
            m_parameters.data(), &JobParameters::setExtraPrepend);

    ui.kcfg_extraClazy->setPlaceholderText(ui.kcfg_extraClazy->toolTip());
    connect(ui.kcfg_extraClazy, &QLineEdit::textChanged,
            m_parameters.data(), &JobParameters::setExtraClazy);

    // =============================================================================================

    auto updateCommandLine = [this, ui]() {
        ui.commandLineWidget->setText(m_parameters->commandLine().join(QLatin1Char(' ')));
    };

    connect(m_parameters.data(), &JobParameters::changed, this, updateCommandLine);
    updateCommandLine();
}

ProjectConfigPage::~ProjectConfigPage() = default;

QIcon ProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("clazy"));
}

QString ProjectConfigPage::name() const
{
    return i18n("Clazy");
}

}

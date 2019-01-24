/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "mesonnewbuilddir.h"

#include "mesonbuilder.h"
#include "mesonmanager.h"
#include "ui_mesonnewbuilddir.h"
#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <project/helper.h>

#include <KColorScheme>
#include <kio_version.h>

#include <QDialogButtonBox>
#include <QFileInfo>

#include <algorithm>

using namespace KDevelop;

MesonNewBuildDir::MesonNewBuildDir(IProject* project, QWidget* parent)
    : QDialog(parent)
    , m_project(project)
{
    Q_ASSERT(project); // Just in case
    MesonManager* mgr = dynamic_cast<MesonManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the MesonManager

    setWindowTitle(
        i18n("Configure a build directory - %1", ICore::self()->runtimeController()->currentRuntime()->name()));

    m_ui = new Ui::MesonNewBuildDir;
    m_ui->setupUi(this);

    m_ui->advanced->setSupportedBackends(mgr->supportedMesonBackends());

    connect(m_ui->b_buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* b) {
        if (m_ui->b_buttonBox->buttonRole(b) == QDialogButtonBox::ResetRole) {
            resetFields();
        }
    });

#if KIO_VERSION >= QT_VERSION_CHECK(5, 33, 0)
    m_ui->i_buildDir->setAcceptMode(QFileDialog::AcceptSave);
#else
    m_ui->i_buildDir->fileDialog()->setAcceptMode(QFileDialog::AcceptSave);
#endif

    resetFields();
}

MesonNewBuildDir::~MesonNewBuildDir()
{
    delete m_ui;
}

void MesonNewBuildDir::resetFields()
{
    Meson::MesonConfig cfg = Meson::getMesonConfig(m_project);
    Path projectPath = m_project->path();
    MesonManager* mgr = dynamic_cast<MesonManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the MesonManager

    auto aConf = m_ui->advanced->getConfig();

    // Find a build dir that is not already configured
    Path buildDirPath = projectPath;
    buildDirPath.addPath(QStringLiteral("build"));

    auto checkInCfg = [](Meson::MesonConfig const& cfg, Path const& p) -> bool {
        for (auto const& i : cfg.buildDirs) {
            if (i.buildDir == p) {
                return true;
            }
        }
        return false;
    };

    for (int i = 2; checkInCfg(cfg, buildDirPath); ++i) {
        buildDirPath = projectPath;
        buildDirPath.addPath(QStringLiteral("build%1").arg(i));
    }

    m_ui->i_buildDir->setUrl(buildDirPath.toUrl());

    // Extra args
    aConf.args.clear();

    // Backend
    aConf.backend = mgr->defaultMesonBackend();

    // Meson exe
    aConf.meson = mgr->findMeson();

    m_ui->advanced->setConfig(aConf);
    updated();
}

void MesonNewBuildDir::setStatus(const QString& str, bool validConfig)
{
    m_configIsValid = validConfig;

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;
    if (validConfig) {
        role = KColorScheme::PositiveText;
    } else {
        role = KColorScheme::NegativeText;
    }

    QPalette pal = m_ui->l_statusMessage->palette();
    pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
    m_ui->l_statusMessage->setPalette(pal);
    m_ui->l_statusMessage->setText(str);

    auto okButton = m_ui->b_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(m_configIsValid);
    if (m_configIsValid) {
        auto cancelButton = m_ui->b_buttonBox->button(QDialogButtonBox::Cancel);
        cancelButton->clearFocus();
    }
}

void MesonNewBuildDir::updated()
{
    auto advanced = m_ui->advanced->getConfig();
    Path buildDir = Path(m_ui->i_buildDir->url());
    QFileInfo mesonExe(advanced.meson.toLocalFile());

    if (!mesonExe.exists() || !mesonExe.isExecutable() || !mesonExe.isFile()
        || !mesonExe.permission(QFileDevice::ReadUser | QFileDevice::ExeUser)) {
        setStatus(i18n("Specified meson executable does not exist"), false);
        return;
    }

    MesonBuilder::DirectoryStatus status = MesonBuilder::evaluateBuildDirectory(buildDir, advanced.backend);
    switch (status) {
    case MesonBuilder::CLEAN:
    case MesonBuilder::DOES_NOT_EXIST:
        setStatus(i18n("Creating new build directory"), true);
        break;
    case MesonBuilder::MESON_CONFIGURED:
        setStatus(i18n("Using an already configured build directory"), true);
        break;
    case MesonBuilder::MESON_FAILED_CONFIGURATION:
        setStatus(i18n("Using a broken meson build directory (this should be fine)"), true);
        break;
    case MesonBuilder::INVALID_BUILD_DIR:
        setStatus(i18n("Cannot use specified directory"), false);
        break;
    case MesonBuilder::DIR_NOT_EMPTY:
        setStatus(i18n("There are already files in the build directory"), false);
        break;
    case MesonBuilder::EMPTY_STRING:
        setStatus(i18n("The build directory field must not be empty"), false);
        break;
    case MesonBuilder::___UNDEFINED___:
        setStatus(i18n("You have reached unreachable code. This is a bug"), false);
        break;
    }

    bool buildDirChanged = false;
    if(m_oldBuildDir != buildDir.toLocalFile()) {
        m_oldBuildDir = buildDir.toLocalFile();
        buildDirChanged = true;
    }

    bool mesonHasChanged = m_ui->advanced->hasMesonChanged(); // Outside if to prevent lazy evaluation
    if (!m_ui->options->options() || mesonHasChanged || buildDirChanged) {
        if (status == MesonBuilder::MESON_CONFIGURED) {
            m_ui->options->repopulateFromBuildDir(m_project, currentConfig())->start();
        } else {
            m_ui->options->repopulateFromMesonFile(m_project, advanced.meson)->start();
        }
    }
}

Meson::BuildDir MesonNewBuildDir::currentConfig() const
{
    Meson::BuildDir buildDir;
    if (!m_configIsValid) {
        qCDebug(KDEV_Meson) << "Cannot generate build dir config from invalid config";
        return buildDir;
    }

    auto advanced = m_ui->advanced->getConfig();

    buildDir.buildDir = Path(m_ui->i_buildDir->url());
    buildDir.mesonArgs = advanced.args;
    buildDir.mesonBackend = advanced.backend;
    buildDir.mesonExecutable = advanced.meson;

    return buildDir;
}

QStringList MesonNewBuildDir::mesonArgs() const
{
    auto options = m_ui->options->options();
    if (!options) {
        return {};
    }

    return options->getMesonArgs();
}

bool MesonNewBuildDir::isConfigValid() const
{
    return m_configIsValid;
}

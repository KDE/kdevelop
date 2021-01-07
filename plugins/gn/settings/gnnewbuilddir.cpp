/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

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

#include "gnnewbuilddir.h"

#include "gnbuilder.h"
#include "gnimportprojectjob.h"
#include "gnmanager.h"
#include "ui_gnnewbuilddir.h"
#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <project/helper.h>

#include <KColorScheme>

#include <QDialogButtonBox>
#include <QFileInfo>

#include <algorithm>

using namespace KDevelop;

GNNewBuildDir::GNNewBuildDir(IProject* project, QWidget* parent)
    : QDialog(parent)
    , m_project(project)
{
    Q_ASSERT(project); // Just in case
    auto* mgr = dynamic_cast<GNManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the GNManager

    setWindowTitle(
        i18nc("@title:window", "Configure a Build Directory - %1", ICore::self()->runtimeController()->currentRuntime()->name()));

    m_ui = new Ui::GNNewBuildDir;
    m_ui->setupUi(this);

    connect(m_ui->b_buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* b) {
        if (m_ui->b_buttonBox->buttonRole(b) == QDialogButtonBox::ResetRole) {
            resetFields();
        }
    });

    m_ui->i_buildDir->setAcceptMode(QFileDialog::AcceptSave);

    resetFields();
}

GNNewBuildDir::~GNNewBuildDir()
{
    delete m_ui;
}

void GNNewBuildDir::resetFields()
{
    GN::GNConfig cfg = GN::getGNConfig(m_project);
    Path projectPath = m_project->path();
    auto* mgr = dynamic_cast<GNManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the GNManager

    auto aConf = m_ui->advanced->getConfig();

    // Find a build dir that is not already configured
    Path buildDirPath = projectPath;
    buildDirPath.addPath(QStringLiteral("out"));

    auto checkInCfg = [](const GN::GNConfig& cfg, const Path& p) -> bool {
        for (const auto& i : cfg.buildDirs) {
            if (i.buildDir == p) {
                return true;
            }
        }
        return false;
    };

    for (int i = 2; checkInCfg(cfg, buildDirPath); ++i) {
        buildDirPath = projectPath;
        buildDirPath.addPath(QStringLiteral("out_%1").arg(i));
    }

    m_ui->i_buildDir->setUrl(buildDirPath.toUrl());

    // Extra args
    aConf.args.clear();

    // GN exe
    aConf.gn = mgr->findGN();

    m_ui->advanced->setConfig(aConf);
    updated();
}

void GNNewBuildDir::setStatus(const QString& str, bool validConfig)
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
    pal.setColor(QPalette::WindowText, scheme.foreground(role).color());
    m_ui->l_statusMessage->setPalette(pal);
    m_ui->l_statusMessage->setText(str);

    auto okButton = m_ui->b_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(m_configIsValid);
    if (m_configIsValid) {
        auto cancelButton = m_ui->b_buttonBox->button(QDialogButtonBox::Cancel);
        cancelButton->clearFocus();
    }
}

void GNNewBuildDir::updated()
{
    auto advanced = m_ui->advanced->getConfig();
    Path buildDir = Path(m_ui->i_buildDir->url());
    QFileInfo gnExe(advanced.gn.toLocalFile());

    if (!gnExe.exists() || !gnExe.isExecutable() || !gnExe.isFile()
        || !gnExe.permission(QFileDevice::ReadUser | QFileDevice::ExeUser)) {
        setStatus(i18n("Specified gn executable does not exist"), false);
        return;
    }

    GNBuilder::DirectoryStatus status = GNBuilder::evaluateBuildDirectory(buildDir);
    switch (status) {
    case GNBuilder::CLEAN:
    case GNBuilder::DOES_NOT_EXIST:
        setStatus(i18n("Creating new build directory"), true);
        break;
    case GNBuilder::GN_CONFIGURED:
        setStatus(i18n("Using an already configured build directory"), true);
        break;
    case GNBuilder::GN_FAILED_CONFIGURATION:
        setStatus(i18n("Using a broken gn build directory (this should be fine)"), true);
        break;
    case GNBuilder::INVALID_BUILD_DIR:
        setStatus(i18n("Cannot use specified directory"), false);
        break;
    case GNBuilder::DIR_NOT_EMPTY:
        setStatus(i18n("There are already files in the build directory"), false);
        break;
    case GNBuilder::EMPTY_STRING:
        setStatus(i18n("The build directory field must not be empty"), false);
        break;
    case GNBuilder::___UNDEFINED___:
        setStatus(i18n("You have reached unreachable code. This is a bug"), false);
        break;
    }

// FIXME: Check if the following code is needed
    /*
    bool buildDirChanged = false;
    if (m_oldBuildDir != buildDir.toLocalFile()) {
        m_oldBuildDir = buildDir.toLocalFile();
        buildDirChanged = true;
    }

    bool gnHasChanged = m_ui->advanced->hasGNChanged(); // Outside if to prevent lazy evaluation
    if (gnHasChanged || buildDirChanged) {
        if (status == GNBuilder::GN_CONFIGURED) {
            (new GNImportProjectJob(m_project, buildDir, this))->start();
        } else {
            (new GNImportProjectJob(m_project, advanced.gn, this))->start();
        }
    }
    */
}

GN::BuildDir GNNewBuildDir::currentConfig() const
{
    GN::BuildDir buildDir;
    if (!m_configIsValid) {
        qCDebug(KDEV_GN) << "Cannot generate build dir config from invalid config";
        return buildDir;
    }

    auto advanced = m_ui->advanced->getConfig();

    buildDir.buildDir = Path(m_ui->i_buildDir->url());
    buildDir.gnArgs = advanced.args;
    buildDir.gnExecutable = advanced.gn;

    return buildDir;
}

QStringList GNNewBuildDir::gnArgs() const
{
    // TODO: Save and restore options in/from .kdev4 file
    return {};
}

bool GNNewBuildDir::isConfigValid() const
{
    return m_configIsValid;
}

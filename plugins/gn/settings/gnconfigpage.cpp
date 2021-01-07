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

#include "gnconfigpage.h"

#include "gnbuilder.h"
#include "gnjob.h"
#include "gnmanager.h"
#include "gnnewbuilddir.h"
#include "gnimportprojectjob.h"
#include "ui_gnconfigpage.h"
#include <debug.h>

#include <executecompositejob.h>
#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>

#include <KColorScheme>

#include <QIcon>

using namespace KDevelop;

GNConfigPage::GNConfigPage(IPlugin* plugin, IProject* project, QWidget* parent)
    : ConfigPage(plugin, nullptr, parent)
    , m_project(project)
{
    Q_ASSERT(project); // Catch errors early
    auto* mgr = dynamic_cast<GNManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the GNManager

    m_ui = new Ui::GNConfigPage;
    m_ui->setupUi(this);

    m_config = GN::getGNConfig(m_project);
    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
        reset();
        return;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
    }

    QStringList buildPathList;
    for (auto& i : m_config.buildDirs) {
        buildPathList << i.buildDir.toLocalFile();
    }

    m_ui->i_buildDirs->blockSignals(true);
    m_ui->i_buildDirs->clear();
    m_ui->i_buildDirs->addItems(buildPathList);
    m_ui->i_buildDirs->setCurrentIndex(m_config.currentIndex);
    m_ui->i_buildDirs->blockSignals(false);

    reset();
}

void GNConfigPage::writeConfig()
{
    qCDebug(KDEV_GN) << "Writing config to file";
    if (m_config.currentIndex >= 0) {
        m_config.buildDirs[m_config.currentIndex] = m_current;
    }

    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
    }
    GN::writeGNConfig(m_project, m_config);
}

void GNConfigPage::apply()
{
    qCDebug(KDEV_GN) << "Applying gn config for build dir " << m_current.buildDir;
    readUI();
    writeConfig();

    if (m_config.currentIndex >= 0 && m_configChanged) {
        QList<KJob*> joblist;

        // Check if a configuration is required
        auto status = GNBuilder::evaluateBuildDirectory(m_current.buildDir);
        if (status != GNBuilder::GN_CONFIGURED) {
            joblist << new GNJob(m_current, m_project, nullptr);
        }

        joblist << new GNJob(m_current, m_project, nullptr);
        joblist << new GNImportProjectJob(m_project, m_current, nullptr);
        KJob* job = new ExecuteCompositeJob(nullptr, joblist);
        connect(job, &KJob::result, this, [this]() {
            setDisabled(false);
            updateUI();
        });
        setDisabled(true);
        m_configChanged = false;
        job->start();
    }
}

void GNConfigPage::defaults()
{
    qCDebug(KDEV_GN) << "Restoring build dir " << m_current.buildDir << " to it's default values";
    auto* mgr = dynamic_cast<GNManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr);

    m_current.gnArgs.clear();
    m_current.gnExecutable = mgr->findGN();
    //m_ui->options->resetAll();

    updateUI();
}

void GNConfigPage::reset()
{
    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
        m_ui->i_buildDirs->clear();
        setWidgetsDisabled(true);
        m_ui->b_addDir->setDisabled(false); // Allow adding a new build dir when there are none
        return;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
        m_ui->i_buildDirs->blockSignals(true);
        m_ui->i_buildDirs->setCurrentIndex(m_config.currentIndex);
        m_ui->i_buildDirs->blockSignals(false);
    }

    setWidgetsDisabled(false);
    qCDebug(KDEV_GN) << "Resetting changes for build dir " << m_current.buildDir;

    m_current = m_config.buildDirs[m_config.currentIndex];
    // FIXME: I think we need to run gn first
    (new GNImportProjectJob(m_project, m_current, this))->start();
    updateUI();
}

void GNConfigPage::checkStatus()
{
    // Get the config build dir status
    auto status = GNBuilder::evaluateBuildDirectory(m_current.buildDir);
    auto setStatus = [this](const QString& msg, int color) -> void {
        KColorScheme scheme(QPalette::Normal);
        KColorScheme::ForegroundRole role;
        switch (color) {
        case 0:
            role = KColorScheme::PositiveText;
            break;
        case 1:
            role = KColorScheme::NeutralText;
            break;
        case 2:
        default:
            role = KColorScheme::NegativeText;
            break;
        }

        QPalette pal = m_ui->l_status->palette();
        pal.setColor(QPalette::WindowText, scheme.foreground(role).color());
        m_ui->l_status->setPalette(pal);
        m_ui->l_status->setText(i18n("Status: %1", msg));
    };

    switch (status) {
    case GNBuilder::DOES_NOT_EXIST:
        setStatus(i18n("The current build directory does not exist"), 1);
        break;
    case GNBuilder::CLEAN:
        setStatus(i18n("The current build directory is empty"), 1);
        break;
    case GNBuilder::GN_CONFIGURED:
        setStatus(i18n("Build directory configured"), 0);
        break;
    case GNBuilder::GN_FAILED_CONFIGURATION:
        setStatus(i18n("This gn build directory is not fully configured"), 1);
        break;
    case GNBuilder::INVALID_BUILD_DIR:
        setStatus(i18n("The current build directory is invalid"), 2);
        break;
    case GNBuilder::DIR_NOT_EMPTY:
        setStatus(i18n("This directory does not seem to be a gn build directory"), 2);
        break;
    case GNBuilder::EMPTY_STRING:
        setStatus(i18n("Invalid build directory configuration (empty build directory string)"), 2);
        break;
    case GNBuilder::___UNDEFINED___:
        setStatus(i18n("Something went very wrong. This is a bug"), 2);
        break;
    }
}

void GNConfigPage::updateUI()
{
    auto aConf = m_ui->advanced->getConfig();
    aConf.args = m_current.gnArgs;
    aConf.gn = m_current.gnExecutable;
    m_ui->advanced->setConfig(aConf);

    checkStatus();
}

void GNConfigPage::readUI()
{
    qCDebug(KDEV_GN) << "Reading current build configuration from the UI " << m_current.buildDir.toLocalFile();

    auto aConf = m_ui->advanced->getConfig();
    m_current.gnArgs = aConf.args;
    m_current.gnExecutable = aConf.gn;
}

void GNConfigPage::setWidgetsDisabled(bool disabled)
{
    m_ui->advanced->setDisabled(disabled);
    m_ui->i_buildDirs->setDisabled(disabled);
    m_ui->b_addDir->setDisabled(disabled);
    m_ui->b_rmDir->setDisabled(disabled);
}

void GNConfigPage::addBuildDir()
{
    qCDebug(KDEV_GN) << "Adding build directory";
    auto* mgr = dynamic_cast<GNManager*>(m_project->buildSystemManager());
    auto* bld = dynamic_cast<GNBuilder*>(mgr->builder());
    Q_ASSERT(mgr);
    Q_ASSERT(bld);
    GNNewBuildDir newBD(m_project);

    if (!newBD.exec() || !newBD.isConfigValid()) {
        qCDebug(KDEV_GN) << "Failed to create a new build directory";
        return;
    }

    m_current = newBD.currentConfig();
    m_current.canonicalizePaths();
    m_config.currentIndex = m_config.addBuildDir(m_current);
    m_ui->i_buildDirs->blockSignals(true);
    m_ui->i_buildDirs->addItem(m_current.buildDir.toLocalFile());
    m_ui->i_buildDirs->setCurrentIndex(m_config.currentIndex);
    m_ui->i_buildDirs->blockSignals(false);

    setWidgetsDisabled(true);
    writeConfig();
    // FIXME: ?!?!
    //KJob* job = bld->configure(m_project, m_current, newBD.gnArgs());
    KJob* job = bld->configure(m_project, m_current);
    connect(job, &KJob::result, this, [this]() { reset(); });
    job->start();
}

void GNConfigPage::removeBuildDir()
{
    qCDebug(KDEV_GN) << "Removing current build directory";
    m_ui->i_buildDirs->blockSignals(true);
    m_ui->i_buildDirs->removeItem(m_config.currentIndex);
    m_config.removeBuildDir(m_config.currentIndex);
    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
    }
    m_ui->i_buildDirs->setCurrentIndex(m_config.currentIndex);
    m_ui->i_buildDirs->blockSignals(false);
    reset();
    writeConfig();
}

void GNConfigPage::changeBuildDirIndex(int index)
{
    if (index == m_config.currentIndex || m_config.buildDirs.isEmpty()) {
        return;
    }

    if (index < 0 || index >= m_config.buildDirs.size()) {
        qCWarning(KDEV_GN) << "Invalid build dir index " << index;
        return;
    }

    qCDebug(KDEV_GN) << "Changing build directory to index " << index;

    m_config.currentIndex = index;
    reset();
    writeConfig();
}

void GNConfigPage::emitChanged()
{
    m_configChanged = true;
    checkStatus();
    emit changed();
}

QString GNConfigPage::name() const
{
    return i18nc("@title:tab", "GN");
}

QString GNConfigPage::fullName() const
{
    return i18nc("@title:tab", "GN Project Configuration");
}

QIcon GNConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("run-build"));
}

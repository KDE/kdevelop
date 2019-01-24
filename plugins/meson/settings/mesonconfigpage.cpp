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

#include "mesonconfigpage.h"

#include "mesonbuilder.h"
#include "mesonintrospectjob.h"
#include "mesonjob.h"
#include "mesonmanager.h"
#include "mesonnewbuilddir.h"
#include "ui_mesonconfigpage.h"
#include <debug.h>

#include <executecompositejob.h>
#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>

#include <KColorScheme>

#include <QIcon>


using namespace KDevelop;

MesonConfigPage::MesonConfigPage(IPlugin* plugin, IProject* project, QWidget* parent)
    : ConfigPage(plugin, nullptr, parent)
    , m_project(project)
{
    Q_ASSERT(project); // Catch errors early
    MesonManager* mgr = dynamic_cast<MesonManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr); // This dialog only works with the MesonManager

    m_ui = new Ui::MesonConfigPage;
    m_ui->setupUi(this);
    m_ui->advanced->setSupportedBackends(mgr->supportedMesonBackends());

    m_config = Meson::getMesonConfig(m_project);
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

void MesonConfigPage::writeConfig()
{
    qCDebug(KDEV_Meson) << "Writing config to file";
    if (m_config.currentIndex >= 0) {
        m_config.buildDirs[m_config.currentIndex] = m_current;
    }

    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
    }
    Meson::writeMesonConfig(m_project, m_config);
}

void MesonConfigPage::apply()
{
    qCDebug(KDEV_Meson) << "Applying meson config for build dir " << m_current.buildDir;
    readUI();
    writeConfig();

    if (m_config.currentIndex >= 0 && m_configChanged) {
        QList<KJob*> joblist;

        auto options = m_ui->options->options();
        if (!options) {
            qCWarning(KDEV_Meson) << "Options is nullptr. Can not update meson config";
            return;
        }

        QStringList mesonArgs = options->getMesonArgs();
        if (mesonArgs.empty()) {
            qCDebug(KDEV_Meson) << "Config has not changed --> nothing has to be updated";
            return;
        }

        // Check if a configuration is required
        auto status = MesonBuilder::evaluateBuildDirectory(m_current.buildDir, m_current.mesonBackend);
        if (status != MesonBuilder::MESON_CONFIGURED) {
            joblist << new MesonJob(m_current, m_project, MesonJob::CONFIGURE, mesonArgs, nullptr);
        }

        joblist << new MesonJob(m_current, m_project, MesonJob::SET_CONFIG, mesonArgs, nullptr);
        joblist << m_ui->options->repopulateFromBuildDir(m_project, m_current);
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

void MesonConfigPage::defaults()
{
    qCDebug(KDEV_Meson) << "Restoring build dir " << m_current.buildDir << " to it's default values";
    MesonManager* mgr = dynamic_cast<MesonManager*>(m_project->buildSystemManager());
    Q_ASSERT(mgr);

    m_current.mesonArgs.clear();
    m_current.mesonBackend = mgr->defaultMesonBackend();
    m_current.mesonExecutable = mgr->findMeson();
    m_ui->options->resetAll();

    updateUI();
}

void MesonConfigPage::reset()
{
    if (m_config.buildDirs.isEmpty()) {
        m_config.currentIndex = -1;
        m_ui->i_buildDirs->clear();
        setWidgetsDisabled(true);
        return;
    } else if (m_config.currentIndex < 0 || m_config.currentIndex >= m_config.buildDirs.size()) {
        m_config.currentIndex = 0;
        m_ui->i_buildDirs->blockSignals(true);
        m_ui->i_buildDirs->setCurrentIndex(m_config.currentIndex);
        m_ui->i_buildDirs->blockSignals(false);
    }

    setWidgetsDisabled(false);
    qCDebug(KDEV_Meson) << "Resetting changes for build dir " << m_current.buildDir;

    m_current = m_config.buildDirs[m_config.currentIndex];
    m_ui->options->repopulateFromBuildDir(m_project, m_current)->start();
    updateUI();
}

void MesonConfigPage::checkStatus()
{
    // Get the config build dir status
    auto status = MesonBuilder::evaluateBuildDirectory(m_current.buildDir, m_current.mesonBackend);
    auto setStatus = [this](QString const& msg, int color) -> void {
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
        pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
        m_ui->l_status->setPalette(pal);
        m_ui->l_status->setText(i18n("Status: %1", msg));
    };

    switch (status) {
    case MesonBuilder::DOES_NOT_EXIST:
        setStatus(i18n("The current build directory does not exist"), 1);
        break;
    case MesonBuilder::CLEAN:
        setStatus(i18n("The current build directory is empty"), 1);
        break;
    case MesonBuilder::MESON_CONFIGURED:
        setStatus(i18n("Build directory configured"), 0);
        break;
    case MesonBuilder::MESON_FAILED_CONFIGURATION:
        setStatus(i18n("This meson build directory is not fully configured"), 1);
        break;
    case MesonBuilder::INVALID_BUILD_DIR:
        setStatus(i18n("The current build directory is invalid"), 2);
        break;
    case MesonBuilder::DIR_NOT_EMPTY:
        setStatus(i18n("This directory does not seem to be a meson build directory"), 2);
        break;
    case MesonBuilder::EMPTY_STRING:
        setStatus(i18n("Invalid build directory configuration (empty build directory string)"), 2);
        break;
    case MesonBuilder::___UNDEFINED___:
        setStatus(i18n("Something went very wrong. This is a bug"), 2);
        break;
    }

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;
    int numChanged = 0;
    auto options = m_ui->options->options();

    if (options) {
        numChanged = options->numChanged();
    }

    if (numChanged == 0) {
        role = KColorScheme::NormalText;
        m_ui->l_changed->setText(i18n("No changes"));
    } else {
        role = KColorScheme::NeutralText;
        m_ui->l_changed->setText(i18n("%1 options changed", numChanged));
    }

    QPalette pal = m_ui->l_changed->palette();
    pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
    m_ui->l_changed->setPalette(pal);
}

void MesonConfigPage::updateUI()
{
    auto aConf = m_ui->advanced->getConfig();
    aConf.args = m_current.mesonArgs;
    aConf.backend = m_current.mesonBackend;
    aConf.meson = m_current.mesonExecutable;
    m_ui->advanced->setConfig(aConf);

    checkStatus();
}

void MesonConfigPage::readUI()
{
    qCDebug(KDEV_Meson) << "Reading current build configuration from the UI " << m_current.buildDir.toLocalFile();

    auto aConf = m_ui->advanced->getConfig();
    m_current.mesonArgs = aConf.args;
    m_current.mesonBackend = aConf.backend;
    m_current.mesonExecutable = aConf.meson;
}

void MesonConfigPage::setWidgetsDisabled(bool disabled)
{
    m_ui->advanced->setDisabled(disabled);
    m_ui->i_buildDirs->setDisabled(disabled);
    m_ui->b_addDir->setDisabled(disabled);
    m_ui->b_rmDir->setDisabled(disabled);
    m_ui->options->setDisabled(disabled);
}

void MesonConfigPage::addBuildDir()
{
    qCDebug(KDEV_Meson) << "Adding build directory";
    MesonManager* mgr = dynamic_cast<MesonManager*>(m_project->buildSystemManager());
    MesonBuilder* bld = dynamic_cast<MesonBuilder*>(mgr->builder());
    Q_ASSERT(mgr);
    Q_ASSERT(bld);
    MesonNewBuildDir newBD(m_project);

    if (!newBD.exec() || !newBD.isConfigValid()) {
        qCDebug(KDEV_Meson) << "Failed to create a new build directory";
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
    KJob* job = bld->configure(m_project, m_current, newBD.mesonArgs());
    connect(job, &KJob::result, this, [this]() { reset(); });
    job->start();
}

void MesonConfigPage::removeBuildDir()
{
    qCDebug(KDEV_Meson) << "Removing current build directory";
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

void MesonConfigPage::changeBuildDirIndex(int index)
{
    if (index == m_config.currentIndex || m_config.buildDirs.isEmpty()) {
        return;
    }

    if (index < 0 || index >= m_config.buildDirs.size()) {
        qCWarning(KDEV_Meson) << "Invalid build dir index " << index;
        return;
    }

    qCDebug(KDEV_Meson) << "Changing build directory to index " << index;

    m_config.currentIndex = index;
    reset();
    writeConfig();
}

void MesonConfigPage::emitChanged()
{
    m_configChanged = true;
    checkStatus();
    emit changed();
}

QString MesonConfigPage::name() const
{
    return i18n("Meson");
}

QString MesonConfigPage::fullName() const
{
    return i18n("Meson project configuration");
}

QIcon MesonConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("meson"));
}

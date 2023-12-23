/*
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>
    SPDX-FileCopyrightText: 2011 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakebuilddirchooser.h"
#include "qmakeconfig.h"

#include <KLocalizedString>
#include <KMessageWidget>

#include <QSignalBlocker>
#include <QUrl>

#include <util/path.h>

#include <project/helper.h>
#include <project/interfaces/iprojectbuilder.h>

#include <interfaces/iproject.h>
#include <debug.h>

using namespace KDevelop;

QMakeBuildDirChooser::QMakeBuildDirChooser(KDevelop::IProject* project, QWidget* parent)
    : QWidget(parent)
    , Ui::QMakeBuildDirChooser()
    , m_project(project)
{
    setupUi(this);

    status->hide();
    status->setCloseButtonVisible(false);
    status->setMessageType(KMessageWidget::Error);
    status->setWordWrap(true);

    kcfg_buildDir->setMode(KFile::Directory | KFile::LocalOnly);
    kcfg_installPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    kcfg_qmakeExecutable->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);

    connect(kcfg_qmakeExecutable, &KUrlRequester::textChanged, this, &QMakeBuildDirChooser::changed);
    connect(kcfg_buildDir, &KUrlRequester::textChanged, this, &QMakeBuildDirChooser::changed);
    connect(kcfg_installPrefix, &KUrlRequester::textChanged, this, &QMakeBuildDirChooser::changed);
    connect(kcfg_buildType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QMakeBuildDirChooser::changed);
    connect(kcfg_extraArgs, &KLineEdit::textChanged, this, &QMakeBuildDirChooser::changed);
}

QMakeBuildDirChooser::~QMakeBuildDirChooser()
{
    // don't save in destructor; instead, on click of OK-button
    // saveConfig();
}

QString QMakeBuildDirChooser::errorString() const
{
    return status->text();
}

void QMakeBuildDirChooser::setErrorString(const QString& errorString)
{
    if (errorString.isEmpty()) {
        status->animatedHide();
    } else {
        status->setText(errorString);
        status->animatedShow();
    }
}

IProject* QMakeBuildDirChooser::project() const
{
    return m_project;
}

void QMakeBuildDirChooser::saveConfig()
{
    KConfigGroup config = KConfigGroup(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP()).group(buildDir());
    saveConfig(config);
}

void QMakeBuildDirChooser::saveConfig(KConfigGroup& config)
{
    qCDebug(KDEV_QMAKE) << "Writing config for" << buildDir() << "to config" << config.name();

    config.writeEntry(QMakeConfig::QMAKE_EXECUTABLE, qmakeExecutable());
    config.writeEntry(QMakeConfig::INSTALL_PREFIX, installPrefix());
    config.writeEntry(QMakeConfig::EXTRA_ARGUMENTS, extraArgs());
    config.writeEntry(QMakeConfig::BUILD_TYPE, buildType());
    config.sync();
}

void QMakeBuildDirChooser::loadConfig()
{
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
    loadConfig(cg.readEntry(QMakeConfig::BUILD_FOLDER, KDevelop::proposedBuildFolder(m_project->path()).toLocalFile()));
}

void QMakeBuildDirChooser::loadConfig(const QString& config)
{
    // get correct group
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
    const KConfigGroup build = cg.group(config);

    {
        QSignalBlocker blocker(this); // only emit changed once

        // sets values into fields
        setQMakeExecutable(QMakeConfig::qmakeExecutable(m_project));
        setBuildDir(config);
        setInstallPrefix(build.readEntry(QMakeConfig::INSTALL_PREFIX, QString()));
        setExtraArgs(build.readEntry(QMakeConfig::EXTRA_ARGUMENTS, QString()));
        setBuildType(build.readEntry(QMakeConfig::BUILD_TYPE, 0));
    }
    emit changed();
}

bool QMakeBuildDirChooser::validate(QString* message)
{
    bool valid = true;
    QString msg;
    if (qmakeExecutable().isEmpty()) {
        msg = i18n("Please specify path to QMake executable.");
        valid = false;
    } else {
        QFileInfo info(qmakeExecutable());
        if (!info.exists()) {
            msg = i18n("QMake executable \"%1\" does not exist.", qmakeExecutable());
            valid = false;
        } else if (!info.isFile()) {
            msg = i18n("QMake executable is not a file.");
            valid = false;
        } else if (!info.isExecutable()) {
            msg = i18n("QMake executable is not executable.");
            valid = false;
        } else {
            const QHash<QString, QString> vars = QMakeConfig::queryQMake(info.absoluteFilePath());
            if (vars.isEmpty()) {
                msg = i18n("QMake executable cannot be queried for variables.");
                valid = false;
            } else if (QMakeConfig::findBasicMkSpec(vars).isEmpty()) {
                msg = i18n("No basic MkSpec file could be found for the given QMake executable.");
                valid = false;
            }
        }
    }

    if (buildDir().isEmpty()) {
        msg = i18n("Please specify a build folder.");
        valid = false;
    }
    if (message) {
        *message = msg;
    }
    setErrorString(msg);
    qCDebug(KDEV_QMAKE) << "VALID == " << valid;
    return valid;
}

QString QMakeBuildDirChooser::qmakeExecutable() const
{
    return kcfg_qmakeExecutable->url().toLocalFile();
}

QString QMakeBuildDirChooser::buildDir() const
{
    return kcfg_buildDir->url().toLocalFile();
}

QString QMakeBuildDirChooser::installPrefix() const
{
    return kcfg_installPrefix->url().toLocalFile();
}

int QMakeBuildDirChooser::buildType() const
{
    return kcfg_buildType->currentIndex();
}

QString QMakeBuildDirChooser::extraArgs() const
{
    return kcfg_extraArgs->text();
}

void QMakeBuildDirChooser::setQMakeExecutable(const QString& executable)
{
    kcfg_qmakeExecutable->setUrl(QUrl::fromLocalFile(executable));
}

void QMakeBuildDirChooser::setBuildDir(const QString& buildDir)
{
    kcfg_buildDir->setUrl(QUrl::fromLocalFile(buildDir));
}

void QMakeBuildDirChooser::setInstallPrefix(const QString& prefix)
{
    kcfg_installPrefix->setUrl(QUrl::fromLocalFile(prefix));
}

void QMakeBuildDirChooser::setBuildType(int type)
{
    kcfg_buildType->setCurrentIndex(type);
}

void QMakeBuildDirChooser::setExtraArgs(const QString& args)
{
    kcfg_extraArgs->setText(args);
}

#include "moc_qmakebuilddirchooser.cpp"

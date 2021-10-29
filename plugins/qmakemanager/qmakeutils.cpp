/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "qmakeutils.h"
#include <debug.h>

#include "qmakebuilddirchooserdialog.h"
#include "qmakeconfig.h"

#include <interfaces/iproject.h>
#include <util/path.h>

#include <QFile>
#include <QPointer>
#include <QString>

using namespace KDevelop;

bool QMakeUtils::checkForNeedingConfigure(IProject* project)
{
    Q_ASSERT(project);

    qCDebug(KDEV_QMAKE) << "Checking whether" << project->name() << "needs a configure run";

    const auto buildDir = QMakeConfig::buildDirFromSrc(project, project->path());
    if (!buildDir.isValid()) {
        QPointer<QMakeBuildDirChooserDialog> chooser = new QMakeBuildDirChooserDialog(project);
        if (chooser->exec() == QDialog::Rejected) {
            delete chooser;
            return false; // cancelled, can't configure => false
        }
        delete chooser;
    }

    qCDebug(KDEV_QMAKE) << "Build directory for" << project->name() << "is" << buildDir;

    if (!QMakeConfig::isConfigured(project)) {
        return true;
    }
    const QString qmakeExecutable = QMakeConfig::qmakeExecutable(project);
    if (qmakeExecutable.isEmpty()) {
        return true;
    }
    const QHash<QString, QString> vars = queryQMake(project);
    if (vars.isEmpty()) {
        return true;
    }
    if (QMakeConfig::findBasicMkSpec(vars).isEmpty()) {
        return true;
    }

    if (!QFile::exists(buildDir.toLocalFile())) {
        qCDebug(KDEV_QMAKE) << "build dir" << buildDir << "configured, but does not exist yet";
        return true;
    }

    qCDebug(KDEV_QMAKE) << "No configure needed for project" << project->name();
    return false;
}

QHash<QString, QString> QMakeUtils::queryQMake(IProject* project)
{
    if (!project->path().toUrl().isLocalFile())
        return QHash<QString, QString>();

    return QMakeConfig::queryQMake(QMakeConfig::qmakeExecutable(project));
}

/*
 * Copyright 2015 Kevin Funk <kfunk@kde.org>
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

#include "qmakeutils.h"
#include "debug.h"

#include "qmakebuilddirchooserdialog.h"
#include "qmakeconfig.h"

#include <interfaces/iproject.h>
#include <util/path.h>

#include <QFile>
#include <QString>

using namespace KDevelop;

bool QMakeUtils::checkForNeedingConfigure(IProject* project)
{
    Q_ASSERT(project);

    qCDebug(KDEV_QMAKE) << "Checking whether" << project->name() << "needs a configure run";

    const auto buildDir = QMakeConfig::buildDirFromSrc(project, project->path());
    if (!buildDir.isValid()) {
        QMakeBuildDirChooserDialog chooser(project);
        if (chooser.exec() == QDialog::Rejected) {
            return false; // cancelled, can't configure => false
        }
    }

    qCDebug(KDEV_QMAKE) << "Build directory for" << project->name() << "is" << buildDir;

    if (!QMakeConfig::isConfigured(project)) {
        return true;
    }
    const QString qmakeBinary = QMakeConfig::qmakeBinary(project);
    if (qmakeBinary.isEmpty()) {
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

    return QMakeConfig::queryQMake(QMakeConfig::qmakeBinary(project));
}

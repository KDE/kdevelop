/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "gnconfig.h"

#include "gnmanager.h"
#include <debug.h>

#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KLocalizedString>

#include <QFileDialog>

using namespace KDevelop;
using namespace GN;

static const QString ROOT_CONFIG = QStringLiteral("GNManager");
static const QString NUM_BUILD_DIRS = QStringLiteral("Number of Build Directories");
static const QString CURRENT_INDEX = QStringLiteral("Current Build Directory Index");

static const QString BUILD_DIR_SEC = QStringLiteral("BuildDir %1");
static const QString BUILD_DIR_PATH = QStringLiteral("Build Directory Path");
static const QString GN_EXE = QStringLiteral("GN executable");
static const QString EXTRA_ARGS = QStringLiteral("Additional gn arguments");

int GNConfig::addBuildDir(BuildDir dir)
{
    int newIndex = buildDirs.size();
    dir.canonicalizePaths();
    qCDebug(KDEV_GN) << "BuildDirectories::addBuildDir()=" << dir.buildDir;
    buildDirs.push_back(dir);

    // Make sure m_currentIndex is valid
    if (currentIndex < 0) {
        currentIndex = newIndex;
    }

    return newIndex;
}

bool GNConfig::removeBuildDir(int index)
{
    if (index > buildDirs.size() || index < 0) {
        return false;
    }

    buildDirs.removeAt(index);

    if (currentIndex >= buildDirs.size()) {
        currentIndex = buildDirs.size() - 1;
    }

    return true;
}

KConfigGroup GN::rootGroup(IProject* project)
{
    if (!project) {
        qCWarning(KDEV_GN) << "GN::rootGroup: IProject pointer is nullptr";
        return KConfigGroup();
    }

    return project->projectConfiguration()->group(ROOT_CONFIG);
}

GNConfig GN::getGNConfig(IProject* project)
{
    KConfigGroup root = rootGroup(project);
    GNConfig result;

    int numDirs = root.readEntry(NUM_BUILD_DIRS, 0);
    result.currentIndex = root.readEntry(CURRENT_INDEX, -1);

    for (int i = 0; i < numDirs; ++i) {
        QString section = BUILD_DIR_SEC.arg(i);
        if (!root.hasGroup(section)) {
            continue;
        }

        KConfigGroup current = root.group(section);
        BuildDir currBD;
        currBD.buildDir = Path(current.readEntry(BUILD_DIR_PATH, QString()));
        currBD.gnExecutable = Path(current.readEntry(GN_EXE, QString()));
        currBD.gnArgs = current.readEntry(EXTRA_ARGS, QString());

        currBD.canonicalizePaths();

        // Try to find gn if the config is bad
        if (currBD.gnExecutable.isEmpty()) {
            Q_ASSERT(project);
            IBuildSystemManager* ibsm = project->buildSystemManager();
            auto* bsm = dynamic_cast<GNManager*>(ibsm);
            if (bsm) {
                currBD.gnExecutable = bsm->findGN();
            }
        }

        result.buildDirs.push_back(currBD);
    }

    if (result.buildDirs.isEmpty()) {
        result.currentIndex = -1;
    } else if (result.currentIndex < 0 || result.currentIndex >= result.buildDirs.size()) {
        result.currentIndex = 0;
    }

    return result;
}

void GN::writeGNConfig(IProject* project, const GNConfig& cfg)
{
    KConfigGroup root = rootGroup(project);

    // Make sure that the config we write is valid
    int currentIndex = cfg.currentIndex;
    if (cfg.buildDirs.isEmpty()) {
        currentIndex = -1;
    } else if (currentIndex < 0 || currentIndex >= cfg.buildDirs.size()) {
        currentIndex = 0;
    }

    root.writeEntry(NUM_BUILD_DIRS, cfg.buildDirs.size());
    root.writeEntry(CURRENT_INDEX, currentIndex);

    int counter = 0;
    for (const auto& i : cfg.buildDirs) {
        KConfigGroup current = root.group(BUILD_DIR_SEC.arg(counter++));

        current.writeEntry(BUILD_DIR_PATH, i.buildDir.path());
        current.writeEntry(GN_EXE, i.gnExecutable.path());
        current.writeEntry(EXTRA_ARGS, i.gnArgs);
    }
}

BuildDir GN::currentBuildDir(IProject* project)
{
    Q_ASSERT(project);
    GNConfig cfg = getGNConfig(project);
    if (cfg.currentIndex < 0 || cfg.currentIndex >= cfg.buildDirs.size()) {
        cfg.currentIndex = 0; // Default to the first build dir

        // Return an invalid build dir
        if (cfg.buildDirs.isEmpty()) {
            return BuildDir();
        }
    }

    return cfg.buildDirs[cfg.currentIndex];
}

bool GN::BuildDir::isValid() const
{
    return !(buildDir.isEmpty() || gnExecutable.isEmpty());
}

void GN::BuildDir::canonicalizePaths()
{
    for (auto* i : { &buildDir, &gnExecutable }) {
        // canonicalFilePath checks if the file / directory exists and returns "" if it doesn't.
        QString tmp = QFileInfo(i->toLocalFile()).canonicalFilePath();
        if (!tmp.isEmpty()) {
            *i = Path(tmp);
        }
    }
}

QString GN::absoluteFilePath(const QString &rootPath, const QString &path)
{
    if (path.size() < 2 || (path.startsWith(QLatin1Char('/')) && !path.startsWith(QLatin1String("//")))) {
        return path;
    }
    return QFileInfo{rootPath + path.mid(1)}.absoluteFilePath();
}

QString GN::relativeFilePath(const QString &path)
{
    if (path.size() < 2 || (path.startsWith(QLatin1Char('/')) && !path.startsWith(QLatin1String("//")))) {
        return path;
    }
    return path.mid(2);
}

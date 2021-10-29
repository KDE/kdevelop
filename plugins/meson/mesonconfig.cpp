/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonconfig.h"

#include "mesonmanager.h"
#include <debug.h>

#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KLocalizedString>

#include <QFileDialog>

using namespace KDevelop;
using namespace Meson;

static const QString ROOT_CONFIG = QStringLiteral("MesonManager");
static const QString NUM_BUILD_DIRS = QStringLiteral("Number of Build Directories");
static const QString CURRENT_INDEX = QStringLiteral("Current Build Directory Index");

static const QString BUILD_DIR_SEC = QStringLiteral("BuildDir %1");
static const QString BUILD_DIR_PATH = QStringLiteral("Build Directory Path");
static const QString MESON_EXE = QStringLiteral("Meson executable");
static const QString EXTRA_ARGS = QStringLiteral("Additional meson arguments");
static const QString BACKEND = QStringLiteral("Meson Generator Backend");

int MesonConfig::addBuildDir(BuildDir dir)
{
    int newIndex = buildDirs.size();
    dir.canonicalizePaths();
    qCDebug(KDEV_Meson) << "BuildDirectories::addBuildDir()=" << dir.buildDir;
    buildDirs.push_back(dir);

    // Make sure m_currentIndex is valid
    if (currentIndex < 0) {
        currentIndex = newIndex;
    }

    return newIndex;
}

bool MesonConfig::removeBuildDir(int index)
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

KConfigGroup Meson::rootGroup(IProject* project)
{
    if (!project) {
        qCWarning(KDEV_Meson) << "Meson::rootGroup: IProject pointer is nullptr";
        return KConfigGroup();
    }

    return project->projectConfiguration()->group(ROOT_CONFIG);
}

MesonConfig Meson::getMesonConfig(IProject* project)
{
    KConfigGroup root = rootGroup(project);
    MesonConfig result;

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
        currBD.mesonExecutable = Path(current.readEntry(MESON_EXE, QString()));
        currBD.mesonBackend = current.readEntry(BACKEND, QString());
        currBD.mesonArgs = current.readEntry(EXTRA_ARGS, QString());

        currBD.canonicalizePaths();

        // Try to find meson if the config is bad
        if (currBD.mesonExecutable.isEmpty()) {
            Q_ASSERT(project);
            IBuildSystemManager* ibsm = project->buildSystemManager();
            auto* bsm = dynamic_cast<MesonManager*>(ibsm);
            if (bsm) {
                currBD.mesonExecutable = bsm->findMeson();
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

void Meson::writeMesonConfig(IProject* project, const MesonConfig& cfg)
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
        current.writeEntry(MESON_EXE, i.mesonExecutable.path());
        current.writeEntry(BACKEND, i.mesonBackend);
        current.writeEntry(EXTRA_ARGS, i.mesonArgs);
    }
}

BuildDir Meson::currentBuildDir(IProject* project)
{
    Q_ASSERT(project);
    MesonConfig cfg = getMesonConfig(project);
    if (cfg.currentIndex < 0 || cfg.currentIndex >= cfg.buildDirs.size()) {
        cfg.currentIndex = 0; // Default to the first build dir

        // Return an invalid build dir
        if (cfg.buildDirs.isEmpty()) {
            return BuildDir();
        }
    }

    return cfg.buildDirs[cfg.currentIndex];
}

bool Meson::BuildDir::isValid() const
{
    return !(buildDir.isEmpty() || mesonExecutable.isEmpty());
}

void Meson::BuildDir::canonicalizePaths()
{
    for (auto* i : { &buildDir, &mesonExecutable }) {
        // canonicalFilePath checks if the file / directory exists and returns "" if it doesn't.
        QString tmp = QFileInfo(i->toLocalFile()).canonicalFilePath();
        if (!tmp.isEmpty()) {
            *i = Path(tmp);
        }
    }
}

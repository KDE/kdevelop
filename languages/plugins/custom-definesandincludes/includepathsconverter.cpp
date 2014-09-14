/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "includepathsconverter.h"

#include <KSharedConfig>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDirIterator>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>

#include "settingsmanager.h"

using namespace KDevelop;

namespace {
KSharedConfig::Ptr openConfigFile(const QString& configFile)
{
    return KSharedConfig::openConfig(configFile, KConfig::SimpleConfig);
}

QString findconfigFile(const QString& projectDir)
{
    QDirIterator dirIterator(projectDir + '/' + ".kdev4");
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (dirIterator.fileName().endsWith(".kdev4")) {
            return dirIterator.fileInfo().canonicalFilePath();
        }
    }
    return {};
}

QString findProject(const QString& subdirectory)
{
    QDir project(subdirectory);
    do {
        if (project.exists(".kdev4")) {
            return project.path();
        }
    } while(project.cdUp());

    return {};
}
}

IncludePathsConverter::IncludePathsConverter()
: m_settings(new SettingsManager())
{
}

bool IncludePathsConverter::addIncludePaths(const QStringList& includeDirectories, const QString& projectConfigFile, const QString& subdirectory)
{
    auto configFile = openConfigFile(projectConfigFile);
    if (!configFile) {
        return false;
    }

    auto configEntries = m_settings->readPaths(configFile.data());
    QString path = subdirectory.isEmpty() ? "." : subdirectory;

    ConfigEntry config;
    for (auto& entry: configEntries) {
        if (path == entry.path) {
            config = entry;
            config.includes += includeDirectories;
            config.includes.removeDuplicates();
            entry = config;
            break;
        }
    }

    if (config.path.isEmpty()) {
        config.path = path;
        config.includes = includeDirectories;
        configEntries.append(config);
    }

    m_settings->writePaths(configFile.data(), configEntries);

    return true;
}

bool IncludePathsConverter::removeIncludePaths(const QStringList& includeDirectories, const QString& projectConfigFile, const QString& subdirectory)
{
    auto configFile = openConfigFile(projectConfigFile);
    if (!configFile) {
        return false;
    }

    auto configEntries = m_settings->readPaths(configFile.data());
    QString path = subdirectory.isEmpty() ? "." : subdirectory;

    for (auto& entry: configEntries) {
        if (path == entry.path) {
            for(const auto& include: includeDirectories) {
                entry.includes.removeAll(include);
            }
            m_settings->writePaths(configFile.data(), configEntries);
            return true;
        }
    }

    return true;
}

QStringList IncludePathsConverter::readIncludePaths(const QString& projectConfigFile, const QString& subdirectory) const
{
    auto configFile = openConfigFile(projectConfigFile);
    if (!configFile) {
        return  {};
    }

    QString path = subdirectory.isEmpty() ? "." : subdirectory;
    auto configEntries = m_settings->readPaths(configFile.data());
    for (const auto& entry: configEntries) {
        if (path == entry.path) {
            return entry.includes;
        }
    }

    return {};
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kdev_includepathsconverter");

    QCommandLineParser parser;
    parser.setApplicationDescription("\nAdds, removes or shows include directories of a project. Also it can be used as a tool to convert include directories from .kdev_include_paths file to the new format.\n\n"
    "Examples:\ncat /project/path/.kdev_include_paths | xargs -d '\\n' kdev_includepathsconverter -a /project/path/\n\n"
    "kdev_includepathsconverter -r /project/path/subdirectory/ \"/some/include/dir\" \"/another/include/dir\" \n\n"
    "kdev_includepathsconverter -l /project/path/another/subdirectory/");
    parser.addHelpOption();

    QCommandLineOption listOption("l", QCoreApplication::translate("main", "Shows include directories used by the project"),
                                     QCoreApplication::translate("main", "project"));
    parser.addOption(listOption);
    QCommandLineOption addOption("a", QCoreApplication::translate("main", "Adds include directories to the project"),
                                     QCoreApplication::translate("main", "project"));
    parser.addOption(addOption);
    QCommandLineOption removeOption("r", QCoreApplication::translate("main", "Removes include directories from the project"),
                                     QCoreApplication::translate("main", "project"));
    parser.addOption(removeOption);

    parser.process(app);

    QString projectDir;
    QStringList includeDirectories(parser.positionalArguments());

    std::transform(includeDirectories.begin(), includeDirectories.end(), includeDirectories.begin(), [](const QString& path) {
        return path.trimmed();
    } );
    includeDirectories.erase(std::remove_if(includeDirectories.begin(), includeDirectories.end(), [](const QString& path) {
        return path.isEmpty();
    } ), includeDirectories.end());

    bool show = parser.isSet(listOption);
    bool add = parser.isSet(addOption);
    bool remove = parser.isSet(removeOption);
    if (show) {
        projectDir = parser.value(listOption);
    } else if(add) {
        projectDir = parser.value(addOption);
    } else if(remove) {
        projectDir = parser.value(removeOption);
    }

    if (projectDir.isEmpty()) {
        parser.showHelp(-1);
    }

    QString subdirectory = projectDir;
    projectDir = findProject(projectDir);

    QString configFile = findconfigFile(projectDir);

    QTextStream out(stdout);

    if (configFile.isEmpty()) {
        out << QCoreApplication::translate("main", "No project found for: ") << subdirectory;
        return -1;
    }

    if (add || remove) {
        if (includeDirectories.isEmpty()) {
            parser.showHelp(-1);
        }
    }

    {
        auto subdirCanonical = QFileInfo(subdirectory).canonicalFilePath();
        auto projectCanonical = QFileInfo(projectDir).canonicalFilePath();
        if (subdirCanonical != projectCanonical) {
            subdirectory = subdirCanonical.mid(projectCanonical.size());
            if (subdirectory.startsWith('/')) {
                subdirectory.remove(0,1);
            }
        } else {
            subdirectory.clear();
        }
    }

    IncludePathsConverter converter;

    if (remove) {
        if (!converter.removeIncludePaths(includeDirectories, configFile, subdirectory)) {
            out << QCoreApplication::translate("main", "Can't remove include paths");
        }
    }

    if (add) {
        if (!converter.addIncludePaths(includeDirectories, configFile, subdirectory)) {
            out << QCoreApplication::translate("main", "Can't add include paths");
        }
    }

    if (show) {
        for (const auto& include: converter.readIncludePaths(configFile, subdirectory)) {
            out << include << "\n";
        }
    }
}

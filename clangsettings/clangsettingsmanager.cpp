/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include "clangsettingsmanager.h"

#include <KConfigGroup>
#include <KConfig>

#include <QThread>
#include <QCoreApplication>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iproject.h>
#include <util/path.h>

#include <project/projectmodel.h>

using namespace KDevelop;

namespace
{
    const QString settingsGroup = QStringLiteral("Clang Settings");

    const QString parserGroup = QStringLiteral("group");
    const QString parserPath = QStringLiteral("path");
    const QString parserOptions = QStringLiteral("options");

    const QString overrideHelper = QStringLiteral("overrideHelper");
    const QString implementHelper = QStringLiteral("implementHelper");
    const QString macros = QStringLiteral("macros");

    const QString forwardDeclare = QStringLiteral("forwardDeclare");
    const QString includePath = QStringLiteral("includePath");
    const QString adaptSignature = QStringLiteral("adaptSignature");

AssistantsSettings readAssistantsSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    AssistantsSettings settings;

    settings.adaptSignature = grp.readEntry(adaptSignature, true);
    settings.forwardDeclare = grp.readEntry(forwardDeclare, true);
    settings.includePath = grp.readEntry(includePath, true);

    return settings;
}

CodeCompletionSettings readCodeCompletionSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    CodeCompletionSettings settings;

    settings.implementHelper = grp.readEntry(implementHelper, true);
    settings.macros = grp.readEntry(macros, true);
    settings.overrideHelper = grp.readEntry(overrideHelper, true);

    return settings;
}

ParserSettings parserOptionsForItem(const QList<ParserSettingsEntry>& paths, const Path itemPath, const Path /*rootDirectory*/)
{
    Path closestPath;
    ParserSettings settings;

    // find parser options configured to a path closest to the requested item
    for (const auto& entry : paths) {
        auto settingsEntry = entry;
        // TODO: store paths without project path
        Path targetDirectory = Path(entry.path);//rootDirectory;

        //targetDirectory.addPath(entry.path);

        if (targetDirectory == itemPath){
            return settingsEntry.settings;
        }

        if (targetDirectory.isParentOf(itemPath)) {
            if(settings.parserOptions.isEmpty() || targetDirectory.segments().size() > closestPath.segments().size()){
                settings = settingsEntry.settings;
                closestPath = targetDirectory;
            }
        }
    }

    return settings;
}

}

ClangSettingsManager* ClangSettingsManager::self()
{
    static ClangSettingsManager manager;
    return &manager;
}

AssistantsSettings ClangSettingsManager::assistantsSettings() const
{
    auto cfg = ICore::self()->activeSession()->config();
    return readAssistantsSettings(cfg.data());
}

CodeCompletionSettings ClangSettingsManager::codeCompletionSettings() const
{
    auto cfg = ICore::self()->activeSession()->config();
    return readCodeCompletionSettings(cfg.data());
}

ParserSettings ClangSettingsManager::parserSettings(KDevelop::ProjectBaseItem* item) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        // TODO: default value;
        return {};
    }

   return parserOptionsForItem(readPaths(item->project()), item->path(), item->project()->path());
}

ClangSettingsManager::ClangSettingsManager()
{}

ParserSettings ClangSettingsManager::parserSettings(const QString& item, KDevelop::IProject* project) const
{
    const Path itemPath(item);
    const Path rootDirectory = project->path();
    return parserOptionsForItem(readPaths(project), itemPath, rootDirectory);
}

QList<ParserSettingsEntry> ClangSettingsManager::readPaths(KDevelop::IProject* project) const
{
    auto cfg = project->projectConfiguration().data();
    auto grp = cfg->group(settingsGroup);

    QList<ParserSettingsEntry> paths;
    for (const auto& grpName : grp.groupList()) {
        if (!grpName.startsWith(parserGroup)) {
            continue;
        }
        KConfigGroup pathgrp = grp.group(grpName);

        ParserSettingsEntry path;
        path.path = pathgrp.readEntry(parserPath, "");
        if(path.path.isEmpty()){
            continue;
        }
        // TODO: default value
        path.settings.parserOptions = pathgrp.readEntry(parserOptions, "");
        paths.append(path);
    }

    return paths;
}

void ClangSettingsManager::writePaths(KDevelop::IProject* project, const QList<ParserSettingsEntry>& paths)
{
    auto cfg = project->projectConfiguration().data();
    auto grp = cfg->group(settingsGroup);
    grp.deleteGroup();

    int pathIndex = 0;
    for (const auto& path : paths) {
        auto pathgrp = grp.group(parserGroup + QString::number(pathIndex++));
        pathgrp.writeEntry(parserPath, path.path);
        pathgrp.writeEntry(parserOptions, path.settings.parserOptions);
    }
}

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

    const QString macros = QStringLiteral("macros");

    const QString forwardDeclare = QStringLiteral("forwardDeclare");

AssistantsSettings readAssistantsSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    AssistantsSettings settings;

    settings.forwardDeclare = grp.readEntry(forwardDeclare, true);

    return settings;
}

CodeCompletionSettings readCodeCompletionSettings(KConfig* cfg)
{
    auto grp = cfg->group(settingsGroup);
    CodeCompletionSettings settings;

    settings.macros = grp.readEntry(macros, true);

    return settings;
}

ParserSettings parserOptionsForItem(const QVector<ParserSettingsEntry>& paths, const Path itemPath, const Path rootDirectory)
{
    Path closestPath;
    ParserSettings settings;

    // find parser options configured to a path closest to the requested item
    for (const auto& entry : paths) {
        auto settingsEntry = entry;
        Path targetDirectory = rootDirectory;

        targetDirectory.addPath(entry.path);

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

    return settings.parserOptions.isEmpty() ? ClangSettingsManager::self()->defaultParserSettings() : settings;
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
        return defaultParserSettings();
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

QVector<ParserSettingsEntry> ClangSettingsManager::readPaths(KDevelop::IProject* project) const
{
    auto cfg = project->projectConfiguration().data();
    auto grp = cfg->group(settingsGroup);

    QVector<ParserSettingsEntry> paths;
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

        path.settings.parserOptions = pathgrp.readEntry(parserOptions, defaultParserSettings().parserOptions);
        paths.append(path);
    }

    return paths;
}

void ClangSettingsManager::writePaths(KDevelop::IProject* project, const QVector<ParserSettingsEntry>& paths)
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

ParserSettings ClangSettingsManager::defaultParserSettings() const
{
    return {QStringLiteral("-fspell-checking -Wdocumentation -std=c++11 -Wall")};
}

bool ParserSettings::isCpp() const
{
    return parserOptions.contains(QStringLiteral("-std=c++"));
}

QVector<QByteArray> ParserSettings::toClangAPI() const
{
    // TODO: This is not efficient.
    auto list = parserOptions.split(QLatin1Char(' '), QString::SkipEmptyParts);
    QVector<QByteArray> result;
    result.reserve(list.size());

    std::transform(list.constBegin(), list.constEnd(),
                   std::back_inserter(result),
                   [] (const QString &argument) { return argument.toUtf8(); });

    return result;
}

bool ParserSettings::operator==(const ParserSettings& rhs) const
{
    return parserOptions == rhs.parserOptions;
}

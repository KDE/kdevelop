/*
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>
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

#include "customdefinesandincludesmanager.h"

#include "configentry.h"

#include "settingsmanager.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <QList>

using KDevelop::IProject;
using KDevelop::ProjectBaseItem;

class CustomDefinesAndIncludesManager::ManagerPrivate
{
public:
    ///@return: The best fitted ConfigEntry, based on path of @p item
    ConfigEntry findConfigForItem(const QList<ConfigEntry>& paths, const ProjectBaseItem* item) const {
        ConfigEntry candidateEntry;
        KUrl candidateTargetDirectory;

        auto itemPath = item->path();
        KUrl rootDirectory = item->project()->folder();

        foreach (const ConfigEntry& entry, paths) {
            KUrl targetDirectory = rootDirectory;
            // note: a dot represents the project root
            if (entry.path != ".") {
                targetDirectory.addPath(entry.path);
            }

            //TODO: maybe merge all parent includes too?
            if (targetDirectory.isParentOf(itemPath.toUrl())) {
                if (candidateTargetDirectory.isEmpty() || candidateTargetDirectory.isParentOf(targetDirectory)) {
                    candidateEntry = entry;
                    candidateTargetDirectory = targetDirectory;
                }
            }
        }
        return candidateEntry;
    }

    Path::List includeDirectories(const ProjectBaseItem* item) const {
        if (!item) {
            return {};
        }

        KConfig* cfg = item->project()->projectConfiguration().data();

        return KDevelop::toPathList(findConfigForItem(SettingsManager::self()->readSettings(cfg), item).includes);
    }

    QHash< QString, QString > defines(const ProjectBaseItem* item) const {
        if (!item) {
            return {};
        }

        KConfig* cfg = item->project()->projectConfiguration().data();

        const auto result = findConfigForItem(SettingsManager::self()->readSettings(cfg), item).defines;
        QHash<QString, QString> defines;
        for (auto it = result.constBegin(); it != result.constEnd(); it++) {
            defines[it.key()] = it.value().toString();
        }
        return defines;
    }
};

CustomDefinesAndIncludesManager::CustomDefinesAndIncludesManager(): d(new ManagerPrivate())
{}

QHash< QString, QString > CustomDefinesAndIncludesManager::defines(const ProjectBaseItem* item) const
{
    return d->defines(item);
}

Path::List CustomDefinesAndIncludesManager::includes(const ProjectBaseItem* item) const
{
    return d->includeDirectories(item);
}

CustomDefinesAndIncludesManager* CustomDefinesAndIncludesManager::self()
{
    static CustomDefinesAndIncludesManager instance;
    return &instance;
}

CustomDefinesAndIncludesManager::~CustomDefinesAndIncludesManager()
{}
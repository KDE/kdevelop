/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef KDEVPLATFORM_PLUGIN_MESONMANAGER_H
#define KDEVPLATFORM_PLUGIN_MESONMANAGER_H

#include <project/abstractfilemanagerplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>

class MesonBuilder;

class MesonManager: public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )

public:
    explicit MesonManager( QObject* parent = nullptr, const QVariantList& args = QVariantList() );

    KDevelop::IProjectBuilder* builder() const override;

    KJob * createImportJob(KDevelop::ProjectFolderItem * item) override;
    void setProjectData(KDevelop::IProject* project, const QJsonObject &data);

    // FIXME now: should use compile_commands.json for these (i.e. m_projects)
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem*) const override { return {}; }
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem*) const override { return {}; }
    QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const override { return {}; }
    QString extraArguments(KDevelop::ProjectBaseItem* /*item*/) const override { return {}; }
    bool hasBuildInfo(KDevelop::ProjectBaseItem* /*item*/) const override { return false; }

    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;

    // fill if&when we have targets
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override { return {}; }

    // you can ignore these for now I guess, but TODO
    KDevelop::ProjectTargetItem* createTarget(const QString& /*target*/, KDevelop::ProjectFolderItem */*parent*/) override { return nullptr; }
    bool removeTarget(KDevelop::ProjectTargetItem */*target*/) override { return false; }
    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*> &/*files*/, KDevelop::ProjectTargetItem */*target*/) override { return false; }
    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*> &/*files*/) override { return false; }

private:
    MesonBuilder* m_builder;
    QHash<KDevelop::IProject*, QJsonObject> m_projects;
};

#endif

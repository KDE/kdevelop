/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef CUSTOMBUILDSYSTEMPLUGIN_H
#define CUSTOMBUILDSYSTEMPLUGIN_H

#include <interfaces/iplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/abstractfilemanagerplugin.h>

class KConfigGroup;
class KDialogBase;

namespace KDevelop
{
class ProjectBaseItem;
class IOutputView;
class IProject;
}

class CustomBuildSystem : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IProjectBuilder, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IBuildSystemManager )
public:
    explicit CustomBuildSystem( QObject *parent = 0, const QVariantList &args = QVariantList() );
    virtual ~CustomBuildSystem();

// ProjectBuilder API
    KJob* build( KDevelop::ProjectBaseItem* dom );
    KJob* clean( KDevelop::ProjectBaseItem* dom );
    KJob* prune( KDevelop::IProject* );
    KJob* install( KDevelop::ProjectBaseItem* item );
    KJob* configure( KDevelop::IProject* );
signals:
    void built( KDevelop::ProjectBaseItem *dom );
    void installed( KDevelop::ProjectBaseItem* );
    void cleaned( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem *dom );
    void configured( KDevelop::IProject* );
    void pruned( KDevelop::IProject* );

// AbstractFileManagerPlugin API
public:
    Features features() const;
    virtual KDevelop::ProjectFolderItem* createFolderItem( KDevelop::IProject* project, 
                    const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent = 0 );

// BuildSystemManager API
public:
    bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*>& file, KDevelop::ProjectTargetItem* parent );
    bool hasIncludesOrDefines( KDevelop::ProjectBaseItem* ) const;
    KDevelop::Path buildDirectory( KDevelop::ProjectBaseItem* ) const;
    IProjectBuilder* builder() const;
    KDevelop::ProjectTargetItem* createTarget( const QString& target, KDevelop::ProjectFolderItem* parent );
    QHash<QString, QString> defines( KDevelop::ProjectBaseItem* ) const;
    KDevelop::Path::List includeDirectories( KDevelop::ProjectBaseItem* ) const;
    bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*>& );
    bool removeTarget( KDevelop::ProjectTargetItem* target );
    QList<KDevelop::ProjectTargetItem*> targets( KDevelop::ProjectFolderItem* ) const;
    KConfigGroup configuration( KDevelop::IProject* ) const;
    KConfigGroup findMatchingPathGroup( const KConfigGroup& cfg, KDevelop::ProjectBaseItem* item ) const;

// IPlugin API
public:
    virtual int perProjectConfigPages() const override;
    virtual KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;
};

#endif

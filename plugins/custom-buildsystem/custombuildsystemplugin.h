/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

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
class IProject;
}

class CustomBuildSystem : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IProjectBuilder, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IBuildSystemManager )
public:
    explicit CustomBuildSystem(QObject* parent, const KPluginMetaData& metaData,
                               const QVariantList& args = QVariantList());
    ~CustomBuildSystem() override;

// ProjectBuilder API
    KJob* build( KDevelop::ProjectBaseItem* dom ) override;
    KJob* clean( KDevelop::ProjectBaseItem* dom ) override;
    KJob* prune( KDevelop::IProject* ) override;

    /// @p installPrefix will be passed as DESTDIR environment variable
    KJob* install( KDevelop::ProjectBaseItem* item, const QUrl &installPrefix ) override;
    KJob* configure( KDevelop::IProject* ) override;

    KDevelop::Path compiler(KDevelop::ProjectTargetItem * p) const override;

Q_SIGNALS:
    void built( KDevelop::ProjectBaseItem *dom );
    void installed( KDevelop::ProjectBaseItem* );
    void cleaned( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem *dom );
    void configured( KDevelop::IProject* );
    void pruned( KDevelop::IProject* );

// AbstractFileManagerPlugin API
public:
    Features features() const override;
    KDevelop::ProjectFolderItem* createFolderItem( KDevelop::IProject* project,
                    const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent = nullptr ) override;

// BuildSystemManager API
public:
    bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*>& file, KDevelop::ProjectTargetItem* parent ) override;
    bool hasBuildInfo( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path buildDirectory( KDevelop::ProjectBaseItem* ) const override;
    IProjectBuilder* builder() const override;
    KDevelop::ProjectTargetItem* createTarget( const QString& target, KDevelop::ProjectFolderItem* parent ) override;
    QHash<QString, QString> defines( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path::List includeDirectories( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path::List frameworkDirectories( KDevelop::ProjectBaseItem* ) const override;
    QString extraArguments(KDevelop::ProjectBaseItem *item) const override;
    bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*>& ) override;
    bool removeTarget( KDevelop::ProjectTargetItem* target ) override;
    QList<KDevelop::ProjectTargetItem*> targets( KDevelop::ProjectFolderItem* ) const override;
    KConfigGroup configuration( KDevelop::IProject* ) const;
    KConfigGroup findMatchingPathGroup( const KConfigGroup& cfg, KDevelop::ProjectBaseItem* item ) const;

// IPlugin API
public:
    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;
};

#endif

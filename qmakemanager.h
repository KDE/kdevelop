/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEMANAGER_H
#define QMAKEMANAGER_H

#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>

class QMakeFolderItem;
class IQMakeBuilder;
class QMakeCache;

class QMakeProjectManager : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )

public:
    explicit QMakeProjectManager( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual ~QMakeProjectManager();

    static QMakeProjectManager* self();

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

    //BEGIN AbstractFileManager
    KDevelop::ProjectFolderItem* import( KDevelop::IProject* project ) override;
    KDevelop::ProjectFolderItem* createFolderItem( KDevelop::IProject* project, const KDevelop::Path& path,
                                                   KDevelop::ProjectBaseItem* parent = 0 ) override;
    Features features() const override;
    bool isValid( const KDevelop::Path& path, const bool isFolder, KDevelop::IProject* project ) const override;
    //END AbstractFileManager

    //BEGIN IBuildSystemManager
    //TODO
    KDevelop::IProjectBuilder*  builder() const override;
    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem*) const override;
    QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const override;
    bool hasIncludesOrDefines(KDevelop::ProjectBaseItem*) const override;

    KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* ) override
    {
        return nullptr;
    }

    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*>&, KDevelop::ProjectTargetItem*) override
    {
        return false;
    }

    bool removeTarget( KDevelop::ProjectTargetItem* ) override
    {
        return false;
    }

    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*>&) override
    {
        return false;
    }

    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override;
    //END IBuildSystemManager

private slots:
    void slotFolderAdded( KDevelop::ProjectFolderItem* folder );
    void slotRunQMake();
    void slotDirty(const QString& path);

private:    
    KDevelop::ProjectFolderItem* projectRootItem( KDevelop::IProject* project, const KDevelop::Path& path );
    KDevelop::ProjectFolderItem* buildFolderItem( KDevelop::IProject* project, const KDevelop::Path& path,
                                                  KDevelop::ProjectBaseItem* parent );
    QHash<QString,QString> queryQMake( KDevelop::IProject* ) const;
    QMakeCache* findQMakeCache( KDevelop::IProject* project, const KDevelop::Path &path = {} ) const;
    bool projectNeedsConfiguration(KDevelop::IProject* project);
    
    IQMakeBuilder* m_builder;
    mutable QString m_qtIncludeDir;
    QAction* m_runQmake;
    QMakeFolderItem* m_actionItem;

    static QMakeProjectManager* m_self;
};

#endif


/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

#ifndef KROSSPROJECTMANAGER_H
#define KROSSPROJECTMANAGER_H

#include <iprojectfilemanager.h>
#include <ibuildsystemmanager.h>
#include <iplugin.h>

class IKrossProjectManager;

class KrossProjectManager : public KDevelop::IPlugin, public KDevelop::IBuildSystemManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IBuildSystemManager )
Q_INTERFACES( KDevelop::IProjectFileManager )
public:
//     Q_PROPERTY(Features features READ features WRITE setFeatures)

    explicit KrossProjectManager( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual ~KrossProjectManager();

    virtual IProjectFileManager::Features features() const;

    virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
    virtual KDevelop::ProjectFolderItem* import( KDevelop::IProject *project );

    virtual KDevelop::IProjectBuilder* builder(KDevelop::ProjectFolderItem*) const;
    virtual KUrl buildDirectory(KDevelop::ProjectBaseItem*) const;
    
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem *) const;
    virtual QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const;
    virtual QHash<QString,QString> environment(KDevelop::ProjectBaseItem*) const { return QHash<QString, QString>(); }
    virtual QList<KDevelop::ProjectTargetItem*> targets() const;
    virtual QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem* folder) const;

    virtual KDevelop::ProjectFolderItem* addFolder( const KUrl& /*folder */, KDevelop::ProjectFolderItem* /*parent*/ );
    virtual KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* );
    virtual KDevelop::ProjectFileItem* addFile( const KUrl&, KDevelop::ProjectFolderItem* );
    virtual bool addFileToTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* );
    virtual bool removeFolder( KDevelop::ProjectFolderItem* );
    virtual bool removeTarget( KDevelop::ProjectTargetItem* );
    virtual bool removeFile( KDevelop::ProjectFileItem* );
    virtual bool removeFileFromTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* );
    virtual bool renameFile(KDevelop::ProjectFileItem*, const KUrl&);
    virtual bool renameFolder(KDevelop::ProjectFolderItem*, const KUrl&);

private:
    QMap<KDevelop::IProject*, IKrossProjectManager*> m_scripts;
    QString m_name;
};

#endif

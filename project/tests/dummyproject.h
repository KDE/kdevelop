/***************************************************************************
 *   Copyright 2010 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVELOP_PROJECT_DUMMYPROJECT_INCLUDED
#define KDEVELOP_PROJECT_DUMMYPROJECT_INCLUDED

#include <interfaces/iproject.h>
#include <language/duchain/indexedstring.h>

namespace KDevelop
{
class ProjectModel;
class ProjectFileItem;
class IPlugin;
class ProjectBaseItem;
class ProjectFolderItem;
class IBuildSystemManager;
class IProjectFileManager;
}

class DummyProject : public KDevelop::IProject
{
Q_OBJECT
public:
    DummyProject( const QString& name_, QObject* parent );
    void setProjectItem( KDevelop::ProjectFolderItem* item );
    Q_SCRIPTABLE virtual KDevelop::IProjectFileManager* projectFileManager() const;
    Q_SCRIPTABLE virtual KDevelop::IBuildSystemManager* buildSystemManager() const;
    Q_SCRIPTABLE virtual KDevelop::IPlugin* managerPlugin() const;
    Q_SCRIPTABLE virtual KDevelop::IPlugin* versionControlPlugin() const;
    Q_SCRIPTABLE virtual KDevelop::ProjectFolderItem* projectItem() const;
    Q_SCRIPTABLE virtual int fileCount() const;
    Q_SCRIPTABLE virtual KDevelop::ProjectFileItem* fileAt( int pos) const;
    Q_SCRIPTABLE virtual QList<KDevelop::ProjectFileItem*> files() const;
    Q_SCRIPTABLE virtual QList<KDevelop::ProjectBaseItem*> itemsForUrl( const KUrl& url ) const;
    Q_SCRIPTABLE virtual QList<KDevelop::ProjectFileItem*> filesForUrl( const KUrl& file ) const;
    Q_SCRIPTABLE virtual QList<KDevelop::ProjectFolderItem*> foldersForUrl( const KUrl& folder ) const;
    Q_SCRIPTABLE virtual void reloadModel();
    Q_SCRIPTABLE virtual KUrl projectFileUrl() const;
    virtual KSharedConfig::Ptr projectConfiguration() const;

    virtual void addToFileSet( const KDevelop::IndexedString& );
    virtual void removeFromFileSet( const KDevelop::IndexedString& );
    virtual QSet<KDevelop::IndexedString> fileSet() const;

    virtual bool isReady() const;

public Q_SLOTS:
    virtual Q_SCRIPTABLE const KUrl folder() const;
    virtual Q_SCRIPTABLE QString name() const;
    virtual KUrl relativeUrl(const KUrl& absoluteUrl) const;
    virtual bool inProject(const KUrl &url) const;
private:
    QString m_name;
    KDevelop::ProjectFolderItem* m_projectItem;
};

#endif

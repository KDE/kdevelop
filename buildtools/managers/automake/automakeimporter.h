/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
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

#ifndef AUTOMAKEIMPORTER_H
#define AUTOMAKEIMPORTER_H

#include <QList>
#include <QString>
#include "kdevbuildmanager.h"

class QObject;

class AutoMakeItem;
class MakefileInterface;
class QDir;


namespace KDevelop
{
    class Project;
    class ProjectItem;
    class ProjectBuilder;
    class ProjectFileItem;
    class ProjectFolderItem;
    class ProjectTargetItem;
}


class AutoMakeImporter : public KDevelop::BuildManager
{
public:
	explicit AutoMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

	virtual ~AutoMakeImporter();

	virtual KDevelop::Project* project() const;
    virtual KDevelop::ProjectBuilder* builder() const { return 0; }
    virtual KUrl buildDirectory() const;
    virtual KUrl::List includeDirectories() const { return KUrl::List(); }
    virtual KUrl::List preprocessorDefines() const { return KUrl::List(); }

	virtual KDevelop::ProjectFolderItem* addFolder( const KUrl& /*folder */,
	                        KDevelop::Project* /*parent*/ ) { return false; }

    virtual KDevelop::ProjectTargetItem* createTarget( const QString&,
                                                 KDevelop::ProjectFolderItem* ) { return false; }

    virtual KDevelop::ProjectFileItem* addFile( const KUrl&,
	                                     KDevelop::ProjectFolderItem* ) { return false; }

    virtual bool addFileToTarget( KDevelop::ProjectFileItem*, KDevelop::ProjectTargetItem* ) { return false; }

	virtual bool removeFolder( KDevelop::ProjectFolderItem* ) { return false; }

    virtual bool removeTarget( KDevelop::ProjectTargetItem* ) { return false; }

    virtual bool removeFile( KDevelop::ProjectFileItem*,
	                         KDevelop::ProjectFolderItem* ) { return false; }
	virtual bool removeFileFromTarget( KDevelop::ProjectFileItem*,
	                                   KDevelop::ProjectTargetItem* ) { return false; }

    QList<KDevelop::ProjectTargetItem*> targets() const;

	virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
	virtual KDevelop::ProjectItem* import( KDevelop::ProjectModel* model,
	                                 const KUrl& fileName );
	virtual KUrl findMakefile( KDevelop::ProjectFolderItem* dom ) const;
    virtual KUrl::List findMakefiles( KDevelop::ProjectFolderItem* dom ) const;

	void createProjectItems( const KUrl&, KDevelop::ProjectItem* );

private:
	KDevelop::Project* m_project;
	KDevelop::ProjectItem* m_rootItem;

	MakefileInterface* m_interface;
};

#endif
// kate: indent-mode cstyle; space-indent off; tab-width 4; auto-insert-doxygen on;


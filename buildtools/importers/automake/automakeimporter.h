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


namespace Koncrete
{
    class Project;
    class ProjectItem;
    class ProjectBuilder;
    class ProjectFileItem;
    class ProjectFolderItem;
    class ProjectTargetItem;
}


class AutoMakeImporter : public Koncrete::BuildManager
{
public:
	AutoMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

	virtual ~AutoMakeImporter();

	virtual Koncrete::Project* project() const;
    virtual Koncrete::ProjectBuilder* builder() const { return 0; }
    virtual KUrl buildDirectory() const;
    virtual KUrl::List includeDirectories() const { return KUrl::List(); }
    virtual KUrl::List preprocessorDefines() const { return KUrl::List(); }

	virtual Koncrete::ProjectFolderItem* addFolder( const KUrl& /*folder */,
	                        Koncrete::Project* /*parent*/ ) { return false; }

    virtual Koncrete::ProjectTargetItem* createTarget( const QString&,
                                                 Koncrete::ProjectFolderItem* ) { return false; }

    virtual Koncrete::ProjectFileItem* addFile( const KUrl&,
	                                     Koncrete::ProjectFolderItem* ) { return false; }

    virtual bool addFileToTarget( Koncrete::ProjectFileItem*, Koncrete::ProjectTargetItem* ) { return false; }

	virtual bool removeFolder( Koncrete::ProjectFolderItem* ) { return false; }

    virtual bool removeTarget( Koncrete::ProjectTargetItem* ) { return false; }

    virtual bool removeFile( Koncrete::ProjectFileItem*,
	                         Koncrete::ProjectFolderItem* ) { return false; }
	virtual bool removeFileFromTarget( Koncrete::ProjectFileItem*,
	                                   Koncrete::ProjectTargetItem* ) { return false; }

    QList<Koncrete::ProjectTargetItem*> targets() const;

	virtual QList<Koncrete::ProjectFolderItem*> parse( Koncrete::ProjectFolderItem* dom );
	virtual Koncrete::ProjectItem* import( Koncrete::ProjectModel* model,
	                                 const KUrl& fileName );
	virtual KUrl findMakefile( Koncrete::ProjectFolderItem* dom ) const;
    virtual KUrl::List findMakefiles( Koncrete::ProjectFolderItem* dom ) const;

	void createProjectItems( const KUrl&, Koncrete::ProjectItem* );

private:
	Koncrete::Project* m_project;
	Koncrete::ProjectItem* m_rootItem;

	MakefileInterface* m_interface;
};

#endif
// kate: indent-mode cstyle; space-indent off; tab-width 4; auto-insert-doxygen on;


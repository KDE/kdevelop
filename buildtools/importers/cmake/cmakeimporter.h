/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef CMAKEIMPORTER_H
#define CMAKEIMPORTER_H

#include <QList>
#include <QString>

#include "domutil.h"
#include "kdevprojectmodel.h"
#include "kdevbuildmanager.h"
#include "cmakexmlparser.h"

class QDir;
class QObject;

namespace KDevelop
{
class Project;
class ProjectItem;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectBuilder;
}

class cmLocalGenerator;

class CMakeImporter : public KDevelop::BuildManager
{
public:
    CMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

    virtual ~CMakeImporter();

    virtual KDevelop::Project* project() const;
    virtual KDevelop::ProjectBuilder* builder() const { return 0; }
    virtual KUrl buildDirectory() const;
    virtual KUrl::List includeDirectories() const;
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


private:

    KDevelop::Project* m_project;
    KDevelop::ProjectItem* m_rootItem;
    KDevelop::ProjectBuilder* m_builder;

    KUrl::List m_includeDirList;
    CMakeXmlParser m_xmlParser;
    ProjectInfo m_projectInfo;

};

#endif



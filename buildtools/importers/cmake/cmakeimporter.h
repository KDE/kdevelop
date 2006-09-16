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
#include "cmake.h"

class QObject;
class KDevProject;
class KDevProjectItem;
class QDir;
class KDevProjectFolderItem;
class KDevProjectFileItem;
class KDevProjectTargetItem;
class KDevProjectBuilder;

class cmLocalGenerator;

class CMakeImporter : public KDevBuildManager
{
public:
    CMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

    virtual ~CMakeImporter();

    virtual KDevProject* project() const;
    virtual KDevProjectBuilder* builder() const { return 0; }
    virtual KUrl buildDirectory() const;
    virtual KUrl::List includeDirectories() const;
    virtual KUrl::List preprocessorDefines() const { return KUrl::List(); }

    virtual KDevProjectFolderItem* addFolder( const KUrl& /*folder */,
                                              KDevProject* /*parent*/ ) { return false; }

    virtual KDevProjectTargetItem* createTarget( const QString&,
                                                 KDevProjectFolderItem* ) { return false; }

    virtual KDevProjectFileItem* addFile( const KUrl&,
                                          KDevProjectFolderItem* ) { return false; }

    virtual bool addFileToTarget( KDevProjectFileItem*, KDevProjectTargetItem* ) { return false; }

    virtual bool removeFolder( KDevProjectFolderItem* ) { return false; }

    virtual bool removeTarget( KDevProjectTargetItem* ) { return false; }

    virtual bool removeFile( KDevProjectFileItem*,
                             KDevProjectFolderItem* ) { return false; }
    virtual bool removeFileFromTarget( KDevProjectFileItem*,
                                       KDevProjectTargetItem* ) { return false; }

    QList<KDevProjectTargetItem*> targets() const;

    virtual QList<KDevProjectFolderItem*> parse( KDevProjectFolderItem* dom );
    virtual KDevProjectItem* import( KDevProjectModel* model,
                                     const KUrl& fileName );
    virtual KUrl findMakefile( KDevProjectFolderItem* dom ) const;
    virtual KUrl::List findMakefiles( KDevProjectFolderItem* dom ) const;

    void createProjectItems( cmLocalGenerator*, KDevProjectItem* );

    //cmake progress callback
    static void updateProgress( const char*, float, void* );


private:

    KDevProject* m_project;
    KDevProjectItem* m_rootItem;
    KDevProjectBuilder* m_builder;
    cmake m_cmakeEngine;

    KUrl::List m_includeDirList;

};

#endif
// kate: indent-mode cstyle; space-indent off; tab-width 4; replace-tabs on; auto-insert-doxygen on;


/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKEIMPORTER_H
#define QMAKEIMPORTER_H

#include <QList>
#include <QString>

#include "domutil.h"
#include "kdevprojectmodel.h"
#include "kdevbuildmanager.h"

class QObject;
class KDevProject;
class KDevProjectItem;
class QDir;
class KDevProjectFolderItem;
class KDevProjectFileItem;
class KDevProjectTargetItem;
class KDevProjectBuilder;

class QMakeImporter : public KDevBuildManager
{
public:
    QMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

    virtual ~QMakeImporter();

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


private:

    KDevProject* m_project;
    KDevProjectItem* m_rootItem;
    KDevProjectBuilder* m_builder;

    KUrl::List m_includeDirList;

};

#endif



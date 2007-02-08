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

#include <ibuildmanager.h>
#include <iplugin.h>

template <typename T> class QList;
class QString;

//#include "domutil.h"
//#include "projectmodel.h"

class QDir;
class QObject;

namespace Koncrete
{
class IProject;
class ProjectItem;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class IProjectBuilder;
}

class QMakeImporter : public Koncrete::IPlugin, public Koncrete::IBuildManager
{
    Q_OBJECT
    Q_INTERFACES( Koncrete::IBuildManager )
    Q_INTERFACES( Koncrete::IFileManager )
    public:
        QMakeImporter( QObject* parent = 0, const QStringList& args = QStringList() );

        virtual ~QMakeImporter();

        virtual Koncrete::IProjectBuilder* builder(Koncrete::ProjectItem*) const { return 0; }
        virtual KUrl buildDirectory(Koncrete::ProjectItem*) const;
        virtual KUrl::List includeDirectories(Koncrete::ProjectBaseItem*) const;
        virtual KUrl::List preprocessorDefines(Koncrete::ProjectBaseItem*) const { return KUrl::List(); }

        virtual Koncrete::ProjectTargetItem* createTarget( const QString&,
                Koncrete::ProjectFolderItem* ) { return false; }

        virtual bool addFileToTarget( Koncrete::ProjectFileItem*,
                                      Koncrete::ProjectTargetItem* ) { return false; }

        virtual bool removeTarget( Koncrete::ProjectTargetItem* ) { return false; }

        virtual bool removeFileFromTarget( Koncrete::ProjectFileItem*,
                                           Koncrete::ProjectTargetItem* ) { return false; }

        virtual QList<Koncrete::ProjectTargetItem*> targets(Koncrete::ProjectItem*) const;

        virtual QList<Koncrete::ProjectFolderItem*> parse( Koncrete::ProjectFolderItem* dom );
        virtual Koncrete::ProjectItem* import( Koncrete::IProject* );
        virtual KUrl findMakefile( Koncrete::ProjectFolderItem* folder ) const;
        virtual KUrl::List findMakefiles( Koncrete::ProjectFolderItem* folder ) const;

        virtual Koncrete::ProjectFolderItem* addFolder( const KUrl&,
                Koncrete::ProjectFolderItem* ) { return 0; }

        virtual Koncrete::ProjectFileItem* addFile( const KUrl&,
                Koncrete::ProjectFolderItem* ) { return 0; }

        virtual bool removeFile( Koncrete::ProjectFileItem* ) { return false; }
        virtual bool removeFolder( Koncrete::ProjectFolderItem* ) { return false; }
        virtual bool renameFile( Koncrete::ProjectFileItem*, const KUrl& ) { return false; }
        virtual bool renameFolder( Koncrete::ProjectFolderItem*, const KUrl&  ) { return false; }

        virtual void registerExtensions();
        virtual void unregisterExtensions();
        virtual QStringList extensions() const;

    private:

        Koncrete::IProject* m_project;
        Koncrete::ProjectFolderItem* m_rootItem;
        Koncrete::IProjectBuilder* m_builder;

        KUrl::List m_includeDirList;

};

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

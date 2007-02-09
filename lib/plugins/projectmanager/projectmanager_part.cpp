/* This file is part of KDevelop
   Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "projectmanager_part.h"
#include "projectmodel.h"
#include "projectmanager.h"
#include "icore.h"
#include "kdevconfig.h"
#include "iproject.h"
#include "ifilemanager.h"
#include "ibuildmanager.h"
#include "iuicontroller.h"
#include "iprojectbuilder.h"
#include "iprojectcontroller.h"
#include "importprojectjob.h"

#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kplugininfo.h>

#include <kparts/componentfactory.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QList>

namespace KDevelop
{

typedef KGenericFactory<ProjectManagerPart> ProjectManagerFactory;
K_EXPORT_COMPONENT_FACTORY( kdevprojectmanager, ProjectManagerFactory( "kdevprojectmanager" ) )

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevProjectManagerViewFactory( ProjectManagerPart *part ): m_part( part )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new ProjectManager( m_part, parent );
        }
    private:
        ProjectManagerPart *m_part;
};

class ProjectManagerPartPrivate
{
public:
    KDevProjectManagerViewFactory *factory;
};

ProjectManagerPart::ProjectManagerPart( QObject *parent, const QStringList& )
        : IPlugin( ProjectManagerFactory::componentData(), parent ), d(new ProjectManagerPartPrivate)
{
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView( "Project Manager", d->factory );

    setXMLFile( "kdevprojectmanager.rc" );
}

ProjectManagerPart::~ProjectManagerPart()
{
    delete d;
}

void ProjectManagerPart::openURL( const KUrl &url )
{
    core()->uiController()->openUrl( url );
}

// ProjectFolderItem *ProjectManagerPart::activeFolder()
// {
//     return m_projectOverview->currentFolderItem();
// }
//
// ProjectTargetItem *ProjectManagerPart::activeTarget()
// {
//     return m_projectOverview->currentTargetItem();
// }
//
// ProjectFileItem * ProjectManagerPart::activeFile()
// {
//     return m_projectOverview->currentFileItem();
// }

Qt::DockWidgetArea ProjectManagerPart::dockWidgetAreaHint() const
{
    return Qt::RightDockWidgetArea;
}


bool ProjectManagerPart::computeChanges( const QStringList &oldFileList, const QStringList &newFileList )
{
    QMap<QString, bool> oldFiles, newFiles;

    for ( QStringList::ConstIterator it = oldFileList.begin(); it != oldFileList.end(); ++it )
        oldFiles.insert( *it, true );

    for ( QStringList::ConstIterator it = newFileList.begin(); it != newFileList.end(); ++it )
        newFiles.insert( *it, true );

    // created files: oldFiles - newFiles
    for ( QStringList::ConstIterator it = oldFileList.begin(); it != oldFileList.end(); ++it )
        newFiles.remove( *it );

    // removed files: newFiles - oldFiles
    for ( QStringList::ConstIterator it = newFileList.begin(); it != newFileList.end(); ++it )
        oldFiles.remove( *it );
    /* FIXME port me!
      if (!newFiles.isEmpty())
        emit addedFilesToProject(newFiles.keys());

      if (!oldFiles.isEmpty())
        emit removedFilesFromProject(oldFiles.keys());
    */
    return false; //FIXME
}

void ProjectManagerPart::updateDetails( ProjectBaseItem * )
{}

void ProjectManagerPart::unload()
{
    core()->uiController()->removeToolView(d->factory);
}

}
#include "projectmanager_part.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

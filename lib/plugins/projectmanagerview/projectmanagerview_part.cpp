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
#include "projectmanagerview_part.h"
#include "projectmodel.h"
#include "projectmanagerview.h"
#include "icore.h"
// #include "kdevconfig.h"
#include "iproject.h"
#include "iprojectfilemanager.h"
#include "ibuildsystemmanager.h"
#include "iuicontroller.h"
#include "idocumentcontroller.h"
#include "iprojectbuilder.h"
#include "iprojectcontroller.h"
#include "importprojectjob.h"
#include "context.h"

#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kplugininfo.h>

#include <kparts/componentfactory.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QSignalMapper>

namespace KDevelop
{

typedef KGenericFactory<ProjectManagerViewPart> ProjectManagerFactory;
K_EXPORT_COMPONENT_FACTORY( kdevprojectmanagerview, ProjectManagerFactory( "kdevprojectmanagerview" ) )

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevProjectManagerViewFactory( ProjectManagerViewPart *part ): m_part( part )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new ProjectManagerView( m_part, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
        {
            return Qt::RightDockWidgetArea;
        }
    private:
        ProjectManagerViewPart *m_part;
};

class ProjectManagerViewPartPrivate
{
public:
    ProjectManagerViewPartPrivate() : build_objectname("projectmanagerview_buildaction")
    {}
    KDevProjectManagerViewFactory *factory;
    QMap<QString, Context*> contexts;
    QSignalMapper* contextMenuMapper;
    const QString build_objectname;
};

ProjectManagerViewPart::ProjectManagerViewPart( QObject *parent, const QStringList& )
        : IPlugin( ProjectManagerFactory::componentData(), parent ), d(new ProjectManagerViewPartPrivate)
{
    d->factory = new KDevProjectManagerViewFactory( this );
    d->contextMenuMapper = new QSignalMapper( this );
    connect( d->contextMenuMapper, SIGNAL( mapped( const QString& ) ),
             this, SLOT( executeContextMenuAction( const QString& ) ) );
    core()->uiController()->addToolView( "Project Manager", d->factory );
    setXMLFile( "kdevprojectmanagerview.rc" );
}

ProjectManagerViewPart::~ProjectManagerViewPart()
{
    delete d;
}

void ProjectManagerViewPart::openURL( const KUrl &url )
{
    core()->documentController()->openDocument( url );
}

// ProjectFolderItem *ProjectManagerViewPart::activeFolder()
// {
//     return m_projectOverview->currentFolderItem();
// }
//
// ProjectTargetItem *ProjectManagerViewPart::activeTarget()
// {
//     return m_projectOverview->currentTargetItem();
// }
//
// ProjectFileItem * ProjectManagerViewPart::activeFile()
// {
//     return m_projectOverview->currentFileItem();
// }

bool ProjectManagerViewPart::computeChanges( const QStringList &oldFileList, const QStringList &newFileList )
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

void ProjectManagerViewPart::updateDetails( ProjectBaseItem * )
{}

void ProjectManagerViewPart::unload()
{
    core()->uiController()->removeToolView(d->factory);
}

QPair<QString, QList<QAction*> > ProjectManagerViewPart::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() == KDevelop::Context::ProjectItemContext )
    {
        QList<QAction*> actions;
        KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
        KDevelop::ProjectBaseItem* item = ctx->item();
        if ( KDevelop::ProjectFolderItem *folder = item->folder() )
        {
            actions << new QAction( i18n( "Folder: %1", folder->url().directory() ), this );
//             QAction* buildaction = new QAction( i18n( "Build this project" ), this );
//             buildaction->setObjectName(d->build_objectname);
//             d->contextMenuMapper->setMapping( buildaction, buildaction->objectName() );
//             d->contexts[buildaction->objectName()] = context;
//             connect( buildaction, SIGNAL(triggered() ), d->contextMenuMapper, SLOT( map() ) );
//             actions << buildaction;
        }
        else if ( KDevelop::ProjectFileItem *file = item->file() )
        {
            actions << new QAction( i18n( "File: %1", file->url().fileName() ), this );
        }
        else if ( KDevelop::ProjectTargetItem *target = item->target() )
        {
            actions << new QAction( i18n( "Target: %1", target->text() ), this );

//             QAction* targetBldAction = new QAction( i18n( "Build this target" ), this );
//             targetBldAction->setObjectName( d->build_objectname );
//             d->contextMenuMapper->setMapping( targetBldAction, targetBldAction->objectName() );
//             d->contexts[ targetBldAction->objectName() ] = context;
//             connect( targetBldAction, SIGNAL(triggered()), d->contextMenuMapper, SLOT( map() ) );
//             actions << targetBldAction;
        }
        return qMakePair(QString("Project Management"), actions);
    }
    return IPlugin::requestContextMenuActions( context );
}

void ProjectManagerViewPart::executeContextMenuAction( const QString& objectname )
{
    if( !d->contexts.contains(objectname) )
        return;
    Context* ctxt = d->contexts[objectname];
    if( ctxt && objectname == d->build_objectname &&
        ctxt->type() == KDevelop::Context::ProjectItemContext )
    {
        ProjectItemContext* prjctxt = dynamic_cast<ProjectItemContext*>(ctxt);
        executeProjectBuilder( prjctxt->item() );
    }
}

void ProjectManagerViewPart::executeProjectBuilder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return;
    IProject* project = item->project();
    ProjectItem* prjitem = project->projectItem();
    IPlugin* fmgr = project->managerPlugin();
    IBuildSystemManager* mgr = fmgr->extension<IBuildSystemManager>();
    if( mgr )
    {
        IProjectBuilder* builder = mgr->builder( prjitem );
        if( builder)
          builder->build( item );
    }
}
}
#include "projectmanagerview_part.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

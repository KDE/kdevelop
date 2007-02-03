/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
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

#include "projectmanager.h"
#include "projectmanager_part.h"
#include "projectmodel.h"
#include "projectmanagerdelegate.h"
//#include "projectfilter.h"
#include "projecttreeview.h"

#include <QtGui/QHeaderView>

#include "icore.h"
#include "iprojectcontroller.h"
#include "iuicontroller.h"
#include "iproject.h"
#include <kmainwindow.h>
// #include <kdevcontext.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/QDebug>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStandardItem>

namespace Koncrete
{

class ProjectManagerPrivate
{
public:
    ProjectManagerPart *m_part;
    ProjectTreeView *m_projectOverview;
    ProjectTreeView *m_projectDetails;
    QStringList m_cachedFileList;

    void fileDirty( const QString &fileName )
    {
        Q_UNUSED(fileName)
    }
    void fileCreated( const QString &fileName )
    {
        Q_UNUSED(fileName)
    }
    void fileDeleted( const QString &fileName )
    {
        Q_UNUSED(fileName)
    }

    void pressed( const QModelIndex & index )
    {
        QStandardItem* item = m_part->core()->projectController()->activeProject()->model()->itemFromIndex( index );
        if ( item->type() == ProjectItem::File )
        {
            ProjectItem* projectItem = dynamic_cast<ProjectItem*>( item );
            if ( projectItem && projectItem->file() )
                m_part->core()->uiController()->openUrl( projectItem->file()->url() );
        }
    }
};

ProjectManager::ProjectManager( ProjectManagerPart *_part, QWidget *parent )
        : QWidget( parent ),
        d(new ProjectManagerPrivate)
{
    d->m_part = _part;
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );

    ProjectManagerDelegate *delegate = new ProjectManagerDelegate( this );

    IProject* project = part()->core()->projectController()->activeProject();
    d->m_projectOverview = new ProjectTreeView( part(), this );
    d->m_projectOverview->setItemDelegate( delegate );
    d->m_projectOverview->setWhatsThis( i18n( "Project Overview" ) );
    vbox->addWidget( d->m_projectOverview );
        //   connect(m_projectOverview, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
    connect( d->m_projectOverview, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( pressed( QModelIndex ) ) );



#ifdef WITH_PROJECT_DETAILS
    d->m_projectDetails = new ProjectTreeView( part(), this );
    d->m_projectDetails->setItemDelegate( delegate );
    d->m_projectDetails->setWhatsThis( i18n( "Project Details" ) );
    vbox->add( d->m_projectDetails );

        //   connect(m_projectDetails, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
    connect( d->m_projectDetails, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( pressed( QModelIndex ) ) );
    connect( d->m_projectOverview->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             d->m_projectDetails, SLOT( setRootIndex( QModelIndex ) ) );
#endif

    if( project )
    {
        QAbstractItemModel *overviewModel = project->model();


        d->m_projectOverview->setModel( overviewModel );

#ifdef WITH_PROJECT_DETAILS
        QAbstractItemModel *detailsModel = m_projectModel;

        d->m_projectDetails->setModel( detailsModel );
#endif

    }
    setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    setWindowTitle( i18n( "Project Manager" ) );
    setWhatsThis( i18n( "Project Manager" ) );
}

ProjectManager::~ProjectManager()
{
    delete d;
}

ProjectManagerPart *ProjectManager::part() const
{
    return d->m_part;
}


}
#include "projectmanager.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "projectmanagerview.h"
#include "projectmanagerview_part.h"
#include "projectmodel.h"
#include "projectmanagerdelegate.h"
//#include "projectfilter.h"
#include "projecttreeview.h"

#include <QtGui/QHeaderView>

#include "icore.h"
#include "iprojectcontroller.h"
#include "iuicontroller.h"
#include "idocumentcontroller.h"
#include "iproject.h"
#include "ibuildsystemmanager.h"
#include "iprojectbuilder.h"
#include <kxmlguiwindow.h>
// #include <kdevcontext.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <QtCore/QDebug>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStandardItem>

using namespace KDevelop;

class ProjectManagerPrivate
{
public:
    ProjectManagerViewPart *m_part;
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

    void openUrl( const KUrl& url )
    {
        m_part->core()->documentController()->openDocument( url );
    }
};

ProjectManagerView::ProjectManagerView( ProjectManagerViewPart *_part, QWidget *parent )
        : QWidget( parent ),
        d(new ProjectManagerPrivate)
{
    setWindowTitle("Project Manager");

    d->m_part = _part;
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );

    ProjectManagerDelegate *delegate = new ProjectManagerDelegate( this );

    d->m_projectOverview = new ProjectTreeView( part(), this );
    d->m_projectOverview->setItemDelegate( delegate );
    d->m_projectOverview->setWhatsThis( i18n( "Project Overview" ) );
    vbox->addWidget( d->m_projectOverview );
    connect(d->m_projectOverview, SIGNAL(activateUrl(const KUrl&)), this, SLOT(openUrl(const KUrl&)));
    //connect( d->m_projectOverview, SIGNAL( currentChanged( KDevelop::ProjectBaseItem* ) ),
           //d->m_part->core()->projectController(), SLOT(changeCurrentProject( KDevelop::ProjectBaseItem* ) ) );


#ifdef WITH_PROJECT_DETAILS
    d->m_projectDetails = new ProjectTreeView( part(), this );
    d->m_projectDetails->setItemDelegate( delegate );
    d->m_projectDetails->setWhatsThis( i18n( "Project Details" ) );
    vbox->add( d->m_projectDetails );

    connect(m_projectDetails, SIGNAL(activateUrl(const KUrl&)), this, SLOT(openUrl(const KUrl&)));
    connect( d->m_projectOverview->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             d->m_projectDetails, SLOT( setRootIndex( QModelIndex ) ) );
#endif

    QAbstractItemModel *overviewModel = d->m_part->core()->projectController()->projectModel();


    d->m_projectOverview->setModel( overviewModel );

    d->m_projectOverview->setSelectionModel(
            new QItemSelectionModel( overviewModel, d->m_projectOverview ) );

#ifdef WITH_PROJECT_DETAILS
    QAbstractItemModel *detailsModel = m_projectModel;

    d->m_projectDetails->setModel( detailsModel );
#endif

    setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    setWindowTitle( i18n( "Project Manager" ) );
    setWhatsThis( i18n( "Project Manager" ) );
}

ProjectManagerView::~ProjectManagerView()
{
    delete d;
}

ProjectManagerViewPart *ProjectManagerView::part() const
{
    return d->m_part;
}

#include "projectmanagerview.moc"


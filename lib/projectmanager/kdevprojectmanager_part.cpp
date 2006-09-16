/* This file is part of KDevelop
   Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
#include "kdevprojectmanager_part.h"
#include "kdevprojectmanagerdelegate.h"
#include "kdevprojectmodel.h"
#include "kdevprojectmanager.h"
#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevproject.h"
#include "kdevfilemanager.h"
#include "kdevbuildmanager.h"
#include "kdevprojectbuilder.h"
#include "kdevprojectfilter.h"
#include "importprojectjob.h"

#include <kfiltermodel.h>
#include <kdevcore.h>
#include <kdevdocumentcontroller.h>
#include <kdevmainwindow.h>
#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kplugininfo.h>

#include <kparts/componentfactory.h>

#include <QtGui/QVBoxLayout>
#include <QDir>
#include <qfileinfo.h>
#include <QTimer>
#include <QLineEdit>
#include <QtGui/QStandardItem>
#include <QList>

typedef KGenericFactory<KDevProjectManagerPart> KDevProjectManagerFactory;
K_EXPORT_COMPONENT_FACTORY( kdevprojectmanager, KDevProjectManagerFactory( "kdevprojectmanager" ) )

KDevProjectManagerPart::KDevProjectManagerPart( QObject *parent, const QStringList& )
        : KDevPlugin( KDevProjectManagerFactory::instance(), parent )
{
    setInstance( KDevProjectManagerFactory::instance() );

    m_widget = new QWidget( 0 );
    QVBoxLayout *vbox = new QVBoxLayout( m_widget );
    vbox->setMargin( 0 );

#if 0
    QLineEdit *editor = new QLineEdit( m_widget );
    vbox->addWidget( editor );
    editor->hide();
#endif

    KDevProjectManagerDelegate *delegate = new KDevProjectManagerDelegate( this );

    QAbstractItemModel *overviewModel = KDevCore::activeProject()->model();
#ifdef USE_KFILTER_MODEL
    overviewModel = new KDevProjectOverviewFilter( m_projectModel, this );
#endif

    m_projectOverview = new KDevProjectManager( this, m_widget );
    m_projectOverview->setModel( overviewModel );
    m_projectOverview->setItemDelegate( delegate );
    m_projectOverview->setWhatsThis( i18n( "Project Overview" ) );
    vbox->addWidget( m_projectOverview );

    //   connect(m_projectOverview, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
    connect( m_projectOverview, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( pressed( QModelIndex ) ) );



#ifdef WITH_PROJECT_DETAILS
    QAbstractItemModel *detailsModel = m_projectModel;
#ifdef USE_KFILTER_MODEL
    detailsModel = new KDevProjectDetailsFilter( m_projectModel, this );
#endif

    m_projectDetails = new KDevProjectManager( this, m_widget );
    m_projectDetails->setModel( detailsModel );
    m_projectDetails->setItemDelegate( delegate );
    m_projectDetails->setWhatsThis( i18n( "Project Details" ) );
    vbox->add( m_projectDetails );

    //   connect(m_projectDetails, SIGNAL(activateURL(KUrl)), this, SLOT(openURL(KUrl)));
    connect( m_projectDetails, SIGNAL( pressed( QModelIndex ) ),
             this, SLOT( pressed( QModelIndex ) ) );
    connect( m_projectOverview->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
             m_projectDetails, SLOT( setRootIndex( QModelIndex ) ) );
#endif

    m_widget->setObjectName( i18n( "Project Manager" ) );

    m_widget->setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    m_widget->setWindowTitle( i18n( "Project Manager" ) );
    m_widget->setWhatsThis( i18n( "Project Manager" ) );

    setXMLFile( "kdevprojectmanager.rc" );
}

KDevProjectManagerPart::~KDevProjectManagerPart()
{
    delete m_widget;
    m_widget = 0;
}

void KDevProjectManagerPart::openURL( const KUrl &url )
{
    KDevCore::documentController() ->editDocument( url );
}

KDevProjectFolderItem *KDevProjectManagerPart::activeFolder()
{
    return m_projectOverview->currentFolderItem();
}

KDevProjectTargetItem *KDevProjectManagerPart::activeTarget()
{
    return m_projectOverview->currentTargetItem();
}

KDevProjectFileItem * KDevProjectManagerPart::activeFile()
{
    return m_projectOverview->currentFileItem();
}

QWidget *KDevProjectManagerPart::pluginView() const
{
  return m_widget;
}

Qt::DockWidgetArea KDevProjectManagerPart::dockWidgetAreaHint() const
{
  return Qt::RightDockWidgetArea;
}

void KDevProjectManagerPart::fileDirty( const QString &fileName )
{
    Q_UNUSED( fileName );
}

void KDevProjectManagerPart::fileDeleted( const QString &fileName )
{
    Q_UNUSED( fileName );
}

void KDevProjectManagerPart::fileCreated( const QString &fileName )
{
    Q_UNUSED( fileName );
}

bool KDevProjectManagerPart::computeChanges( const QStringList &oldFileList, const QStringList &newFileList )
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

void KDevProjectManagerPart::updateDetails( KDevProjectItem * )
{}

void KDevProjectManagerPart::pressed( const QModelIndex & index )
{
  QStandardItem* item = KDevCore::activeProject()->model()->itemFromIndex( index );
  if ( item->type() == KDevProjectItem::File )
  {
    KDevProjectItem* projectItem = dynamic_cast<KDevProjectItem*>( item );
    if ( projectItem && projectItem->file() )
      KDevCore::documentController() ->editDocument( projectItem->file()->url() );
  }

}

#include "kdevprojectmanager_part.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;

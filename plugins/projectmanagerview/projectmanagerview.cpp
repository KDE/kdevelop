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

#include <QtGui/QHeaderView>
#include <QtCore/QDebug>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStandardItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolButton>

#include <kxmlguiwindow.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kcombobox.h>

#include "icore.h"
#include "iprojectcontroller.h"
#include "iuicontroller.h"
#include "idocumentcontroller.h"
#include "iproject.h"
#include "ibuildsystemmanager.h"
#include "iprojectbuilder.h"
#include "projectmodel.h"

#include "projectbuildsetwidget.h"
#include "projectmanagerviewplugin.h"
#include "projectmanagerdelegate.h"
#include "projecttreeview.h"

using namespace KDevelop;

class ProjectManagerPrivate
{
public:
    ProjectManagerViewPlugin *mplugin;
    ProjectTreeView *m_projectOverview;
    ProjectBuildSetWidget* m_buildView;
    KComboBox* m_detailSwitcher;
    QStackedWidget* m_detailStack;
    QStringList m_cachedFileList;
    QToolButton* hideDetailsButton;

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
        mplugin->core()->documentController()->openDocument( url );
    }
};

ProjectManagerView::ProjectManagerView( ProjectManagerViewPlugin *plugin, QWidget *parent )
        : QWidget( parent ),
        d(new ProjectManagerPrivate)
{
    setWindowTitle("Project Manager");

    d->mplugin = plugin;
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );

    ProjectManagerDelegate *delegate = new ProjectManagerDelegate( this );

    d->m_projectOverview = new ProjectTreeView( plugin, this );
    d->m_projectOverview->setItemDelegate( delegate );
    d->m_projectOverview->setWhatsThis( i18n( "Project Overview" ) );
    vbox->addWidget( d->m_projectOverview );
    connect(d->m_projectOverview, SIGNAL(activateUrl(const KUrl&)), this, SLOT(openUrl(const KUrl&)));
//     connect( d->m_projectOverview, SIGNAL( currentChanged( KDevelop::ProjectBaseItem* ) ),
//              this, SLOT(currentItemChanged( KDevelop::ProjectBaseItem* ) ) );

    QHBoxLayout* hbox = new QHBoxLayout();
    vbox->addLayout( hbox );

    d->m_detailSwitcher = new KComboBox( false, this );
    d->m_detailSwitcher->insertItem( 0, i18n( "Buildset" ) );
    hbox->addWidget( d->m_detailSwitcher );

    d->hideDetailsButton = new QToolButton( this );
    d->hideDetailsButton->setIcon( KIcon( "arrow-down-double.png" ) );
    connect( d->hideDetailsButton, SIGNAL( clicked() ),
             this, SLOT( switchDetailView() ) );
    hbox->addWidget( d->hideDetailsButton );

    d->m_detailStack = new QStackedWidget( this );
    vbox->addWidget( d->m_detailStack );

    connect( d->m_detailSwitcher, SIGNAL( activated( int ) ),
             d->m_detailStack, SLOT( setCurrentIndex( int ) ) );

    d->m_buildView = new ProjectBuildSetWidget( this, d->mplugin, d->m_detailStack );
//     d->m_buildView->setItemDelegate( delegate );
    d->m_buildView->setWhatsThis( i18n( "Build Items:" ) );
    d->m_detailStack->insertWidget( 0, d->m_buildView );

    QAbstractItemModel *overviewModel = d->mplugin->core()->projectController()->projectModel();

    d->m_projectOverview->setModel( overviewModel );

    d->m_projectOverview->setSelectionModel(
            new QItemSelectionModel( overviewModel, d->m_projectOverview ) );

    setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    setWindowTitle( i18n( "Project Manager" ) );
    setWhatsThis( i18n( "Project Manager" ) );
}

ProjectManagerView::~ProjectManagerView()
{
    delete d;
}

ProjectManagerViewPlugin *ProjectManagerView::plugin() const
{
    return d->mplugin;
}

QList<KDevelop::ProjectBaseItem*> ProjectManagerView::selectedItems() const
{
    QList<KDevelop::ProjectBaseItem*> items;
    foreach( QModelIndex idx, d->m_projectOverview->selectionModel()->selectedIndexes() )
    {
        KDevelop::ProjectBaseItem* item = d->mplugin->core()->projectController()->projectModel()->item( idx );
        if( item )
            items << item;
    }
    return items;
}

void ProjectManagerView::switchDetailView()
{
    if( d->m_detailStack->isHidden() )
    {
        d->hideDetailsButton->setIcon( KIcon( "arrow-down-double" ) );
        d->m_detailStack->show();
    }else
    {
        d->hideDetailsButton->setIcon( KIcon( "arrow-up-double" ) );
        d->m_detailStack->hide();
    }
}

#include "projectmanagerview.moc"


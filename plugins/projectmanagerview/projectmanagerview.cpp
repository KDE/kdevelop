/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2008 Aleix Pol <aleixpol@gmail.com>

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

#include <QtCore/QDebug>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStandardItem>
#include <QtGui/QToolButton>

#include <kxmlguiwindow.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kfadewidgeteffect.h>
#include <kcombobox.h>
#include <kjob.h>

#include <interfaces/iselectioncontroller.h>
#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

#include "tests/common/modeltest.h"
#include "projectproxymodel.h"
#include "projectmanagerviewplugin.h"
#include "proxyselectionmodel.h"
#include "builderjob.h"
#include "ui_projectmanagerview.h"

using namespace KDevelop;

ProjectManagerView::ProjectManagerView( ProjectManagerViewPlugin* plugin, QWidget *parent )
        : QWidget( parent ), m_ui(new Ui::ProjectManagerView), m_plugin(plugin)
{
    setWindowTitle(i18n("Projects"));

    m_ui->setupUi( this );

    m_syncAction = new KAction(this);
    m_syncAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_syncAction->setText(i18n("Locate Current Document"));
    m_syncAction->setToolTip(i18n("Locates the current document in the project tree and selects it."));
    m_syncAction->setIcon(KIcon("dirsync"));
    connect(m_syncAction, SIGNAL(triggered(bool)), this, SLOT(locateCurrentDocument()));
    addAction(m_syncAction);
    updateSyncAction();

    addAction(plugin->actionCollection()->action("project_build"));
    addAction(plugin->actionCollection()->action("project_install"));
    addAction(plugin->actionCollection()->action("project_clean"));
    
    m_ui->projectTreeView->setWhatsThis( i18n( "Project Overview" ) );
    QSizePolicy pol( QSizePolicy::Expanding, QSizePolicy::Expanding );
    pol.setVerticalStretch( 6 );
    m_ui->projectTreeView->setSizePolicy( pol );
    connect(m_ui->projectTreeView, SIGNAL(activateUrl(const KUrl&)), this, SLOT(openUrl(const KUrl&)));

//     m_filters = new KLineEdit(this);
//     m_filters->setClearButtonShown(true);
//     connect(d->m_filters, SIGNAL(returnPressed()), this, SLOT(filtersChanged()));
//     vbox->addWidget( m_filters );

    pol = QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    pol.setVerticalStretch( 2 );
    m_ui->buildSetView->setProjectView( this );
    m_ui->buildSetView->setSizePolicy( pol );
    m_ui->buildSetView->setWhatsThis( i18n( "Build Items:" ) );

    QStandardItemModel *overviewModel = ICore::self()->projectController()->projectModel();
    m_modelFilter = new ProjectProxyModel( this );
    m_modelFilter->setSourceModel(overviewModel);
    m_modelFilter->setDynamicSortFilter(true);
    m_ui->projectTreeView->setModel( m_modelFilter );
    m_ui->projectTreeView->setSortingEnabled(true);
    m_ui->projectTreeView->sortByColumn( 0, Qt::AscendingOrder );
    m_ui->projectTreeView->setSelectionModel( new ProxySelectionModel( m_ui->projectTreeView, ICore::self()->projectController()->projectSelectionModel(), this ) );
//     m_ui->projectTreeView->setModel( overviewModel );c
    setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    setWindowTitle( i18n( "Project Manager" ) );
    setWhatsThis( i18n( "Project Manager" ) );
    connect( m_ui->projectTreeView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection&, const QItemSelection&) ),
             this, SLOT(selectionChanged() ) );
    connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*) ),
             SLOT(updateSyncAction()));
    connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentOpened(KDevelop::IDocument*) ),
             SLOT(updateSyncAction()));
    selectionChanged();
}

void ProjectManagerView::selectionChanged()
{
    m_ui->buildSetView->selectionChanged();
    QList<ProjectBaseItem*> selected;
    foreach( const QModelIndex& idx, m_ui->projectTreeView->selectionModel()->selectedRows() )
    {
        selected << m_modelFilter->itemFromProxyIndex( idx );
    }
    KDevelop::ICore::self()->selectionController()->updateSelection( new ProjectItemContext( selected ) );
}

void ProjectManagerView::updateSyncAction()
{
    m_syncAction->setEnabled( !KDevelop::ICore::self()->documentController()->openDocuments().isEmpty() );
}

ProjectManagerView::~ProjectManagerView()
{
}

QList<KDevelop::ProjectBaseItem*> ProjectManagerView::selectedItems() const
{
    QList<KDevelop::ProjectBaseItem*> items;
    foreach( const QModelIndex &idx, m_ui->projectTreeView->selectionModel()->selectedIndexes() )
    {
        KDevelop::ProjectBaseItem* item =
                ICore::self()->projectController()->projectModel()->item( m_modelFilter->mapToSource(idx) );
        if( item )
            items << item;
        else
            kDebug(9511) << "adding an unknown item";
    }
    return items;
}

void ProjectManagerView::locateCurrentDocument()
{
    KDevelop::IDocument *doc = ICore::self()->documentController()->activeDocument();

    foreach (IProject* proj, ICore::self()->projectController()->projects()) {
        foreach (KDevelop::ProjectFileItem* item, proj->filesForUrl(doc->url())) {
            QModelIndex index = m_modelFilter->proxyIndexFromItem(item);
            if (index.isValid()) {
                m_ui->projectTreeView->setCurrentIndex(index);
                m_ui->projectTreeView->expand(index);
                m_ui->projectTreeView->scrollTo(index);
                return;
            }
        }
    }
}

void ProjectManagerView::fileDirty( const QString &fileName )
{
    Q_UNUSED(fileName)
}
void ProjectManagerView::fileCreated( const QString &fileName )
{
    Q_UNUSED(fileName)
}
void ProjectManagerView::fileDeleted( const QString &fileName )
{
    Q_UNUSED(fileName)
}

void ProjectManagerView::openUrl( const KUrl& url )
{
        ICore::self()->documentController()->openDocument( url );
}

#include "projectmanagerview.moc"


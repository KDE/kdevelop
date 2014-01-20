/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "vcseventwidget.h"

#include <QAction>
#include <QHeaderView>
#include <QAction>
#include <QClipboard>

#include <KLocalizedString>
#include <kdebug.h>
#include <kmenu.h>
#include <kdialog.h>

#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "ui_vcseventwidget.h"
#include "vcsdiffwidget.h"

#include "../vcsjob.h"
#include "../interfaces/ibasicversioncontrol.h"
#include "../vcsrevision.h"
#include "../vcsevent.h"
#include "../vcslocation.h"

#include "../models/vcsitemeventmodel.h"
#include "../models/vcseventmodel.h"


namespace KDevelop
{

class VcsEventWidgetPrivate
{
public:
    VcsEventWidgetPrivate( VcsEventWidget* w )
        : q( w )
    {
        m_copyAction = new QAction(QIcon::fromTheme("edit-copy"), i18n("Copy revision number"), q);
        m_copyAction->setShortcut(Qt::ControlModifier+Qt::Key_C);
        QObject::connect(m_copyAction, SIGNAL(triggered(bool)), q, SLOT(copyRevision()));
    }

    Ui::VcsEventWidget* m_ui;
    VcsItemEventModel* m_detailModel;
    VcsEventModel *m_logModel;
    KUrl m_url;
    QModelIndex m_contextIndex;
    VcsEventWidget* q;
    QAction* m_copyAction;
    IBasicVersionControl* m_iface;
    void eventViewCustomContextMenuRequested( const QPoint &point );
    void eventViewClicked( const QModelIndex &index );
    void jobReceivedResults( KDevelop::VcsJob* job );
    void copyRevision();
    void diffToPrevious();
    void diffRevisions();
    void currentRowChanged(const QModelIndex& start, const QModelIndex& end);
};

void VcsEventWidgetPrivate::eventViewCustomContextMenuRequested( const QPoint &point )
{
    m_contextIndex = m_ui->eventView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug() << "contextMenu is not in TreeView";
        return;
    }

    KMenu menu( m_ui->eventView );
    menu.addAction(m_copyAction);
    menu.addAction(i18n("Diff to previous revision"), q, SLOT(diffToPrevious()));
    QAction* action = menu.addAction(i18n("Diff between revisions"), q, SLOT(diffRevisions()));
    action->setEnabled(m_ui->eventView->selectionModel()->selectedRows().size()>=2);

    menu.exec( m_ui->eventView->viewport()->mapToGlobal(point) );
}

void VcsEventWidgetPrivate::currentRowChanged(const QModelIndex& start, const QModelIndex& end)
{
    Q_UNUSED(end);
    if(start.isValid())
        eventViewClicked(start);
}

void VcsEventWidgetPrivate::eventViewClicked( const QModelIndex &index )
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( index );
    m_detailModel->removeRows(0, m_detailModel->rowCount());
    
    if( ev.revision().revisionType() != KDevelop::VcsRevision::Invalid )
    {
        m_ui->itemEventView->setEnabled(true);
        m_ui->message->setEnabled(true);
        m_ui->message->setPlainText( ev.message() );
        m_detailModel->addItemEvents( ev.items() );
    }else
    {
        m_ui->itemEventView->setEnabled(false);
        m_ui->message->setEnabled(false);
        m_ui->message->clear();
    }

    QHeaderView* header = m_ui->itemEventView->header();
    for(int i=0, count=m_detailModel->columnCount(); i<count; i++) {
        header->setResizeMode( i, (i==m_detailModel->columnCount()-1) ? QHeaderView::Stretch : QHeaderView::ResizeToContents);
    }
}

void VcsEventWidgetPrivate::copyRevision()
{
    qApp->clipboard()->setText(m_contextIndex.sibling(m_contextIndex.row(), 0).data().toString());
}

void VcsEventWidgetPrivate::diffToPrevious()
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( m_contextIndex );
    KDevelop::VcsRevision prev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Previous);
    KDevelop::VcsJob* job = m_iface->diff( m_url, prev, ev.revision() );

    VcsDiffWidget* widget = new VcsDiffWidget( job );
    widget->setRevisions( prev, ev.revision() );
    KDialog* dlg = new KDialog( q );
    
    widget->connect(widget, SIGNAL(destroyed(QObject*)), dlg, SLOT(deleteLater()));
    
    dlg->setCaption( i18n("Difference To Previous") );
    dlg->setButtons( KDialog::Ok );
    dlg->setMainWidget( widget );
    dlg->show();
}

void VcsEventWidgetPrivate::diffRevisions()
{
    QModelIndexList l = m_ui->eventView->selectionModel()->selectedRows();
    KDevelop::VcsEvent ev1 = m_logModel->eventForIndex( l.first() );
    KDevelop::VcsEvent ev2 = m_logModel->eventForIndex( l.last() );
    KDevelop::VcsJob* job = m_iface->diff( m_url, ev1.revision(), ev2.revision() );

    VcsDiffWidget* widget = new VcsDiffWidget( job );
    widget->setRevisions( ev1.revision(), ev2.revision() );
    KDialog* dlg = new KDialog( q );

    widget->connect(widget, SIGNAL(destroyed(QObject*)), dlg, SLOT(deleteLater()));

    dlg->setCaption( i18n("Difference between Revisions") );
    dlg->setButtons( KDialog::Ok );
    dlg->setMainWidget( widget );
    dlg->show();
}

VcsEventWidget::VcsEventWidget( const KUrl& url, const VcsRevision& rev, KDevelop::IBasicVersionControl* iface, QWidget* parent )
    : QWidget(parent), d(new VcsEventWidgetPrivate(this) )
{
    d->m_iface = iface;
    d->m_url = url;
    d->m_ui = new Ui::VcsEventWidget();
    d->m_ui->setupUi(this);

    d->m_logModel= new VcsEventModel(iface, rev, url, this);
    d->m_ui->eventView->setModel( d->m_logModel );
    d->m_ui->eventView->sortByColumn(0, Qt::DescendingOrder);
    d->m_ui->eventView->setContextMenuPolicy( Qt::CustomContextMenu );
    QHeaderView* header = d->m_ui->eventView->header();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::Stretch );
    header->setResizeMode( 2, QHeaderView::ResizeToContents );
    header->setResizeMode( 3, QHeaderView::ResizeToContents );

    d->m_detailModel = new VcsItemEventModel(this);
    d->m_ui->itemEventView->setModel( d->m_detailModel );

    connect( d->m_ui->eventView, SIGNAL(clicked(QModelIndex)),
             this, SLOT(eventViewClicked(QModelIndex)) );
    connect( d->m_ui->eventView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
             this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect( d->m_ui->eventView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(eventViewCustomContextMenuRequested(QPoint)) );
}

VcsEventWidget::~VcsEventWidget()
{
    delete d->m_ui;
    delete d;
}

}


#include "moc_vcseventwidget.cpp"

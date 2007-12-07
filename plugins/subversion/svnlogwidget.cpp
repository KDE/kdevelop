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

#include "svnlogwidget.h"

#include <QHeaderView>
#include <QAction>

#include <kdebug.h>
#include <kmenu.h>

#include <vcsjob.h>
#include <iplugin.h>
#include <ibasicversioncontrol.h>
#include <vcsrevision.h>
#include <vcslocation.h>

#include "vcs/models/vcsitemeventmodel.h"
#include "vcs/models/vcseventmodel.h"
#include "svndiffwidget.h"

SvnLogWidget::SvnLogWidget( const KUrl& url, KDevelop::VcsJob *job, QWidget *parent )
    : QWidget(parent), Ui::SvnLogWidget(), m_job(job), m_url(url)
{

    Ui::SvnLogWidget::setupUi(this);

    m_logModel= new VcsEventModel(this);
    eventView->setModel( m_logModel );
    eventView->sortByColumn(0, Qt::DescendingOrder);
    eventView->setContextMenuPolicy( Qt::CustomContextMenu );
    QHeaderView* header = eventView->horizontalHeader();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::ResizeToContents );
    header->setResizeMode( 2, QHeaderView::ResizeToContents );
    header->setResizeMode( 3, QHeaderView::Stretch );

    m_detailModel = new VcsItemEventModel(this);
    itemEventView->setModel( m_detailModel );
    header = itemEventView->horizontalHeader();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::Stretch );
    header->setResizeMode( 2, QHeaderView::ResizeToContents );
    header->setResizeMode( 3, QHeaderView::Stretch );
    header->setResizeMode( 4, QHeaderView::ResizeToContents );

    connect( m_job, SIGNAL(resultsReady( KDevelop::VcsJob*) ),
             this, SLOT( jobReceivedResults( KDevelop::VcsJob* ) ) );
    m_job->start();
}
SvnLogWidget::~SvnLogWidget()
{
}

void SvnLogWidget::on_eventView_customContextMenuRequested( const QPoint &point )
{
    m_contextIndex = eventView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug(9500) << "contextMenu is not in TreeView";
        return;
    }

    KMenu menu( this );

    QAction* action = menu.addAction(i18n("Diff to previous revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(diffToPrevious()) );

    action = menu.addAction(i18n("Diff between revisions"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(diffRevisions()) );

    menu.exec( eventView->viewport()->mapToGlobal(point) );
}

void SvnLogWidget::on_eventView_clicked( const QModelIndex &index )
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( index );
    if( ev.revision().revisionType() != KDevelop::VcsRevision::Invalid )
    {
        message->setPlainText( ev.message() );
        m_detailModel->clear();
        m_detailModel->addItemEvents( ev.items() );
    }else
    {
        message->clear();
        m_detailModel->clear();
    }
}

void SvnLogWidget::jobReceivedResults( KDevelop::VcsJob* job )
{
    if( job == m_job )
    {
        QList<QVariant> l = job->fetchResults().toList();
        QList<KDevelop::VcsEvent> newevents;
        foreach( QVariant v, l )
        {
            if( qVariantCanConvert<KDevelop::VcsEvent>( v ) )
            {
                newevents << qVariantValue<KDevelop::VcsEvent>( v );
            }
        }
        m_logModel->addEvents( newevents );
    }
}


void SvnLogWidget::diffToPrevious()
{
    KDevelop::IPlugin* plugin = m_job->vcsPlugin();
    if( plugin )
    {
        KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>();
        if( iface )
        {
            KDevelop::VcsEvent ev = m_logModel->eventForIndex( m_contextIndex );
            KDevelop::VcsRevision prev;
            prev.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Previous ),
                                   KDevelop::VcsRevision::Special );
            KDevelop::VcsLocation loc( m_url );

            KDevelop::VcsJob* job = iface->diff( loc, loc, prev, ev.revision() );


            SvnDiffDialog* dlg = new SvnDiffDialog( job );
            connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
            dlg->show();
        }
    }
}

void SvnLogWidget::diffRevisions()
{
    KDevelop::IPlugin* plugin = m_job->vcsPlugin();
    if( plugin )
    {
        KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>();
        if( iface )
        {
            QModelIndexList l = eventView->selectionModel()->selectedRows();
            KDevelop::VcsEvent ev1 = m_logModel->eventForIndex( l.first() );
            KDevelop::VcsEvent ev2 = m_logModel->eventForIndex( l.last() );
            KDevelop::VcsLocation loc( m_url );
            KDevelop::VcsJob* job = iface->diff( loc, loc, ev1.revision(), ev2.revision() );

            SvnDiffDialog* dlg = new SvnDiffDialog( job );
            connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
            dlg->show();
        }
    }
}


#include "svnlogwidget.moc"

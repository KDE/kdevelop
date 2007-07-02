/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
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

#include "outputwidget.h"
#include "ioutputviewmodel.h"

#include "standardoutputview.h"
#include <QtGui/QStandardItemModel>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QListView>
#include <QtGui/QToolButton>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>


OutputWidget::OutputWidget(QWidget* parent, StandardOutputView* view)
    : KTabWidget( parent ), m_outputView(view)
{
    m_closeButton = new QToolButton( this );
    connect( m_closeButton, SIGNAL( clicked() ),
             this, SLOT(closeActiveView() ) );
    m_closeButton->setIcon( KIcon("tab-remove") );
    m_closeButton->adjustSize();
    m_closeButton->setToolTip( i18n( "Close the currently active output view") );
    setCornerWidget( m_closeButton, Qt::TopRightCorner );
    connect( m_outputView, SIGNAL( modelChanged( int ) ),
             this, SLOT( changeModel( int ) ) );

    foreach( int id, m_outputView->registeredViews() )
    {
        changeModel( id );
    }

}

void OutputWidget::changeModel( int id )
{
    kDebug(9004) << "model changed for id:" << id << endl;
    if( m_listviews.contains( id ) )
        m_listviews[id]->setModel(m_outputView->registeredModel(id));
    else
    {
        kDebug(9004) << "creating listview" << endl;
        QListView* listview = new QListView(this);
        listview->setModel( m_outputView->registeredModel(id) );
        listview->setEditTriggers( QAbstractItemView::NoEditTriggers );
        m_listviews[id] = listview;
        m_widgetMap[listview] = id;
//         connect( listview, SIGNAL(activated(const QModelIndex&)),
//                  this, SIGNAL(activated(const QModelIndex&)));
        connect( listview, SIGNAL(clicked(const QModelIndex&)),
                 this, SLOT(activate(const QModelIndex&)));
        connect( listview, SIGNAL(activated(const QModelIndex&)),
                 this, SLOT( activate(const QModelIndex&) ) );
        addTab( listview, m_outputView->registeredTitle(id) );
    }
}

void OutputWidget::removeView( int id )
{
    if( m_listviews.contains( id ) )
    {
        QWidget* w = m_listviews[id];
        int idx = indexOf( w );
        if( idx != -1 )
        {
            removeTab( idx );
            m_widgetMap.remove( w );
            m_listviews.remove( id );
            delete w;
            emit viewRemoved( id );
        }
    }
}

void OutputWidget::closeActiveView()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    if( m_widgetMap.contains( widget ) )
    {
        int id = m_widgetMap[widget];
        if( m_outputView->closeBehaviour( id ) == KDevelop::IOutputView::AllowUserClose )
        {
            removeView( id );
        }else kDebug(9004) << "OOops, the view is not user closeable" << endl;
    }else kDebug(9004) << "OOops, the selected tab is not in our list??" << endl;
}

void OutputWidget::selectNextItem()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        QModelIndex index = iface->nextHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            iface->activate( index );
        }
    }
}

void OutputWidget::selectPrevItem()
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        QModelIndex index = iface->previousHighlightIndex( view->currentIndex() );
        if( index.isValid() )
        {
            view->setCurrentIndex( index );
            iface->activate( index );
        }
    }
}

void OutputWidget::activate(const QModelIndex& index)
{
    QWidget* widget = currentWidget();
    if( !widget )
        return;
    QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(widget);
    if( !view )
        return;

    QAbstractItemModel *absmodel = view->model();
    KDevelop::IOutputViewModel *iface = dynamic_cast<KDevelop::IOutputViewModel*>(absmodel);
    if( iface )
    {
        iface->activate( index );
    }
}

#include "outputwidget.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

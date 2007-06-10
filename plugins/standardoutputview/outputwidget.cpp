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
    connect( m_outputView, SIGNAL( modelChanged( const QString& ) ),
             this, SLOT( changeModel( const QString& ) ) );

    foreach( QString id, m_outputView->registeredViews() )
    {
        changeModel( id );
    }
}

void OutputWidget::changeModel(const QString& id )
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
        int num = addTab( listview, m_outputView->registeredTitle(id) );
        m_tabToIds[num] = id;
    }
}

void OutputWidget::removeView( const QString& id )
{
    if( m_listviews.contains( id ) )
    {
        QWidget* w = m_listviews[id];
        int idx = indexOf( w );
        if( idx != -1 )
        {
            removeTab( idx );
            delete w;
            m_tabToIds.remove( idx );
            m_listviews.remove( id );
        }
    }
}

void OutputWidget::closeActiveView()
{
    int idx = currentIndex();
    if( m_tabToIds.contains( idx ) )
    {
        QString id = m_tabToIds[currentIndex()];
        if( m_outputView->closeBehaviour( id ) == KDevelop::IOutputView::AllowUserClose )
        {
            QWidget* widget = currentWidget();
            removeTab( idx );
            delete widget;
            m_tabToIds.remove( idx );
            m_listviews.remove( id );
            emit viewRemoved( id );
        }else kDebug(9004) << "OOops, the view is not user closeable" << endl;
    }else kDebug(9004) << "OOops, the selected tab is not in our list??" << endl;
}

#include "outputwidget.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

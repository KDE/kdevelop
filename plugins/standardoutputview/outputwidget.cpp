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
#include <kmenu.h>
#include <kdebug.h>

OutputWidget::OutputWidget(QWidget* parent, StandardOutputView* view)
    : KTabWidget( parent ), m_outputView(view)
{
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
        addTab( listview, m_outputView->registeredTitle(id) );
    }
}

void OutputWidget::customContextMenuRequested( const QPoint & point )
{
    kDebug(9004) << "Custom context menu for outputview" << endl;
}
#include "outputwidget.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

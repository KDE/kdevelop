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

#include "svnannotationwidget.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <vcsjob.h>

#include "vcs/models/vcsannotationmodel.h"

SvnAnnotationWidget::SvnAnnotationWidget( const KUrl& url, KDevelop::VcsJob* job, QWidget* parent )
    : QWidget( parent ), m_job( job )
{
    setupUi( this );
    m_model = new VcsAnnotationModel( url );
    annotations->setModel( m_model );
    QHeaderView* header = annotations->horizontalHeader();
    header->setResizeMode(0, QHeaderView::ResizeToContents );
    header->setResizeMode(1, QHeaderView::ResizeToContents );
//     header->setResizeMode(2, QHeaderView::ResizeToContents );
//     header->setResizeMode(3, QHeaderView::ResizeToContents );
    header->setResizeMode(2, QHeaderView::Stretch );
//     annotations->setIndentation(-7);
    connect( job, SIGNAL( resultsReady( KDevelop::VcsJob* ) ),
             this, SLOT( addAnnotations( KDevelop::VcsJob* ) ) );
    m_job->start();
}

SvnAnnotationWidget::~SvnAnnotationWidget()
{
    delete m_model;
}

void SvnAnnotationWidget::addAnnotations( KDevelop::VcsJob* job )
{
    if( job == m_job )
    {
        QList<QVariant> result = job->fetchResults().toList();
        QList<KDevelop::VcsAnnotationLine> lines;
        foreach( QVariant v, result )
        {
            if( v.canConvert<KDevelop::VcsAnnotationLine>() )
            {
                lines << v.value<KDevelop::VcsAnnotationLine>();
            }
        }
        m_model->addLines( lines );
    }
}

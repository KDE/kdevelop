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

#include "vcsannotationwidget.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "../vcsjob.h"
#include "../models/vcsannotationmodel.h"
#include "../vcsannotation.h"
#include "ui_vcsannotationwidget.h"

namespace KDevelop
{
class VcsAnnotationWidgetPrivate
{
public:
    VcsAnnotationModel* m_model;
    KDevelop::VcsJob* m_job;
    Ui::VcsAnnotationWidget* m_ui;
    void addAnnotations(KDevelop::VcsJob* job)
    {
        if( job == m_job )
        {
            QList<QVariant> result = job->fetchResults().toList();
            QList<KDevelop::VcsAnnotationLine> lines;
            foreach( const QVariant &v, result )
            {
                if( v.canConvert<KDevelop::VcsAnnotationLine>() )
                {
                    lines << v.value<KDevelop::VcsAnnotationLine>();
                }
            }
            m_model->addLines( lines );
        }

    }
};

VcsAnnotationWidget::VcsAnnotationWidget( const KUrl& url, KDevelop::VcsJob* job, QWidget* parent )
    : QWidget( parent ), d( new VcsAnnotationWidgetPrivate)
{
    d->m_job = job;
    d->m_ui = new Ui::VcsAnnotationWidget();
    d->m_ui->setupUi( this );
    d->m_model = new VcsAnnotationModel( url );
    d->m_ui->annotations->setModel( d->m_model );
    QHeaderView* header = d->m_ui->annotations->horizontalHeader();
    header->setResizeMode(0, QHeaderView::ResizeToContents );
    header->setResizeMode(1, QHeaderView::ResizeToContents );
//     header->setResizeMode(2, QHeaderView::ResizeToContents );
//     header->setResizeMode(3, QHeaderView::ResizeToContents );
    header->setResizeMode(2, QHeaderView::Stretch );
//     annotations->setIndentation(-7);
    connect( d->m_job, SIGNAL(resultsReady(KDevelop::VcsJob*)),
             this, SLOT(addAnnotations(KDevelop::VcsJob*)) );
    d->m_job->setAutoDelete(false);
    ICore::self()->runController()->registerJob( d->m_job );
}

VcsAnnotationWidget::~VcsAnnotationWidget()
{
    delete d->m_model;
    delete d->m_ui;
    delete d;
}

}

#include "vcsannotationwidget.moc"

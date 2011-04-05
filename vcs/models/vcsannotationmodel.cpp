/***************************************************************************
 *   This file is part of KDevelop                                         *
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

#include "vcsannotationmodel.h"

#include "../vcsannotation.h"
#include "../vcsrevision.h"
#include "../vcsjob.h"

#include <QDateTime>
#include <QtGlobal>
#include <QBrush>
#include <QHash>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

namespace KDevelop
{

class VcsAnnotationModelPrivate
{
public:
    VcsAnnotationModelPrivate( VcsAnnotationModel* q_ ) : q(q_) {}
    KDevelop::VcsAnnotation m_annotation;
    QHash<KDevelop::VcsRevision,QBrush> m_brushes;
    VcsAnnotationModel* q;
    VcsJob* job;
    void addLines( KDevelop::VcsJob* job )
    {
        if( job == this->job )
        {
            foreach( const QVariant& v, job->fetchResults().toList() )
            {
                if( v.canConvert<KDevelop::VcsAnnotationLine>() )
                {
                    VcsAnnotationLine l = v.value<KDevelop::VcsAnnotationLine>();
                    if( !m_brushes.contains( l.revision() ) )
                    {
                        int r = ( float(qrand()) / RAND_MAX ) * 255;
                        int g = ( float(qrand()) / RAND_MAX ) * 255;
                        int b = ( float(qrand()) / RAND_MAX ) * 255;
                        m_brushes.insert( l.revision(), QBrush( QColor( r, g, b, 80 ) ) );
                    }
                    m_annotation.insertLine( l.lineNumber(), l );
                    emit q->lineChanged( l.lineNumber() );
                }
            }
        }
    }
};

VcsAnnotationModel::VcsAnnotationModel( VcsJob* job, const KUrl& url, QObject* parent )
    : d( new VcsAnnotationModelPrivate( this ) )
{
    setParent( parent );
    d->m_annotation.setLocation( url );
    d->job = job;
    qsrand( QDateTime().toTime_t() );
    connect( d->job, SIGNAL(resultsReady(KDevelop::VcsJob*)),SLOT(addLines(KDevelop::VcsJob*)) );
    ICore::self()->runController()->registerJob( d->job );
}
VcsAnnotationModel::~VcsAnnotationModel()
{
    delete d;
}

QVariant VcsAnnotationModel::data( int line, Qt::ItemDataRole role ) const
{
    if( line < 0 || !d->m_annotation.containsLine( line ) )
    {
        return QVariant();
    }

    KDevelop::VcsAnnotationLine aline = d->m_annotation.line( line );
    if( role == Qt::BackgroundRole )
    {
        return QVariant( d->m_brushes[aline.revision()] );
    } else if( role == Qt::DisplayRole )
    {
        return QVariant( aline.revision().revisionValue() );
    } else if( role == Qt::ToolTipRole )
    {
        return QVariant( i18n("Author: %1\nDate: %2\nCommit Message: %3", aline.author(), aline.date().toString(), aline.commitMessage() ) );
    }
    return QVariant();
}

VcsRevision VcsAnnotationModel::revisionForLine( int line ) const
{
    ///FIXME: update the annotation bar on edit/reload somehow
    ///BUG: https://bugs.kde.org/show_bug.cgi?id=269757
    if (!d->m_annotation.containsLine()) {
        return VcsRevision();
    }

    Q_ASSERT(line > 0 && d->m_annotation.containsLine( line ));
    return d->m_annotation.line( line ).revision();
}

}

#include "vcsannotationmodel.moc"

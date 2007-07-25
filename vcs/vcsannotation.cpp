/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "vcsannotation.h"

#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include "vcsrevision.h"
#include <kurl.h>

namespace KDevelop
{

class VcsAnnotationPrivate
{
public:
    QList<QString> authors;
    QList<QDateTime> dates;
    QList<QString> lines;
    KUrl location;
    QList<VcsRevision> revisions;
};
VcsAnnotation::VcsAnnotation()
    : d(new VcsAnnotationPrivate)
{
}

VcsAnnotation::VcsAnnotation( const VcsAnnotation& rhs )
    : d(new VcsAnnotationPrivate)
{
    d->authors = rhs.d->authors;
    d->dates = rhs.d->dates;
    d->lines = rhs.d->lines;
    d->location = rhs.d->location;
    d->revisions = rhs.d->revisions;
}

VcsAnnotation::~VcsAnnotation()
{
    delete d;
}

KUrl VcsAnnotation::location() const
{
    return d->location;
}

unsigned int VcsAnnotation::lineCount() const
{
    return d->lines.count();
}

QString VcsAnnotation::line( unsigned int linenum ) const
{
    int inum = linenum;
    if( inum < d->lines.count() )
        return d->lines.at( linenum );
    return QString();
}

VcsRevision VcsAnnotation::revision( unsigned int linenum ) const
{
    int inum = linenum;
    if( inum < d->revisions.count() )
        return d->revisions.at( linenum );
    return VcsRevision();
}

QString VcsAnnotation::author( unsigned int linenum ) const
{
    int inum = linenum;
    if( inum < d->authors.count() )
        return d->authors.at( linenum );
    return QString();
}

QDateTime VcsAnnotation::date( unsigned int linenum ) const
{
    int inum = linenum;
    if( inum < d->dates.count() )
        return d->dates.at( linenum );
    return QDateTime();
}

void VcsAnnotation::setLines( const QStringList& l )
{
    d->lines = l;
}

void VcsAnnotation::setAuthors( const QStringList& l )
{
    d->authors = l;
}

void VcsAnnotation::setDates( const QList<QDateTime>& l )
{
    d->dates = l;
}

void VcsAnnotation::setRevisions( const QList<KDevelop::VcsRevision>& l )
{
    d->revisions = l;
}

void VcsAnnotation::setLocation(const KUrl& u)
{
    d->location = u;
}

VcsAnnotation& VcsAnnotation::operator=( const VcsAnnotation& rhs)
{
    if(this == &rhs)
        return *this;
    d->authors = rhs.d->authors;
    d->revisions = rhs.d->revisions;
    d->location = rhs.d->location;
    d->lines = rhs.d->lines;
    d->dates = rhs.d->dates;
    return *this;
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

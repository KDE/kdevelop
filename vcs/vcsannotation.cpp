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
#include <QtCore/QHash>
#include <KUrl>

#include "vcsrevision.h"

namespace KDevelop
{

class VcsAnnotationPrivate
{
public:
    QHash<int, VcsAnnotationLine> lines;
    KUrl location;
};

class VcsAnnotationLinePrivate
{
public:
    QString author;
    QDateTime date;
    QString text;
    QString line;
    VcsRevision revision;
    QString message;
    int lineno;
};

VcsAnnotationLine::VcsAnnotationLine()
    : d( new VcsAnnotationLinePrivate )
{
    d->lineno = -1;
}

VcsAnnotationLine::VcsAnnotationLine( const VcsAnnotationLine& rhs )
    : d( new VcsAnnotationLinePrivate )
{
    d->author = rhs.d->author;
    d->line = rhs.d->line;
    d->revision = rhs.d->revision;
    d->lineno = rhs.d->lineno;
    d->date = rhs.d->date;
    d->text = rhs.d->text;
    d->message = rhs.d->message;
}

VcsAnnotationLine::~VcsAnnotationLine()
{
    delete d;
}

int VcsAnnotationLine::lineNumber() const
{
    return d->lineno;
}

QString VcsAnnotationLine::text() const
{
    return d->text;
}

QString VcsAnnotationLine::author() const
{
    return d->author;
}

VcsRevision VcsAnnotationLine::revision() const
{
    return d->revision;
}

QDateTime VcsAnnotationLine::date() const
{
    return d->date;
}

void VcsAnnotationLine::setLineNumber( int lineno )
{
    d->lineno = lineno;
}

void VcsAnnotationLine::setText( const QString& text )
{
    d->text = text;
}

void VcsAnnotationLine::setAuthor( const QString& author )
{
    d->author = author;
}

void VcsAnnotationLine::setRevision( const VcsRevision& revision )
{
    d->revision = revision;
}

void VcsAnnotationLine::setDate( const QDateTime& date )
{
    d->date = date;
}

VcsAnnotationLine& VcsAnnotationLine::operator=( const VcsAnnotationLine& rhs)
{
    if(this == &rhs)
        return *this;
    d->author = rhs.d->author;
    d->line = rhs.d->line;
    d->revision = rhs.d->revision;
    d->lineno = rhs.d->lineno;
    d->date = rhs.d->date;
    d->text = rhs.d->text;
    d->message = rhs.d->message;
    return *this;
}

QString VcsAnnotationLine::commitMessage() const
{
    return d->message;
}


void VcsAnnotationLine::setCommitMessage ( const QString& msg )
{
    d->message = msg;
}

VcsAnnotation::VcsAnnotation()
    : d(new VcsAnnotationPrivate)
{
}

VcsAnnotation::VcsAnnotation( const VcsAnnotation& rhs )
    : d(new VcsAnnotationPrivate)
{
    d->lines = rhs.d->lines;
    d->location = rhs.d->location;
}

VcsAnnotation::~VcsAnnotation()
{
    delete d;
}

KUrl VcsAnnotation::location() const
{
    return d->location;
}

int VcsAnnotation::lineCount() const
{
    return d->lines.count();
}

void VcsAnnotation::insertLine( int lineno, const VcsAnnotationLine& line )
{
    if( lineno < 0 )
    {
        return;
    }
    d->lines.insert( lineno, line );
}

void VcsAnnotation::setLocation(const KUrl& u)
{
    d->location = u;
}

VcsAnnotationLine VcsAnnotation::line( int lineno ) const
{
    return d->lines[lineno];
}

VcsAnnotation& VcsAnnotation::operator=( const VcsAnnotation& rhs)
{
    if(this == &rhs)
        return *this;
    d->location = rhs.d->location;
    d->lines = rhs.d->lines;
    return *this;
}

bool VcsAnnotation::containsLine( int lineno ) const
{
    return d->lines.contains( lineno );
}

}


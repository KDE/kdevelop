/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsannotation.h"

#include <QSharedData>
#include <QDateTime>
#include <QHash>
#include <QUrl>

#include "vcsrevision.h"

namespace KDevelop
{

class VcsAnnotationPrivate : public QSharedData
{
public:
    QHash<int, VcsAnnotationLine> lines;
    QUrl location;
};

class VcsAnnotationLinePrivate : public QSharedData
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
    : d(rhs.d)
{
}

VcsAnnotationLine::~VcsAnnotationLine() = default;

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

void KDevelop::VcsAnnotationLine::setRevision( const KDevelop::VcsRevision& revision )
{
    d->revision = revision;
}

void VcsAnnotationLine::setDate( const QDateTime& date )
{
    d->date = date;
}

VcsAnnotationLine& VcsAnnotationLine::operator=( const VcsAnnotationLine& rhs)
{
    d = rhs.d;
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
    : d(rhs.d)
{
}

VcsAnnotation::~VcsAnnotation() = default;

QUrl VcsAnnotation::location() const
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

void VcsAnnotation::clearLines()
{
    d->lines.clear();
}

void VcsAnnotation::setLocation(const QUrl& u)
{
    d->location = u;
}

VcsAnnotationLine VcsAnnotation::line( int lineno ) const
{
    return d->lines[lineno];
}

VcsAnnotation& VcsAnnotation::operator=( const VcsAnnotation& rhs)
{
    d = rhs.d;
    return *this;
}

bool VcsAnnotation::containsLine( int lineno ) const
{
    return d->lines.contains( lineno );
}

}


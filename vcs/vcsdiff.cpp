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

#include "vcsdiff.h"

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QHash>

namespace KDevelop
{

class VcsDiffPrivate
{
public:
    QHash<VcsLocation,QByteArray> leftBinaries;
    QHash<VcsLocation,QByteArray> rightBinaries;
    QHash<VcsLocation,QString> leftTexts;
    QHash<VcsLocation,QString> rightTexts;
    QUrl baseDiff;
    QString diff;
    VcsDiff::Type type;
    VcsDiff::Content content;
};

VcsDiff::VcsDiff()
    : d(new VcsDiffPrivate)
{
}

VcsDiff::~VcsDiff()
{
    delete d;
}

VcsDiff::VcsDiff( const VcsDiff& rhs )
    : d(new VcsDiffPrivate)
{
    d->leftBinaries = rhs.d->leftBinaries;
    d->rightBinaries = rhs.d->rightBinaries;
    d->leftTexts = rhs.d->leftTexts;
    d->rightTexts = rhs.d->rightTexts;
    d->diff = rhs.d->diff;
    d->type = rhs.d->type;
    d->content =  rhs.d->content;
    d->baseDiff =  rhs.d->baseDiff;
}

bool VcsDiff::isEmpty() const
{
    return d->diff.isEmpty() && d->leftBinaries.isEmpty() && d->rightBinaries.isEmpty()
                             && d->leftTexts.isEmpty() && d->rightTexts.isEmpty();
}

VcsDiff::Type VcsDiff::type() const
{
    return d->type;
}

VcsDiff::Content VcsDiff::contentType() const
{
    return d->content;
}

QHash<VcsLocation, QByteArray> VcsDiff::leftBinaries() const
{
    return d->leftBinaries;
}

QHash<VcsLocation, QByteArray> VcsDiff::rightBinaries() const
{
    return d->rightBinaries;
}


QHash<VcsLocation, QString> VcsDiff::leftTexts() const
{
    return d->leftTexts;
}

QHash<VcsLocation, QString> VcsDiff::rightTexts() const
{
    return d->rightTexts;
}

QString VcsDiff::diff() const
{
    return d->diff;
}


void VcsDiff::setDiff( const QString& s )
{
    d->diff = s;
}

void VcsDiff::addLeftBinary( const VcsLocation& loc, const QByteArray& b )
{
    d->leftBinaries[loc] = b;
}

void VcsDiff::addRightBinary( const VcsLocation& loc, const QByteArray& b )
{
    d->rightBinaries[loc] = b;
}

void VcsDiff::removeLeftBinary( const VcsLocation& loc )
{
    d->leftBinaries.remove( loc );
}

void VcsDiff::removeRightBinary( const VcsLocation& loc )
{
    d->rightBinaries.remove( loc );
}

void VcsDiff::addLeftText( const VcsLocation& loc, const QString& b )
{
    d->leftTexts[loc] = b;
}

void VcsDiff::addRightText( const VcsLocation& loc, const QString& b )
{
    d->rightTexts[loc] = b;
}

void VcsDiff::removeLeftText( const VcsLocation& loc )
{
    d->leftTexts.remove( loc );
}

void VcsDiff::removeRightText( const VcsLocation& loc )
{
    d->rightTexts.remove( loc );
}

void VcsDiff::setType( VcsDiff::Type t )
{
    d->type = t;
}

void VcsDiff::setContentType( VcsDiff::Content c )
{
    d->content = c;
}

VcsDiff& VcsDiff::operator=( const VcsDiff& rhs)
{
    if(this == &rhs)
        return *this;
    d->content = rhs.d->content;
    d->type = rhs.d->type;
    d->leftBinaries = rhs.d->leftBinaries;
    d->rightBinaries = rhs.d->rightBinaries;
    d->leftTexts = rhs.d->leftTexts;
    d->rightTexts = rhs.d->rightTexts;
    d->diff = rhs.d->diff;
    d->baseDiff = rhs.d->baseDiff;
    return *this;
}

QUrl VcsDiff::baseDiff() const
{
    return d->baseDiff;
}

void VcsDiff::setBaseDiff(const QUrl& url) const
{
    d->baseDiff=url;
}

void VcsDiff::clear()
{
//     d->content;
//     d->type;
    d->leftBinaries.clear();
    d->rightBinaries.clear();
    d->leftTexts.clear();
    d->rightTexts.clear();
    d->diff.clear();
    d->baseDiff.clear();
}


}


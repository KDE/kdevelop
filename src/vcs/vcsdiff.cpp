/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copysecond (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

namespace KDevelop
{

class VcsDiffPrivate
{
public:
    QByteArray firstBinary;
    QByteArray secondBinary;
    QString diff;
    QString firstText;
    QString secondText;
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
    d->firstBinary = rhs.d->firstBinary;
    d->secondBinary = rhs.d->secondBinary;
    d->firstText = rhs.d->firstText;
    d->secondText = rhs.d->secondText;
    d->diff = rhs.d->diff;
    d->type = rhs.d->type;
    d->content =  rhs.d->content;
}

VcsDiff::Type VcsDiff::type() const
{
    return d->type;
}

VcsDiff::Content VcsDiff::contentType() const
{
    return d->content;
}

QByteArray VcsDiff::firstBinary() const
{
    return d->firstBinary;
}

QByteArray VcsDiff::secondBinary() const
{
    return d->secondBinary;
}

QString VcsDiff::firstText() const
{
    return d->firstText;
}

QString VcsDiff::secondText() const
{
    return d->secondText;
}

QString VcsDiff::diff() const
{
    return d->diff;
}


void VcsDiff::setDiff( const QString& s )
{
    d->diff = s;
}

void VcsDiff::setFirstBinary( const QByteArray& b )
{
    d->firstBinary = b;
}

void VcsDiff::setSecondBinary( const QByteArray& b )
{
    d->secondBinary = b;
}

void VcsDiff::setFirstText( const QString& s )
{
    d->firstText = s;
}

void VcsDiff::setSecondText( const QString& s )
{
    d->secondText = s;
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
    d->secondText = rhs.d->secondText;
    d->secondBinary = rhs.d->secondBinary;
    d->firstText = rhs.d->firstText;
    d->firstBinary = rhs.d->firstBinary;
    d->diff = rhs.d->diff;
    return *this;
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

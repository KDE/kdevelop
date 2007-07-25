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

#include "vcsrevision.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>

namespace KDevelop
{

class VcsRevisionPrivate
{
    public:
        QString value;
        VcsRevision::RevisionType type;
        VcsRevision::RevisionSpecialType specialType;
        QMap<QString,QVariant> internalValues;
};

VcsRevision::VcsRevision()
    : d(new VcsRevisionPrivate)
{
}

VcsRevision::VcsRevision( const VcsRevision& rhs )
    : d(new VcsRevisionPrivate)
{
    d->value = rhs.d->value;
    d->internalValues = rhs.d->internalValues;
    d->type = rhs.d->type;
    d->specialType = rhs.d->specialType;
}

VcsRevision::~VcsRevision()
{
    delete d;
}

VcsRevision& VcsRevision::operator=( const VcsRevision& rhs)
{
    if(this == &rhs)
        return *this;
    d->value = rhs.d->value;
    d->specialType = rhs.d->specialType;
    d->type = rhs.d->type;
    d->internalValues = rhs.d->internalValues;
    return *this;
}

void VcsRevision::setRevisionValue( const QString& rev, VcsRevision::RevisionType type )
{
    d->value = rev;
    d->type = type;
}

VcsRevision::RevisionType VcsRevision::revisionType() const
{
    return d->type;
}

QString VcsRevision::revisionValue() const
{
    return d->value;
}

QStringList VcsRevision::keys() const
{
    return d->internalValues.keys();
}

QVariant VcsRevision::getValue( const QString& key ) const
{
    if( d->internalValues.contains(key) )
    {
        return d->internalValues[key];
    }
    return QVariant();
}

void VcsRevision::setValue( const QString& key, const QVariant& value )
{
    d->internalValues[key] = value;
}

void VcsRevision::setType( RevisionType t)
{
    d->type = t;
}

void VcsRevision::setSpecialType( RevisionSpecialType t)
{
    d->specialType = t;
}

void VcsRevision::setValue( const QString& v )
{
    d->value = v;
}

}
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "vcshelpers.h"

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace KDevelop
{

class VcsMappingPrivate
{
    public:
        QMap<QString,QPair<QString, VcsMapping::MappingFlag> > mapping;
};

VcsMapping::VcsMapping()
    : d(new VcsMappingPrivate)
{
}

VcsMapping::~VcsMapping()
{
    delete d;
}

VcsMapping::VcsMapping( const VcsMapping& rhs )
    : d(new VcsMappingPrivate)
{
    d->mapping = rhs.d->mapping;
}

void VcsMapping::addMapping( const QString& sourceLocation,
                    const QString& destinationLocation,
                    VcsMapping::MappingFlag recursion )
{
    QPair<QString,VcsMapping::MappingFlag> val = qMakePair(destinationLocation,recursion);
    d->mapping[sourceLocation] = val;
}

void VcsMapping::removeMapping( const QString& sourceLocation)
{
    if( d->mapping.contains(sourceLocation) )
        d->mapping.remove(sourceLocation);
}

QStringList VcsMapping::sourceLocations() const
{
    return d->mapping.keys();
}

QString VcsMapping::destinationLocation( const QString& sourceLocation ) const
{
    if( d->mapping.contains( sourceLocation ) )
        return d->mapping[sourceLocation].first;
    return QString();
}

VcsMapping::MappingFlag VcsMapping::mappingFlag( const QString& sourceLocation ) const
{
    if( d->mapping.contains( sourceLocation ) )
        return d->mapping[sourceLocation].second;
    return NonRecursive;
}


VcsMapping& VcsMapping::operator=( const VcsMapping& rhs)
{
    if(this == &rhs)
        return *this;
    d->mapping = rhs.d->mapping;
    return *this;
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

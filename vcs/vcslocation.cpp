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

#include "vcslocation.h"
#include <QtCore/QVariant>
#include <kurl.h>

namespace KDevelop
{

class VcsLocationPrivate
{
public:
    KUrl m_localUrl;
    QString m_repoLocation;
    VcsLocation::LocationType m_type;
};

VcsLocation::VcsLocation()
    : d(new VcsLocationPrivate)
{
    d->m_type = VcsLocation::LocalLocation;
}


VcsLocation::VcsLocation( const KUrl& u )
    : d(new VcsLocationPrivate)
{
    setLocalUrl( u );
}

VcsLocation::VcsLocation( const QString& s )
    : d(new VcsLocationPrivate)
{
    setRepositoryLocation( s );
}

VcsLocation::~VcsLocation()
{
    delete d;
}

VcsLocation::VcsLocation( const VcsLocation& rhs )
    : d(new VcsLocationPrivate)
{
    d->m_type = rhs.d->m_type;
    d->m_localUrl = rhs.d->m_localUrl;
    d->m_repoLocation = rhs.d->m_repoLocation;
}

VcsLocation& VcsLocation::operator=( const VcsLocation& rhs )
{
    if( &rhs == this )
        return *this;
    d->m_type = rhs.d->m_type;
    d->m_localUrl = rhs.d->m_localUrl;
    d->m_repoLocation = rhs.d->m_repoLocation;
    return *this;
}

KUrl VcsLocation::localUrl() const
{
    return d->m_localUrl;
}
QString VcsLocation::repositoryLocation() const
{
    return d->m_repoLocation;
}

VcsLocation::LocationType VcsLocation::type() const
{
    return d->m_type;
}

void VcsLocation::setLocalUrl( const KUrl& url )
{
    d->m_repoLocation = QString();
    d->m_type = VcsLocation::LocalLocation;
    d->m_localUrl = url;
}
void VcsLocation::setRepositoryLocation( const QString& location )
{
    d->m_repoLocation = location;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl = KUrl();
}

bool VcsLocation::operator==( const KDevelop::VcsLocation& rhs )
{
    return( type() == rhs.type()
            && repositoryLocation() == rhs.repositoryLocation()
            && localUrl() == rhs.localUrl() );
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

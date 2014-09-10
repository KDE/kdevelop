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

#include "vcsstatusinfo.h"

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace KDevelop
{

class VcsStatusInfoPrivate
{
public:
    int state;
    QUrl url;
};

VcsStatusInfo::VcsStatusInfo()
    : d( new VcsStatusInfoPrivate)
{
    d->state = VcsStatusInfo::ItemUnknown;
}

VcsStatusInfo::~VcsStatusInfo()
{
    delete d;
}

VcsStatusInfo::VcsStatusInfo( const VcsStatusInfo& rhs )
    : d(new VcsStatusInfoPrivate)
{
    d->state = rhs.d->state;
    d->url = rhs.d->url;
}

VcsStatusInfo& VcsStatusInfo::operator=( const VcsStatusInfo& rhs)
{
    if(this == &rhs)
        return *this;
    d->state = rhs.d->state;
    d->url = rhs.d->url;
    return *this;
}

bool VcsStatusInfo::operator==( const KDevelop::VcsStatusInfo& rhs) const
{
    return ( d->state == rhs.d->state && d->url == rhs.d->url );
}

bool VcsStatusInfo::operator!=( const KDevelop::VcsStatusInfo& rhs) const
{
    return !(operator==(rhs));
}

void VcsStatusInfo::setUrl( const QUrl& url )
{
    d->url = url;
}

void VcsStatusInfo::setExtendedState( int newstate )
{
    d->state = newstate;
}

void VcsStatusInfo::setState( VcsStatusInfo::State state )
{
    d->state = state;
}

int VcsStatusInfo::extendedState() const
{
    return d->state;
}

QUrl VcsStatusInfo::url() const
{
    return d->url;
}

VcsStatusInfo::State VcsStatusInfo::state() const
{
    return VcsStatusInfo::State(d->state);
}


}


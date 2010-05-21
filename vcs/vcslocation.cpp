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
    QString m_repoModule;
    QString m_repoBranch;
    QString m_repoTag;
    QVariant m_userData;
};

VcsLocation::VcsLocation()
    : d(new VcsLocationPrivate)
{}

VcsLocation::VcsLocation( const KUrl& u )
    : d(new VcsLocationPrivate)
{
    setLocalUrl( u );
}

VcsLocation::~VcsLocation()
{
    delete d;
}

VcsLocation::VcsLocation( const VcsLocation& rhs )
    : d(new VcsLocationPrivate)
{
    d->m_localUrl = rhs.d->m_localUrl;
    d->m_repoModule = rhs.d->m_repoModule;
    d->m_repoBranch = rhs.d->m_repoBranch;
    d->m_repoTag = rhs.d->m_repoTag;
    d->m_userData = rhs.d->m_userData;
}

VcsLocation& VcsLocation::operator=( const VcsLocation& rhs )
{
    if( &rhs == this )
        return *this;
    d->m_localUrl = rhs.d->m_localUrl;
    d->m_repoModule = rhs.d->m_repoModule;
    d->m_repoBranch = rhs.d->m_repoBranch;
    d->m_repoTag = rhs.d->m_repoTag;
    d->m_userData = rhs.d->m_userData;
    return *this;
}

KUrl VcsLocation::localUrl() const
{
    return d->m_localUrl;
}
QString VcsLocation::repositoryServer() const
{
    return d->m_localUrl.host();
}

VcsLocation::LocationType VcsLocation::type() const
{
    return d->m_localUrl.isLocalFile() ? LocalLocation : RepositoryLocation;
}

bool VcsLocation::isValid() const
{
    return true;
}

void VcsLocation::setLocalUrl( const KUrl& url )
{
    d->m_repoModule.clear();
    d->m_repoBranch.clear();
    d->m_repoTag.clear();
    d->m_localUrl = url;
}
void VcsLocation::setRepositoryServer( const QString& location )
{
    d->m_localUrl.clear();
    d->m_localUrl.setHost(location);
}

bool VcsLocation::operator==( const KDevelop::VcsLocation& rhs )
{
    return(
            localUrl() == rhs.localUrl()
            && repositoryModule() == rhs.repositoryModule()
            && repositoryBranch() == rhs.repositoryBranch()
            && repositoryTag() == rhs.repositoryTag()
            && userData() == rhs.userData() );
}


QString VcsLocation::repositoryModule( ) const
{
    return d->m_repoModule;
}

QString VcsLocation::repositoryTag( ) const
{
    return d->m_repoTag;
}

QString VcsLocation::repositoryBranch( ) const
{
    return d->m_repoBranch;
}

QString VcsLocation::repositoryPath( ) const
{
    return d->m_localUrl.path();
}

void VcsLocation::setRepositoryModule( const QString & module )
{
    d->m_repoModule = module;
}

void VcsLocation::setRepositoryBranch( const QString & branch )
{
    d->m_repoBranch = branch;
}

void VcsLocation::setRepositoryTag( const QString & tag )
{
    d->m_repoTag = tag;
}

void VcsLocation::setRepositoryPath( const QString & path )
{
    d->m_localUrl.setPath(path);
}

QVariant VcsLocation::userData( ) const
{
    return d->m_userData;
}

void VcsLocation::setUserData( const QVariant& data )
{
    d->m_userData = data;
}

}


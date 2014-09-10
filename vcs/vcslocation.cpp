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

namespace KDevelop
{

class VcsLocationPrivate
{
public:
    QUrl m_localUrl;
    QString m_repoServer;
    QString m_repoPath;
    QString m_repoModule;
    QString m_repoBranch;
    QString m_repoTag;
    VcsLocation::LocationType m_type;
    QVariant m_userData;
};

VcsLocation::VcsLocation()
    : d(new VcsLocationPrivate)
{
    d->m_type = VcsLocation::LocalLocation;
}


VcsLocation::VcsLocation( const QUrl& u )
    : d(new VcsLocationPrivate)
{
    setLocalUrl( u );
}

VcsLocation::VcsLocation( const QString& s )
    : d(new VcsLocationPrivate)
{
    setRepositoryServer( s );
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
    d->m_repoServer = rhs.d->m_repoServer;
    d->m_repoPath = rhs.d->m_repoPath;
    d->m_repoModule = rhs.d->m_repoModule;
    d->m_repoBranch = rhs.d->m_repoBranch;
    d->m_repoTag = rhs.d->m_repoTag;
    d->m_userData = rhs.d->m_userData;
}

VcsLocation& VcsLocation::operator=( const VcsLocation& rhs )
{
    if( &rhs == this )
        return *this;
    d->m_type = rhs.d->m_type;
    d->m_localUrl = rhs.d->m_localUrl;
    d->m_repoServer = rhs.d->m_repoServer;
    d->m_repoPath = rhs.d->m_repoPath;
    d->m_repoModule = rhs.d->m_repoModule;
    d->m_repoBranch = rhs.d->m_repoBranch;
    d->m_repoTag = rhs.d->m_repoTag;
    d->m_userData = rhs.d->m_userData;
    return *this;
}

QUrl VcsLocation::localUrl() const
{
    return d->m_localUrl;
}
QString VcsLocation::repositoryServer() const
{
    return d->m_repoServer;
}

VcsLocation::LocationType VcsLocation::type() const
{
    return d->m_type;
}

bool VcsLocation::isValid() const
{
    return( ( d->m_localUrl.isValid()
              && d->m_type == VcsLocation::LocalLocation )
            || ( !d->m_repoServer.isEmpty()
                && d->m_type == VcsLocation::RepositoryLocation ) );
}

void VcsLocation::setLocalUrl( const QUrl& url )
{
    d->m_repoServer.clear();
    d->m_repoModule.clear();
    d->m_repoBranch.clear();
    d->m_repoTag.clear();
    d->m_repoPath.clear();
    d->m_type = VcsLocation::LocalLocation;
    d->m_localUrl = url;
}
void VcsLocation::setRepositoryServer( const QString& location )
{
    d->m_repoServer = location;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl = QUrl();
}

bool VcsLocation::operator==( const KDevelop::VcsLocation& rhs )
{
    return( type() == rhs.type()
            && repositoryServer() == rhs.repositoryServer()
            && localUrl() == rhs.localUrl()
            && repositoryPath() == rhs.repositoryPath()
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
    return d->m_repoPath;
}

void VcsLocation::setRepositoryModule( const QString & module )
{
    d->m_repoModule = module;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl.clear();
}

void VcsLocation::setRepositoryBranch( const QString & branch )
{
    d->m_repoBranch = branch;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl.clear();
}

void VcsLocation::setRepositoryTag( const QString & tag )
{
    d->m_repoTag = tag;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl.clear();
}

void VcsLocation::setRepositoryPath( const QString & path )
{
    d->m_repoPath = path;
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl.clear();
}


QVariant VcsLocation::userData( ) const
{
    return d->m_userData;
}

void VcsLocation::setUserData( const QVariant& data )
{
    d->m_type = VcsLocation::RepositoryLocation;
    d->m_localUrl.clear();
    d->m_userData = data;
}

}


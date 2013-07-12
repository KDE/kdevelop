/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VCSLOCATION_H
#define KDEVPLATFORM_VCSLOCATION_H

#include "vcsexport.h"
#include <QtCore/QList>
#include <QtCore/QHash>

#include <KDE/KUrl>


namespace KDevelop
{
/**
 * Denotes a local or repository location for a Vcs system.
 *
 * For the RepositoryLocation type, most of the information
 * is vcs-specific.
 */
class KDEVPLATFORMVCS_EXPORT VcsLocation
{
public:
    enum LocationType
    {
        LocalLocation = 0      /**< this is a local location */,
        RepositoryLocation = 1 /**< this is a repository location */
    };

    VcsLocation();
    VcsLocation( const KUrl& );
    VcsLocation( const QString& );
    ~VcsLocation();
    VcsLocation( const VcsLocation& );
    VcsLocation& operator=( const VcsLocation& );

    /**
     * @returns Local url if this location is a LocalLocation
     */
    KUrl localUrl() const;

    /**
     * Returns a string for the repository, usually this identifies the server.
     * @returns a vcs-implementation-specific string identifying the server
     */
    QString repositoryServer() const;
    /**
     * Returns the module or module path inside the server.
     * @returns a vcs-implementation-specific string identifying the module
     */
    QString repositoryModule() const;
    /**
     * Identifies the tag which this location belongs to.
     * @returns a vcs-implementation-specific string identifying the tag
     */
    QString repositoryTag() const;
    /**
     * Identifies the branch to which this location belongs to.
     * @returns a vcs-implementation-specific string identifying the branch
     */
    QString repositoryBranch() const;
    /**
     * This can define a path relative to the module. This is used
     * when identifying a subdirectory or file inside a repository location
     * @returns a path relative to module
     */
    QString repositoryPath() const;
    /**
     * @returns the type of this location
     */
    LocationType type() const;

    /**
     * Set the local url for this location, automatically sets the type to LocalLocation
     * @param url the local url
     */
    void setLocalUrl( const KUrl& url );

    /**
     * Set the server string for this location, automatically sets the type to RepositoryLocation
     */
    void setRepositoryServer( const QString& );
    /**
     * Set the module for this location, automatically sets the type to RepositoryLocation
     */
    void setRepositoryModule( const QString& );
    /**
     * Set the branch string for this location, automatically sets the type to RepositoryLocation
     */
    void setRepositoryBranch( const QString& );
    /**
     * Set the tag string for this location, automatically sets the type to RepositoryLocation
     */
    void setRepositoryTag( const QString& );
    /**
     * Set the path for this location, automatically sets the type to RepositoryLocation
     */
    void setRepositoryPath( const QString& );

    /**
     * Allows to add vcs-specific data to this location.
     * Automatically sets the type to RepositoryLocation
     * @param data the vcs-specific data
     */
    void setUserData( const QVariant& );

    /**
     * retrieve vcs-specific data
     */
    QVariant userData() const;

    bool operator==( const KDevelop::VcsLocation& );

    bool isValid() const;

private:
    class VcsLocationPrivate* d;
};

inline uint qHash( const KDevelop::VcsLocation& loc )
{
    if( loc.type() == KDevelop::VcsLocation::LocalLocation )
    {
        return qHash(loc.localUrl());
    }else
    {
        return qHash(loc.repositoryServer());
    }
}

inline bool operator==( const KDevelop::VcsLocation& lhs, const KDevelop::VcsLocation& rhs )
{
    return( lhs.type() == rhs.type()
            && lhs.repositoryServer() == rhs.repositoryServer()
            && lhs.localUrl() == rhs.localUrl() );
}

}

Q_DECLARE_METATYPE( KDevelop::VcsLocation )

#endif


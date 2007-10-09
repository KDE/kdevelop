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

#ifndef KDEVVCSLOCATION_H
#define KDEVVCSLOCATION_H

#include <vcsexport.h>
#include <QtCore/QString>
#include <kurl.h>

class KUrl;
class QString;

namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT VcsLocation
{
public:
    enum LocationType
    {
        LocalLocation = 0,
        RepositoryLocation = 1
    };

    VcsLocation();
    VcsLocation( const KUrl& );
    VcsLocation( const QString& );
    ~VcsLocation();
    VcsLocation( const VcsLocation& );
    VcsLocation& operator=( const VcsLocation& );

    KUrl localUrl() const;
    QString repositoryLocation() const;
    VcsLocation::LocationType type() const;

    void setLocalUrl( const KUrl& );
    void setRepositoryLocation( const QString& );

    bool operator==( const KDevelop::VcsLocation& );

private:
    class VcsLocationPrivate* d;
};

}

inline uint qHash( const KDevelop::VcsLocation& loc )
{
    if( loc.type() == KDevelop::VcsLocation::LocalLocation )
    {
        return qHash(loc.localUrl());
    }else
    {
        return qHash(loc.repositoryLocation());
    }
}

inline bool operator==( const KDevelop::VcsLocation& lhs, const KDevelop::VcsLocation& rhs )
{
    return( lhs.type() == rhs.type()
            && lhs.repositoryLocation() == rhs.repositoryLocation()
            && lhs.localUrl() == rhs.localUrl() );
}

Q_DECLARE_METATYPE( KDevelop::VcsLocation )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

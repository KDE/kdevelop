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

#ifndef VCSMAPPING_H
#define VCSMAPPING_H

#include <QtCore/QVariant>

#include <kurl.h>

#include "vcsexport.h"

class QString;
class QStringList;

namespace KDevelop
{

class VcsLocation;

/**
 * This class stores mappings of vcs locations. Usually one location is a local
 * location, the other is a repository location.
 */
class KDEVPLATFORMVCS_EXPORT VcsMapping
{
public:
    enum MappingFlag
    {
        Recursive = 1,
        NonRecursive = 2
    };

    VcsMapping();
    virtual ~VcsMapping();
    VcsMapping(const VcsMapping&);

    void addMapping( const VcsLocation& sourceLocation,
                     const VcsLocation& destinationLocation,
                     MappingFlag recursion );
    void removeMapping( const VcsLocation& sourceLocation);
    QList<VcsLocation> sourceLocations() const;
    VcsLocation destinationLocation( const VcsLocation& sourceLocation ) const;
    MappingFlag mappingFlag( const VcsLocation& sourceLocation ) const;

    VcsMapping& operator=( const VcsMapping& rhs);
private:
    class VcsMappingPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsMapping )

#endif


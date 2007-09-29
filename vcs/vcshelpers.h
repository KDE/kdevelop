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

#ifndef VCSHELPERS_H
#define VCSHELPERS_H

#include <QtCore/QVariant>

#include <kurl.h>

#include <vcsexport.h>

class QString;
class QStringList;

namespace KDevelop
{

class VcsStatusInfo
{
public:
    /**
     * Status of a local file
     */
    enum State
    {
        ItemUnknown      = 0   /**<No VCS information about a file is known (or file is not under VCS control).*/,
        ItemUpToDate     = 1   /**<Item was updated or it is already at up to date version.*/,
        ItemAdded        = 2   /**<Item was added to the repository but not committed.*/,
        ItemModified     = 3   /**<Item was modified locally.*/,
        ItemDeleted      = 4   /**<Item is scheduled to be deleted. */,
        ItemHasConflicts = 5   /**<Local version has conflicts that need to be resolved before commit.*/,
	ItemUserState    = 1000 /**special states for individual vcs implementations should use this as base.*/
    };

    VcsStatusInfo();
    virtual ~VcsStatusInfo();
    VcsStatusInfo(const VcsStatusInfo&);

    KUrl url() const;
    void setUrl( const KUrl& );

    VcsStatusInfo::State state() const;
    void setState( VcsStatusInfo::State ); 

    int extendedState() const;
    void setExtendedState( int ); 
    VcsStatusInfo& operator=( const VcsStatusInfo& rhs);
private:
    class VcsStatusInfoPrivate* d;
};


/**
 * Small container class that has a mapping of
 * repository-location -> local location including a recursion flag
 *
 * Just a convenient API around QMap\<KUrl, QPair\<KUrl, MappingFlag\>\>
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

    void addMapping( const QString& sourceLocation,
                     const QString& destinationLocation,
                     MappingFlag recursion );
    void removeMapping( const QString& sourceLocation);
    QStringList sourceLocations() const;
    QString destinationLocation( const QString& sourceLocation ) const;
    MappingFlag mappingFlag( const QString& sourceLocation ) const;

    VcsMapping& operator=( const VcsMapping& rhs);
private:
    class VcsMappingPrivate* const d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsMapping )
Q_DECLARE_METATYPE( KDevelop::VcsStatusInfo )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

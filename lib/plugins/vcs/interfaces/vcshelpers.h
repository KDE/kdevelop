/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

namespace KDevelop
{

enum VCSState
{
    Unknown        /**<No VCS information about a file is known (or file is not under VCS control).*/,
    Added          /**<File was added to the repository but not commited.*/,
    Uptodate       /**<File was updated or it is already at up to date version.*/,
    Modified       /**<File was modified locally.*/,
    Conflict       /**<Local version has conflicts that need to be resolved before commit.*/,
    Deleted        /**<File or Directory is scheduled to be deleted. */ ,
};

class Revision
{
public:
    enum RevisionType
    {
        GlobalNumber    /**<Global repository version when file was last changed./*/,
        FileNumber      /**<File version number, may be the same as GlobalNumber.*/,

        Date,
        Range,
        Keyword,
        Branch,
        Tag,
        PluginSpecific

    };
    setRevisionValue( const QVariant& rev, RevisionType );
    setSourceRevision( const Revision& );
    setTargetRevision( const Revision& );
    RevisionType revisionType() const;
    QString revisionValue() const;
    Revision sourceRevision() const;
    Revision targetRevision() const;
};

/**
 * Small container class that has a mapping of
 * repository-location -> local location including a recursion flag
 *
 * Just a convenient API around QMap<KUrl, QPair<KUrl, RecursionMode>>
 *
 */
class VCSMapping
{
public:
    enum MappingFlag
    {
        Recursive = 1,
        NonRecursive = 2
    };
    Q_DECLARE_FLAGS( MappingFlags, MappingFlag )

    void addMapping( const KUrl& repositoryLocation, const KUrl& localLocation,
               MappingFlags recursion );
    void removeMapping( const KUrl& repositoryLocation);
    KUrl::List repositoryLocations();
    KUrl localLocation( const KUrl& repositoryLocation );
    MappingFlags mappingFlags( const KUrl& repositoryLocation ) const;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::MappingFlags )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

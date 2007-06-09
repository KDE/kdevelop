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

#ifndef VCSREVISION_H
#define VCSREVISION_H

#include "vcsexport.h"
#include <QtCore/QVariant>
class QStringList;
class QString;

namespace KDevelop
{

/**
 * Encapsulates a vcs revision number, date or range of revisions
 */
class KDEVPLATFORMVCS_EXPORT VcsRevision
{
public:

    /**
     * @note Not all VCS's support both FileNumber and GlobalNumber. For those
     * that don't, asking for one may give you the other, therefore you should
     * check which is returned. For example, CVS does not support GlobalNumber,
     * and Subversion does not support FileNumber, while Perforce supports both.
     */
    enum RevisionType
    {
        Special         /**<One of the special versions in RevisionSpecialType.*/,
        GlobalNumber    /**<Global repository version when item was last changed.*/,
        FileNumber      /**<Item's independent version number.*/,
        Date,
    };
    enum RevisionSpecialType
    {
        Head            /**<Latest revision in the repository.*/,
        Working         /**<The local copy (including any changes made).*/,
        Base            /**<The repository source of the local copy.*/,
        Previous        /**<The version prior the other one (only valid in functions that take two revisions).*/,
    };

    VcsRevision();
    virtual ~VcsRevision();

    VcsRevision( const VcsRevision& );

    VcsRevision& operator=( const VcsRevision& );

    /**
     * Set the value of this revision
     */
    void setRevisionValue( const QString& rev, RevisionType type );

    /**
     * returns the type of the revision
     */
    RevisionType revisionType() const;

    /**
     * return the value of this revision
     */
    QString revisionValue() const;

protected:
    /**
     * Get the keys that make up the internal data of this revision instance
     */
    QStringList keys() const;
    /**
     * get the value for a given key, this retrieves internal data and is
     * meant to be used by subclasses
     */
    QVariant getValue( const QString& key ) const;
    /**
     * change the value of the given internal data
     */
    void setValue( const QString& key, const QVariant& value );

    /**
     * write methods for subclasses to easily set the type and value
     */
    void setType( RevisionType t);
    void setSpecialType( RevisionSpecialType t);
    void setValue( const QString& );

private:
    class VcsRevisionPrivate* const d;
};

}

Q_DECLARE_METATYPE(KDevelop::VcsRevision)

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

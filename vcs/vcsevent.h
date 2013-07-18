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

#ifndef KDEVPLATFORM_VCSEVENT_H
#define KDEVPLATFORM_VCSEVENT_H

#include <QtCore/QVariant>

#include "vcsexport.h"

class QString;
class QDateTime;
template <typename T> class QList;

namespace KDevelop
{
class VcsRevision;

/**
 * Small container class that contains information about a history event of a
 * single repository item.
 */
class KDEVPLATFORMVCS_EXPORT VcsItemEvent
{
public:
    /**
     * Class that tells you what happened to a given repository location in a
     * specific revision.
     *
     * Combinations of some of the flags are possible, for example Add|Modified,
     * Copy|Modified or Merge|Modified, or when returned from VcsEvent::actions().
     */
    enum Action
    {
        Added            = 1<<0 /**< Item was added. */,
        Deleted          = 1<<1 /**< Item was deleted. */,
        Modified         = 1<<2 /**< Item was modified, for example by editing. */,
        Copied           = 1<<3 /**< Item was copied. */,
        Merged           = 1<<4 /**< Item had changes merged into it. */,
        ContentsModified = 1<<5 /**< Directory was not changed (only contents changed). */,
        Replaced         = 1<<6 /**< Item was replaced. */
    };
    Q_DECLARE_FLAGS( Actions, Action )

    VcsItemEvent();
    virtual ~VcsItemEvent();
    VcsItemEvent(const VcsItemEvent& );

    QString repositoryLocation() const;
    QString repositoryCopySourceLocation() const; // may be empty
    VcsRevision repositoryCopySourceRevision() const; // may be invalid, even if rCSL is not
    Actions actions() const;

    void setRepositoryLocation( const QString& );
    void setRepositoryCopySourceLocation( const QString& );
    void setRepositoryCopySourceRevision( const KDevelop::VcsRevision& );
    void setActions( Actions );

    VcsItemEvent& operator=( const VcsItemEvent& rhs);
private:
    class VcsItemEventPrivate* const d;
};

/**
 * Small container class that contains information about a single revision.
 *
 * @note log() only returns information about the specific item that was asked
 * about. When working with a VCS that supports atomic commits (i.e. where a
 * revision might affect more than one item), use change() to retrieve
 * information about all items affected by a particular revision.
 */
class KDEVPLATFORMVCS_EXPORT VcsEvent
{
public:
    VcsEvent();
    virtual ~VcsEvent();
    VcsEvent( const VcsEvent& );

    VcsRevision revision() const;
    QString author() const;
    QDateTime date() const;
    QString message() const;
    QList<VcsItemEvent> items() const;

    void setRevision( const VcsRevision& );
    void setAuthor( const QString& );
    void setDate( const QDateTime& );
    void setMessage(const QString& );
    void setItems( const QList<VcsItemEvent>& );
    void addItem(const VcsItemEvent& item);
    VcsEvent& operator=( const VcsEvent& rhs);
private:
    class VcsEventPrivate* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::VcsItemEvent::Actions )
Q_DECLARE_METATYPE( KDevelop::VcsEvent )
Q_DECLARE_METATYPE( KDevelop::VcsItemEvent )
#endif


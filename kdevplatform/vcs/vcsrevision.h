/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSREVISION_H
#define KDEVPLATFORM_VCSREVISION_H

#include "vcsexport.h"

#include <QSharedDataPointer>
#include <QStringList>
#include <QVariant>

namespace KDevelop
{

/**
 * Encapsulates a vcs revision number, date or range of revisions.
 *
 * The type of the QVariant value depends on the type of the revision,
 * the following table lists the standard types and the according datatype
 * in the QVariant:
 *
 * <table>
 * <tr><th>Revision type</th><th>QVariant type</th></tr>
 * <tr><td>GlobalNumber</td><td>qlonglong/QString</td></tr>
 * <tr><td>FileNumber</td><td>qlonglong/QString</td></tr>
 * <tr><td>Date</td><td>QDateTime</td></tr>
 * <tr><td>Special</td><td>KDevelop::VcsRevision::RevisionSpecialType or int, see explanation below</td></tr>
 * </table>
 *
 * The vcs plugins need to register the Revision and RevisionSpecialType with
 * qRegisterMetaType.
 *
 * Also Users of this class should set RevisionSpecialType QVariant values via
 * @code
 * setRevisionValue(QVariant::fromValue<KDevelop::VcsRevision::RevisionSpecialType>(val), KDevelop::VcsRevision::Special);
 * @endcode
 * instead of
 * @code
 * setRevisionValue(QVariant::fromValue(val), KDevelop::VcsRevision::Special);
 * @endcode
 *
 * If the latter method is used the QVariant will be an Integer, which might not
 * be handled by the vcs plugin and is possibly ambiguous with the qlonglong
 * parameters.
 *
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
        Special = 0         /**< One of the special versions in RevisionSpecialType. */,
        GlobalNumber = 1    /**< Global repository version when item was last changed. */,
        FileNumber = 2      /**< Item's independent version number. */,
        Date = 3,           /**< The date of the revision to check out */
        Invalid = 4         /**< The type is not set, this is an invalid revision. */,
        UserType = 1000     /**< This should be used by subclasses as base for their own types. */
    };
    enum RevisionSpecialType
    {
        Head = 0                   /**< Latest revision in the repository. */,
        Working = 1                /**< The local copy (including any changes made). */,
        Base = 2                   /**< The repository source of the local copy. */,
        Previous = 3               /**< The version prior the other one (only valid in functions that take two revisions). */,
        Start = 4,                 /**< The first commit in a repository. */
        UserSpecialType = 1000     /**< This should be used by subclasses as base for their own special types. */
    };

    /**
     * Creates an invalid revision.
     */
    VcsRevision();
    virtual ~VcsRevision();

    VcsRevision( const VcsRevision& );

    VcsRevision& operator=( const VcsRevision& );

    /**
     * Set the value of this revision
     */
    void setRevisionValue( const QVariant& rev, RevisionType type );

    /**
     * returns the type of the revision
     */
    RevisionType revisionType() const;

    RevisionSpecialType specialType() const;

    /**
     * Return the value of this revision.
     *
     * See the class documentation for the different QVariant types
     */
    QVariant revisionValue() const;

    /**
     * This returns the value of the revision, suitable for displaying to the
     * user. For numbers it just returns the number converted to a string, for
     * the special types it returns the literal value of the special type and
     * for a datetime value it returns a localized string of the datetime value.
     */
    QString prettyValue() const;

    bool operator==( const KDevelop::VcsRevision&) const;

    /**
     * Helper function to create a vcs revision for one of the special types
     */
    static VcsRevision createSpecialRevision( KDevelop::VcsRevision::RevisionSpecialType type );
protected:
    /**
     * Get the keys that make up the internal data of this revision instance
     */
    QStringList keys() const;
    /**
     * get the value for a given key, this retrieves internal data and is
     * meant to be used by subclasses
     */
    QVariant value(const QString& key) const;
    /**
     * change the value of the given internal data
     */
    void setValue( const QString& key, const QVariant& value );

    /**
     * write methods for subclasses to easily set the type and value
     */
    void setType( RevisionType t);
    void setSpecialType( RevisionSpecialType t);
    void setValue( const QVariant& );


private:
    QSharedDataPointer<class VcsRevisionPrivate> d;
};

KDEVPLATFORMVCS_EXPORT size_t qHash(const KDevelop::VcsRevision& rev);
}

Q_DECLARE_METATYPE(KDevelop::VcsRevision)
Q_DECLARE_TYPEINFO(KDevelop::VcsRevision, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::VcsRevision::RevisionSpecialType)



#endif


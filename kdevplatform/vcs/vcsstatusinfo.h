/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSSTATUSINFO_H
#define KDEVPLATFORM_VCSSTATUSINFO_H

#include "vcsexport.h"
#include <QMetaType>
#include <QSharedDataPointer>

class QUrl;

namespace KDevelop
{

/**
 *
 * Class that encapsulates status information
 * for one local url.
 *
 * The extendedState functions allow to transport
 * extended status information
 *
 * Note for VCS implementations:
 * If you want to use this class in queued signal/slot connections
 * you should call qRegisterMetaType<KDevelop::VcsStatusInfo>()
 * in the constructor of the plugin class
 */
class KDEVPLATFORMVCS_EXPORT VcsStatusInfo
{
public:
    /**
     * Status of a local file
     */
    enum State
    {
        ItemUnknown       = 0    /**< No VCS information about a file is known (or file is not under VCS control). */,
        ItemUpToDate      = 1    /**< Item was updated or it is already at up to date version. */,
        ItemAdded         = 2    /**< Item was added to the repository but not committed. */,
        ItemModified      = 3    /**< Item was modified locally. */,
        ItemDeleted       = 4    /**< Item is scheduled to be deleted. */,
        ItemHasConflicts  = 8    /**< Local version has conflicts that need to be resolved before commit. */,

        ItemUserState     = 1000 /**< special states for individual vcs implementations should use this as base. */,

    };

    VcsStatusInfo();
    virtual ~VcsStatusInfo();
    VcsStatusInfo(const VcsStatusInfo&);

    /**
     * retrieves the url of this status information item
     * @return the url
     */
    QUrl url() const;
    /**
     * Change the url of this status information item
     * @param url the url
     */
    void setUrl( const QUrl& url );

    VcsStatusInfo::State state() const;
    void setState( VcsStatusInfo::State );

    /**
     * @returns the extended state which may be set by individual vcs implementations
     */
    int extendedState() const;

    /**
     * Individual vcs implementations may use this function to set a special vcs status.
     * They should also always set an appropriate basic state, if possible.
     */
    void setExtendedState( int );

    VcsStatusInfo& operator=( const VcsStatusInfo& rhs);
    bool operator==( const KDevelop::VcsStatusInfo& rhs) const;
    bool operator!=( const KDevelop::VcsStatusInfo& rhs) const;

private:
    QSharedDataPointer<class VcsStatusInfoPrivate> d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsStatusInfo )
Q_DECLARE_TYPEINFO( KDevelop::VcsStatusInfo, Q_MOVABLE_TYPE );

KDEVPLATFORMVCS_EXPORT QDebug operator<<(QDebug s, const KDevelop::VcsStatusInfo& statusInfo);

#endif


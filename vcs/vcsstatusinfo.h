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

#ifndef KDEVPLATFORM_VCSSTATUSINFO_H
#define KDEVPLATFORM_VCSSTATUSINFO_H

#include <QtCore/QVariant>
#include <QUrl>

#include "vcsexport.h"

class QString;
class QStringList;

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
        ItemUserState     = 1000 /**< special states for individual vcs implementations should use this as base. */
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
    void setUrl( const QUrl& );

    VcsStatusInfo::State state() const;
    void setState( VcsStatusInfo::State );

    int extendedState() const;
    void setExtendedState( int );

    VcsStatusInfo& operator=( const VcsStatusInfo& rhs);
    bool operator==( const KDevelop::VcsStatusInfo& rhs) const;
    bool operator!=( const KDevelop::VcsStatusInfo& rhs) const;

private:
    class VcsStatusInfoPrivate* d;
};

}

Q_DECLARE_METATYPE( KDevelop::VcsStatusInfo )

#endif


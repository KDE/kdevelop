/* KDevelop
 *
 * Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
 * Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>g>
 * Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
 * Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
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

#include "iversioncontrol.h"

#include <QtCore/QList>
#include <kurl.h>

namespace KDevelop
{
class VcsFileInfoPrivate
{
public:
    /**The FULL file or directory path.*/
    KUrl filePath;
    /**The working revision number.*/
    QString workRevision;
    /**The repository revision number.*/
    QString repoRevision;
    /**The state of a file.*/
    VcsFileInfo::VcsFileState state;
};


VcsFileInfo::VcsFileInfo()
    : d( new VcsFileInfoPrivate )
{
}

VcsFileInfo::VcsFileInfo( const KUrl &fn, const QString& workRev,
                          const QString& repoRev, VcsFileInfo::VcsFileState aState )
    : d( new VcsFileInfoPrivate )
{
    d->filePath= fn;
    d->workRevision = workRev;
    d->repoRevision = repoRev;
    d->state = aState;
}

VcsFileInfo::VcsFileInfo( const VcsFileInfo &rhs )
    :d( new VcsFileInfoPrivate )
{
    d->filePath = rhs.filePath();
    d->workRevision = rhs.workingCopyRevision();
    d->repoRevision = rhs.repositoryRevision();
    d->state = rhs.state();
}

VcsFileInfo& VcsFileInfo::operator = ( const VcsFileInfo &rhs )
{
    d->filePath = rhs.filePath();
    d->workRevision = rhs.workingCopyRevision();
    d->repoRevision = rhs.repositoryRevision();
    d->state = rhs.state();
    return *this;
}

VcsFileInfo::~VcsFileInfo()
{
    delete d;
}

KUrl VcsFileInfo::filePath() const
{
    return d->filePath;
}

QString VcsFileInfo::workingCopyRevision() const
{
    return d->workRevision;
}

QString VcsFileInfo::repositoryRevision() const
{
    return d->repoRevision;
}

VcsFileInfo::VcsFileState VcsFileInfo::state() const
{
    return d->state;
}

QString VcsFileInfo::toString() const
{
    return '(' + d->filePath.fileName() + ", " + d->workRevision + ", " +
            d->repoRevision + ", " + state2String( d->state ) + ')';
}

QString VcsFileInfo::state2String( VcsFileInfo::VcsFileState state )
{
    switch (state)
    {
        case Added: return "added";
        case Uptodate: return "up-to-date";
        case Modified: return "modified";
        case Conflict: return "conflict";
        case Sticky: return "sticky";
        case NeedsPatch: return "needs update";
        case NeedsCheckout: return "needs check-out";
        case Directory: return "directory";
        case Deleted: return "deleted";
        case Replaced: return "replaced";
        case Unknown:
        default:
            return "unknown";
    }
}
}
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

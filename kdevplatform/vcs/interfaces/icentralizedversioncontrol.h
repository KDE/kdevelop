/***************************************************************************
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef KDEVPLATFORM_ICENTRALIZEDVERSIONCONTROL_H
#define KDEVPLATFORM_ICENTRALIZEDVERSIONCONTROL_H

#include "ibasicversioncontrol.h"

namespace KDevelop
{

/**
* This is the interface that all Centralized Version Control plugins need to implement. 
* None of the methods in this interface are optional.
*
* This only works on a local checkout from the repository, if your plugin should
* offer functionality that works solely on the server see the
* IRepositoryVersionControl interface
*
*/
class ICentralizedVersionControl : public KDevelop::IBasicVersionControl
{
public:
    virtual ~ICentralizedVersionControl(){}

    /**
     * this is for files only, it makes a file editable, this may be a no-op
     */
    virtual VcsJob* edit( const QUrl& localLocation ) = 0;

    /**
     * this is for files only, it makes a file un-editable, this may be a no-op
     * This is different from revert because it doesn't change the content of the
     * file (it may fail if the file has changed).
     */
    virtual VcsJob* unedit( const QUrl& localLocation ) = 0;

    /**
     * retrieves status information for a file or dir recursive is only
     * active for directories
     */

    /**
     * gives the revision of file/dir, that is the revision to which this files
     * was updated when update() was run the last time
     */
    virtual VcsJob* localRevision( const QUrl& localLocation,
                                   VcsRevision::RevisionType ) = 0;

    /**
     * The following two methods are part of the basic interface so other plugins
     * can depend on them, for example the appwizard. These two don't need a
     * valid VCS-dir, in fact they should be invoked with a local directory that
     * is either empty or contains a non-VCed project
     */

    /**
     * take a mapping of local to repository locations and import that into the repository
     */
    virtual VcsJob* import(const QString & commitMessage, const QUrl & sourceDirectory, const VcsLocation & destinationRepository) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ICentralizedVersionControl, "org.kdevelop.ICentralizedVersionControl" )

#endif

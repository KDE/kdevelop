/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

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
    ~ICentralizedVersionControl() override = default;

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

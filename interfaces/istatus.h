/***************************************************************************
 *   Copyright 2007 Hamish Rodda <rodda@kde.org>                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef ISTATUS_H
#define ISTATUS_H

#include <interfacesexport.h>
#include <iextension.h>

namespace KDevelop {

/**
 * An interface for plugins, languages etc. to provide status updates.
 */
class KDEVPLATFORMINTERFACES_EXPORT IStatus
{
public:
    virtual ~IStatus();

    /**
     * Return a name for the status object
     */
    virtual QString statusName() const = 0L;
    
Q_SIGNALS:
    /**
     * Request the current message for this plugin to be cleared.
     */
    void clearMessage();

    /**
     * Request a status \a message to be shown for this plugin, with a given \a timeout.
     *
     * \param message Message to display
     * \param timeout Timeout in miliseconds, or pass 0 for no timeout.
     */
    void showMessage(const QString & message, int timeout = 0);

    /**
     * Hide the progress bar.
     */
    void hideProgress();

    /**
     * Show a progress bar, with the given \a percentage.
     */
    void showProgress(int minimum, int maximum, int value);
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IStatus, "org.kdevelop.IStatus" )
Q_DECLARE_INTERFACE( KDevelop::IStatus, "org.kdevelop.IStatus" )

#endif


/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/



#ifndef _KDEVAPPFRONTEND_H_
#define _KDEVAPPFRONTEND_H_

#include <qstringlist.h>
#include "kdevplugin.h"

/**
 * \short The interface to a app frontend.
 *
 * This interface is responsible for handling the running of an application in KDevelop.
 * Currently, this interface defines ways to do the following:
 * \li Check if the application is running
 * \li Execute the application
 * \li Stop the currently running application
 * \li Control the output view as seen in the 'Application' tool dock
 */
class KDevAppFrontend : public KDevPlugin
{
    Q_OBJECT

public:

    KDevAppFrontend( const QString& pluginName, const QString& icon, QObject *parent=0, const char *name=0 );
    ~KDevAppFrontend();

    /**
     * Returns whether the application is currently running.
     */
    virtual bool isRunning() = 0;

public slots:
    /**
     * The component shall start to execute an app-like command.
     * Running the application is always asynchronous.
     * If directory is empty it will use the user's home directory.
     * If inTerminal is true, the program is started in an external
     * konsole.
     */
    virtual void startAppCommand(const QString &directory, const QString &program, bool inTerminal) = 0;
    /**
     * Stop the currently running application
     */
    virtual void stopApplication() = 0;
    /**
     * Inserts a string into the view.
     */
    virtual void insertStdoutLine(const QString &line) = 0;
    /**
     * Inserts a string into the view marked as stderr output
     * (colored in the current implementation).
     */
    virtual void insertStderrLine(const QString &line) = 0;
    /**
     * Clears the output view
     */
    virtual void clearView() = 0;
};

#endif

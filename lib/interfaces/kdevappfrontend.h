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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVAPPFRONTEND_H
#define KDEVAPPFRONTEND_H

#include <qstringlist.h>
#include <kdevplugin.h>
#include "kdevexport.h"
/**
@file kdevappfrontend.h
Application frontend interface.
*/

/**
Application frontend interface. 
This interface is responsible for handling the running of an application in KDevelop.
Currently, this interface defines ways to do the following:
- check if the application is running;
- execute the application;
- stop the currently running application;
- control the output view as seen in the 'Application' tool dock.

Instances that implement this interface are available through extension architecture:
@code
KDevAppFrontend *apf = extension<KDevAppFrontend>("KDevelop/AppFrontend");
if (apf) {
    // do something
} else {
    // fail
}
@endcode
@sa KDevPlugin::extension method documentation.
*/
class KDEVINTERFACES_EXPORT KDevAppFrontend : public KDevPlugin
{
    Q_OBJECT

public:
    /**Constructor.
    @param info Important information about the plugin - plugin internal and generic
    (GUI) name, description, a list of authors, etc. That information is used to show
    plugin information in various places like "about application" dialog, plugin selector
    dialog, etc. Plugin does not take ownership on info object, also its lifetime should
    be equal to the lifetime of the plugin.
    @param parent The parent object for the plugin. Parent object must implement @ref KDevApi
    interface. Otherwise the plugin will not be constructed.*/
    KDevAppFrontend(const KDevPluginInfo *info, QObject *parent=0)
        :KDevPlugin(info, parent) {}

    /**@return Whether the application is currently running.*/
    virtual bool isRunning() = 0;

public slots:
    /**
     * The component shall start to execute an app-like command.
     * Running the application is always asynchronous.
     * @param directory The working directory to start the app in, 
     * if empty then the user's home directory is used.
     * @param program A program to start.
     * @param inTerminal If true then the program is started in an external konsole.
     */
    virtual void startAppCommand(const QString &directory, const QString &program, bool inTerminal) = 0;
    
    /**
     * Stops the currently running application.
     */
    virtual void stopApplication() = 0;
    
    /**
     * Inserts a string into the application output view.
     * @param line A string to insert.
     */
    virtual void insertStdoutLine(const QString &line) = 0;
    
    /**
     * Inserts a string into the application output view marked as stderr output
     * (usually colored).
     * @param line An error string to insert.
     */
    virtual void insertStderrLine(const QString &line) = 0;
    
    /**
     * Clears the output view.
     */
    virtual void clearView() = 0;
};

#endif

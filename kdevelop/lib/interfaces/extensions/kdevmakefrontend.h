/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVMAKEFRONTEND_H
#define KDEVMAKEFRONTEND_H

#include <qstringlist.h>
#include <kdevplugin.h>

/**
@file kdevmakefrontend.h
Make frontend interface.
*/

/**
KDevelop make frontend interface.
This is the abstract base class for plugins that are able to run "make"
or similar commands to build a project, api documentation, etc.

Instances that implement this interface are available through extension architecture:
@code
KDevMakeFrontend *mf = extension<KDevMakeFrontend>("KDevelop/MakeFrontend");
if (mf) {
    // do something
} else {
    // fail
}
@endcode
@sa KDevPlugin::extension method documentation.
*/
class KDevMakeFrontend : public KDevPlugin
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
    interface. Otherwise the plugin will not be constructed.
    @param name The internal name which identifies the plugin.*/
    KDevMakeFrontend(const KDevPluginInfo *info, QObject *parent=0, const char *name=0 )
        :KDevPlugin(info, parent, name ? name : "KDevMakeFrontend") {}

    /**@return The widget where the make output is shown.*/
    virtual QWidget* widget() { return 0L; }

    /**The component shall start to execute a make-like command.
     * Commands are always asynchronous. You can submit several jobs
     * without caring about another job already running. There are
     * executed in the order in which they are submitted. If one of
     * then fails, all following jobs are dropped.
     * You should not make any assumptions about the directory in which
     * the command is started. If the command depends on that, put and
     * explicit 'cd' into the command. 
     * @param dir A starting directory to find files when parsing compiler error
     * messages.
     * @param command A shell command to execute.
     */
    virtual void queueCommand(const QString &dir, const QString &command) = 0;
    
    /**@return Whether the application is currently running.*/
    virtual bool isRunning() = 0;
    
    /**Advices to synchronize the settings from KConfig because they were changed externally.*/
    virtual void updateSettingsFromConfig() = 0;

signals:
    /**
     * Only emitted if the command was succesfully finished.
     */
    void commandFinished(const QString &command);
    
    /**
     * Emitted if a command failed.
     */
    void commandFailed(const QString &command);
};

#endif

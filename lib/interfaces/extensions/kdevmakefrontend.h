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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
/**
 * The interface to a make frontend
 */

#ifndef _KDEVMAKEFRONTEND_H_
#define _KDEVMAKEFRONTEND_H_

#include <qstringlist.h>
#include "kdevplugin.h"


class KDevMakeFrontend : public KDevPlugin
{
    Q_OBJECT

public:

    KDevMakeFrontend( const QString& pluginName, const QString& icon, QObject *parent=0, const char *name=0 );
    ~KDevMakeFrontend();

    virtual QWidget* widget() { return 0L; }

    /**
     * The component shall start to execute a make-like command.
     * Commands are always asynchronous. You can submit several jobs
     * without caring about another job already running. There are
     * executed in the order in which they are submitted. If one of
     * then fails, all following jobs are dropped.
     * You should not make any assumptions about the directory in which
     * the command is started. If the command depends on that, put and
     * explicit 'cd' into the command. The parameter dir is interpreted
     * as a starting directory to find files when parsing compiler error
     * messages.
     */
    virtual void queueCommand(const QString &dir, const QString &command) = 0;
    /**
     * Returns whether the application is currently running.
     */
    virtual bool isRunning() = 0;
    /**
     * Advices to synchronize the settings from KConfig because they've changed externally.
     */
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

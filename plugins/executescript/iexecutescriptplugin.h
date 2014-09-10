/* KDevelop
 *
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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

#ifndef KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_IEXECUTESCRIPTPLUGIN_H

#include <QObject>
#include <QtCore/QString>

namespace KDevelop
{
class ILaunchConfiguration;
}

class KJob;
class QUrl;
class QStringList;

/**
@author Andreas Pakulat
@author Niko Sams
*/

class IExecuteScriptPlugin
{
public:
    virtual ~IExecuteScriptPlugin() {}

    virtual QString interpreter( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QUrl script( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QString remoteHost( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QStringList arguments( KDevelop::ILaunchConfiguration*, QString& ) const = 0;
    virtual QUrl workingDirectory( KDevelop::ILaunchConfiguration* ) const = 0;
    virtual QString environmentGroup( KDevelop::ILaunchConfiguration* ) const = 0;
    //virtual bool useTerminal( KDevelop::ILaunchConfiguration* ) const = 0;
    virtual QString scriptAppConfigTypeId() const = 0;
    virtual int outputFilterModeId( KDevelop::ILaunchConfiguration* ) const = 0;
    virtual bool runCurrentFile( KDevelop::ILaunchConfiguration* ) const = 0;
};

Q_DECLARE_INTERFACE( IExecuteScriptPlugin, "org.kdevelop.IExecuteScriptPlugin" )

#endif


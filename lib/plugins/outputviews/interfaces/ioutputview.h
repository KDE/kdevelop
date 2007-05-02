/* KDevelop Output View
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef KDEVIOUTPUTVIEW_H
#define KDEVIOUTPUTVIEW_H

#include "iextension.h"

class KUrl;
class QString;
template <typename T1, typename T2> class QMap;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

// Idea for later: Let the output view work on executable commands
// class IExecutableCommand
// {
// public:
//     virtual ~IExecutableCommand(){}
//     virtual void setWorkingDir(const KUrl&) = 0;
//     virtual void setCommandList(const QStringList&) = 0;
//     virtual void setEnvironment(const QMap<QString, QString>&) = 0;
//     virtual QMap<QString, QString> environment() const = 0;
//     virtual QStringList commandlist() const = 0;
//     virtual KUrl workingDirectory() const = 0;
// };

class IOutputView
{
public:

    virtual ~IOutputView() {}

public:
    /**
     * Execute the Command in a K3Process and capture the output in
     * a new tab with the command as title
     */
    virtual void queueCommand( const KUrl& workingdir, const QStringList&, const QMap<QString, QString>& ) = 0;

    /**
     * Register a new Tab for Outputting logging information, this can be used
     * for example to log things that the VCS plugin does
     */
    virtual void registerLogView( const QString& id, const QString& title ) = 0;

    /**
     * Add the line or lines to the registered tab identified via title
     */
    virtual void appendLine( const QString& id, const QString& line ) = 0;
    virtual void appendLines( const QString& id, const QStringList& lines ) = 0;

Q_SIGNALS:
    virtual void commandFinished( const QString& id ) = 0;
    virtual void commandFailed( const QString& id ) = 0;
};
}
KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IOutputView, "org.kdevelop.IOutputView" )
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;


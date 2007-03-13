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
#include <QtDesigner/QAbstractExtensionFactory>

class KUrl;
class QString;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

// Idea for later: Let the output view work on executable commands
// class ExecutableCommand
// {
// public:
//     ExecutableCommand();
//     void setWorkingDir(const KUrl&);
//     void setCommandList(const QStringList&);
//     void setEnvironment(const QStringList&);
//     QStringList environment() const;
//     QStringList commandlist() const;
//     KUrl workingDirectory() const;
// private:
//     struct ExecutableCommandPrivate* const d;
// };

class IOutputView
{
public:

    virtual ~IOutputView() {}

public:
    virtual void queueCommand( const KUrl&, const QStringList&, const QStringList& ) = 0;

Q_SIGNALS:
    virtual void commandFinished( const QStringList& ) = 0;
    virtual void commandFailed( const QStringList& ) = 0;
};
}
KDEV_DECLARE_EXTENSION_INTERFACE( KDevelop, IOutputView, "org.kdevelop.IOutputView" )
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;


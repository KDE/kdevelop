/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Niko Sams <niko.sams@gmail.com>                        *
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


#ifndef KDEVPLATFORM_IDEBUGCONTROLLER_H
#define KDEVPLATFORM_IDEBUGCONTROLLER_H

#include <QtCore/QObject>
#include "interfacesexport.h"

namespace KDevelop {

class VariableCollection;
class BreakpointModel;
class FrameStackModel;
class IDebugSession;

/** Top level debugger object. Exists as long as KDevelop exists
    and holds some global debugger state, like breakpoints.
    Also holds the IDebugSession for the specific application
    that is being debugged.
*/
class KDEVPLATFORMINTERFACES_EXPORT IDebugController : public QObject
{
    Q_OBJECT
public:
    IDebugController(QObject *parent = 0);
    virtual ~IDebugController();

    /** Each plugin using the debugger actions needs to call this function to setup the gui */
    virtual void initializeUi() = 0;

    virtual void addSession(IDebugSession* session) = 0;
    
    /** Return the current debug session. At present, only
        one session may be active at a time.  */
    virtual IDebugSession *currentSession() = 0;

    virtual BreakpointModel *breakpointModel() = 0;
    virtual VariableCollection *variableCollection() = 0;

Q_SIGNALS:
    void currentSessionChanged(KDevelop::IDebugSession* session);
};

}

#endif

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


#ifndef IDEBUGCONTROLLER_H
#define IDEBUGCONTROLLER_H

#include <QtCore/QObject>
#include "interfacesexport.h"

namespace KDevelop {
class ContextMenuExtension;
class Context;
class IDebugSession;

class KDEVPLATFORMINTERFACES_EXPORT IDebugController : public QObject
{
    Q_OBJECT
public:
    IDebugController(QObject *parent = 0);
    virtual ~IDebugController();

    virtual void addSession(IDebugSession* session) = 0;
    virtual ContextMenuExtension contextMenuExtension( Context* context ) = 0;

Q_SIGNALS:
    void sessionAdded(IDebugSession* session);
};

}

#endif

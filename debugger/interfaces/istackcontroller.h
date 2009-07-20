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

#ifndef KDEV_ISTACKCONTROLLER_H
#define KDEV_ISTACKCONTROLLER_H

#include <QObject>

#include "../debuggerexport.h"
#include "idebugsession.h"

namespace KDevelop {

class FrameStackModel;
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT IStackController : public QObject
{
    Q_OBJECT
public:
    IStackController(IDebugSession *session);

    virtual void fetchThreads() = 0;
    virtual void fetchFrames(int threadNumber, int from, int to) = 0;

protected:
    FrameStackModel *frameStackModel();
};

}

#endif

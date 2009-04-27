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

#include "idebugsession.h"

namespace KDevelop {


IDebugSession::IDebugSession()
    : m_job(0), m_breakpointController(0)
{
}

IDebugSession::~IDebugSession()
{
}

KJob *IDebugSession::job() const
{
    return m_job;
}

bool IDebugSession::isRunning() const
{
    DebuggerState s = state();
    return (s == ActiveState || s == PausedState);
}

KDevelop::IBreakpointController* IDebugSession::breakpointController()
{
    return m_breakpointController;
}


}

#include "idebugsession.moc"

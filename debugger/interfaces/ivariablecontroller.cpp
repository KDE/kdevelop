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

#include "ivariablecontroller.h"

#include "idebugsession.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"
#include "../variable/variablecollection.h"

namespace KDevelop {

    
IVariableController::IVariableController(IDebugSession* parent)
    : QObject(parent)
{
    connect(parent, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)),
             SLOT(stateChanged(KDevelop::IDebugSession::DebuggerState)));
}

VariableCollection* IVariableController::variableCollection()
{
    return ICore::self()->debugController()->variableCollection();
}


void IVariableController::stateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::EndedState) {
        // Remove all locals.
        variableCollection()->locals()->clear();

        KDevelop::Variable::markAllDead();
    }
}

}

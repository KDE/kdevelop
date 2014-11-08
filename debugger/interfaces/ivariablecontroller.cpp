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
#include "util/debug.h"
#include "iframestackmodel.h"


namespace KDevelop {


IVariableController::IVariableController(IDebugSession* parent)
    : QObject(parent), m_activeThread(-1), m_activeFrame(-1)
{
    connect(parent, &IDebugSession::stateChanged,
            this, &IVariableController::stateChanged);
}

VariableCollection* IVariableController::variableCollection()
{
    if (!ICore::self()) return 0;
    return ICore::self()->debugController()->variableCollection();
}

IDebugSession* IVariableController::session() const
{
    return static_cast<IDebugSession*>(parent());
}

void IVariableController::stateChanged(IDebugSession::DebuggerState state)
{
    if (!ICore::self() || ICore::self()->shuttingDown()) {
        return;
    }

    if (state == IDebugSession::ActiveState) {
        //variables are now outdated, update them
        m_activeThread = -1;
        m_activeFrame = -1;
    } else if (state == IDebugSession::EndedState || state == IDebugSession::NotStartedState) {
        // Remove all locals.
        foreach (Locals *l, variableCollection()->allLocals()) {
            l->deleteChildren();
            l->setHasMore(false);
        }

        for (int i=0; i < variableCollection()->watches()->childCount(); ++i) {
            Variable *var = dynamic_cast<Variable*>(variableCollection()->watches()->child(i));
            if (var) {
                var->setInScope(false);
            }
        }
    }
}

void IVariableController::updateIfFrameOrThreadChanged()
{
    IFrameStackModel *sm = session()->frameStackModel();
    if (sm->currentThread() != m_activeThread || sm->currentFrame() != m_activeFrame) {
        m_activeThread = sm->currentThread();
        m_activeFrame = sm->currentFrame();
        variableCollection()->root()->resetChanged();
        update();
    }
}

void IVariableController::handleEvent(IDebugSession::event_t event)
{
    if (!variableCollection()) return;

    switch (event) {
    case IDebugSession::thread_or_frame_changed:
        qCDebug(DEBUGGER) << m_autoUpdate;
        if (!(m_autoUpdate & UpdateLocals)) {
            foreach (Locals *l, variableCollection()->allLocals()) {
                if (!l->isExpanded() && !l->childCount()) {
                    l->setHasMore(true);
                }
            }
        }
        if (m_autoUpdate != UpdateNone) {
            updateIfFrameOrThreadChanged();
        }
        break;

    default:
        break;
    }
}

void IVariableController::setAutoUpdate(QFlags<UpdateType> autoUpdate)
{
    IDebugSession::DebuggerState state = session()->state();
    m_autoUpdate = autoUpdate;
    qCDebug(DEBUGGER) << m_autoUpdate;
    if (m_autoUpdate != UpdateNone && state == IDebugSession::PausedState) {
        update();
    }
}

QFlags<IVariableController::UpdateType> IVariableController::autoUpdate()
{
    return m_autoUpdate;
}

}


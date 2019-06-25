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
#include <debug.h>
#include "iframestackmodel.h"


namespace KDevelop {

class IVariableControllerPrivate
{
public:
    QFlags<IVariableController::UpdateType> autoUpdate;
    int activeThread = -1;
    int activeFrame = -1;
};

IVariableController::IVariableController(IDebugSession* parent)
    : QObject(parent)
    , d_ptr(new IVariableControllerPrivate)
{
    connect(parent, &IDebugSession::stateChanged,
            this, &IVariableController::stateChanged);
}

IVariableController::~IVariableController() = default;

VariableCollection* IVariableController::variableCollection()
{
    if (!ICore::self()) return nullptr;
    return ICore::self()->debugController()->variableCollection();
}

IDebugSession* IVariableController::session() const
{
    return static_cast<IDebugSession*>(parent());
}

void IVariableController::stateChanged(IDebugSession::DebuggerState state)
{
    Q_D(IVariableController);

    if (!ICore::self() || ICore::self()->shuttingDown()) {
        return;
    }

    if (state == IDebugSession::ActiveState) {
        //variables are now outdated, update them
        d->activeThread = -1;
        d->activeFrame = -1;
    } else if (state == IDebugSession::EndedState || state == IDebugSession::NotStartedState) {
        // Remove all locals.
        const auto locals = variableCollection()->allLocals();
        for (Locals* l : locals) {
            l->deleteChildren();
            l->setHasMore(false);
        }

        for (int i=0; i < variableCollection()->watches()->childCount(); ++i) {
            auto *var = qobject_cast<Variable*>(variableCollection()->watches()->child(i));
            if (var) {
                var->setInScope(false);
            }
        }
    }
}

void IVariableController::updateIfFrameOrThreadChanged()
{
    Q_D(IVariableController);

    IFrameStackModel *sm = session()->frameStackModel();
    if (sm->currentThread() != d->activeThread || sm->currentFrame() != d->activeFrame) {
        variableCollection()->root()->resetChanged();
        update();
    }
}

void IVariableController::handleEvent(IDebugSession::event_t event)
{
    Q_D(IVariableController);

    if (!variableCollection()) return;

    switch (event) {
    case IDebugSession::thread_or_frame_changed:
        qCDebug(DEBUGGER) << d->autoUpdate;
        if (!(d->autoUpdate & UpdateLocals)) {
            const auto locals = variableCollection()->allLocals();
            for (Locals* l : locals) {
                if (!l->isExpanded() && !l->childCount()) {
                    l->setHasMore(true);
                }
            }
        }
        if (d->autoUpdate != UpdateNone) {
            updateIfFrameOrThreadChanged();
        }

        // update our cache of active thread/frame regardless of d->autoUpdate
        // to keep them synced when user currently hides the variable list
        d->activeThread = session()->frameStackModel()->currentThread();
        d->activeFrame = session()->frameStackModel()->currentFrame();
        break;

    default:
        break;
    }
}

void IVariableController::setAutoUpdate(QFlags<UpdateType> autoUpdate)
{
    Q_D(IVariableController);

    IDebugSession::DebuggerState state = session()->state();
    d->autoUpdate = autoUpdate;
    qCDebug(DEBUGGER) << d->autoUpdate;
    if (d->autoUpdate != UpdateNone && state == IDebugSession::PausedState) {
        update();
    }
}

QFlags<IVariableController::UpdateType> IVariableController::autoUpdate()
{
    Q_D(IVariableController);

    return d->autoUpdate;
}

}


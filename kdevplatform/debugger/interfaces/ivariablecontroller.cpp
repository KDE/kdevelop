/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

void IVariableController::handleEvent(IDebugSession::event_t event)
{
    Q_D(IVariableController);

    if (!variableCollection()) return;

    if (event != IDebugSession::thread_or_frame_changed) {
        return;
    }

    const auto thread = session()->frameStackModel()->currentThread();
    const auto frame = session()->frameStackModel()->currentFrame();
    if (thread < 0 || frame < 0) {
        // Update only when both the current thread and the current frame become valid.
        // Redundant updating breaks highlighting changed variables, slows down
        // debugging by issuing unneeded MI commands, and may cause other bugs.
        qCDebug(DEBUGGER).nospace() << "not handling a change to invalid thread (" << thread << ") or frame (" << frame
                                    << ')';
        return;
    }

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
        if (thread != d->activeThread || frame != d->activeFrame) {
            qCDebug(DEBUGGER) << "handling a change: thread" << d->activeThread << "=>" << thread << ';' << "frame"
                              << d->activeFrame << "=>" << frame;

            variableCollection()->root()->resetChanged();
            update();
        }
    }

    // update our cache of active thread/frame regardless of d->autoUpdate
    // to keep them synced when user currently hides the variable list
    d->activeThread = thread;
    d->activeFrame = frame;
}

void IVariableController::setAutoUpdate(QFlags<UpdateType> autoUpdate)
{
    Q_D(IVariableController);

    IDebugSession::DebuggerState state = session()->state();
    d->autoUpdate = autoUpdate;
    qCDebug(DEBUGGER) << d->autoUpdate;
    if (d->autoUpdate != UpdateNone && state == IDebugSession::PausedState) {
        if (d->activeThread < 0 || d->activeFrame < 0) {
            // Cannot update variables for an invalid thread or frame. Our handler of the event
            // thread_or_frame_changed will update variables once the current thread and frame become valid.
            qCDebug(DEBUGGER).nospace() << "skipping initial update of variables because the current thread ("
                                        << d->activeThread << ") or frame (" << d->activeFrame << ") is invalid";
            return;
        }
        update();
    }
}

QFlags<IVariableController::UpdateType> IVariableController::autoUpdate()
{
    Q_D(IVariableController);

    return d->autoUpdate;
}

}

#include "moc_ivariablecontroller.cpp"

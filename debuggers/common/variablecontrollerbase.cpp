/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "variablecontrollerbase.h"

#include "debuglog.h"
#include "debugsessionbase.h"
#include "mivariable.h"
#include "mi/mi.h"
#include "mi/micommand.h"
#include "stringhelpers.h"

#include <debugger/interfaces/iframestackmodel.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>

#include <KTextEditor/Document>

using namespace KDevelop;
using namespace KDevDebugger;
using namespace KDevDebugger::MI;
using KTextEditor::Cursor;
using KTextEditor::Document;
using KTextEditor::Range;

VariableControllerBase::VariableControllerBase(DebugSessionBase *parent)
    : IVariableController(parent)
{
    Q_ASSERT(parent);
    connect(parent, &DebugSessionBase::inferiorStopped,
            this, &VariableControllerBase::programStopped);
    connect(parent, &DebugSessionBase::stateChanged,
            this, &VariableControllerBase::stateChanged);
}

DebugSessionBase *VariableControllerBase::debugSession() const
{
    return static_cast<DebugSessionBase*>(const_cast<QObject*>(QObject::parent()));
}

void VariableControllerBase::programStopped(const AsyncRecord& r)
{
    if (debugSession()->debuggerStateIsOn(s_shuttingDown)) return;

    if (r.hasField("reason") && r["reason"].literal() == "function-finished"
        && r.hasField("gdb-result-var"))
    {
        variableCollection()->watches()->addFinishResult(r["gdb-result-var"].literal());
    } else {
        variableCollection()->watches()->removeFinishResult();
    }
}

void VariableControllerBase::update()
{
    qCDebug(DEBUGGERGDB) << autoUpdate();
    if (autoUpdate() & UpdateWatches) {
        variableCollection()->watches()->reinstall();
    }

   if (autoUpdate() & UpdateLocals) {
        updateLocals();
   }

   if ((autoUpdate() & UpdateLocals) ||
       ((autoUpdate() & UpdateWatches) && variableCollection()->watches()->childCount() > 0))
    {
        debugSession()->addCommand(
            new MICommand(VarUpdate, "--all-values *", this,
                       &VariableControllerBase::handleVarUpdate));
    }
}

void VariableControllerBase::handleVarUpdate(const ResultRecord& r)
{
    const Value& changed = r["changelist"];
    for (int i = 0; i < changed.size(); ++i)
    {
        const Value& var = changed[i];
        MIVariable* v = MIVariable::findByVarobjName(var["name"].literal());
        // v can be NULL here if we've already removed locals after step,
        // but the corresponding -var-delete command is still in the queue.
        if (v) {
            v->handleUpdate(var);
        }
    }
}

class StackListArgumentsHandler : public MICommandHandler
{
public:
    StackListArgumentsHandler(QStringList localsName)
        : m_localsName(localsName)
    {}

    void handle(const ResultRecord &r) override
    {
        if (!KDevelop::ICore::self()->debugController()) return; //happens on shutdown

        // FIXME: handle error.
        const Value& locals = r["stack-args"][0]["args"];

        for (int i = 0; i < locals.size(); i++) {
            m_localsName << locals[i].literal();
        }
        QList<Variable*> variables = KDevelop::ICore::self()->debugController()->variableCollection()
                ->locals()->updateLocals(m_localsName);
        foreach (Variable *v, variables) {
            v->attachMaybe();
        }
    }

private:
    QStringList m_localsName;
};

class StackListLocalsHandler : public MICommandHandler
{
public:
    StackListLocalsHandler(DebugSessionBase *session)
        : m_session(session)
    {}

    void handle(const ResultRecord &r) override
    {
        // FIXME: handle error.

        const Value& locals = r["locals"];

        QStringList localsName;
        for (int i = 0; i < locals.size(); i++) {
            const Value& var = locals[i];
            localsName << var["name"].literal();
        }
        int frame = m_session->frameStackModel()->currentFrame();
        m_session->addCommand(                    //dont'show value, low-frame, high-frame
            new MICommand(StackListArguments, QString("0 %1 %2").arg(frame).arg(frame),
                          new StackListArgumentsHandler(localsName)));
    }

private:
    DebugSessionBase *m_session;
};

void VariableControllerBase::updateLocals()
{
    debugSession()->addCommand(
        new MICommand(StackListLocals, "--simple-values",
                      new StackListLocalsHandler(debugSession())));
}

Range VariableControllerBase::expressionRangeUnderCursor(Document* doc, const Cursor& cursor)
{
    QString line = doc->line(cursor.line());
    int index = cursor.column();
    QChar c = line[index];
    if (!c.isLetterOrNumber() && c != '_')
        return {};

    int start = Utils::expressionAt(line, index+1);
    int end = index;
    for (; end < line.size(); ++end)
    {
        QChar c = line[end];
        if (!(c.isLetterOrNumber() || c == '_'))
            break;
    }
    if (!(start < end))
        return {};

    return { Cursor{cursor.line(), start}, Cursor{cursor.line(), end} };
}


void VariableControllerBase::addWatch(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to MIVariable, not poke at varobj. In that case,
    // we will be able to make addWatch a generic method, not
    // gdb-specific one.
    if (MIVariable *gv = dynamic_cast<MIVariable*>(variable))
    {
        debugSession()->addCommand(
            new MICommand(VarInfoPathExpression,
                           gv->varobj(),
                           this,
                           &VariableControllerBase::addWatch));
    }
}

void VariableControllerBase::addWatchpoint(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to MIVariable, not poke at varobj. In that case,
    // we will be able to make addWatchpoint a generic method, not
    // gdb-specific one.
    if (MIVariable *gv = dynamic_cast<MIVariable*>(variable))
    {
        debugSession()->addCommand(
            new MICommand(VarInfoPathExpression,
                           gv->varobj(),
                           this,
                           &VariableControllerBase::addWatchpoint));
    }
}

void VariableControllerBase::addWatch(const ResultRecord& r)
{
    // FIXME: handle error.
    if (r.reason == "done" &&  !r["path_expr"].literal().isEmpty()) {
        variableCollection()->watches()->add(r["path_expr"].literal());
    }
}

void VariableControllerBase::addWatchpoint(const ResultRecord& r)
{
    if (r.reason == "done" && !r["path_expr"].literal().isEmpty()) {
        KDevelop::ICore::self()->debugController()->breakpointModel()->addWatchpoint(r["path_expr"].literal());
    }
}

Variable* VariableControllerBase::createVariable(TreeModel* model, TreeItem* parent,
                                                 const QString& expression, const QString& display)
{
    return new MIVariable(model, parent, expression, display);
}

void VariableControllerBase::handleEvent(IDebugSession::event_t event)
{
    IVariableController::handleEvent(event);
}

void VariableControllerBase::stateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::EndedState) {
        MIVariable::markAllDead();
    }
}

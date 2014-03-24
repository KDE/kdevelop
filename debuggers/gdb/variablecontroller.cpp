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

#include "variablecontroller.h"

#include <debugger/variable/variablecollection.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/interfaces/iframestackmodel.h>

#include "gdbcommand.h"
#include "debugsession.h"
#include "stringhelpers.h"
#include "gdbvariable.h"

#include <KDebug>
#include <KTextEditor/Document>

using namespace GDBDebugger;
using namespace KDevelop;

VariableController::VariableController(DebugSession* parent)
    : KDevelop::IVariableController(parent)
{
    Q_ASSERT(parent);
    connect(parent, SIGNAL(programStopped(GDBMI::ResultRecord)), SLOT(programStopped(GDBMI::ResultRecord)));
    connect(parent, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)), SLOT(stateChanged(KDevelop::IDebugSession::DebuggerState)));
}

DebugSession *VariableController::debugSession() const
{
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}

void VariableController::programStopped(const GDBMI::ResultRecord& r)
{
    if (debugSession()->stateIsOn(s_shuttingDown)) return;

    if (r.hasField("reason") && r["reason"].literal() == "function-finished"
        && r.hasField("gdb-result-var"))
    {
        variableCollection()->watches()->addFinishResult(r["gdb-result-var"].literal());
    } else {
        variableCollection()->watches()->removeFinishResult();
    }
}

void VariableController::update()
{
    kDebug() << autoUpdate();
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
            new GDBCommand(GDBMI::VarUpdate, "--all-values *", this,
                       &VariableController::handleVarUpdate));
    }
}

void VariableController::handleVarUpdate(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& changed = r["changelist"];
    for (int i = 0; i < changed.size(); ++i)
    {
        const GDBMI::Value& var = changed[i];
        GdbVariable* v = GdbVariable::findByVarobjName(var["name"].literal());
        // v can be NULL here if we've already removed locals after step,
        // but the corresponding -var-delete command is still in the queue.
        if (v) {
            v->handleUpdate(var);
        }
    }
}
class StackListArgumentsHandler : public GDBCommandHandler
{
public:
    StackListArgumentsHandler(QStringList localsName)
        : m_localsName(localsName)
    {}

    virtual void handle(const GDBMI::ResultRecord &r)
    {
        if (!KDevelop::ICore::self()->debugController()) return; //happens on shutdown

        // FIXME: handle error.
        const GDBMI::Value& locals = r["stack-args"][0]["args"];

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

class StackListLocalsHandler : public GDBCommandHandler
{
public:
    StackListLocalsHandler(DebugSession *session)
        : m_session(session)
    {}

    virtual void handle(const GDBMI::ResultRecord &r)
    {
        // FIXME: handle error.

        const GDBMI::Value& locals = r["locals"];

        QStringList localsName;
        for (int i = 0; i < locals.size(); i++) {
            const GDBMI::Value& var = locals[i];
            localsName << var["name"].literal();
        }
        int frame = m_session->frameStackModel()->currentFrame();
        m_session->addCommand(                    //dont'show value, low-frame, high-frame
            new GDBCommand(GDBMI::StackListArguments, QString("0 %1 %2").arg(frame).arg(frame),
                        new StackListArgumentsHandler(localsName)));
    }

private:
    DebugSession *m_session;
};

void VariableController::updateLocals()
{
    debugSession()->addCommand(
        new GDBCommand(GDBMI::StackListLocals, "--simple-values",
                        new StackListLocalsHandler(debugSession())));
}

QString VariableController::expressionUnderCursor(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor)
{
    QString line = doc->line(cursor.line());
    int index = cursor.column();
    QChar c = line[index];
    if (!c.isLetterOrNumber() && c != '_')
        return QString();

    int start = Utils::expressionAt(line, index+1);
    int end = index;
    for (; end < line.size(); ++end)
    {
        QChar c = line[end];
        if (!(c.isLetterOrNumber() || c == '_'))
            break;
    }
    if (!(start < end))
        return QString();

    QString expression(line.mid(start, end-start));
    expression = expression.trimmed();
    return expression;
}


void VariableController::addWatch(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to GdbVariable, not poke at varobj. In that case,
    // we will be able to make addWatch a generic method, not
    // gdb-specific one.
    if (GdbVariable *gv = dynamic_cast<GdbVariable*>(variable))
    {
        debugSession()->addCommand(
            new GDBCommand(GDBMI::VarInfoPathExpression,
                           gv->varobj(),
                           this,
                           &VariableController::addWatch));
    }
}

void VariableController::addWatchpoint(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to GdbVariable, not poke at varobj. In that case,
    // we will be able to make addWatchpoint a generic method, not
    // gdb-specific one.
    if (GdbVariable *gv = dynamic_cast<GdbVariable*>(variable))
    {
        debugSession()->addCommand(
            new GDBCommand(GDBMI::VarInfoPathExpression,
                           gv->varobj(),
                           this,
                           &VariableController::addWatchpoint));
    }
}

void VariableController::addWatch(const GDBMI::ResultRecord& r)
{
    // FIXME: handle error.
    if (r.reason == "done" &&  !r["path_expr"].literal().isEmpty()) {
        variableCollection()->watches()->add(r["path_expr"].literal());
    }
}

void VariableController::addWatchpoint(const GDBMI::ResultRecord& r)
{
    if (r.reason == "done" && !r["path_expr"].literal().isEmpty()) {
        KDevelop::ICore::self()->debugController()->breakpointModel()->addWatchpoint(r["path_expr"].literal());
    }
}

KDevelop::Variable* VariableController::
createVariable(TreeModel* model, TreeItem* parent, 
               const QString& expression, const QString& display)
{
    return new GdbVariable(model, parent, expression, display);
}

void VariableController::handleEvent(IDebugSession::event_t event)
{
    IVariableController::handleEvent(event);
}

void VariableController::stateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::EndedState) {
        GdbVariable::markAllDead();
    }
}



#include "variablecontroller.moc"

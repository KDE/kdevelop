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

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/variable/variablecollection.h>

#include "gdbcommand.h"
#include "debugsession.h"
#include "gdbcontroller.h"
#include "stringhelpers.h"
#include <debugger/breakpoint/breakpointmodel.h>

using namespace GDBDebugger;

int VariableController::nextId_ = 0;

VariableController::VariableController(DebugSession* parent)
    : KDevelop::IVariableController(parent)
{
    Q_ASSERT(parent);
    connect(parent, SIGNAL(programStopped(GDBMI::ResultRecord)), SLOT(programStopped(GDBMI::ResultRecord)));
}

DebugSession *VariableController::debugSession() const
{
    return static_cast<DebugSession*>(const_cast<QObject*>(QObject::parent()));
}

GDBController *VariableController::controller() const
{
    return debugSession()->controller();
}

void VariableController::programStopped(const GDBMI::ResultRecord& r)
{
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
    variableCollection()->watches()->reinstall();

    updateLocals();

    controller()->addCommand(
        new GDBCommand(GDBMI::VarUpdate, "--all-values *", this,
                       &VariableController::handleVarUpdate));

}

void VariableController::handleVarUpdate(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& changed = r["changelist"];
    for (int i = 0; i < changed.size(); ++i)
    {
        const GDBMI::Value& var = changed[i];
        KDevelop::Variable* v = KDevelop::Variable::findByName(var["name"].literal());
        // v can be NULL here if we've already removed locals after step,
        // but the corresponding -var-delete command is still in the queue.
        if (v) {
            if (var.hasField("type_changed")
                && var["type_changed"].literal() == "true")
            {
                v->deleteChildren();
                v->setHasMore(var["new_num_children"].toInt() != 0);
            }

            if (var.hasField("in_scope") && var["in_scope"].literal() == "false")
            {
                v->setInScope(false);
            }
            else
            {
                v->setInScope(true);
                v->setValue(var["value"].literal());
            }
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
        // FIXME: handle error.

        const GDBMI::Value& locals = r["stack-args"][0]["args"];

        for (int i = 0; i < locals.size(); i++) {
            m_localsName << locals[i].literal();
        }
        KDevelop::ICore::self()->debugController()->variableCollection()
                ->locals()->updateLocals(m_localsName);
    }

private:
    QStringList m_localsName;
};

class StackListLocalsHandler : public GDBCommandHandler
{
public:
    StackListLocalsHandler(GDBController *controller)
        : m_controller(controller)
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
        m_controller->addCommand(                    //dont'show value, low-frame, high-frame
            new GDBCommand(GDBMI::StackListArguments, "0 0 0",
                        new StackListArgumentsHandler(localsName)));
    }

private:
    GDBController *m_controller;
};

void VariableController::updateLocals()
{
    controller()->addCommand(
        new GDBCommand(GDBMI::StackListLocals, "--all-values",
                        new StackListLocalsHandler(controller())));
}

class CreateVarobjHandler : public GDBCommandHandler
{
public:
    CreateVarobjHandler(KDevelop::Variable *variable, QObject *callback, const char *callbackMethod)
        : m_variable(variable), m_callback(callback), m_callbackMethod(callbackMethod)
    {}

    virtual void handle(const GDBMI::ResultRecord &r)
    {
        if (!m_variable) return;
        bool hasValue = false;
        if (r.reason == "error") {
            /* Probably should mark this disabled, or something.  */
        } else {
            m_variable->deleteChildren();
            m_variable->setInScope(true);
            m_variable->setVarobj(r["name"].literal());
            m_variable->setHasMore(r["numchild"].toInt());
            m_variable->setValue(r["value"].literal());
            hasValue = !r["value"].literal().isEmpty();
            if (m_variable->isExpanded() && r["numchild"].toInt()) {
                m_variable->fetchMoreChildren();
            }
        }

        if (m_callback && m_callbackMethod) {
            QMetaObject::invokeMethod(m_callback, m_callbackMethod, Q_ARG(bool, hasValue));
        }
    }
    virtual bool handlesError() { return true; }

private:
    QPointer<KDevelop::Variable> m_variable;
    QObject *m_callback;
    const char *m_callbackMethod;
};

void VariableController::createVarobj(KDevelop::Variable *variable, QObject *callback, const char *callbackMethod)
{
    controller()->addCommand(
        new GDBCommand(
            GDBMI::VarCreate,
            QString("var%1 @ %2").arg(nextId_++).arg(variable->expression()),
            new CreateVarobjHandler(variable, callback, callbackMethod)));
}

class FetchMoreChildrenHandler : public GDBCommandHandler
{
public:
    FetchMoreChildrenHandler(KDevelop::Variable *variable, GDBController *controller)
        : m_variable(variable), m_controller(controller), m_activeCommands(1)
    {}

    virtual void handle(const GDBMI::ResultRecord &r)
    {
        if (!m_variable) return;
        --m_activeCommands;
        const GDBMI::Value& children = r["children"];
        for (int i = 0; i < children.size(); ++i) {
            const GDBMI::Value& child = children[i];
            const QString& exp = child["exp"].literal();
            if (exp == "public" || exp == "protected" || exp == "private") {
                ++m_activeCommands;
                m_controller->addCommand(
                    new GDBCommand(GDBMI::VarListChildren,
                                QString("--all-values \"%1\"").arg(child["name"].literal()),
                                this/*use again as handler*/));
            } else {
                KDevelop::Variable* var = new KDevelop::Variable(m_variable->model(), m_variable, child["exp"].literal());
                var->setTopLevel(false);
                var->setVarobj(child["name"].literal());
                var->setHasMoreInitial(child["numchild"].toInt());
                m_variable->appendChild(var);
                var->setValue(child["value"].literal());
            }
        }

        m_variable->setHasMore(m_activeCommands != 0);
        if (m_activeCommands == 0) {
            delete this;
        }
    }
    virtual bool handlesError() {
        // FIXME: handle error?
        return false;
    }
    virtual bool autoDelete() {
        // we delete ourselve
        return false;
    }

private:
    QPointer<KDevelop::Variable> m_variable;
    GDBController *m_controller;
    int m_activeCommands;
};


void VariableController::fetchMoreChildren(KDevelop::Variable* variable)
{
    controller()->addCommand(
        new GDBCommand(GDBMI::VarListChildren,
                    QString("--all-values \"%1\"").arg(variable->varobj()),
                    new FetchMoreChildrenHandler(variable, controller())));
}


void VariableController::deleteVar(KDevelop::Variable* variable)
{
    controller()->addCommand(
        new GDBCommand(GDBMI::VarDelete, QString("\"%1\"").arg(variable->varobj())));
}

QString VariableController::expressionUnderCursor(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor)
{
    QString line = doc->line(cursor.line());
    int index = cursor.column();
    QChar c = line[index];
    if (!c.isLetterOrNumber() && c != '_')
        return QString();

    int start = Utils::expressionAt(line, index);
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
    controller()->addCommand(
        new GDBCommand(GDBMI::VarInfoPathExpression,
                        variable->varobj(),
                        this,
                        &VariableController::addWatch));
}

void VariableController::addWatchpoint(KDevelop::Variable* variable)
{
    controller()->addCommand(
        new GDBCommand(GDBMI::VarInfoPathExpression,
                        variable->varobj(),
                        this,
                        &VariableController::addWatchpoint));
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

#include "variablecontroller.moc"

/*
 * GDB-specific Variable
 *
 * Copyright 2009 Vladimir Prus <ghost@cs.msu.su>
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

#include "gdbvariable.h"
#include "gdbcommand.h"
#include "debugsession.h"

#include <interfaces/icore.h>
#include <debugger/interfaces/ivariablecontroller.h>

using namespace KDevelop;
using namespace GDBDebugger;

QMap<QString, GdbVariable*> GdbVariable::allVariables_;

static bool hasStartedSession()
{
    IDebugSession *session = ICore::self()->debugController()->currentSession();
    if (!session)
        return false;

    IDebugSession::DebuggerState s = session->state();
    return s != IDebugSession::NotStartedState 
        && s != IDebugSession::EndedState;
}

GdbVariable::GdbVariable(TreeModel* model, TreeItem* parent,
            const QString& expression, const QString& display)
: Variable(model, parent, expression, display)
{
}

GdbVariable::~GdbVariable()
{
    if (!varobj_.isEmpty())
    {
        // Delete only top-level variable objects.
        if (topLevel()) {
            if (hasStartedSession()) {
                // FIXME: Eventually, should be a property of variable.
                IDebugSession* is = ICore::self()->debugController()->currentSession();
                DebugSession* s = static_cast<DebugSession*>(is);
                s->addCommand(new GDBCommand(GDBMI::VarDelete, 
                                             QString("\"%1\"").arg(varobj_)));
            }
        }
        allVariables_.remove(varobj_);
    }
}

GdbVariable* GdbVariable::findByVarobjName(const QString& varobjName)
{
    if (allVariables_.count(varobjName) == 0)
        return 0;
    return allVariables_[varobjName];
}

void GdbVariable::setVarobj(const QString& v)
{
    Q_ASSERT(varobj_.isEmpty());
    varobj_ = v;
    allVariables_[varobj_] = this;
}


static int nextId = 0;

class CreateVarobjHandler : public GDBCommandHandler
{
public:
    CreateVarobjHandler(GdbVariable *variable, QObject *callback, const char *callbackMethod)
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
    QPointer<GdbVariable> m_variable;
    QObject *m_callback;
    const char *m_callbackMethod;
};

void GdbVariable::attachMaybe(QObject *callback, const char *callbackMethod)
{
    if (!varobj_.isEmpty())
        return;

    if (hasStartedSession()) {
        // FIXME: Eventually, should be a property of variable.
        IDebugSession* is = ICore::self()->debugController()->currentSession();
        DebugSession* s = static_cast<DebugSession*>(is);
        s->addCommand(
            new GDBCommand(
                GDBMI::VarCreate,
                QString("var%1 @ %2").arg(nextId++).arg(expression()),
                new CreateVarobjHandler(this, callback, callbackMethod)));
    }
}

void GdbVariable::markAllDead()
{
    QMap<QString, GdbVariable*>::iterator i, e;
    for (i = allVariables_.begin(), e = allVariables_.end(); i != e; ++i)
    {
        i.value()->varobj_.clear();
        i.value()->setInScope(false);
    }
    allVariables_.clear();
}

class FetchMoreChildrenHandler : public GDBCommandHandler
{
public:
    FetchMoreChildrenHandler(GdbVariable *variable, DebugSession *session)
        : m_variable(variable), m_session(session), m_activeCommands(1)
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
                m_session->addCommand(
                    new GDBCommand(GDBMI::VarListChildren,
                                QString("--all-values \"%1\"").arg(child["name"].literal()),
                                this/*use again as handler*/));
            } else {
                KDevelop::Variable* xvar = m_session->variableController()->
                    createVariable(m_variable->model(), m_variable, 
                                   child["exp"].literal());
                GdbVariable* var = static_cast<GdbVariable*>(xvar);
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
    QPointer<GdbVariable> m_variable;
    DebugSession *m_session;
    int m_activeCommands;
};

void GdbVariable::fetchMoreChildren()
{
    // FIXME: should not even try this if app is not started.
    // Probably need to disable open, or something
    if (hasStartedSession()) {
        // FIXME: Eventually, should be a property of variable.
        IDebugSession* is = ICore::self()->debugController()->currentSession();
        DebugSession* s = static_cast<DebugSession*>(is);
        s->addCommand(
            new GDBCommand(GDBMI::VarListChildren,
                           QString("--all-values \"%1\"").arg(varobj_),
                           new FetchMoreChildrenHandler(this, s)));
    }
}

void GdbVariable::handleUpdate(const GDBMI::Value& var)
{
    if (var.hasField("type_changed")
        && var["type_changed"].literal() == "true")
    {
        deleteChildren();
        setHasMore(var["new_num_children"].toInt() != 0);
        fetchMoreChildren();
    }
    
    if (var.hasField("in_scope") && var["in_scope"].literal() == "false")
    {
        setInScope(false);
    }
    else
    {
        setInScope(true);
        setValue(var["value"].literal());
    }
}

const QString& GdbVariable::varobj() const
{
    return varobj_;
}

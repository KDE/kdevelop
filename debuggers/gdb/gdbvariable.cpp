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
    if (!ICore::self()->debugController()) return false; //happens on shutdown

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
    if (!varobj_.isEmpty()) {
        // this should not happen
        // but apperently it does when attachMaybe is called a second time before
        // the first -var-create call returned
        allVariables_.remove(varobj_);
    }
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

    virtual void handle(const GDBMI::ResultRecord &r) override
    {
        if (!m_variable) return;
        bool hasValue = false;
        GdbVariable* variable = m_variable.data();
        variable->deleteChildren();
        variable->setInScope(true);
        if (r.reason == "error") {
            variable->setShowError(true);
        } else {
            variable->setVarobj(r["name"].literal());

            bool hasMore = false;
            if (r.hasField("has_more") && r["has_more"].toInt())
                // GDB swears there are more children. Trust it
                hasMore = true;
            else
                // There are no more children in addition to what
                // numchild reports. But, in KDevelop, the variable
                // is not yet expanded, and those numchild are not
                // fetched yet. So, if numchild != 0, hasMore should
                // be true.
                hasMore = r["numchild"].toInt() != 0;

            variable->setHasMore(hasMore);

            variable->setType(r["type"].literal());
            variable->setValue(r["value"].literal());
            hasValue = !r["value"].literal().isEmpty();
            if (variable->isExpanded() && r["numchild"].toInt()) {
                variable->fetchMoreChildren();
            }

            if (variable->format() != KDevelop::Variable::Natural) {
                //TODO doesn't work for children as they are not yet loaded
                variable->formatChanged();
            }
        }

        if (m_callback && m_callbackMethod) {
            QMetaObject::invokeMethod(m_callback, m_callbackMethod, Q_ARG(bool, hasValue));
        }
    }
    virtual bool handlesError() override { return true; }

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
        IDebugSession* is = ICore::self()->debugController()->currentSession();
        DebugSession* s = static_cast<DebugSession*>(is);
        s->addCommand(
            new GDBCommand(
                GDBMI::VarCreate,
                QString("var%1 @ %2").arg(nextId++).arg(enquotedExpression()),
                new CreateVarobjHandler(this, callback, callbackMethod)));
    }
}

void GdbVariable::markAllDead()
{
    QMap<QString, GdbVariable*>::iterator i, e;
    for (i = allVariables_.begin(), e = allVariables_.end(); i != e; ++i)
    {
        i.value()->varobj_.clear();
    }
    allVariables_.clear();
}

class FetchMoreChildrenHandler : public GDBCommandHandler
{
public:
    FetchMoreChildrenHandler(GdbVariable *variable, DebugSession *session)
        : m_variable(variable), m_session(session), m_activeCommands(1)
    {}

    virtual void handle(const GDBMI::ResultRecord &r) override
    {
        if (!m_variable) return;
        --m_activeCommands;

        GdbVariable* variable = m_variable.data();

        if (r.hasField("children"))
        {
            const GDBMI::Value& children = r["children"];
            for (int i = 0; i < children.size(); ++i) {
                const GDBMI::Value& child = children[i];
                const QString& exp = child["exp"].literal();
                if (exp == "public" || exp == "protected" || exp == "private") {
                    ++m_activeCommands;
                    m_session->addCommand(
                        new GDBCommand(GDBMI::VarListChildren,
                                       QString("--all-values \"%1\"")
                                       .arg(child["name"].literal()),
                                       this/*use again as handler*/));
                } else {
                    KDevelop::Variable* xvar = m_session->variableController()->
                        createVariable(variable->model(), variable,
                                       child["exp"].literal());
                    GdbVariable* var = static_cast<GdbVariable*>(xvar);
                    var->setTopLevel(false);
                    var->setVarobj(child["name"].literal());
                    bool hasMore = child["numchild"].toInt() != 0 || ( child.hasField("dynamic") && child["dynamic"].toInt()!=0 );
                    var->setHasMoreInitial(hasMore);
                    variable->appendChild(var);
                    var->setType(child["type"].literal());
                    var->setValue(child["value"].literal());
                }
            }
        }

        /* Note that we don't set hasMore to true if there are still active
           commands. The reason is that we don't want the user to have
           even theoretical ability to click on "..." item and confuse
           us.  */
        bool hasMore = false;
        if (r.hasField("has_more"))
            hasMore = r["has_more"].toInt();

        variable->setHasMore(hasMore);
        if (m_activeCommands == 0) {
            variable->emitAllChildrenFetched();
            delete this;
        }
    }
    virtual bool handlesError() override {
        // FIXME: handle error?
        return false;
    }
    virtual bool autoDelete() override {
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
    int c = childItems.size();
    // FIXME: should not even try this if app is not started.
    // Probably need to disable open, or something
    if (hasStartedSession()) {
        IDebugSession* is = ICore::self()->debugController()->currentSession();
        DebugSession* s = static_cast<DebugSession*>(is);
        s->addCommand(
            new GDBCommand(GDBMI::VarListChildren,
                           QString("--all-values \"%1\" %2 %3").arg(varobj_)
                           .arg( c ).arg( c + fetchStep ),  // fetch from .. to ..
                           new FetchMoreChildrenHandler(this, s)));
    }
}

void GdbVariable::handleUpdate(const GDBMI::Value& var)
{
    if (var.hasField("type_changed")
        && var["type_changed"].literal() == "true")
    {
        deleteChildren();
        // FIXME: verify that this check is right.
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

        if  (var.hasField("new_num_children")) {
            int nc = var["new_num_children"].toInt();
            Q_ASSERT(nc != -1);
            setHasMore(false);
            while (childCount() > nc) {
                TreeItem *c = child(childCount()-1);
                removeChild(childCount()-1);
                delete c;
            }
        }
                
        // FIXME: the below code is essentially copy-paste from
        // FetchMoreChildrenHandler. We need to introduce GDB-specific
        // subclass of KDevelop::Variable that is capable of creating
        // itself from MI output directly, and relay to that.
        if (var.hasField("new_children"))
        {                  
            const GDBMI::Value& children = var["new_children"];
            for (int i = 0; i < children.size(); ++i) {
                const GDBMI::Value& child = children[i];
                const QString& exp = child["exp"].literal();

                IDebugSession* is = ICore::self()->debugController()->currentSession();
                DebugSession* s = static_cast<DebugSession*>(is);
                KDevelop::Variable* xvar = s->variableController()->
                    createVariable(model(), this, exp);
                GdbVariable* var = static_cast<GdbVariable*>(xvar);
                var->setTopLevel(false);
                var->setVarobj(child["name"].literal());
                bool hasMore = child["numchild"].toInt() != 0 || ( child.hasField("dynamic") && child["dynamic"].toInt()!=0 );
                var->setHasMoreInitial(hasMore);
                appendChild(var);
                var->setType(child["type"].literal());
                var->setValue(child["value"].literal());
                var->setChanged(true);
            }
        }

        if (var.hasField("type_changed") && var["type_changed"].literal() == "true") {
            setType(var["new_type"].literal());
        }
        setValue(var["value"].literal());
        setChanged(true);
        setHasMore(var.hasField("has_more") && var["has_more"].toInt());
    }
}

const QString& GdbVariable::varobj() const
{
    return varobj_;
}

QString GdbVariable::enquotedExpression() const
{
    QString expr = expression();
    expr.replace('"', "\\\"");
    expr = expr.prepend('"').append('"');
    return expr;
}


class SetFormatHandler : public GDBCommandHandler
{
public:
    SetFormatHandler(GdbVariable *var)
        : m_variable(var)
    {}

    virtual void handle(const GDBMI::ResultRecord &r) override
    {
        if(r.hasField("value"))
            m_variable.data()->setValue(r["value"].literal());
    }
private:
    QPointer<GdbVariable> m_variable;
};

void GdbVariable::formatChanged()
{
    if(childCount())
    {
        foreach(TreeItem* item, childItems) {
            Q_ASSERT(dynamic_cast<GdbVariable*>(item));
            if( GdbVariable* var=dynamic_cast<GdbVariable*>(item))
                var->setFormat(format());
        }
    }
    else
    {
        if (hasStartedSession()) {
            IDebugSession* is = ICore::self()->debugController()->currentSession();
            DebugSession* s = static_cast<DebugSession*>(is);
            s->addCommand(
                new GDBCommand(GDBMI::VarSetFormat,
                            QString(" \"%1\" %2 ").arg(varobj_).arg(format2str(format())),
                            new SetFormatHandler(this)
                            )
                        );
        }
    }
}

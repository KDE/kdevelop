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

using namespace KDevelop;
using namespace GDBDebugger;

GdbVariable::GdbVariable(TreeModel* model, TreeItem* parent,
            const QString& expression, const QString& display)
: Variable(model, parent, expression, display)
{
}

static int nextId = 0;

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

void GdbVariable::createVarobj(QObject *callback, const char *callbackMethod)
{
    // FIXME: Eventually, should be a property of variable.
    IDebugSession* is = ICore::self()->debugController()->currentSession();
    DebugSession* s = static_cast<DebugSession*>(is);
    s->addCommand(
        new GDBCommand(
            GDBMI::VarCreate,
            QString("var%1 @ %2").arg(nextId++).arg(expression()),
            new CreateVarobjHandler(this, callback, callbackMethod)));

}


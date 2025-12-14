/*
    SPDX-FileCopyrightText: 2009 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mivariable.h"

#include "debuglog.h"
#include "midebugsession.h"
#include "mi/micommand.h"
#include "stringhelpers.h"

#include <debugger/interfaces/ivariablecontroller.h>
#include <interfaces/icore.h>

using namespace KDevelop;
using namespace KDevMI;
using namespace KDevMI::MI;

bool MIVariable::sessionIsAlive() const
{
    if (!m_debugSession)
        return false;

    IDebugSession::DebuggerState s = m_debugSession->state();
    return s != IDebugSession::NotStartedState 
        && s != IDebugSession::EndedState
        && !m_debugSession->debuggerStateIsOn(s_shuttingDown);
}

MIVariable::MIVariable(MIDebugSession *session, TreeModel* model, TreeItem* parent,
                       const QString& expression, const QString& display)
    : Variable(model, parent, expression, display)
    , m_debugSession(session)
{
}

MIVariable *MIVariable::createChild(const Value& child)
{
    if (!m_debugSession) return nullptr;
    auto var = static_cast<MIVariable*>(m_debugSession->variableController()->createVariable(model(), this, child[QStringLiteral("exp")].literal()));
    var->setTopLevel(false);
    var->setVarobj(child[QStringLiteral("name")].literal());
    bool hasMore = child[QStringLiteral("numchild")].toInt() != 0 || ( child.hasField(QStringLiteral("dynamic")) && child[QStringLiteral("dynamic")].toInt()!=0 );
    var->setHasMoreInitial(hasMore);

    // *this must be parent's child before we can set type and value
    appendChild(var);

    var->setType(child[QStringLiteral("type")].literal());
    var->setValue(formatValue(child[QStringLiteral("value")].literal()));
    var->setChanged(true);
    return var;
}

MIVariable::~MIVariable()
{
    if (!m_varobj.isEmpty())
    {
        // Delete only top-level variable objects.
        if (topLevel()) {
            if (sessionIsAlive()) {
                m_debugSession->addCommand(VarDelete, QStringLiteral("\"%1\"").arg(m_varobj));
            }
        }
        if (m_debugSession)
            m_debugSession->variableMapping().remove(m_varobj);
    }
}

void MIVariable::setVarobj(const QString& v)
{
    if (!m_debugSession) {
        qCWarning(DEBUGGERCOMMON) << "MIVariable::setVarobj called when its session died";
        return;
    }
    if (!m_varobj.isEmpty()) {
        // this should not happen
        // but apparently it does when attachMaybe is called a second time before
        // the first -var-create call returned
        m_debugSession->variableMapping().remove(m_varobj);
    }
    m_varobj = v;
    m_debugSession->variableMapping()[m_varobj] = this;
}


static int nextId = 0;

class CreateVarobjHandler : public MICommandHandler
{
public:
    CreateVarobjHandler(MIVariable *variable, QObject *callback, const char *callbackMethod)
    : m_variable(variable), m_callback(callback), m_callbackMethod(callbackMethod)
    {}

    void handle(const ResultRecord &r) override
    {
        if (!m_variable) return;
        bool hasValue = false;
        MIVariable* variable = m_variable.data();
        variable->deleteChildren();
        variable->setInScope(true);

        const auto isError = r.isReasonError();
        variable->setShowError(isError);
        if (!isError) {
            variable->setVarobj(r[QStringLiteral("name")].literal());

            bool hasMore = false;
            if (r.hasField(QStringLiteral("has_more")) && r[QStringLiteral("has_more")].toInt())
                // GDB swears there are more children. Trust it
                hasMore = true;
            else
                // There are no more children in addition to what
                // numchild reports. But, in KDevelop, the variable
                // is not yet expanded, and those numchild are not
                // fetched yet. So, if numchild != 0, hasMore should
                // be true.
                hasMore = r[QStringLiteral("numchild")].toInt() != 0;

            variable->setHasMore(hasMore);

            variable->setType(r[QStringLiteral("type")].literal());
            variable->setValue(variable->formatValue(r[QStringLiteral("value")].literal()));
            hasValue = !r[QStringLiteral("value")].literal().isEmpty();
            if (variable->isExpanded() && r[QStringLiteral("numchild")].toInt()) {
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
    bool handlesError() override { return true; }

private:
    QPointer<MIVariable> m_variable;
    QObject *m_callback;
    const char *m_callbackMethod;
};

void MIVariable::attachMaybe(QObject *callback, const char *callbackMethod)
{
    if (!m_varobj.isEmpty())
        return;

    // Try find a current session and attach to it
    if (!ICore::self()->debugController()) return; //happens on shutdown
    m_debugSession = static_cast<MIDebugSession*>(ICore::self()->debugController()->currentSession());

    if (sessionIsAlive()) {
        m_debugSession->addCommand(VarCreate,
                                 QStringLiteral("var%1 @ %2").arg(nextId++).arg(enquotedExpression()),
                                 new CreateVarobjHandler(this, callback, callbackMethod));
    }
}

void MIVariable::markAsDead()
{
    m_varobj.clear();
}

class FetchMoreChildrenHandler : public MICommandHandler
{
public:
    FetchMoreChildrenHandler(MIVariable *variable, MIDebugSession *session)
        : m_variable(variable)
        , m_session(session)
    {}

    void handle(const ResultRecord &r) override
    {
        MIVariable* variable = m_variable.data();
        if (!variable)
            return;

        if (r.hasField(QStringLiteral("children")))
        {
            auto* lastHandler = this;
            const Value& children = r[QStringLiteral("children")];
            for (int i = 0; i < children.size(); ++i) {
                const Value& child = children[i];
                const QString& exp = child[QStringLiteral("exp")].literal();
                // Detect pseudo children of a C++ struct or class that designate access specifiers,
                // and expand their children (the actual data members of the variable) directly into the variable.
                // This way the access specifiers are simply omitted from the KDevelop UI.
                // Unlike an actual child variable named e.g. "public" of a C struct or of a dynamic varobj (provided
                // by a Python-based pretty-printer), the record of a pseudo child does not contain the "type" field.
                if (!child.hasField(QStringLiteral("type"))
                    && (exp == QLatin1String("public") || exp == QLatin1String("protected")
                        || exp == QLatin1String("private"))) {
                    // A pseudo child cannot have pseudo children of its own, so the maximum recursion depth here is 1.
                    // lastHandler always points to the last created handler. Only the *very* last created
                    // (and therefore the last to be invoked) handler ends up with m_isLastHandler = true.
                    lastHandler->m_isLastHandler = false;
                    lastHandler = new FetchMoreChildrenHandler(variable, m_session);
                    m_session->addCommand(
                        VarListChildren,
                        QLatin1String("--all-values \"%1\"").arg(child[QStringLiteral("name")].literal()), lastHandler);
                } else {
                    variable->createChild(child);
                    // it's automatically appended to variable's children list
                }
            }
        }

        /* Note that we don't set hasMore to true if there are still active
           commands. The reason is that we don't want the user to have
           even theoretical ability to click on "..." item and confuse
           us.  */
        bool hasMore = false;
        if (r.hasField(QStringLiteral("has_more")))
            hasMore = r[QStringLiteral("has_more")].toInt();

        variable->setHasMore(hasMore);
        if (m_isLastHandler) {
            variable->emitAllChildrenFetched();
        }
    }
    bool handlesError() override {
        // FIXME: handle error?
        return false;
    }

private:
    QPointer<MIVariable> m_variable;
    MIDebugSession *m_session;
    bool m_isLastHandler = true;
};

void MIVariable::fetchMoreChildren()
{
    int c = childItems.size();
    // FIXME: should not even try this if app is not started.
    // Probably need to disable open, or something
    if (sessionIsAlive()) {
        m_debugSession->addCommand(VarListChildren,
                                 QStringLiteral("--all-values \"%1\" %2 %3")
                                 //   fetch    from ..    to ..
                                 .arg(m_varobj).arg(c).arg(c + s_fetchStep),
                                 new FetchMoreChildrenHandler(this, m_debugSession));
    }
}

void MIVariable::handleUpdate(const Value& var)
{
    if (var.hasField(QStringLiteral("type_changed"))
        && var[QStringLiteral("type_changed")].literal() == QLatin1String("true"))
    {
        deleteChildren();
        // FIXME: verify that this check is right.
        setHasMore(var[QStringLiteral("new_num_children")].toInt() != 0);
        fetchMoreChildren();
    }

    if (var.hasField(QStringLiteral("in_scope")) && var[QStringLiteral("in_scope")].literal() == QLatin1String("false"))
    {
        setInScope(false);
    }
    else
    {
        setInScope(true);

        if  (var.hasField(QStringLiteral("new_num_children"))) {
            int nc = var[QStringLiteral("new_num_children")].toInt();
            Q_ASSERT(nc != -1);
            setHasMore(false);
            while (childCount() > nc) {
                TreeItem *c = child(childCount()-1);
                removeChild(childCount()-1);
                delete c;
            }
        }

        if (var.hasField(QStringLiteral("new_children")))
        {
            const Value& children = var[QStringLiteral("new_children")];
            if (m_debugSession) {
                for (int i = 0; i < children.size(); ++i) {
                    createChild(children[i]);
                    // it's automatically appended to this's children list
                }
            }
        }

        if (var.hasField(QStringLiteral("type_changed")) && var[QStringLiteral("type_changed")].literal() == QLatin1String("true")) {
            setType(var[QStringLiteral("new_type")].literal());
        }
        setValue(formatValue(var[QStringLiteral("value")].literal()));
        setChanged(true);
        setHasMore(var.hasField(QStringLiteral("has_more")) && var[QStringLiteral("has_more")].toInt());
    }
}

const QString& MIVariable::varobj() const
{
    return m_varobj;
}

QString MIVariable::enquotedExpression() const
{
    return Utils::quoteExpression(expression());
}


class SetFormatHandler : public MICommandHandler
{
public:
    explicit SetFormatHandler(MIVariable *var)
        : m_variable(var)
    {}

    void handle(const ResultRecord &r) override
    {
        if(m_variable && r.hasField(QStringLiteral("value")))
            m_variable->setValue(m_variable->formatValue(r[QStringLiteral("value")].literal()));
    }
private:
    QPointer<MIVariable> m_variable;
};

void MIVariable::formatChanged()
{
    if(childCount())
    {
        for (TreeItem* item : std::as_const(childItems)) {
            Q_ASSERT(dynamic_cast<MIVariable*>(item));
            if (auto* var = qobject_cast<MIVariable*>(item)) {
                var->setFormat(format());
            }
        }
    }
    else
    {
        if (sessionIsAlive()) {
            m_debugSession->addCommand(VarSetFormat,
                                     QStringLiteral(" %1 %2 ").arg(m_varobj, format2str(format())),
                                     new SetFormatHandler(this));
        }
    }
}

QString MIVariable::formatValue(const QString &rawValue) const
{
    return rawValue;
}

#include "moc_mivariable.cpp"

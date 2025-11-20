/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mivariablecontroller.h"

#include "debuglog.h"
#include "midebugsession.h"
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
using namespace KDevMI;
using namespace KDevMI::MI;
using KTextEditor::Cursor;
using KTextEditor::Document;
using KTextEditor::Range;

MIVariableController::MIVariableController(MIDebugSession *parent)
    : IVariableController(parent)
{
    Q_ASSERT(parent);
    connect(parent, &MIDebugSession::inferiorStopped,
            this, &MIVariableController::programStopped);
    connect(parent, &MIDebugSession::stateChanged,
            this, &MIVariableController::stateChanged);
}

MIDebugSession *MIVariableController::debugSession() const
{
    return static_cast<MIDebugSession *>(const_cast<QObject*>(QObject::parent()));
}

void MIVariableController::programStopped(const AsyncRecord& r)
{
    if (debugSession()->debuggerStateIsOn(s_shuttingDown)) return;

    static const auto returnValueVariableField = QStringLiteral("gdb-result-var");
    if (r.hasField(returnValueVariableField)) {
        variableCollection()->watches()->addFinishResult(r[returnValueVariableField].literal());
    } else {
        variableCollection()->watches()->removeFinishResult();
    }
}

void MIVariableController::update()
{
    qCDebug(DEBUGGERCOMMON) << "autoUpdate =" << autoUpdate();
    if (autoUpdate() & UpdateWatches) {
        variableCollection()->watches()->reinstall();
    }

   if (autoUpdate() & UpdateLocals) {
        updateLocals();
   }

   if ((autoUpdate() & UpdateLocals) ||
       ((autoUpdate() & UpdateWatches) && variableCollection()->watches()->childCount() > 0))
    {
        debugSession()->addCommand(VarUpdate, QStringLiteral("--all-values *"), this,
                                   &MIVariableController::handleVarUpdate);
    }
}

void MIVariableController::handleVarUpdate(const ResultRecord& r)
{
    const Value& changed = r[QStringLiteral("changelist")];
    for (int i = 0; i < changed.size(); ++i)
    {
        const Value& var = changed[i];
        MIVariable* v = debugSession()->findVariableByVarobjName(var[QStringLiteral("name")].literal());
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
    explicit StackListArgumentsHandler(const QStringList& localsName)
        : m_localsName(localsName)
    {}

    void handle(const ResultRecord &r) override
    {
        if (!KDevelop::ICore::self()->debugController()) return; //happens on shutdown

        if (r.hasField(QStringLiteral("stack-args")) && r[QStringLiteral("stack-args")].size() > 0) {
            const Value& locals = r[QStringLiteral("stack-args")][0][QStringLiteral("args")];

            m_localsName.reserve(m_localsName.size() + locals.size());
            for (int i = 0; i < locals.size(); i++) {
                m_localsName << locals[i].literal();
            }
            const QList<Variable*> variables = KDevelop::ICore::self()->debugController()->variableCollection()
                    ->locals()->updateLocals(m_localsName);
            for (Variable* v : variables) {
                v->attachMaybe();
            }
        }
    }

private:
    QStringList m_localsName;
};

class StackListLocalsHandler : public MICommandHandler
{
public:
    explicit StackListLocalsHandler(MIDebugSession *session)
        : m_session(session)
    {}

    void handle(const ResultRecord &r) override
    {
        if (r.hasField(QStringLiteral("locals"))) {
            const Value& locals = r[QStringLiteral("locals")];

            QStringList localsName;
            localsName.reserve(locals.size());
            for (int i = 0; i < locals.size(); i++) {
                const Value& var = locals[i];
                localsName << var[QStringLiteral("name")].literal();
            }
            int frame = m_session->frameStackModel()->currentFrame();
            m_session->addCommand(StackListArguments,
                                // do not show value, low-frame, high-frame
                                QStringLiteral("0 %1 %2").arg(frame).arg(frame),
                                new StackListArgumentsHandler(localsName));
        }
    }

private:
    MIDebugSession *m_session;
};

void MIVariableController::updateLocals()
{
    debugSession()->addCommand(StackListLocals, QStringLiteral("--simple-values"),
                               new StackListLocalsHandler(debugSession()));
}

Range MIVariableController::expressionRangeUnderCursor(Document* doc, const Cursor& cursor)
{
    const QString line = doc->line(cursor.line());
    int index = cursor.column();
    if (index >= line.size()) {
        return {};
    }
    QChar c = line[index];
    if (!c.isLetterOrNumber() && c != QLatin1Char('_')) {
        return {};
    }

    int start = Utils::expressionAt(line, index+1);
    int end = index;
    for (; end < line.size(); ++end)
    {
        QChar c = line[end];
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_'))) {
            break;
        }
    }
    if (!(start < end))
        return {};

    return { Cursor{cursor.line(), start}, Cursor{cursor.line(), end} };
}


void MIVariableController::addWatch(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to MIVariable, not poke at varobj. In that case,
    // we will be able to make addWatch a generic method, not
    // gdb-specific one.
    if (auto* gv = qobject_cast<MIVariable*>(variable)) {
        debugSession()->addCommand(VarInfoPathExpression,
                                   gv->varobj(),
                                   this, &MIVariableController::addWatch);
    }
}

void MIVariableController::addWatchpoint(KDevelop::Variable* variable)
{
    // FIXME: should add async 'get full expression' method
    // to MIVariable, not poke at varobj. In that case,
    // we will be able to make addWatchpoint a generic method, not
    // gdb-specific one.
    if (auto* gv = qobject_cast<MIVariable*>(variable)) {
        debugSession()->addCommand(VarInfoPathExpression,
                                   gv->varobj(),
                                   this, &MIVariableController::addWatchpoint);
    }
}

void MIVariableController::addWatch(const ResultRecord& r)
{
    if (r.reason == QLatin1String("done")
        && r.hasField(QStringLiteral("path_expr"))
        && !r[QStringLiteral("path_expr")].literal().isEmpty()) {
        variableCollection()->watches()->add(r[QStringLiteral("path_expr")].literal());
    }
}

void MIVariableController::addWatchpoint(const ResultRecord& r)
{
    if (r.reason == QLatin1String("done") && !r[QStringLiteral("path_expr")].literal().isEmpty()) {
        KDevelop::ICore::self()->debugController()->breakpointModel()->addWatchpoint(r[QStringLiteral("path_expr")].literal());
    }
}

Variable* MIVariableController::createVariable(TreeModel* model, TreeItem* parent,
                                                 const QString& expression, const QString& display)
{
    return new MIVariable(debugSession(), model, parent, expression, display);
}

void MIVariableController::handleEvent(IDebugSession::event_t event)
{
    IVariableController::handleEvent(event);
}

void MIVariableController::stateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::EndedState) {
        debugSession()->markAllVariableDead();
    }
}

#include "moc_mivariablecontroller.cpp"

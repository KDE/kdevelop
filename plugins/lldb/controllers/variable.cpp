/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "variable.h"

#include "debuglog.h"
#include "debugsession.h"
#include "mi/micommand.h"
#include "stringhelpers.h"

#include <QString>

using namespace KDevelop;
using namespace KDevMI::LLDB;
using namespace KDevMI::MI;

namespace {
/**
 * @return the name of a field that must be present in the record argument of LldbVariable::handleRawUpdate()
 */
[[nodiscard]] QString rawUpdateField()
{
    return QStringLiteral("changelist");
}

} // unnamed namespace

LldbVariable::LldbVariable(DebugSession *session, TreeModel *model, TreeItem *parent,
                           const QString& expression, const QString& display)
    : MIVariable(session, model, parent, expression, display)
{
}

void LldbVariable::refetch()
{
    // NOTE: a comment in the function that calls this one describes bugs that are worked around here.
    if (!topLevel() || varobj().isEmpty()) {
        return;
    }

    if (!sessionIsAlive()) {
        return;
    }

    // update the value itself
    // LLDB-MI does not support floating variable objects (https://github.com/lldb-tools/lldb-mi/issues/105).
    // Therefore it always binds each variable object to the debuggee object, for which it was originally
    // created. Consequently, if another debuggee object with a name that matches a variable object's
    // expression is in scope at a subsequent debugger stop, the value of the variable object returned by
    // -var-evaluate-expression remains equal to the value of the original, now out-of-scope, debuggee object. Also
    // note that even if LLDB-MI starts returning the value of a matching debuggee object that is in scope, KDevelop
    // would display an obsolete type of the variable, seeing as -var-evaluate-expression does not return the type.
    // TODO: -var-create a new variable object to display the types and values of variables that are currently in scope.
    //       If the -var-create command succeeds, i.e. a matching debuggee object is in scope, compare the old and
    //       new values of attributes of the reattached variable, update its attribute isChanged() accordingly
    //       so as to match the behavior of kdevgdb, and -var-delete the replaced variable object to clean up.
    QPointer<LldbVariable> guarded_this(this);
    m_debugSession->addCommand(VarEvaluateExpression, varobj(), [guarded_this](const ResultRecord &r){
        auto* const variable = guarded_this.get();
        if (variable && r.reason == QLatin1String("done")) {
            variable->setValueToOptionalValueFieldOf(r);
        }
    });

    // update children
    // remove all children first, this will cause some gliches in the UI, but there's no good way
    // that we can know if there's anything changed
    if (isExpanded() || !childCount()) {
        deleteChildren();
        fetchMoreChildren();
    }
}

const Value* LldbVariable::handleRawUpdate(const ResultRecord& r)
{
    qCDebug(DEBUGGERLLDB) << "handleRawUpdate for variable" << varobj();
    const auto& changelist = r[rawUpdateField()];
    Q_ASSERT_X(changelist.size() <= 1, "LldbVariable::handleRawUpdate",
               "should only be used with one variable VarUpdate");
    if (changelist.empty()) {
        return nullptr;
    }

    const auto& changes = changelist[0];
    handleUpdate(changes);
    return &changes;
}

std::function<void(const ResultRecord&)> LldbVariable::handlerOfSetFormatCommand()
{
    return [guarded_this = QPointer{this}](const ResultRecord& r) {
        auto* const variable = guarded_this.get();
        if (variable && r.hasField(rawUpdateField())) {
            const auto* const changes = variable->handleRawUpdate(r);
            if (changes && !variable->inScope()) {
                // MIVariable::handleUpdate() updates value() only if the variable is in scope. But
                // LLDB-MI always sends the value in the current format. Thus update value() manually.
                variable->setValueToOptionalValueFieldOf(*changes);
            }
        }
    };
}

QString LldbVariable::formatValue(const QString& value) const
{
    // Data formatter emits value with unicode escape sequence for string and char,
    // translate them back.
    // Only check with first char is enough, as unquote will do the rest check
    if (value.startsWith(QLatin1Char('"'))) {
        return Utils::quote(Utils::unquote(value, true));
    } else if (value.startsWith(QLatin1Char('\''))) {
        return Utils::quote(Utils::unquote(value, true, QLatin1Char('\'')), QLatin1Char('\''));
    } else if (value.startsWith(QLatin1Char('b'))) {
        // this is a byte array, don't translate unicode, simply return without 'b' prefix
        return value.mid(1);
    }
    return value;
}

#include "moc_variable.cpp"

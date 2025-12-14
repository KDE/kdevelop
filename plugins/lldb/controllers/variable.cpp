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
    if (!topLevel() || varobj().isEmpty()) {
        return;
    }

    if (!sessionIsAlive()) {
        return;
    }

    // update the value itself
    QPointer<LldbVariable> guarded_this(this);
    m_debugSession->addCommand(VarEvaluateExpression, varobj(), [guarded_this](const ResultRecord &r){
        if (guarded_this && r.reason == QLatin1String("done") && r.hasField(QStringLiteral("value"))) {
            guarded_this->setValue(guarded_this->formatValue(r[QStringLiteral("value")].literal()));
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

void LldbVariable::handleRawUpdate(const ResultRecord& r)
{
    qCDebug(DEBUGGERLLDB) << "handleRawUpdate for variable" << varobj();
    const auto& changelist = r[rawUpdateField()];
    Q_ASSERT_X(changelist.size() <= 1, "LldbVariable::handleRawUpdate",
               "should only be used with one variable VarUpdate");
    if (changelist.size() == 1)
        handleUpdate(changelist[0]);
}

void LldbVariable::formatChanged()
{
    if(childCount())
    {
        for (TreeItem* item : std::as_const(childItems)) {
            Q_ASSERT(qobject_cast<MIVariable*>(item));
            if (auto* var = qobject_cast<MIVariable*>(item)) {
                var->setFormat(format());
            }
        }
    }
    else
    {
        if (sessionIsAlive()) {
            QPointer<LldbVariable> guarded_this(this);
            m_debugSession->addCommand(
                VarSetFormat,
                QStringLiteral(" %1 %2 ").arg(varobj(), format2str(format())),
                [guarded_this](const ResultRecord &r){
                    auto* const variable = guarded_this.get();
                    if (variable && r.hasField(rawUpdateField())) {
                        variable->handleRawUpdate(r);
                    }
                });
        }
    }
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

/*
 * LLDB-specific variable
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    const Value& changelist = r[QStringLiteral("changelist")];
    Q_ASSERT_X(changelist.size() <= 1, "LldbVariable::handleRawUpdate",
               "should only be used with one variable VarUpdate");
    if (changelist.size() == 1)
        handleUpdate(changelist[0]);
}

void LldbVariable::formatChanged()
{
    if(childCount())
    {
        for (TreeItem* item : qAsConst(childItems)) {
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
                    if(guarded_this && r.hasField(QStringLiteral("changelist"))) {
                        if (r[QStringLiteral("changelist")].size() > 0) {
                            guarded_this->handleRawUpdate(r);
                        }
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

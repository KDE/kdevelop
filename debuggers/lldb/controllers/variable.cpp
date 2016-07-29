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

using namespace KDevelop;
using namespace KDevMI::LLDB;
using namespace KDevMI::MI;

LldbVariable::LldbVariable(DebugSession *session, TreeModel *model, TreeItem *parent,
                         const QString& expression, const QString& display)
    : MIVariable(session, model, parent, expression, display)
{
}

void LldbVariable::handleRawUpdate(const ResultRecord& r)
{
    qCDebug(DEBUGGERLLDB) << "handleRawUpdate for variable" << varobj();
    const Value& changelist = r["changelist"];
    Q_ASSERT_X(changelist.size() <= 1, "LldbVariable::handleRawUpdate",
               "should only be used with one variable VarUpdate");
    if (changelist.size() == 1)
        handleUpdate(changelist[0]);
}

void LldbVariable::formatChanged()
{
    if(childCount())
    {
        foreach(TreeItem* item, childItems) {
            Q_ASSERT(dynamic_cast<MIVariable*>(item));
            if( MIVariable* var=dynamic_cast<MIVariable*>(item))
                var->setFormat(format());
        }
    }
    else
    {
        if (sessionIsAlive()) {
            QPointer<LldbVariable> guarded_this(this);
            debugSession->addCommand(
                VarSetFormat,
                QString(" %1 %2 ").arg(varobj_).arg(format2str(format())),
                [guarded_this](const ResultRecord &r){
                    if(guarded_this && r.hasField("changelist")) {
                        if (r["changelist"].size() > 0) {
                            guarded_this->handleRawUpdate(r);
                        }
                    }
                });
        }
    }
}

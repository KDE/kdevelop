/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDB_VARIABLE_H
#define LLDB_VARIABLE_H

#include "mivariable.h"

namespace KDevelop {
class TreeModel;
class TreeItem;
}

namespace KDevMI { namespace LLDB {
class DebugSession;
class LldbVariable : public MIVariable
{
    Q_OBJECT

public:
    LldbVariable(DebugSession *session, KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
                const QString& expression, const QString& display = QString());

    void refetch();

    using KDevelop::Variable::topLevel;

protected:
    QString formatValue(const QString &value) const override;

private:
    /**
     * Update this variable based on a given update record.
     *
     * @param r a record that contains a list field "changelist" of size 0 or 1
     * @return a pointer to a single element of the changelist or @c nullptr if the changelist is empty
     */
    const MI::Value* handleRawUpdate(const MI::ResultRecord& r);

    [[nodiscard]] std::function<void(const MI::ResultRecord&)> handlerOfSetFormatCommand() override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_VARIABLE_H

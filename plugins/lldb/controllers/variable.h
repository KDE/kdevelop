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

    void handleRawUpdate(const MI::ResultRecord &r);

    void refetch();

    using KDevelop::Variable::topLevel;

    // For testing
    using KDevelop::Variable::childCount;
    using KDevelop::Variable::child;

protected:
    void formatChanged() override;
    QString formatValue(const QString &value) const override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_VARIABLE_H

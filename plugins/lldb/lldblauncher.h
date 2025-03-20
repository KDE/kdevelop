/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBLAUNCHER_H
#define LLDBLAUNCHER_H

#include "midebuglauncher.h"

namespace KDevMI { namespace LLDB {

class LldbLauncher : public MIDebugLauncher
{
public:
    explicit LldbLauncher(MIDebuggerPlugin* plugin, IExecutePlugin* execute);
    QString description() const override;
    QString id() override;
    QString name() const override;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBLAUNCHER_H

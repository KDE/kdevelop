/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBCOMMAND_H
#define LLDBCOMMAND_H

#include "mi/micommand.h"

namespace KDevMI { namespace LLDB {

/**
 * LLDB specific version of MICommand, when LLDB-MI implements all
 * needed mi command, this class would be no longer needed.
 */
class DebugSession;
class LldbCommand : public MI::MICommand
{
protected:
    explicit LldbCommand(MI::CommandType type, const QString& arguments = QString(),
                MI::CommandFlags flags = {});
    friend class KDevMI::LLDB::DebugSession;
public:
    ~LldbCommand() override;

    QString cmdToSend() override;
    QString miCommand() const override;

private:
    QString overrideCmd;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBCOMMAND_H

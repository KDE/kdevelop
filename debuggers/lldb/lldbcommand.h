/*
 * LLDB specific version of MI command
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
    LldbCommand(MI::CommandType type, const QString& arguments = QString(),
                MI::CommandFlags flags = 0);
    friend class KDevMI::LLDB::DebugSession;
public:
    ~LldbCommand();

    QString cmdToSend() override;
    QString miCommand() const override;

private:
    QString overrideCmd;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBCOMMAND_H

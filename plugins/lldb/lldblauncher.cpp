/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lldblauncher.h"

#include "widgets/lldbconfigpage.h"

#include <KLocalizedString>

using namespace KDevMI::LLDB;

LldbLauncher::LldbLauncher(MIDebuggerPlugin* plugin, IExecutePlugin* execute)
    : MIDebugLauncher(plugin, execute, std::make_unique<LldbConfigPageFactory>())
{
}

QString LldbLauncher::id()
{
    return QStringLiteral("lldb");
}

QString LldbLauncher::name() const
{
    return i18n("LLDB");
}

QString LldbLauncher::description() const
{
    return i18n("Debug a native application in LLDB");
}

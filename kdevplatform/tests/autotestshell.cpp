/*
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "autotestshell.h"

#include <QStandardPaths>

using namespace KDevelop;

AutoTestShell::~AutoTestShell() = default;

void AutoTestShell::init(const QStringList& plugins)
{
    // TODO: Maybe generalize, add KDEVELOP_STANDALONE build option
    qputenv("CLEAR_DUCHAIN_DIR", "1"); // Always clear duchain dir (also to avoid popups asking the user to clear it)

    QStandardPaths::setTestModeEnabled(true);

    static auto instance = AutoTestShell();
    instance.m_plugins = plugins;
    s_instance = &instance;
}

/***************************************************************************
 *   Copyright 2013 Kevin Funk <kfunk@kde.org                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "autotestshell.h"

using namespace KDevelop;

AutoTestShell::~AutoTestShell() = default;

void AutoTestShell::init(const QStringList& plugins)
{
    // TODO: Maybe generalize, add KDEVELOP_STANDALONE build option
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    qputenv("KDE_FORK_SLAVES", "1"); // KIO slaves will be forked off instead of being started via DBus
#endif
    qputenv("CLEAR_DUCHAIN_DIR", "1"); // Always clear duchain dir (also to avoid popups asking the user to clear it)

    static auto instance = AutoTestShell();
    instance.m_plugins = plugins;
    s_instance = &instance;
}

/* This file is part of KDevelop
 *  Copyright 2007 Hamish Rodda <rodda@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#include "valgrindwidget.h"

#include <KIcon>
#include <KLocale>

#include "valgrindplugin.h"
#include "valgrindmodel.h"

ValgrindWidget::ValgrindWidget(ValgrindPlugin* plugin, QWidget * parent)
    : QTreeView(parent)
    , m_plugin(plugin)
{
    setWindowIcon(KIcon("fork"));
    setWindowTitle(i18n("Valgrind Output"));
    setModel(plugin->model());

    setWhatsThis( i18n( "<b>Valgrind</b><p>Shows the output of the valgrind. Valgrind detects<br/>"
        "use of uninitialized memory<br/>"
        "reading/writing memory after it has been free'd<br/>"
        "reading/writing off the end of malloc'd blocks<br/>"
        "reading/writing inappropriate areas on the stack<br/>"
        "memory leaks -- where pointers to malloc'd blocks are lost forever<br/>"
        "passing of uninitialised and/or unaddressable memory to system calls<br/>"
        "mismatched use of malloc/new/new [] vs free/delete/delete []<br/>"
        "some abuses of the POSIX pthread API.</p>" ) );
}

ValgrindPlugin * ValgrindWidget::plugin() const
{
    return m_plugin;
}

#include "valgrindwidget.moc"

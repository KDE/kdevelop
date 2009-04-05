/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _FRAMESTACKWIDGET_H_
#define _FRAMESTACKWIDGET_H_

#include <QTreeView>

#include "gdbcontroller.h"
#include "mi/miparser.h"

namespace GDBDebugger
{

class CppDebuggerPlugin;
class ThreadItem;
class FrameStackItem;

/**
 * @author John Birch
 */
class FramestackWidget : public QTreeView
{
    Q_OBJECT

public:
    FramestackWidget( CppDebuggerPlugin* plugin, GDBController* controller,
                      QWidget *parent=0 );
    virtual ~FramestackWidget();

Q_SIGNALS:
    void requestRaise();

public Q_SLOTS:
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
    virtual void showEvent(QShowEvent* event);

private:
    GDBController* controller_;
};

}

#endif

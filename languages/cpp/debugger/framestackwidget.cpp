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

#include "framestackwidget.h"

#include "gdbparser.h"
#include "gdbcommand.h"
#include "debuggerplugin.h"

#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <KIcon>

#include "threaditem.h"
#include "framestackitem.h"
#include "gdbcontroller.h"
#include "stackmanager.h"

using namespace GDBMI;
using namespace GDBDebugger;

FramestackWidget::FramestackWidget(CppDebuggerPlugin* plugin, GDBController* controller,
                                   QWidget *parent)
        : QTreeView(parent),
          controller_(controller)
{
    setToolTip(i18n("<b>Frame stack</b><p>"
                    "Often referred to as the \"call stack\", "
                    "this is a list showing what function is "
                    "currently active and who called each "
                    "function to get to this point in your "
                    "program. By clicking on an item you "
                    "can see the values in any of the "
                    "previous calling functions."));
    setWindowIcon(KIcon("table"));
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setModel(controller->stackManager());

    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(slotSelectionChanged(QItemSelection, QItemSelection)));

    connect(plugin, SIGNAL(raiseFramestackViews()), this, SIGNAL(requestRaise()));
}

FramestackWidget::~FramestackWidget()
{
}

void FramestackWidget::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected);
    if (selected.isEmpty())
        return;

    if (selected.count() > 1) {
        kWarning() << "Selection not single as requested";
        return;
    }

    // Set current frame
    QObject* selectedObject = controller_->stackManager()->objectForIndex(selected.first().topLeft());

    ThreadItem* thread = qobject_cast<ThreadItem*>(selectedObject);
    if (thread)
    {
        controller_->selectFrame(0, thread->thread());
    }
    else
    {
        FrameStackItem *frame = qobject_cast<FrameStackItem*>(selectedObject);
        if (frame)
        {
            controller_->selectFrame(frame->frame(), frame->thread()->thread());
        }
    }
}

void FramestackWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

#include "framestackwidget.moc"

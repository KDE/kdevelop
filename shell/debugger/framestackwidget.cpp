/*
 * This file is part of KDevelop
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com> 
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

#include <KDebug>
#include <KLocalizedString>
#include <KIcon>

#include "../debugcontroller.h"
#include "../../debugger/util/treemodel.h"
#include "../../debugger/interfaces/stackmodel.h"
#include <QHeaderView>

using namespace KDevelop;

FramestackWidget::FramestackWidget(DebugController* controller, QWidget* parent)
    : AsyncTreeView(0, parent), m_controller(controller)
{
    connect(controller, SIGNAL(sessionAdded(IDebugSession*)), SLOT(sessionAdded(IDebugSession*)));

    setToolTip(i18n("<b>Frame stack</b><p>"
                    "Often referred to as the \"call stack\", "
                    "this is a list showing which function is "
                    "currently active, and what called each "
                    "function to get to this point in your "
                    "program. By clicking on an item you "
                    "can see the values in any of the "
                    "previous calling functions.</p>"));
    setWindowIcon(KIcon("view-list-text"));
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    header()->setHighlightSections(false);
}

FramestackWidget::~FramestackWidget() {
}

void FramestackWidget::sessionAdded(KDevelop::IDebugSession* session)
{
    Q_UNUSED(session);
    kDebug() << isVisible();
    if (isVisible()) {
        KDevelop::IDebugSession* session = m_controller->currentSession();
        StackModel* model = session->stackModel();
        model->setAutoUpdate(true);
        setModel(model);
    }
}

void KDevelop::FramestackWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    kDebug();
    if (m_controller->currentSession()) {
        m_controller->currentSession()->stackModel()->setAutoUpdate(false);
    }
}

void KDevelop::FramestackWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    kDebug();
    if (m_controller->currentSession()) {
        StackModel* model = m_controller->currentSession()->stackModel();
        setModel(model);
        model->setAutoUpdate(true);
    }
}


#include "framestackwidget.moc"

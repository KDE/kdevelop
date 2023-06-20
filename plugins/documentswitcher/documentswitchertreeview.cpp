/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documentswitchertreeview.h"

#include <QKeyEvent>

#include <widgetcolorizer.h>
#include <path.h>
#include <projectmodel.h>
#include <interfaces/iproject.h>

#include "documentswitcherplugin.h"

using namespace KDevelop;

DocumentSwitcherTreeView::DocumentSwitcherTreeView(DocumentSwitcherPlugin* plugin_ )
    : QTreeView(nullptr)
    , plugin(plugin_)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
}

void DocumentSwitcherTreeView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        hide();
    } else {
        QTreeView::keyPressEvent(event);
    }
}

void DocumentSwitcherTreeView::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control) {
        plugin->itemActivated(selectionModel()->currentIndex());
        event->accept();
        hide();
    } else {
        QTreeView::keyReleaseEvent(event);
    }
}

void DocumentSwitcherTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    if (WidgetColorizer::colorizeByProject()) {
        if (const auto project = index.data(ProjectRole).value<IProject *>()) {
            const auto projectPath = project->path();
            const QColor color = WidgetColorizer::colorForId(qHash(projectPath), palette(), true);
            WidgetColorizer::drawBranches(this, painter, rect, index, color);
        }
    }
    // don't call the base implementation, as we only want to paint the colorization above
    // this means that for people who have the feature disabled, we get some padding on the left,
    // but that is OK imo
}

#include "moc_documentswitchertreeview.cpp"

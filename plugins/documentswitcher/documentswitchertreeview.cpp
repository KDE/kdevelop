/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

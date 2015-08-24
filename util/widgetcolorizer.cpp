/*
 * Copyright 2015  Kevin Funk <kfunk@kde.org>
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

#include "widgetcolorizer.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KColorScheme>

#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QTreeView>

using namespace KDevelop;

void WidgetColorizer::drawBranches(const QTreeView* treeView, QPainter* painter,
                                              const QRect& rect, const QModelIndex& index, const QColor& baseColor)
{
    // We want this color to be a bit transparent:
    QColor color(baseColor);
    color.setAlpha(!index.parent().isValid() ? 50 : 110);

    QRect newRect(rect);
    newRect.setWidth(treeView->indentation());
    painter->fillRect(newRect, color);
}

QColor WidgetColorizer::colorForId(uint id, const QPalette& activePalette)
{
    const KColorScheme scheme(activePalette.currentColorGroup());
    const QColor schemeColor = scheme.foreground(KColorScheme::NormalText).color();
    int lightness = (schemeColor.blue() + schemeColor.red() + schemeColor.green()) / 3;
    int high = 255;
    int low = 100;

    // Assign a bighter color if the scheme color is dark.
    if (lightness > 100) {
        low += 85;
        return QColor(qAbs((id % (high-low)) + low), qAbs(((id / 50 ) % (high-low)) + low), qAbs(((id / (50 * 50)) % (high-low)) + low));
    }

    return QColor(qAbs(id % (high-low)), qAbs((id / 50 ) % (high-low)), qAbs((id / (50 * 50)) % (high-low)));
}

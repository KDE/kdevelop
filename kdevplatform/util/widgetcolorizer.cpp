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

#include <KColorUtils>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QTreeView>

using namespace KDevelop;

QColor WidgetColorizer::blendForeground(QColor color, float ratio,
                                        const QColor& foreground, const QColor& background)
{
    if (KColorUtils::luma(foreground) > KColorUtils::luma(background)) {
        // for dark color schemes, produce a fitting color first
        color = KColorUtils::tint(foreground, color, 0.5);
    }
    // adapt contrast
    return KColorUtils::mix(foreground, color, ratio);
}

QColor WidgetColorizer::blendBackground(const QColor& color, float ratio,
                                        const QColor& /*foreground*/, const QColor& background)
{
    // adapt contrast
    return KColorUtils::mix(background, color, ratio);
}

void WidgetColorizer::drawBranches(const QTreeView* treeView, QPainter* painter,
                                   const QRect& rect, const QModelIndex& /*index*/,
                                   const QColor& baseColor)
{
    QRect newRect(rect);
    newRect.setWidth(treeView->indentation());
    painter->fillRect(newRect, baseColor);
}

QColor WidgetColorizer::colorForId(uint id, const QPalette& activePalette, bool forBackground)
{
    const int high = 255;
    const int low = 100;
    auto color = QColor(qAbs(id % (high - low)),
                        qAbs((id / 50) % (high - low)),
                        qAbs((id / (50 * 50)) % (high - low)));
    const auto& foreground = activePalette.windowText().color();
    const auto& background = activePalette.window().color();
    if (forBackground) {
        return blendBackground(color, .5, foreground, background);
    } else {
        return blendForeground(color, .5, foreground, background);
    }
}

bool WidgetColorizer::colorizeByProject()
{
    return KSharedConfig::openConfig()->group("UiSettings").readEntry("ColorizeByProject", true);
}

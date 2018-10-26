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

#pragma once

#include <util/utilexport.h>

#include <QtGlobal>

class QColor;
class QModelIndex;
class QPainter;
class QRect;
class QPalette;
class QTreeView;

namespace KDevelop {

namespace WidgetColorizer {
/**
 * Generate a new color by blending the input @p color with the foreground.
 *
 * This function also works nicely on dark color schemes, in contrast to
 * simply setting an alpha channel value on the color.
 *
 * @p color Input color to blend.
 * @p ratio Ratio to decide how strong to do the blending.
 *          When set to 0 you get the foreground color as-is, when set to 1
 *          you get the input color as-is.
 */
KDEVPLATFORMUTIL_EXPORT QColor blendForeground(QColor color, float ratio,
                                               const QColor& foreground, const QColor& background);

/**
 * Generate a new color by blending the input @p color with the background.
 *
 * This function also works nicely on dark color schemes, in contrast to
 * simply setting an alpha channel value on the color.
 *
 * @p color Input color to blend.
 * @p ratio Ratio to decide how strong to do the blending.
 *          When set to 0 you get the background color as-is, when set to 1
 *          you get the input color as-is.
 */
KDEVPLATFORMUTIL_EXPORT QColor blendBackground(const QColor& color, float ratio,
                                               const QColor& foreground, const QColor& background);

KDEVPLATFORMUTIL_EXPORT void drawBranches(const QTreeView* treeView, QPainter* painter,
                                          const QRect& rect, const QModelIndex& index, const QColor& color);

/**
 * Return a random color fit for the active palette.
 *
 * @p id An id which can be generated e.g. by qHash. Same ids will return
 *       the same color.
 * @p activePalette The palette to use for generating the color.
 * @p background If set to true, a background color will be returned,
 *               otherwise a foreground color will be returned by default.
 */
KDEVPLATFORMUTIL_EXPORT QColor colorForId(uint id, const QPalette& activePalette,
                                          bool background = false);

/**
 * Returns true when the setting is enabled to colorize widgets representing
 * files belonging to projects.
 */
KDEVPLATFORMUTIL_EXPORT bool colorizeByProject();
}

}

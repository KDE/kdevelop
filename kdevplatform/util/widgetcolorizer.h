/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
class QTextDocument;

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

/**
 * Convert colors in the @p doc to make them readable if the current palette is using a dark theme
 */
KDEVPLATFORMUTIL_EXPORT void convertDocumentToDarkTheme(QTextDocument* doc);
}

}

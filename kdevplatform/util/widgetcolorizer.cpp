/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "widgetcolorizer.h"

#include <KColorUtils>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QTreeView>
#include <QTextDocument>
#include <QApplication>
#include <QTextCharFormat>
#include <QTextFrame>

#include <optional>

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
    return KSharedConfig::openConfig()->group(QStringLiteral("UiSettings")).readEntry("ColorizeByProject", true);
}

namespace
{
struct FormatRange {
    int start = 0;
    int end = 0;
    QTextCharFormat format;
};

QColor invertColor(const QColor& color)
{
    auto hue = color.hsvHue();
    if (hue == -1) {
        // achromatic color
        hue = 0;
    }
    return QColor::fromHsv(hue, color.hsvSaturation(), 255 - color.value());
}

std::optional<QColor> foregroundColor(const QTextFormat& format)
{
    if (!format.hasProperty(QTextFormat::ForegroundBrush))
        return std::nullopt;
    return format.foreground().color();
}

std::optional<QColor> backgroundColor(const QTextFormat& format)
{
    if (!format.hasProperty(QTextFormat::BackgroundBrush))
        return std::nullopt;
    return format.background().color();
}

/**
 * Inverting is used for white colors, because it is assumed white in light color scheme
 * should be black in dark color scheme. White inverted will give you black, but blending
 * would give you a grey color depending on the ratio. Above 0.5 (middle) will ensure all
 * whites are inverted in blacks (below 0.5) and exactly 0.5 can stay the same.
 * The 0.08 saturation covered all the white tones found in the color schemes tested.
 */
bool canInvertBrightColor(const QColor& color)
{
    // this check here determines if the color can be considered close to white
    return color.valueF() > 0.5 && color.hsvSaturationF() < 0.08;
}

/**
 * Blending is used for non white (colorful?) colors to increase contrast (get a brighter color).
 * Inverting is not possible for non white/black colors and would just create a different color
 * not guaranteed to be brighter.
 */
bool canBlendForegroundColor(const QColor& color)
{
    // a foreground color with other hsv values will give bad contrast against a dark background
    return color.valueF() < 0.7;
}

bool isBrightBackgroundColor(const QColor& color)
{
    // NOTE that foreground contrast and background contrast work differently
    return color.valueF() > 0.3;
}

bool canInvertDarkColor(const QColor& color)
{
    return !isBrightBackgroundColor(color);
}

void collectRanges(QTextFrame* frame, const QColor& fgcolor, const QColor& bgcolor, bool bgSet,
                   std::vector<FormatRange>& ranges)
{
    for (auto it = frame->begin(); it != frame->end(); ++it) {
        if (auto frame = it.currentFrame()) {
            auto fmt = it.currentFrame()->frameFormat();
            if (auto background = backgroundColor(fmt)) {
                collectRanges(frame, fgcolor, *background, true, ranges);
            } else {
                collectRanges(frame, fgcolor, bgcolor, bgSet, ranges);
            }
        }

        const auto block = it.currentBlock();
        if (!block.isValid()) {
            continue;
        }

        for (auto jt = block.begin(); jt != block.end(); ++jt) {
            auto fragment = jt.fragment();
            auto text = QStringView(fragment.text()).trimmed();
            if (!text.isEmpty()) {
                auto fmt = fragment.charFormat();
                auto foreground = foregroundColor(fmt);
                auto background = backgroundColor(fmt);

                if (!bgSet && !background) {
                    if (!foreground || foreground == Qt::black) {
                        fmt.setForeground(fgcolor);
                    } else if (canInvertDarkColor(*foreground)) {
                        fmt.setForeground(invertColor(*foreground));
                    } else if (canBlendForegroundColor(*foreground)) {
                        fmt.setForeground(WidgetColorizer::blendForeground(*foreground, 1.0, fgcolor, bgcolor));
                    }
                } else {
                    auto bg = background.value_or(bgcolor);
                    auto fg = foreground.value_or(fgcolor);
                    if (background && canInvertBrightColor(bg)) {
                        bg = invertColor(bg);
                        fmt.setBackground(bg);
                        if (canInvertDarkColor(fg)) {
                            fmt.setForeground(invertColor(fg));
                        } else if (canBlendForegroundColor(fg)) {
                            fmt.setForeground(WidgetColorizer::blendForeground(fg, 1.0, fgcolor, bg));
                        }
                    } else if (isBrightBackgroundColor(bg) && canInvertBrightColor(fg)) {
                        fg = invertColor(fg);
                        fmt.setForeground(fg);
                    }
                }
                ranges.push_back({fragment.position(), fragment.position() + fragment.length(), fmt});
            }
        }
    }
};
}

// see also: https://invent.kde.org/kdevelop/kdevelop/-/merge_requests/370#note_487717
void WidgetColorizer::convertDocumentToDarkTheme(QTextDocument* doc)
{
    const auto palette = QApplication::palette();
    const auto bgcolor = palette.color(QPalette::Base);
    const auto fgcolor = palette.color(QPalette::Text);

    if (fgcolor.value() < bgcolor.value())
        return;

    std::vector<FormatRange> ranges;
    collectRanges(doc->rootFrame(), fgcolor, bgcolor, false, ranges);

    auto cur = QTextCursor(doc);
    cur.beginEditBlock();
    for (const auto& [start, end, format] : ranges) {
        cur.setPosition(start);
        cur.setPosition(end, QTextCursor::KeepAnchor);
        cur.setCharFormat(format);
    }
    cur.endEditBlock();
}

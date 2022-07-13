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

void collectRanges(QTextFrame* frame, const QColor& fgcolor, const QColor& bgcolor, bool bgSet,
                   std::vector<FormatRange>& ranges)
{
    for (auto it = frame->begin(); it != frame->end(); ++it) {
        if (auto frame = it.currentFrame()) {
            auto fmt = it.currentFrame()->frameFormat();
            if (fmt.hasProperty(QTextFormat::BackgroundBrush)) {
                collectRanges(frame, fgcolor, fmt.background().color(), true, ranges);
            } else {
                collectRanges(frame, fgcolor, bgcolor, bgSet, ranges);
            }
        }

        if (it.currentBlock().isValid()) {
            for (auto jt = it.currentBlock().begin(); jt != it.currentBlock().end(); ++jt) {
                auto fragment = jt.fragment();
                auto text = fragment.text().trimmed();
                if (!text.isEmpty()) {
                    auto fmt = fragment.charFormat();

                    if (!bgSet && !fmt.hasProperty(QTextFormat::BackgroundBrush)) {
                        if (!fmt.hasProperty(QTextFormat::ForegroundBrush) || fmt.foreground().color() == Qt::black)
                            fmt.setForeground(fgcolor);
                        else if (fmt.foreground().color().valueF() < 0.7)
                            fmt.setForeground(WidgetColorizer::blendForeground(fmt.foreground().color(), 1.0, fgcolor,
                                                                               fmt.background().color()));

                    } else {
                        QColor bg = fmt.hasProperty(QTextFormat::BackgroundBrush) ? fmt.background().color() : bgcolor;
                        QColor fg = fmt.hasProperty(QTextFormat::ForegroundBrush) ? fmt.foreground().color() : fgcolor;
                        if (bg.valueF() > 0.3) {
                            if (fmt.hasProperty(QTextFormat::BackgroundBrush) && bg.valueF() > 0.5
                                && bg.hsvSaturationF() < 0.08) {
                                bg = invertColor(bg);
                                fmt.setBackground(bg);
                                if (fg.valueF() < 0.7) {
                                    fmt.setForeground(WidgetColorizer::blendForeground(fg, 1.0, fgcolor, bg));
                                }
                            } else if (fg.valueF() > 0.5 && fg.hsvSaturationF() < 0.08) {
                                fg = invertColor(fg);
                                fmt.setForeground(fg);
                            }
                        }
                    }
                    ranges.push_back({fragment.position(), fragment.position() + fragment.length(), fmt});
                }
            }
        }
    }
};
}

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
    for (const auto& [start, end, format] : ranges) {
        cur.setPosition(start);
        cur.setPosition(end, QTextCursor::KeepAnchor);
        cur.setCharFormat(format);
    }
}

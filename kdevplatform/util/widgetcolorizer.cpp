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

void WidgetColorizer::convertDocumentToDarkTheme(QTextDocument* doc)
{
    const auto palette = QApplication::palette();
    const auto bgcolor = palette.color(QPalette::Base);
    const auto fgcolor = palette.color(QPalette::Text);

    if (fgcolor.value() < bgcolor.value())
        return;

    auto cur = QTextCursor(doc);
    std::vector<std::pair<QTextCursor, QTextCharFormat> > cursors;

    std::function<void(QTextFrame*, QColor, QColor, bool)> iterateFrame = [&iterateFrame, &cur, &cursors](QTextFrame* frame, QColor fgcolor, QColor bgcolor, bool bgSet) {
        for (auto it = frame->begin(); it != frame->end(); ++it) {
            if (auto frame = it.currentFrame()) {
                auto fmt = it.currentFrame()->frameFormat();
                if (fmt.hasProperty(QTextFormat::BackgroundBrush)) {
                    auto bg = fmt.background().color();

                    iterateFrame(frame, fgcolor, bg, true);
                } else {
                    iterateFrame(frame, fgcolor, bgcolor, bgSet);
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
                                fmt.setForeground(WidgetColorizer::blendForeground(fmt.foreground().color(), 1.0, fgcolor, fmt.background().color()));

                        } else {
                            QColor bg = fmt.hasProperty(QTextFormat::BackgroundBrush) ? fmt.background().color() : bgcolor;
                            QColor fg = fmt.hasProperty(QTextFormat::ForegroundBrush) ? fmt.foreground().color() : fgcolor;
                            if (bg.valueF() > 0.3) {
                                if (fmt.hasProperty(QTextFormat::BackgroundBrush) && bg.valueF() > 0.5 && bg.hsvSaturationF() < 0.08) {
                                    bg = QColor::fromHsv(bg.hsvHue(), bg.hsvSaturation(), 255 - bg.value() );
                                    fmt.setBackground(bg);
                                    if (fg.valueF() < 0.7) {
                                        fmt.setForeground(WidgetColorizer::blendForeground(fg, 1.0, fgcolor, bg));
                                    }
                                } else if (fg.valueF() > 0.5 && fg.hsvSaturationF() < 0.08) {
                                    fg = QColor::fromHsv(fg.hsvHue(), fg.hsvSaturation(), 255 - fg.value() );
                                    fmt.setForeground(fg);
                                }
                            }
                        }
                        cur.setPosition(fragment.position());
                        cur.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                        cursors.push_back(std::make_pair(cur, fmt));
                    }
                }
            }
        }
    };
    iterateFrame(doc->rootFrame(), fgcolor, bgcolor, false);

    for (auto p : cursors)
        p.first.setCharFormat(p.second);
}

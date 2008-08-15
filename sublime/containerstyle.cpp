/***************************************************************************
 *   Copyright (C) 2007 Fredrik Höglund <fredrik@kde.org>                  *
 *   Originally created as konqproxystyle.cpp, adapted to KDevelop.        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
#include "containerstyle.h"

#include <QtGui/QWidget>
#include <QStyleOptionTabV2>

#include "container.h"

namespace Sublime {

ContainerStyle::ContainerStyle(QWidget *parent)
    :QStyle(), parent(parent)
{
}

QStyle *ContainerStyle::style() const
{
    return parent->parentWidget()->style();
}

void ContainerStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
    QPainter *painter, const QWidget *widget) const
{
    style()->drawComplexControl(control, option, painter, widget);
}

void ContainerStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
    const QWidget *widget) const
{
    style()->drawControl(element, option, painter, widget);
}

void ContainerStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment,
    const QPixmap &pixmap) const
{
    style()->drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void ContainerStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette,
    bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    style()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void ContainerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
    const QWidget *widget) const
{
    if (element == PE_FrameTabWidget)
    {
        const Container *tw = static_cast<const Sublime::Container*>(widget);
        const QTabBar *tb = tw->tabBar();

        QStyleOptionTabV2 tab;
        tab.initFrom(tb);
        tab.shape = tb->shape();
        int overlap = style()->pixelMetric(PM_TabBarBaseOverlap, &tab, tb);

        if (overlap <= 0 || tw->isTabBarHidden() || tb->count() == 0)
            return;

        QStyleOptionTabBarBase opt;
        opt.initFrom(tb);

        opt.selectedTabRect = tb->tabRect(tb->currentIndex());
        opt.selectedTabRect = QRect(tb->mapToParent(opt.selectedTabRect.topLeft()), opt.selectedTabRect.size());
        opt.tabBarRect = QRect(tb->mapToParent(tb->rect().topLeft()), tb->size());

        if (tw->tabPosition() == QTabWidget::North)
            opt.rect = QRect(option->rect.left(), option->rect.top(), option->rect.width(), overlap);
        else
            opt.rect = QRect(option->rect.left(), option->rect.bottom() - overlap - 1, option->rect.width(), overlap);

        style()->drawPrimitive(PE_FrameTabBarBase, &opt, painter, tb);
        return;
    }

    style()->drawPrimitive(element, option, painter, widget);
}

QPixmap ContainerStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
    const QStyleOption *option) const
{
    return style()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl ContainerStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
    const QPoint &position, const QWidget *widget) const
{
    return style()->hitTestComplexControl(control, option, position, widget);
}

QRect ContainerStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return style()->itemPixmapRect(rectangle, alignment, pixmap);
}

QRect ContainerStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
    bool enabled, const QString &text) const
{
    return style()->itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int ContainerStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return style()->pixelMetric(metric, option, widget);
}

void ContainerStyle::polish(QWidget *widget)
{
    style()->polish(widget);
}

void ContainerStyle::polish(QApplication *application)
{
    style()->polish(application);
}

void ContainerStyle::polish(QPalette &palette)
{
    style()->polish(palette);
}

QSize ContainerStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
    const QSize &contentsSize, const QWidget *widget) const
{
    return style()->sizeFromContents(type, option, contentsSize, widget);
}

QIcon ContainerStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
    const QWidget *widget) const
{
    return style()->standardIcon(standardIcon, option, widget);
}

QPixmap ContainerStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
    const QWidget *widget) const
{
    return style()->standardPixmap(standardPixmap, option, widget);
}

QPalette ContainerStyle::standardPalette() const
{
    return style()->standardPalette();
}

int ContainerStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
    QStyleHintReturn *returnData) const
{
    return style()->styleHint(hint, option, widget, returnData);
}

QRect ContainerStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
    SubControl subControl, const QWidget *widget) const
{
    return style()->subControlRect(control, option, subControl, widget);
}

QRect ContainerStyle::subElementRect(SubElement element, const QStyleOption *option,
    const QWidget *widget) const
{
    if (element == SE_TabWidgetTabContents)
    {
        QRect rect = style()->subElementRect(SE_TabWidgetTabPane, option, widget);

        const Container *tw = static_cast<const Sublime::Container*>(widget);
        const QTabBar *tb = tw->tabBar();

        QStyleOptionTabV2 tab;
        tab.initFrom(tb);
        tab.shape = tb->shape();
        int overlap = style()->pixelMetric(PM_TabBarBaseOverlap, &tab, tw->tabBar());

        if (overlap <= 0 || tw->isTabBarHidden())
            return rect;

        return tw->tabPosition() == QTabWidget::North ?
            rect.adjusted(0, overlap, 0, 0) : rect.adjusted(0, 0, 0, -overlap);
    }
    return style()->subElementRect(element, option, widget);
}

void ContainerStyle::unpolish(QWidget *widget)
{
    style()->unpolish(widget);
}

void ContainerStyle::unpolish(QApplication *application)
{
    style()->unpolish(application);
}

}

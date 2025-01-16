/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2018 Amish Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#include "idealtoolbutton.h"
#include <KAcceleratorManager>
#include <QStyleOption>
#include <QStylePainter>
#include <QApplication>

IdealToolButton::IdealToolButton(Qt::DockWidgetArea area, QWidget *parent)
    : QToolButton(parent), _area(area)
{
    setFocusPolicy(Qt::NoFocus);
    KAcceleratorManager::setNoAccel(this);
    setCheckable(true);
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    setContextMenuPolicy(Qt::CustomContextMenu);

    QSizePolicy sizePolicy = this->sizePolicy();
    if (orientation() == Qt::Horizontal) {
        sizePolicy.setHorizontalPolicy(QSizePolicy::Maximum);
    } else {
        sizePolicy.setVerticalPolicy(QSizePolicy::Maximum);
    }
    setSizePolicy(sizePolicy);

    connect(this, &QAbstractButton::pressed, [this] {
        m_isPressed = true;
    });
    connect(this, &QAbstractButton::released, [this] {
        m_isPressed = false;
    });
}

Qt::Orientation IdealToolButton::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

bool IdealToolButton::isPressed() const
{
    return m_isPressed;
}

QSize IdealToolButton::sizeHint() const
{
    ensurePolished();

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QFontMetrics fm = fontMetrics();

    const int charWidth = fm.horizontalAdvance(QLatin1Char('x'));
    QSize textSize;
    // Use text size only if we request text
    if (toolButtonStyle() != Qt::ToolButtonIconOnly || opt.icon.isNull()) {
        textSize = fm.size(Qt::TextShowMnemonic, opt.text);
        textSize.rwidth() += 2 * charWidth;
    }

    int iconwidth = 0, iconheight = 0;
    // Use icon size only if it's requested and the icon is valid
    if (toolButtonStyle() != Qt::ToolButtonTextOnly && !opt.icon.isNull()) {
        if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
            iconwidth = opt.iconSize.width();
            iconheight = opt.iconSize.height();
        } else {
            iconwidth = opt.iconSize.height();
            iconheight = opt.iconSize.width();
        }
    }
    // adding +4 to be consistent with qtoolbutton
    int width = textSize.width() + iconwidth + 4;
    int height = qMax(textSize.height(), iconheight);
    QSize size = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(width, height), this);

    if (orientation() == Qt::Vertical) {
        return QSize(size.height(), size.width()); // transposed
    } else {
        return size;
    }
}

QSize IdealToolButton::minimumSizeHint() const
{
    ensurePolished();

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QSize minimumSize;
    // if style has icons, minimumSize is the size of the icon
    if (toolButtonStyle() != Qt::ToolButtonTextOnly && !opt.icon.isNull()) {
        minimumSize = opt.iconSize;
        if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea) {
            minimumSize.transpose();
        }
    } else {
        // if no icon, set an arbitrary minimum size
        QFontMetrics fm = fontMetrics();
        minimumSize = fm.size(Qt::TextShowMnemonic, opt.text.left(4));
    }

    minimumSize = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, minimumSize, this);

    if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
        return minimumSize;
    }
    return minimumSize.transposed();
}


void IdealToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    QStyleOptionToolButton option;
    initStyleOption(&option);

    if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
        // elide text
        int iconWidth = 0;
        if (toolButtonStyle() != Qt::ToolButtonTextOnly && !option.icon.isNull()) {
            iconWidth = option.iconSize.width();
        }

        // subtract 4 to be consistent with the size calculated by sizeHint, which adds 4,
        // again to be consistent with QToolButton
        option.text = fontMetrics().elidedText(text(), Qt::ElideRight, contentsRect().width() - iconWidth - 4);
        painter.drawComplexControl(QStyle::CC_ToolButton, option);
    } else {
        // rotated paint
        // elide text
        int iconHeight = 0;
        if (toolButtonStyle() != Qt::ToolButtonTextOnly && !option.icon.isNull()) {
            iconHeight = option.iconSize.height();
        }
        QString textToDraw = fontMetrics().elidedText(text(), Qt::ElideRight, contentsRect().height() - iconHeight - 4);
        // first draw normal frame and not text/icon
        option.text = QString();
        option.icon = QIcon();
        painter.drawComplexControl(QStyle::CC_ToolButton, option);

        // rotate the options
        QSize size( option.rect.size() );
        size.transpose();
        option.rect.setSize( size );

        // rotate the painter
        if(_area == Qt::LeftDockWidgetArea) {
            painter.translate( 0, height() );
            painter.rotate( -90 );
        } else {
            painter.translate( width(), 0 );
            painter.rotate( 90 );
        }

        // paint text and icon
        option.text = textToDraw;

        QIcon::Mode iconMode = (option.state & QStyle::State_MouseOver) ? QIcon::Active : QIcon::Normal;
        QPixmap ic = icon().pixmap(option.iconSize, iconMode, QIcon::On);
        QTransform tf;
        if(_area == Qt::LeftDockWidgetArea) {
            tf.rotate(90);
        } else {
            tf.rotate(-90);
        }
        option.icon = ic.transformed( tf, Qt::SmoothTransformation );
        painter.drawControl(QStyle::CE_ToolButtonLabel, option);
        painter.end();
    }
}

#include "moc_idealtoolbutton.cpp"

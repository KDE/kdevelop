/* 
   Copyright 2010 Unknown Author (Qt Centre)
   Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "richtextpushbutton.h"

#include <QPainter>
#include <QPixmap>
#include <QTextDocument>
#include <QIcon>
#include <QSize>
#include <QMenu>
#include <QStylePainter>

using namespace KDevelop;

RichTextPushButton::RichTextPushButton(QWidget *parent) :
    QPushButton(parent)
{
}

void RichTextPushButton::setHtml(const QString &text)
{
    htmlText = text;
    isRichText = true;
 
    QPalette palette;
    palette.setBrush(QPalette::ButtonText, Qt::transparent);
    setPalette(palette);
}

void RichTextPushButton::setText(const QString &text)
{
    isRichText = false;
    QPushButton::setText(text);
}

QSize RichTextPushButton::sizeHint() const
{
    if(!isRichText) {
        return QPushButton::sizeHint();
    } else{
        QTextDocument richTextLabel;
        richTextLabel.setHtml(htmlText);
        return richTextLabel.size().toSize();
    }
}

void RichTextPushButton::paintEvent(QPaintEvent *event)
{
    if (isRichText) {
        QStylePainter p(this);
 
        QRect buttonRect = rect();
        QPoint point;
 
        QTextDocument richTextLabel;
        richTextLabel.setHtml(htmlText);
 
        QPixmap richTextPixmap(richTextLabel.size().width(), richTextLabel.size().height());
        richTextPixmap.fill(Qt::transparent);
        QPainter richTextPainter(&richTextPixmap);
        richTextLabel.drawContents(&richTextPainter, richTextPixmap.rect());
 
        if (!icon().isNull())
            point = QPoint(buttonRect.x() + buttonRect.width() / 2 + iconSize().width() / 2 + 2, buttonRect.y() + buttonRect.height() / 2);
        else
            point = QPoint(buttonRect.x() + buttonRect.width() / 2 - 1, buttonRect.y() + buttonRect.height() / 2);
 
        buttonRect.translate(point.x() - richTextPixmap.width() / 2, point.y() - richTextPixmap.height() / 2);
 
        p.drawControl(QStyle::CE_PushButton, getStyleOption());
        p.drawPixmap(buttonRect.left(), buttonRect.top(), richTextPixmap.width(), richTextPixmap.height(),richTextPixmap);
    } else
        QPushButton::paintEvent(event);
}

QStyleOptionButton RichTextPushButton::getStyleOption() const
{
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.features = QStyleOptionButton::None;
    if (isFlat())
        opt.features |= QStyleOptionButton::Flat;
    if (menu())
        opt.features |= QStyleOptionButton::HasMenu;
    if (autoDefault() || isDefault())
        opt.features |= QStyleOptionButton::AutoDefaultButton;
    if (isDefault())
        opt.features |= QStyleOptionButton::DefaultButton;
    if (isDown() || (menu() && menu()->isVisible()))
        opt.state |= QStyle::State_Sunken;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (!isFlat() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (!isRichText)
        opt.text = QPushButton::text();
    opt.icon = icon();
    opt.iconSize = iconSize();
    return opt;
}

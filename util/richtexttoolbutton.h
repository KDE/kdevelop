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

#ifndef KDEVPLATFORM_RICHTEXTTOOLBUTTON_H
#define KDEVPLATFORM_RICHTEXTTOOLBUTTON_H

#include <QToolButton>
#include <QString>
#include <QStyleOptionButton>
#include "utilexport.h"

namespace KDevelop {
class KDEVPLATFORMUTIL_EXPORT RichTextToolButton : public QToolButton
{
Q_OBJECT
public:
    explicit RichTextToolButton(QWidget *parent = 0);

    void setHtml(const QString &text);
    void setText(const QString &text);
    QString text() const;

    virtual QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    QString htmlText;
    bool isRichText;

    QStyleOptionButton getStyleOption() const;
};

}

#endif // KDEVPLATFORM_RICHTEXTPUSHBUTTON_H

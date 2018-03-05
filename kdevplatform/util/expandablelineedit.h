/*
 * Copyright 2018 Anton Anikin <anton@anikin.xyz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef EXPANDABLE_LINE_EDIT_H
#define EXPANDABLE_LINE_EDIT_H

#include "utilexport.h"
#include <QLineEdit>

/** This class implements a line edit widget which tries to expand its
 * width to fit typed text. When no text typed or text is too short
 * the widget's width is defined by the preferredWidth() value
 * (default is 200 px).
 */

class KDEVPLATFORMUTIL_EXPORT KExpandableLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit KExpandableLineEdit(QWidget* parent = nullptr);

    ~KExpandableLineEdit() override;

    int preferredWidth() const;
    void setPreferredWidth(int width);

    QSize sizeHint() const override;

protected:
    int m_preferredWidth;
};

#endif

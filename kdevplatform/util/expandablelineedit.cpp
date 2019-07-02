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

#include "expandablelineedit.h"

KExpandableLineEdit::KExpandableLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(this, &KExpandableLineEdit::textChanged, this, &KExpandableLineEdit::updateGeometry);
}

KExpandableLineEdit::~KExpandableLineEdit()
{
}

int KExpandableLineEdit::preferredWidth() const
{
    return m_preferredWidth;
}

void KExpandableLineEdit::setPreferredWidth(int width)
{
    if (m_preferredWidth != width) {
        m_preferredWidth = width;
        updateGeometry();
    }
}

QSize KExpandableLineEdit::sizeHint() const
{
    auto idealSize = QLineEdit::sizeHint();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int idealWidth = fontMetrics().horizontalAdvance(text());
#else
    int idealWidth = fontMetrics().width(text());
#endif
    if (isClearButtonEnabled()) {
        idealWidth += 2 * idealSize.height();
    }
    idealSize.setWidth(qMax(idealWidth, m_preferredWidth));

    return idealSize;
}

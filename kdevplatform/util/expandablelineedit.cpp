/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

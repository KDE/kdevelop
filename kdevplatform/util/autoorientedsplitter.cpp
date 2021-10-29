/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "autoorientedsplitter.h"

#include <QResizeEvent>

using namespace KDevelop;

AutoOrientedSplitter::AutoOrientedSplitter(QWidget* parent)
    : QSplitter(parent)
{
}

AutoOrientedSplitter::AutoOrientedSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
{
}

void AutoOrientedSplitter::resizeEvent(QResizeEvent* e)
{
    const QSize size = e->size();
    const float ratio = ( float )size.width() / size.height();
    const Qt::Orientation orientation = (ratio < 1.0 ? Qt::Vertical : Qt::Horizontal);
    setOrientation(orientation);

    QSplitter::resizeEvent(e);
}

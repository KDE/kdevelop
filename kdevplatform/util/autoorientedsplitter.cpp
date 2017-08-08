/*  This file is part of KDevelop
    Copyright 2014 Kevin Funk <kfunk@kde.org>

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
    const float ratio = (float)size.width() / size.height();
    const Qt::Orientation orientation = (ratio < 1.0 ? Qt::Vertical : Qt::Horizontal);
    setOrientation(orientation);

    QSplitter::resizeEvent(e);
}


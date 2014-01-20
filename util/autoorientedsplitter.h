/*  This file is part of KDevelop
    Copyright 2014 Kevin Funk <kevin@kfunk.org>

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

#ifndef KDEVPLATFORM_AUTOORIENTEDSPLITTER_H
#define KDEVPLATFORM_AUTOORIENTEDSPLITTER_H

#include "utilexport.h"

#include <QSplitter>

namespace KDevelop {

/**
 * Auto-oriented version of QSplitter based on the aspect ratio of the widget size
 *
 * In case this widget is resized, we check whether we're currently in
 * "portrait" (width < height) or "landscape" (width >= height) mode.
 * Consequently, in "portrait" mode the QSplitter orientation is set to Qt::Vertical
 * in order to get a vertical layout of the items -- Qt::Horizontal is set for
 * "landscape" mode
 */
class KDEVPLATFORMUTIL_EXPORT AutoOrientedSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit AutoOrientedSplitter(QWidget* parent = 0);
    explicit AutoOrientedSplitter(Qt::Orientation orientation, QWidget* parent = 0);

protected:
    virtual void resizeEvent(QResizeEvent*);
};

}

#endif

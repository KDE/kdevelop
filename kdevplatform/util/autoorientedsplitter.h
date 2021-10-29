/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit AutoOrientedSplitter(QWidget* parent = nullptr);
    explicit AutoOrientedSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent*) override;
};

}

#endif

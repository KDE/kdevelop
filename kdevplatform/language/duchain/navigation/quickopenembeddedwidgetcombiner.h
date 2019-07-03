/*
 * This file is part of KDevelop
 *
 * Copyright 2019 Milian Wolff <mail@milianw.de>
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
#ifndef KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H
#define KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H

#include <QWidget>

#include <interfaces/quickopendataprovider.h>

namespace KDevelop {
class QuickOpenEmbeddedWidgetCombinerPrivate;

/**
 * A widget that implements the QuickOpenEmbeddedWidgetInterface by asking its direct children.
 *
 * I.e. add widgets into the combiner's layout. If the widgets support the QuickOpenEmbeddedWidgetInterface,
 * then they will be used to implement the keyboard navigation features.
 */
class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenEmbeddedWidgetCombiner : public QWidget, public QuickOpenEmbeddedWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::QuickOpenEmbeddedWidgetInterface)

public:
    explicit QuickOpenEmbeddedWidgetCombiner(QWidget* parent = nullptr);
    ~QuickOpenEmbeddedWidgetCombiner() override;

    bool next() override;
    bool previous() override;
    bool up() override;
    bool down() override;
    void back() override;
    void accept() override;
    void resetNavigationState() override;

private:
    const QScopedPointer<QuickOpenEmbeddedWidgetCombinerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QuickOpenEmbeddedWidgetCombiner)
};
}

#endif // KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H

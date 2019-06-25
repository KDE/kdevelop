/*
    Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDEVPLATFORM_SUBLIMEVIEWBARCONTAINER_H
#define KDEVPLATFORM_SUBLIMEVIEWBARCONTAINER_H

#include "sublimeexport.h"
// Qt
#include <QWidget>

namespace Sublime {

class ViewBarContainerPrivate;

/**
@short Container for view bars.

Keeps a list of view bars and shows only one at a time.
*/
class KDEVPLATFORMSUBLIME_EXPORT ViewBarContainer: public QWidget
{
    Q_OBJECT

public:
    explicit ViewBarContainer(QWidget *parent = nullptr);
    ~ViewBarContainer() override;

public:
    /**
     * Adds @p viewBar to the container.
     * It will be reparented to the container object.
     * Though it remains the duty of the caller to manage the lifetime of the object.
     *
     * If on deletion of the container the @p viewBar has not been removed before,
     * the @p viewBar will be get nullptr set as parent.
     */
    void addViewBar(QWidget* viewBar);
    /**
     * Removes @p viewBar from the container.
     * The ownership of @p viewBar is not changed and should be handled by the caller.
     */
    void removeViewBar(QWidget* viewBar);
    /**
     * Sets @p viewBar as the one currently in front.
     */
    void setCurrentViewBar(QWidget* viewBar);
    /**
     * Ensures @p viewBar is the one currently in front, and being shown.
     */
    void showViewBar(QWidget* viewBar);
    /**
     * Ensures @p viewBar is the one currently in front, but also hidden.
     */
    void hideViewBar(QWidget* viewBar);

private:
    const QScopedPointer<class ViewBarContainerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ViewBarContainer)
};

}

#endif


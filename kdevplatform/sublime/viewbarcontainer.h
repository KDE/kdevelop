/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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


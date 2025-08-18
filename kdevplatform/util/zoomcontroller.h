/*
    SPDX-FileCopyrightText: 2016 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_ZOOMCONTROLLER_H
#define KDEVPLATFORM_ZOOMCONTROLLER_H

#include "utilexport.h"

#include <QObject>
#include <QScopedPointer>

class KConfigGroup;
class QKeyEvent;
class QWheelEvent;

namespace KDevelop {

class ZoomControllerPrivate;

/**
 * @brief Stores zoom factor; provides common zoom operations and notifications
 */
class KDEVPLATFORMUTIL_EXPORT ZoomController : public QObject
{
    Q_OBJECT

public:
    /**
     * @param configGroup a writable (not read-only) config group to store the zoom factor in
     */
    explicit ZoomController(const KConfigGroup& configGroup,
                            QObject* parent = nullptr);

    ~ZoomController() override;

    /**
     * @return Current zoom factor
     */
    double factor() const;

    /**
     * @brief Sets current zoom factor to @p factor
     */
    void setFactor(double factor);

    /**
     * @brief Changes current zoom factor according to @p scale
     *
     * @param scale If equal to 0, factor remains the same;
     *        otherwise, larger scale leads to larger factor.
     *        Can be both positive (zoom in) and negative (zoom out).
     */
    void zoomBy(double scale);

    /**
     * @brief Changes current zoom factor in response to designated shortcuts
     *
     * @param event Event to be analyzed and possibly accepted
     *
     * @return true If the event was accepted as a zoom shortcut
     */
    bool handleKeyPressEvent(QKeyEvent* event);

    /**
     * @brief Changes current zoom factor in response to Ctrl+mouse_scroll
     *
     * @param event Event to be analyzed and possibly accepted
     *
     * @return true If the event was accepted as a zoom scroll
     */
    bool handleWheelEvent(QWheelEvent* event);

public Q_SLOTS:
    /**
     * @brief Increases current zoom factor by the default multiplier
     */
    void zoomIn();

    /**
     * @brief Decreases current zoom factor by the default multiplier
     */
    void zoomOut();

    /**
     * @brief Sets current zoom factor to the default value (1.0)
     */
    void resetZoom();

Q_SIGNALS:
    /**
     * @brief Notifies when current zoom factor is changed
     *
     * @param factor Current zoom factor
     */
    void factorChanged(double factor);

private:
    const QScopedPointer<class ZoomControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ZoomController)
};

}

#endif // KDEVPLATFORM_ZOOMCONTROLLER_H

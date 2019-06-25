/*
 * Copyright 2016 Igor Kushnir <igorkuo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "zoomcontroller.h"

#include <KConfigGroup>

#include <QPoint>
#include <QKeyEvent>
#include <QWheelEvent>

#include <cmath>

namespace {

constexpr const char* factorConfigEntryKey() { return "Zoom Factor"; }

constexpr double defaultFactor{1};
constexpr double defaultMultiplier{1.1};

double multiplier(double scale)
{
    // Allow finer-grained control over zoom factor compared to defaultMultiplier
    constexpr double smallMultiplier{1.05};
    return std::pow(smallMultiplier, scale);
}

} // namespace

using KDevelop::ZoomControllerPrivate;
using KDevelop::ZoomController;

class KDevelop::ZoomControllerPrivate
{
public:
    explicit ZoomControllerPrivate(const KConfigGroup& configGroup);
    void writeConfig();

    KConfigGroup m_configGroup;
    double m_factor{defaultFactor};
};

ZoomControllerPrivate::ZoomControllerPrivate(const KConfigGroup& configGroup)
    : m_configGroup{configGroup}
{
    m_factor = m_configGroup.readEntry(factorConfigEntryKey(), defaultFactor);
}

void ZoomControllerPrivate::writeConfig()
{
    m_configGroup.writeEntry(factorConfigEntryKey(), m_factor);
    m_configGroup.sync();
}

ZoomController::ZoomController(const KConfigGroup& configGroup, QObject* parent)
    : QObject(parent)
    , d_ptr{new ZoomControllerPrivate(configGroup)}
{
}

ZoomController::~ZoomController() = default;

double ZoomController::factor() const
{
    Q_D(const ZoomController);

    return d->m_factor;
}

void ZoomController::setFactor(double factor)
{
    Q_D(ZoomController);

    factor = qBound(0.1, factor, 10.0);
    if (d->m_factor == factor) {
        return;
    }
    d->m_factor = factor;
    d->writeConfig();
    emit factorChanged(d->m_factor);
}

void ZoomController::zoomBy(double scale)
{
    Q_D(ZoomController);

    setFactor(d->m_factor * multiplier(scale));
}

bool ZoomController::handleKeyPressEvent(QKeyEvent* event)
{
    Q_ASSERT(event);

    const auto requiredModifiers = Qt::ControlModifier;
    if (!(event->modifiers() & requiredModifiers)) {
        return false;
    }
    // Qt::ShiftModifier is required for the 0 key in some keyboard layouts
    // (such as Programmer Dvorak). Allow Qt::KeypadModifier to support numpad keys.
    // Don't allow other modifiers, such as Alt and Meta, to minimize shortcut conflicts.
    const auto allowedModifiers = Qt::ControlModifier | Qt::ShiftModifier | Qt::KeypadModifier;
    if (event->modifiers() & ~allowedModifiers) {
        return false;
    }

    if (event->key() == Qt::Key_0) {
        resetZoom();
        event->accept();
        return true;
    }

    return false;
}

bool ZoomController::handleWheelEvent(QWheelEvent* event)
{
    Q_ASSERT(event);

    if (!(event->modifiers() & Qt::ControlModifier)) {
        return false;
    }

    constexpr double minStandardDelta{120};
    const QPoint delta = event->angleDelta();
    const double scale{(delta.x() + delta.y()) / minStandardDelta};

    zoomBy(scale);
    event->accept();
    return true;
}

void ZoomController::zoomIn()
{
    Q_D(ZoomController);

    setFactor(d->m_factor * defaultMultiplier);
}

void ZoomController::zoomOut()
{
    Q_D(ZoomController);

    setFactor(d->m_factor / defaultMultiplier);
}

void ZoomController::resetZoom()
{
    setFactor(defaultFactor);
}

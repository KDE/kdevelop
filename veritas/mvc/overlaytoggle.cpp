/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "overlaytoggle.h"
#include <QTimeLine>
#include <KGlobalSettings>

using Veritas::OverlayButton;

QModelIndex OverlayButton::index()
{
    return m_index;
}

void OverlayButton::setIndex(const QModelIndex& index)
{
    m_index = index;
}

void OverlayButton::reset()
{
    m_index = QModelIndex();
    hide();
}


void OverlayButton::setVisible(bool visible)
{
    QAbstractButton::setVisible(visible);

    stopFading();
    if (visible) {
        startFading();
    }

}

void OverlayButton::setFadingValue(int value)
{
    m_fadingValue = value;
    if (m_fadingValue >= 255) {
        Q_ASSERT(m_fadingTimeLine != 0);
        m_fadingTimeLine->stop();
    }
    update();
}

void OverlayButton::startFading()
{
    Q_ASSERT(m_fadingTimeLine == 0);

    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 600 : 1;

    m_fadingTimeLine = new QTimeLine(duration, this);
    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));
    m_fadingTimeLine->setFrameRange(0, 255);
    m_fadingTimeLine->start();
    m_fadingValue = 0;
}

void OverlayButton::stopFading()
{
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
        delete m_fadingTimeLine;
        m_fadingTimeLine = 0;
    }
    m_fadingValue = 0;
}

#include "overlaytoggle.moc"

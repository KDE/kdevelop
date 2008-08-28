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

#ifndef VERITAS_OVERLAYBUTTON_H
#define VERITAS_OVERLAYBUTTON_H

#include <QAbstractButton>
#include <QModelIndex>
#include "../veritasexport.h"

class QTimeLine;

namespace Veritas
{
class Test;
class OverlayButton : public QAbstractButton
{
Q_OBJECT

public:
    OverlayButton(QWidget* parent);
    virtual ~OverlayButton() {}

    /*! Resets the selection toggle so that it is hidden and stays
     * visually invisible for at least one second after it is shown again. */
    virtual void reset();

    virtual int offset() = 0;
    virtual QModelIndex index();
    virtual void setIndex(const QModelIndex&);
    virtual bool shouldShow(Test*) = 0;

public slots:
    void setVisible(bool visible);

protected slots:
    /**
     * Sets the alpha value for the fading animation and is
     * connected with m_fadingTimeLine.
     */
    void setFadingValue(int value);

protected:
    virtual void leaveEvent(QEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    void startFading();
    void stopFading();

protected:
    QModelIndex m_index;
    int m_fadingValue;
    QTimeLine* m_fadingTimeLine;
    bool m_isHovered;
    QPixmap m_icon;

};

}

#endif // VERITAS_OVERLAYBUTTON_H

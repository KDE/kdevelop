/* KDevelop Coverage lcov settings
*
* Copyright 2009  Daniel Calviño Sánchez <danxuliu@gmail.com>
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

#include "colorrangebar.h"
#include "colorrange.h"

#include <QPainter>

using Veritas::ColorRange;
using Veritas::StopPoints;

ColorRangeBar::ColorRangeBar(ColorRange* colorRange, QWidget* parent):
        QWidget(parent) {
    mColorRange = colorRange;
}

void ColorRangeBar::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (mColorRange->mode() == ColorRange::Discrete) {
        paintDiscreteColorRange(this->contentsRect(), &painter);
    } else {
        paintGradientColorRange(this->contentsRect(), &painter);
    }
}

void ColorRangeBar::paintDiscreteColorRange(const QRect& rectangle,
                                            QPainter* painter) {
    QBrush brush(Qt::SolidPattern);
    QRectF rectangleRange(rectangle);

    StopPoints stopPoints = mColorRange->stopPoints();

    for (int i=0; i<stopPoints.size(); ++i) {
        double width = rectangle.width();
        if (i == 0) {
            width = width * stopPoints[i].first;
        } else {
            width = width * (stopPoints[i].first - stopPoints[i-1].first);
        }
        rectangleRange.setWidth(width);

        brush.setColor(stopPoints[i].second);
        painter->fillRect(rectangleRange, brush);
        rectangleRange.translate(width, 0);
    }
}

void ColorRangeBar::paintGradientColorRange(const QRect& rectangle,
                                            QPainter* painter) {
    QLinearGradient linearGradient(0, 0.5, 1, 0.5);
    linearGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    linearGradient.setStops(mColorRange->stopPoints());

    painter->fillRect(rectangle, QBrush(linearGradient));
}

#include "colorrangebar.moc"

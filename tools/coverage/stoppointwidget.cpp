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

#include "stoppointwidget.h"
#include "ui_stoppointwidget.h"

#include <KIcon>

using Veritas::ColorRange;
using Veritas::StopPoint;

StopPointWidget::StopPointWidget(QWidget* parent): QWidget(parent)
{
    mIndex = -1;
    mMode = ColorRange::Discrete;

    mUi = new Ui::StopPointWidget();
    mUi->setupUi(this);

    mUi->addButton->setIcon(KIcon("list-add"));
    mUi->removeButton->setIcon(KIcon("list-remove"));

    connect(mUi->colorChooser, SIGNAL(changed(QColor)),
            this, SLOT(changeColor(QColor)));
    connect(mUi->percentageSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(changePosition(double)));
    connect(mUi->addButton, SIGNAL(clicked(bool)),
            this, SLOT(addStopPointAfter()));
    connect(mUi->removeButton, SIGNAL(clicked(bool)),
            this, SLOT(removeStopPoint()));
}

StopPointWidget::~StopPointWidget()
{
    delete mUi;
}

void StopPointWidget::setStopPoint(const StopPoint& stopPoint,
                                   ColorRange::Mode mode)
{
    mUi->percentageSpinBox->setValue(stopPoint.first * 100);
    mUi->colorChooser->setColor(stopPoint.second);

    if (stopPoint.first == 1) {
        mUi->addButton->setEnabled(false);
        mUi->removeButton->setEnabled(false);
        mUi->percentageSpinBox->setEnabled(false);
    } else if (stopPoint.first == 0 && mode == ColorRange::Gradient) {
        mUi->addButton->setEnabled(true);
        mUi->removeButton->setEnabled(false);
        mUi->percentageSpinBox->setEnabled(false);
    } else {
        mUi->addButton->setEnabled(true);
        mUi->removeButton->setEnabled(true);
        mUi->percentageSpinBox->setEnabled(true);
    }

    mMode = mode;
}

void StopPointWidget::setStopPointIndex(int index)
{
    mIndex = index;
}

void StopPointWidget::setRange(double minimum, double maximum)
{
    if (mUi->percentageSpinBox->value() == 100 ||
                (mUi->percentageSpinBox->value() == 0 && 
                 mMode == ColorRange::Gradient)) {
        return;
    }

    mUi->percentageSpinBox->setMinimum(minimum * 100 + 0.01);
    mUi->percentageSpinBox->setMaximum(maximum * 100 - 0.01);
}

void StopPointWidget::setDummy(bool dummy) {
    if (dummy) {
        mUi->horizontalSpacer->changeSize(5, 5, QSizePolicy::Expanding);
        mUi->colorLabel->hide();
        mUi->colorChooser->hide();
        mUi->percentageLabel->hide();
        mUi->percentageSpinBox->hide();
        mUi->removeButton->hide();
    } else {
        mUi->horizontalSpacer->changeSize(0, 0);
        mUi->colorLabel->show();
        mUi->colorChooser->show();
        mUi->percentageLabel->show();
        mUi->percentageSpinBox->show();
        mUi->removeButton->show();
    }
}

void StopPointWidget::changeColor(const QColor& color)
{
    emit colorChanged(mIndex, color);
}

void StopPointWidget::changePosition(double percentage)
{
    emit positionChanged(mIndex, percentage/100);
}

void StopPointWidget::addStopPointAfter()
{
    emit stopPointToAddAfter(mIndex);
}

void StopPointWidget::removeStopPoint()
{
    emit stopPointToRemove(mIndex);
}

#include "stoppointwidget.moc"

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

#include "lcovprefswidget.h"
#include "lcovsettingsbase.h"
#include "colorrangebar.h"
#include "stoppointwidget.h"

using Veritas::ColorRange;
using Veritas::StopPoint;
using Veritas::StopPoints;

LCovPrefsWidget::LCovPrefsWidget(LCovSettingsBase* settings, QWidget* parent):
        QWidget(parent)
{
    mUi.setupUi(this);
    mSettings = settings;

    reloadColorRange();

    QHBoxLayout* layout = new QHBoxLayout(mUi.colorRangeBarFrame);
    ColorRangeBar* colorRangeBar = new ColorRangeBar(mSettings->colorRange(),
                                                     this);
    layout->addWidget(colorRangeBar);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(this, SIGNAL(changed(bool)), colorRangeBar, SLOT(update()));

    connect(mUi.modeComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(changeMode(QString)));
}

LCovPrefsWidget::~LCovPrefsWidget()
{
}

void LCovPrefsWidget::changeMode(const QString& mode)
{
    if (mode ==
            i18nc("@item:inlistbox Discrete color range type", "Discrete")) {
        mSettings->colorRange()->setMode(ColorRange::Discrete);
    } else {
        mSettings->colorRange()->setMode(ColorRange::Gradient);
    }

    reloadStopPoints();

    checkIfChanged();
}

void LCovPrefsWidget::changeColor(int index, const QColor& color)
{
    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    stopPoints[index].second = color;
    mSettings->colorRange()->setStopPoints(stopPoints);

    checkIfChanged();
}

void LCovPrefsWidget::changePosition(int index, double position)
{
    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    stopPoints[index].first = position;
    mSettings->colorRange()->setStopPoints(stopPoints);

    if (index > 0) {
        updateRange(index - 1);
    }
    updateRange(index + 1);

    checkIfChanged();
}

void LCovPrefsWidget::addStopPointAfter(int index)
{
    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    StopPoint newStopPoint;
    if (index == -1) {
        newStopPoint.first = 0.0001;
        newStopPoint.second = stopPoints[0].second;
    } else {
        newStopPoint = stopPoints[index];
        newStopPoint.first += 0.0001;
    }
    stopPoints.insert(index + 1, newStopPoint);

    //Ensure that next values don't overlap
    for (int i=index+2; i<stopPoints.size(); ++i) {
        double minimum = stopPoints[i-1].first + 0.0001;
        if (stopPoints[i].first < minimum) {
            stopPoints[i].first = minimum;
        }
    }

    mSettings->colorRange()->setStopPoints(stopPoints);

    reloadStopPoints();

    checkIfChanged();
}

void LCovPrefsWidget::removeStopPoint(int index)
{
    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    stopPoints.remove(index);
    mSettings->colorRange()->setStopPoints(stopPoints);

    reloadStopPoints();

    checkIfChanged();
}

void LCovPrefsWidget::reloadColorRange()
{
    if (mSettings->colorRange()->mode() == ColorRange::Discrete) {
        mUi.modeComboBox->setCurrentIndex(mUi.modeComboBox->findText(
            i18nc("@item:inlistbox Discrete color range type", "Discrete")));
    } else {
        mUi.modeComboBox->setCurrentIndex(mUi.modeComboBox->findText(
            i18nc("@item:inlistbox Gradient color range type", "Gradient")));
    }

    reloadStopPoints();

    checkIfChanged();
}

void LCovPrefsWidget::reloadStopPoints()
{
    QLayout* layout = mUi.stopPointsScrollArea->widget()->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != 0) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    QVBoxLayout* boxLayout = qobject_cast<QVBoxLayout*>(layout);

    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    if (stopPoints.size() == 1) {
        boxLayout->addWidget(newDummyStopPointWidget());
    }
    for (int i=0; i<stopPoints.size(); ++i) {
        ColorRange::Mode mode = mSettings->colorRange()->mode();
        boxLayout->addWidget(newStopPointWidgetFor(stopPoints, i, mode));
    }

    boxLayout->addStretch();
}

StopPointWidget* LCovPrefsWidget::newStopPointWidgetFor(
                const StopPoints& stopPoints, int index, ColorRange::Mode mode)
{
    StopPointWidget* stopPointWidget = new StopPointWidget(this);

    stopPointWidget->setStopPoint(stopPoints[index], mode);
    stopPointWidget->setStopPointIndex(index);

    double minimum = index>0 ? stopPoints[index-1].first : 0;
    double maximum = index<stopPoints.size()-1 ? stopPoints[index+1].first : 1;
    stopPointWidget->setRange(minimum, maximum);

    connect(stopPointWidget, SIGNAL(colorChanged(int, QColor)),
            this, SLOT(changeColor(int, QColor)));
    connect(stopPointWidget, SIGNAL(positionChanged(int, double)),
            this, SLOT(changePosition(int, double)));
    connect(stopPointWidget, SIGNAL(stopPointToAddAfter(int)),
            this, SLOT(addStopPointAfter(int)));
    connect(stopPointWidget, SIGNAL(stopPointToRemove(int)),
            this, SLOT(removeStopPoint(int)));

    return stopPointWidget;
}

StopPointWidget* LCovPrefsWidget::newDummyStopPointWidget()
{
    StopPointWidget* stopPointWidget = new StopPointWidget(this);
    stopPointWidget->setDummy(true);
    stopPointWidget->setStopPointIndex(-1);

    connect(stopPointWidget, SIGNAL(stopPointToAddAfter(int)),
            this, SLOT(addStopPointAfter(int)));

    return stopPointWidget;
}

StopPointWidget* LCovPrefsWidget::getStopPointWidget(int index)
{
    QLayout* layout = mUi.stopPointsScrollArea->widget()->layout();
    QLayoutItem* item = layout->itemAt(index);
    return qobject_cast<StopPointWidget*>(item->widget());
}

void LCovPrefsWidget::updateRange(int index)
{
    StopPointWidget* stopPointWidget = getStopPointWidget(index);

    StopPoints stopPoints = mSettings->colorRange()->stopPoints();
    double minimum = index>0 ? stopPoints[index-1].first : 0;
    double maximum = index<stopPoints.size()-1 ? stopPoints[index+1].first : 1;
    stopPointWidget->setRange(minimum, maximum);
}

void LCovPrefsWidget::checkIfChanged()
{
    if (mSettings->isSavedColorRange()) {
        emit changed(false);
    } else {
        emit changed(true);
    }
}

#include "lcovprefswidget.moc"

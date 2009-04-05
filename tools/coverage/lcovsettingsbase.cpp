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

#include "lcovsettingsbase.h"

using Veritas::ColorRange;
using Veritas::StopPoint;
using Veritas::StopPoints;

LCovSettingsBase::~LCovSettingsBase()
{
}

Veritas::ColorRange* LCovSettingsBase::colorRange()
{
    return &mColorRange;
}

bool LCovSettingsBase::isDefaultColorRange() const
{
    return mColorRange == mDefaultColorRange;
}

bool LCovSettingsBase::isSavedColorRange() const
{
    return mColorRange == mSavedColorRange;
}

LCovSettingsBase::LCovSettingsBase(const QString &configurationFileName):
    KConfigSkeleton(configurationFileName)
{
    mDefaultColorRange.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor("black")));
    stopPoints.append(StopPoint(0.3333, QColor("red")));
    stopPoints.append(StopPoint(0.6667, QColor("orange")));
    stopPoints.append(StopPoint(1, QColor("green")));
    mDefaultColorRange.setStopPoints(stopPoints);

    mDefaultColorRange.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.25, QColor("black")));
    stopPoints.append(StopPoint(0.50, QColor("red")));
    stopPoints.append(StopPoint(0.75, QColor("orange")));
    stopPoints.append(StopPoint(1, QColor("green")));
    mDefaultColorRange.setStopPoints(stopPoints);

    mColorRange = mDefaultColorRange;
    mSavedColorRange = mDefaultColorRange;
}

void LCovSettingsBase::usrReadConfig()
{
    KConfigGroup coverageGroup = config()->group("Code Coverage");
    if (coverageGroup.hasGroup("Color Range")) {
        mSavedColorRange.load(coverageGroup.group("Color Range"));
    } else {
        mSavedColorRange = mDefaultColorRange;
    }

    if (!isSavedColorRange()) {
        mColorRange = mSavedColorRange;
        emit colorRangeChanged();
    }
}

void LCovSettingsBase::usrSetDefaults()
{
    if (!isDefaultColorRange()) {
        mColorRange = mDefaultColorRange;
        emit colorRangeChanged();
    }
}

bool LCovSettingsBase::usrUseDefaults(bool)
{
    //Implemented following the implementation of
    //KConfigSkeletonGenericItem::swapDefaults(), which is called in
    //KCoreConfigSkeleton::useDefaults(bool)
    ColorRange temporal = mColorRange;
    mColorRange = mDefaultColorRange;
    mDefaultColorRange = temporal;

    emit colorRangeChanged();

    return false;
}

void LCovSettingsBase::usrWriteConfig()
{
    KConfigGroup coverageGroup = config()->group("Code Coverage");
    coverageGroup.deleteGroup("Color Range");

    if (!isDefaultColorRange()) {
        KConfigGroup colorRangeGroup(&coverageGroup, "Color Range");
        mColorRange.save(colorRangeGroup);
    }

    //After writing the configuration, it is read again in KConfigSkeleton, so
    //there is no need to set mSavedColorRange here.
}

#include "lcovsettingsbase.moc"

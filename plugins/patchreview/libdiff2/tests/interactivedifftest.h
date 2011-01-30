/*
 * This file is part of KDevelop
 * Copyright 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PATCHREVIEWTEST_H
#define PATCHREVIEWTEST_H

#include <QtCore/QObject>

#include "../difference.h"

class InteractiveDiffTest : public QObject
{
    Q_OBJECT
private slots:
    void testOneLineChange();
    void testSameLine();
    void testLineNumbers_data();
    void testLineNumbers();
    void testDifferenceContents_data();
    void testDifferenceContents();
    void testAppliedTouch();
    void testAppliedIntersect();
    void testExistingAndApplied();
    void testOneLineDeletionUnapplied();
    void testApplyUnapply();
private:
    void CompareDifferenceStringList(const Diff2::DifferenceStringList& actual, const QStringList& expected);
};

#endif //  PATCHREVIEWTEST_H

/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
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

#include "test_makeoutputmodel.h"

#include "../makeoutputmodel.h"

#include <QStringList>
#include <QTest>

QTEST_MAIN(TestMakeOutputModel)

TestMakeOutputModel::TestMakeOutputModel(QObject* parent): QObject(parent)
{

}

void TestMakeOutputModel::benchAddLines()
{
    MakeOutputModel model(KUrl("/tmp/build-foo"));
    QStringList lines;
    const int numLines = 1000;
    const int charsPerLine = 100;
    for(int i = 0; i < numLines; ++i) {
        lines << (QString::number(i) + QString().fill('a', charsPerLine));
    }
    QBENCHMARK {
        model.addLines(lines);
    }
}

#include "test_makeoutputmodel.moc"

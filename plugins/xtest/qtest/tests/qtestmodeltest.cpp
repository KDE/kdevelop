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

#include "qtestmodeltest.h"
#include <QByteArray>
#include <QBuffer>

using QTest::ut::QTestModelTest;

void QTestModelTest::processSingleSuite()
{
    QByteArray xml =
        "<?xml version="1.0" encoding="ISO - 8859 - 1"?>"
        "<root><suite name=\"suite1\" /></root>";
    QBuffer buff(&xml);
    QTestModel model;
    model.registerFromXml(&buff);
    KVERIFY(model.data());
}

QTEST_KDEMAIN(QTestModelTest, NoGUI)

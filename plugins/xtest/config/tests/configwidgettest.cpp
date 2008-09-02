/*
 * This file is part of KDevelop
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

#include "configwidgettest.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include <QLabel>

#include "../configwidget.h"

using Veritas::ConfigWidgetTest;

void ConfigWidgetTest::init()
{
}

void ConfigWidgetTest::cleanup()
{
}

void ConfigWidgetTest::construct()
{
    ConfigWidget* w = new ConfigWidget;
    w->show();
    QTest::qWait(5000);
}

void ConfigWidgetTest::expandDetails()
{
    ConfigWidget* w = new ConfigWidget;
    QList<QLabel*> labels = w->findChildren<QLabel*>();
    QVERIFY(labels.isEmpty());
    w->fto_clickExpandDetails();
    w->show();
    labels = w->findChildren<QLabel*>();
    QVERIFY(!labels.isEmpty());
}

QTEST_KDEMAIN( ConfigWidgetTest, GUI )
#include "configwidgettest.moc"

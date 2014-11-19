/*  This file is part of KDevelop
    Copyright 2014 Kevin Funk <kfunk@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "../manpagedocumentation.h"
#include "../manpagemodel.h"

#include <tests/modeltest.h>

#include <QDebug>
#include <QSignalSpy>
#include <QtTest>

class TestManPageModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testModel();
    void testDocumentation();
};

void TestManPageModel::testModel()
{
    ManPageModel model;
    QSignalSpy spy(&model, SIGNAL(manPagesLoaded()));
    spy.wait();

    if (model.isLoaded()) {
        QVERIFY(model.rowCount() > 0);
        new ModelTest(&model);
    } else {
        QCOMPARE(model.rowCount(), 0);
    }
}

void TestManPageModel::testDocumentation()
{
    ManPageDocumentation documentation("dlopen", QUrl("man: (3)/dlopen"));
    QSignalSpy spy(&documentation, SIGNAL(descriptionChanged()));
    QVERIFY(spy.wait());

    const QString description = documentation.description();
    if (!description.isEmpty()) {
        qDebug() << description;
        // check that we've found the correct page by checking some references
        QVERIFY(description.contains("dlclose"));
        QVERIFY(description.contains("dlerror"));
        QVERIFY(description.contains("dlopen"));
    }
}

QTEST_MAIN(TestManPageModel)

#include "test_manpagemodel.moc"

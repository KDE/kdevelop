/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../manpagedocumentation.h"
#include "../manpagemodel.h"
// Qt
#include <QAbstractItemModelTester>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include <QStandardPaths>

class TestManPageModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }
    void testModel();
    void testDocumentation();
};

void TestManPageModel::testModel()
{
    ManPageModel model;

    new QAbstractItemModelTester(&model, this);

    QTRY_VERIFY(model.isLoaded() || model.hasError());
    if (model.isLoaded())
        QVERIFY(model.rowCount() > 0);
}

void TestManPageModel::testDocumentation()
{
    ManPageDocumentation documentation(QStringLiteral("dlopen"), QUrl(QStringLiteral("man: (3)/dlmopen")));
    QSignalSpy spy(&documentation, SIGNAL(descriptionChanged()));
    QVERIFY(spy.wait());

    const QString description = documentation.description();
    qDebug() << "Description:" << description;
    if (description.isEmpty() || description.contains(QLatin1String("No man page matching to dlmopen found"))) {
        QSKIP("This test requires installed man pages for dlmopen & friends");
    }

    // check that we've found the correct page by checking some references
    QVERIFY(description.contains("dlclose"));
    QVERIFY(description.contains("dlerror"));
    QVERIFY(description.contains("dlopen"));
}

QTEST_MAIN(TestManPageModel)

#include "test_manpagemodel.moc"

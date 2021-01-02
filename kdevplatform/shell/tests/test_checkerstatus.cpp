/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

#include <QTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <shell/checkerstatus.h>

#include <KLocalizedString>

using namespace KDevelop;

class TestCheckerStatus : public QObject
{
    Q_OBJECT

public:
    TestCheckerStatus();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testName();
    void testStart();
    void testItemChecked();
    void testStop();

private:
    QScopedPointer<CheckerStatus> m_status;
};

TestCheckerStatus::TestCheckerStatus()
{
    qRegisterMetaType<IStatus*>();
}

void TestCheckerStatus::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_status.reset(new CheckerStatus());
}

void TestCheckerStatus::cleanupTestCase()
{
}

void TestCheckerStatus::testName()
{
    QString name = QStringLiteral("TESTNAME");
    QString s = i18nc("@info:progress", "Running %1", name);

    m_status->setCheckerName(name);

    QCOMPARE(m_status->statusName(), s);
}

static void checkValues(const QList<QVariant> &signal, int min, int max, int value)
{
    QCOMPARE(qvariant_cast<int>(signal[1]), min);
    QCOMPARE(qvariant_cast<int>(signal[2]), max);
    QCOMPARE(qvariant_cast<int>(signal[3]), value);
}

void TestCheckerStatus::testStart()
{
    QSignalSpy spy(m_status.data(), &CheckerStatus::showProgress);

    m_status->setMaxItems(100);
    m_status->start();

    QCOMPARE(spy.count(), 1);
    checkValues(spy.takeFirst(), 0, 100, 0);
}

void TestCheckerStatus::testItemChecked()
{
    QSignalSpy spy(m_status.data(), &CheckerStatus::showProgress);

    m_status->itemChecked();

    QCOMPARE(spy.count(), 1);
    checkValues(spy.takeFirst(), 0, 100, 1);
}

void TestCheckerStatus::testStop()
{
    QSignalSpy spy(m_status.data(), &CheckerStatus::showProgress);

    m_status->stop();

    QCOMPARE(spy.count(), 1);
    checkValues(spy.takeFirst(), 0, 100, 100);
}

QTEST_MAIN(TestCheckerStatus)

#include "test_checkerstatus.moc"

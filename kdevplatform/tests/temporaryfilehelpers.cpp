/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "temporaryfilehelpers.h"

#include <QByteArray>
#include <QStringView>
#include <QTemporaryFile>
#include <QTest>

void KDevelop::openWriteAndCloseTemporaryFile(QTemporaryFile& file, QStringView fileContents)
{
    QVERIFY(file.open());

    file.write(fileContents.toUtf8());
    QCOMPARE(file.error(), QFileDevice::NoError);

    QVERIFY(file.flush());

    file.close();
    QCOMPARE(file.error(), QFileDevice::NoError);
}

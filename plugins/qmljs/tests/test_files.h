/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_FILES_H
#define TEST_FILES_H

#include <QObject>

class TestFiles : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testQMLCustomComponent();
    void testJSUsesBetweenFiles();
    void testNodeJS();
    void testFiles_data();
    void testFiles();
    void testQMLTypes();
    void testTypeMismatchFalsePositives();

private:
    void parseAndCheck(const QString& fileName, bool check = true);
};

#endif // TEST_FILES_H

/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_FILES_H
#define TEST_FILES_H

#include <QObject>

class TestEnvironmentProvider;

class TestFiles : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testFiles_data();
    void testFiles();
private:
    TestEnvironmentProvider *m_provider;
};

#endif // TEST_FILES_H

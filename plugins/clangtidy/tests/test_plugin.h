/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <QObject>

class TestClangTidyPlugin : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testPlugin();
};

#endif // TEST_PLUGIN_H

/*
    SPDX-FileCopyrightText: 2009 Fabian Wiesel <fabian.wiesel@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNRECURSIVEADD_H
#define KDEVPLATFORM_PLUGIN_SVNRECURSIVEADD_H

#include <QObject>

class TestSvnRecursiveAdd
            : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test();
};

#endif // KDEVPLATFORM_PLUGIN_SVNRECURSIVEADD_H

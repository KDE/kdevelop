/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSREVISION_H
#define KDEVPLATFORM_TESTVCSREVISION_H

#include <QObject>

class TestVcsRevision : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();
};

#endif // KDEVPLATFORM_TESTVCSREVISION_H

/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSSTATUSINFO_H
#define KDEVPLATFORM_TESTVCSSTATUSINFO_H

#include <QObject>

#include <vcs/vcsstatusinfo.h>

class TestVcsStatusInfo : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();
};

#endif // KDEVPLATFORM_TESTVCSSTATUSINFO_H

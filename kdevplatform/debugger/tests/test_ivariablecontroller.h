/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_IVARIABLECONTROLLER_H
#define KDEVPLATFORM_TEST_IVARIABLECONTROLLER_H

#include <QObject>

namespace KDevelop
{
class TestDebugSession;

class TestIVariableController : public QObject
{
Q_OBJECT
public:
    explicit TestIVariableController(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    /**
     * Test for bug 333759. see https://bugs.kde.org/show_bug.cgi?id=333759
     */
    void updateRightAfterEnableAutoUpdate();
    void updateRightAfterEnableAutoUpdate_data();

private:
    TestDebugSession *m_debugSession = nullptr;
};

}
#endif // KDEVPLATFORM_TEST_IVARIABLECONTROLLER_H

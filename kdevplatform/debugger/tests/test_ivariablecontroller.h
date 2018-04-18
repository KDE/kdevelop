/*
 * KDevelop Debugger Support
 *
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

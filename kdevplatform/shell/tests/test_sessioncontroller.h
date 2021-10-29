/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_SESSIONCONTROLLER_H
#define KDEVPLATFORM_TEST_SESSIONCONTROLLER_H

#include <QObject>

namespace KDevelop
{
class Core;
class Session;
class SessionController;
}

class TestSessionController : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void createSession_data();
    void createSession();
    void deleteSession();
    void cloneSession();
    void renameSession();
    void canRenameActiveSession();
    void readFromConfig();
    void temporary();
    void tryLockSession();
private:

    KDevelop::Core* m_core;
    KDevelop::SessionController* m_sessionCtrl;
};

#endif // KDEVPLATFORM_TEST_SESSIONCONTROLLER_H

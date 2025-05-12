/*
    SPDX-FileCopyrightText: 2024, 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DEBUGGER_TEST_BASE_H
#define DEBUGGER_TEST_BASE_H

#include <QObject>
#include <QUrl>
#include <QString>

class IExecutePlugin;

namespace KDevelop {
class Breakpoint;
class BreakpointModel;
class ICore;
class VariableCollection;
}

namespace KDevMI {
class MIDebugSession;

/**
 * A convenience macro for use in member functions of DebuggerTestBase and derived classes.
 */
#define START_DEBUGGING_E(session, launchConfiguration)                                                                \
    QVERIFY(session->startDebugging(&launchConfiguration, executePlugin()))

/**
 * A convenience macro for use in member functions of DebuggerTestBase and derived classes.
 */
#define START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, launchConfiguration)                                      \
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &launchConfiguration, executePlugin())

class DebuggerTestBase : public QObject
{
    Q_OBJECT
protected:
    [[nodiscard]] KDevelop::ICore* core() const;
    [[nodiscard]] IExecutePlugin* executePlugin() const;
    [[nodiscard]] KDevelop::VariableCollection* variableCollection() const;
    [[nodiscard]] KDevelop::BreakpointModel* breakpoints() const;

    /**
     * @return the path to the test file debugee.cpp
     */
    [[nodiscard]] QString debugeeFilePath() const;
    /**
     * @return the URL of the test file debugee.cpp
     */
    [[nodiscard]] QUrl debugeeUrl() const;
    /**
     * @return the location of the test file debugee.cpp at a given
     *         one-based MI line number, e.g. "/path/to/debugee.cpp:30"
     */
    [[nodiscard]] QString debugeeLocationAt(int miLine) const;

    /**
     * Add a code breakpoint to debugee.cpp at a given one-based MI line.
     */
    KDevelop::Breakpoint* addDebugeeBreakpoint(int miLine);

    [[nodiscard]] virtual MIDebugSession* createTestDebugSession() = 0;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testEnvironmentSet();
    void testUnsupportedUrlExpressionBreakpoints();
    void testBreakpointsOnNoOpLines();
    void testBreakpointErrors();
    void testInsertAndRemoveBreakpointWhileRunning();
    void testChangeBreakpointWhileRunning();

private:
    [[nodiscard]] virtual bool isLldb() const = 0;

    /**
     * This function is called at the beginning of the slot initTestCase().
     */
    virtual void startInitTestCase()
    {
    }
    /**
     * This function is called at the end of the slot init().
     */
    virtual void finishInit()
    {
    }

    KDevelop::ICore* m_core;
    IExecutePlugin* m_executePlugin;
    QString m_debugeeFilePath;
    QUrl m_debugeeUrl;
};
} // namespace KDevMI

#endif // DEBUGGER_TEST_BASE_H

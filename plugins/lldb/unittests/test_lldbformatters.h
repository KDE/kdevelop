/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBFORMATTERSTEST_H
#define LLDBFORMATTERSTEST_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QStringList>

class IExecutePlugin;

namespace KDevelop {
class Breakpoint;
class TestCore;
class VariableCollection;
}
namespace KDevMI { namespace LLDB {
class DebugSession;
class LldbVariable;
}
}
class LldbFormattersTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testQChar();
    void testQString();
    void testQByteArray();
    void testQListContainer_data();
    void testQListContainer();
    void testQListPOD();
    void testQMapInt();
    void testQMapString();
    void testQMapStringBool();
    void testQHashInt();
    void testQHashString();
    void testQSetInt();
    void testQSetString();
    void testQDate();
    void testQTime();
    void testQDateTime();
    void testQUrl();
    void testQUuid();
    void testKTextEditorTypes();
    void testKDevelopTypes();

private:
    // helpers
    bool verifyVariable(int index, const QString &name,
                        const QString &expectedSummary, QList<QPair<QString, QString>> expectedChildren,
                        const char *file, int line,
                        bool isLocal = true, bool useRE = false, bool unordered = false);

    bool verifyVariable(int index, const QString &name,
                        const QString &expectedSummary, QStringList expectedChildren,
                        const char *file, int line,
                        bool isLocal = true, bool useRE = false, bool unordered = false);

private:
    KDevelop::Breakpoint* addCodeBreakpoint(const QUrl& location, int line);
    KDevelop::VariableCollection *variableCollection();
    QModelIndex watchVariableIndexAt(int i, int col = 0);
    QModelIndex localVariableIndexAt(int i, int col = 0);

    KDevelop::TestCore *m_core;
    IExecutePlugin *m_iface;
    QPointer<KDevMI::LLDB::DebugSession> m_session;
};

#endif

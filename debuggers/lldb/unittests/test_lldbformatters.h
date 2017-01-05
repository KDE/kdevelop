/*
 * Unit tests for LLDB debugger plugin
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

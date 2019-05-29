/*
 * Copyright 2016 Milian Wolff <mail@milianw.de>
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
 */

#include "bench_codecompletion.h"

#include <QTest>
#include <QSignalSpy>

#include <KTextEditor/Cursor>

#include <tests/testfile.h>

#include <language/duchain/duchainlock.h>

#include "duchain/parsesession.h"
#include "duchain/clangindex.h"

#include "codecompletion/model.h"

QTEST_MAIN(BenchCodeCompletion)

using namespace KDevelop;

BenchCodeCompletion::BenchCodeCompletion()
    : m_index(new ClangIndex)
    , m_model(new ClangCodeCompletionModel(m_index.data(), this))
{
    m_model->initialize();
}

BenchCodeCompletion::~BenchCodeCompletion() = default;

void BenchCodeCompletion::benchCodeCompletion_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<KTextEditor::Cursor>("position");

    QTest::newRow("empty") << "" << KTextEditor::Cursor(0, 0);

    QTest::newRow("stl") << R"(
    #include <vector>
    #include <unordered_map>
    #include <unordered_set>

    int main()
    {
        return 0;
    }
    )" << KTextEditor::Cursor(7, 0);

    QTest::newRow("clib") << R"(
    #include <cstring>
    #include <cstdio>
    #include <cmath>

    int main()
    {
        return 0;
    }
    )" << KTextEditor::Cursor(7, 0);
}

void BenchCodeCompletion::benchCodeCompletion()
{
    QFETCH(QString, code);
    QFETCH(KTextEditor::Cursor, position);

    TestFile file(code, "cpp");
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST, 1, 5000));

    auto view = createView(file.url().toUrl());

    QSignalSpy spy(m_model, &QAbstractItemModel::modelReset);
    QBENCHMARK {
        m_model->completionInvoked(view.get(), {position, position}, KTextEditor::CodeCompletionModel::UserInvocation);
        do {
            spy.wait();
        } while (!m_model->rowCount());
    }
}

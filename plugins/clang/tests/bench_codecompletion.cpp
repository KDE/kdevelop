/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "moc_bench_codecompletion.cpp"

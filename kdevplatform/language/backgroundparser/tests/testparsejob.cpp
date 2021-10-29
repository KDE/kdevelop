/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testparsejob.h"

#include <QTest>

TestParseJob::TestParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
    : ParseJob(url, languageSupport)
    , duration_ms(0)
{
}

void TestParseJob::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    qDebug() << "Running parse job for" << document();
    if (run_callback) {
        run_callback(document());
    }
    if (duration_ms) {
        qDebug() << "waiting" << duration_ms << "ms";
        QTest::qWait(duration_ms);
    }
}

ControlFlowGraph* TestParseJob::controlFlowGraph()
{
    return nullptr;
}

DataAccessRepository* TestParseJob::dataAccessInformation()
{
    return nullptr;
}

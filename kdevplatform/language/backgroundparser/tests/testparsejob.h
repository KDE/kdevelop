/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTPARSEJOB_H
#define KDEVPLATFORM_TESTPARSEJOB_H

#include "language/backgroundparser/parsejob.h"

#include <functional>

using namespace KDevelop;

class TestParseJob
    : public KDevelop::ParseJob
{
    Q_OBJECT

public:
    TestParseJob(const IndexedString& url, ILanguageSupport* languageSupport);
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
    ControlFlowGraph* controlFlowGraph() override;
    DataAccessRepository* dataAccessInformation() override;

    int duration_ms;
    std::function<void( const IndexedString& )> run_callback;
};

#endif

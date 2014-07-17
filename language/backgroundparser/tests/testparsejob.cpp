/*
 * This file is part of KDevelop
 *
 * Copyright 2012 by Sven Brauch <svenbrauch@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    kDebug() << "Running parse job for" << document().toUrl();
    if (duration_ms) {
        kDebug() << "waiting" << duration_ms << "ms";
        QTest::qWait(duration_ms);
    }
}

ControlFlowGraph* TestParseJob::controlFlowGraph()
{
    return 0;
}

DataAccessRepository* TestParseJob::dataAccessInformation()
{
    return 0;
}




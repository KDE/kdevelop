/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testlanguagesupport.h"
#include "testparsejob.h"
#include "test_backgroundparser.h"

using namespace KDevelop;

ParseJob* TestLanguageSupport::createParseJob(const IndexedString& url)
{
    qDebug() << "creating test language support parse job";
    ParseJob* job = nullptr;
    emit aboutToCreateParseJob(url, &job);
    if (!job) {
        job = new TestParseJob(url, this);
    }
    emit parseJobCreated(job);
    return job;
}

QString TestLanguageSupport::name() const
{
    return QStringLiteral("TestLanguageSupport");
}

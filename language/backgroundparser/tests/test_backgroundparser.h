/*
 * This file is part of KDevelop
 *
 * Copyright 2012 by Sven Brauch <svenbrauch@googlemail.com>
 * Copyright 2012 by Milian Wolff <mail@milianw.de>
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

#ifndef KDEVPLATFORM_TEST_BACKGROUNDPARSER_H
#define KDEVPLATFORM_TEST_BACKGROUNDPARSER_H

#include <QObject>

#include <serialization/indexedstring.h>
#include <language/duchain/topducontext.h>

#include <language/backgroundparser/parsejob.h>

#include "testlanguagesupport.h"

class JobPrototype
{
public:
    JobPrototype()
    : m_priority(0)
    , m_duration(0)
    , m_flags(ParseJob::IgnoresSequentialProcessing)
    {
    }
    JobPrototype(const QUrl& url, int priority, ParseJob::SequentialProcessingFlags flags, int duration = 0)
    : m_url(url)
    , m_priority(priority)
    , m_duration(duration)
    , m_flags(flags)
    {
        Q_ASSERT(url.isValid());
    }
    IndexedString m_url;
    int m_priority;
    int m_duration;
    ParseJob::SequentialProcessingFlags m_flags;
};

Q_DECLARE_TYPEINFO(JobPrototype, Q_MOVABLE_TYPE);

class TestParseJob;

class JobPlan : public QObject
{
    Q_OBJECT

public:
    JobPlan();

    void addJob(const JobPrototype& job);

    bool runJobs(int timeoutMS);

    void clear();

    JobPrototype jobForUrl(const IndexedString& url);

private slots:
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& context);
    void parseJobCreated(KDevelop::ParseJob*);

private:
    QVector<JobPrototype> m_jobs;
    QVector<IndexedString> m_finishedJobs;
    QVector<IndexedString> m_createdJobs;
};

class TestBackgroundparser : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testParseOrdering_simple();
    void testParseOrdering_lockup();
    void testParseOrdering_foregroundThread();
    void testParseOrdering_noSequentialProcessing();

    void benchmark();

    void benchmarkDocumentChanges();

private:
    JobPlan m_jobPlan;
};

#endif // KDEVPLATFORM_TEST_BACKGROUNDPARSER_H

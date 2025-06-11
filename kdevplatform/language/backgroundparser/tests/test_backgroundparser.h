/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

class JobPlan
    : public QObject
{
    Q_OBJECT

public:
    JobPlan();

    void addJob(const JobPrototype& job);

    void addJobsToParser();
    void runJobs(int timeoutMS);

    void clear();

    JobPrototype jobForUrl(const IndexedString& url) const;

    int numJobs() const;
    int numFinishedJobs() const;
    int numCreatedJobs() const;

private Q_SLOTS:
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& context);
    void parseJobCreated(KDevelop::ParseJob*);

private:
    friend class TestBackgroundparser;

    QVector<JobPrototype> m_jobs;
    QVector<IndexedString> m_finishedJobs;
    QVector<IndexedString> m_createdJobs;
};

class TestBackgroundparser
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testShutdownWithRunningJobs();

    void testParseOrdering_simple();
    void testParseOrdering_lockup();
    void testParseOrdering_foregroundThread();
    void testParseOrdering_noSequentialProcessing();

    void testNoDeadlockInJobCreation();
    void testSuspendResume();

    void benchmark();

    void benchmarkDocumentChanges();

private:
    JobPlan m_jobPlan;
    TestLanguageSupport* m_langSupport = nullptr;
};

#endif // KDEVPLATFORM_TEST_BACKGROUNDPARSER_H

/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2007 Kris Wong <kris.p.wong@gmail.com>
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

#include "backgroundparser.h"

#include <QList>
#include <QFile>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>
#include <QThread>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <State.h>
#include <ThreadWeaver.h>
#include <JobCollection.h>
#include <DebuggingAids.h>

#include "icore.h"
#include "ilanguagecontroller.h"
#include "ilanguage.h"
#include "ilanguagesupport.h"

#include "parsejob.h"
#include "parserdependencypolicy.h"

namespace KDevelop
{

class BackgroundParserPrivate
{
public:
    BackgroundParserPrivate()
    {
        m_timer.setSingleShot(true);
        m_delay = 500;
        m_threads = 1;

        ThreadWeaver::setDebugLevel(true, 1);

        QObject::connect(&m_timer, SIGNAL(timeout()), &m_parser, SLOT(parseDocuments()));

        loadSettings(); // Start the weaver
    }
    ~BackgroundParserPrivate()
    {
        suspend();

        m_weaver.dequeue();
        m_weaver.requestAbort();
        m_weaver.finish();

        // Release dequeued jobs
        QHashIterator<KUrl, ParseJob*> it = m_parseJobs;
        while (it.hasNext()) {
            it.next();
            it.value()->setBackgroundParser(0);
            delete it.value();
        }
    }

    // Non-mutex guarded functions, only call with m_mutex acquired.
    void parseDocumentsInternal()
    {
        kDebug() << "BackgroundParser::parseDocumentsInternal" << endl;
        // First create the jobs, then enqueue them, because they may
        // need to access each other for generating dependencies.
        QList<ParseJob*> jobs;

        for (QMap<KUrl, bool>::Iterator it = m_documents.begin();
             it != m_documents.end(); ++it)
        {
            // When a document is scheduled for parsing while it is being parsed, it will be parsed
            // again once the job finished, but not now.
            if (m_parseJobs.contains(it.key()) ) {
                kDebug() << "skipping " << it.key() << " because it is already being parsed" << endl;
                continue;
            }

            kDebug() << "adding document " << it.key() << endl;
            KUrl url = it.key();
            bool &p = it.value();
            if (p) {
                QList<ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);
                foreach (ILanguage* language, languages) {
                    ParseJob* job = language->languageSupport()->createParseJob(url);
                    if (!job) {
                        continue; // Language part did not produce a valid ParseJob.
                    }

                    job->setBackgroundParser(&m_parser);

                    QObject::connect(job, SIGNAL(done(ThreadWeaver::Job*)),
                                     &m_parser, SLOT(parseComplete(ThreadWeaver::Job*)));

                    m_parseJobs.insert(url, job);
                    jobs.append(job);
                }

                p = false; // Don't parse for next time.
            }
        }

        // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

        foreach (ParseJob* job, jobs) {
            kDebug() << k_funcinfo << "Enqueue " << job << endl;
            m_weaver.enqueue(job);
        }

        for (QMap<KUrl, bool>::Iterator it = m_documents.begin(); it != m_documents.end();) {
            if (*it) {
                ++it;
            } else {
                m_documents.erase(it++);
            }
        }
    }

    void loadSettings()
    {
        KConfigGroup config(KGlobal::config(), "Background Parser");

        m_delay = config.readEntry("Delay", 500);
        m_timer.setInterval(m_delay);
        m_threads = config.readEntry("Number of Threads", 1);
        m_weaver.setMaximumNumberOfThreads(m_threads);

        if (config.readEntry("Enabled", true)) {
            resume();
        } else {
            suspend();
        }
    }

    void suspend()
    {
        bool s = m_weaver.state().stateId() == ThreadWeaver::Suspended ||
                 m_weaver.state().stateId() == ThreadWeaver::Suspending;

        if (s) { // Already suspending
            return;
        }

        m_timer.stop();
        m_weaver.suspend();
    }

    void resume()
    {
        bool s = m_weaver.state().stateId() == ThreadWeaver::Suspended ||
                 m_weaver.state().stateId() == ThreadWeaver::Suspending;

        if (m_timer.isActive() && !s) { // Not suspending
            return;
        }

        m_timer.start(m_delay);
        m_weaver.resume();
    }

    BackgroundParser m_parser;
    ICore* m_core;

    QTimer m_timer;
    int m_delay;
    int m_threads;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KUrl, bool> m_documents;
    // Current parse jobs
    QHash<KUrl, ParseJob*> m_parseJobs;

    ThreadWeaver::Weaver m_weaver;
    ParserDependencyPolicy m_dependencyPolicy;

    QMutex m_mutex;
};

K_GLOBAL_STATIC(BackgroundParserPrivate, sdBgParserPrivate)

BackgroundParser::BackgroundParser()
    : QObject(0)
{
}

BackgroundParser::~BackgroundParser()
{
}

BackgroundParser* BackgroundParser::self()
{
    return &sdBgParserPrivate->m_parser;
}

void BackgroundParser::clear(QObject* parent)
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    QHashIterator<KUrl, ParseJob*> it = sdBgParserPrivate->m_parseJobs;
    while (it.hasNext()) {
        it.next();
        if (it.value()->parent() == parent) {
            it.value()->requestAbort();
        }
    }
}

void BackgroundParser::loadSettings(bool projectIsLoaded)
{
    Q_UNUSED(projectIsLoaded)

    sdBgParserPrivate->loadSettings();
}

void BackgroundParser::saveSettings(bool projectIsLoaded)
{
    Q_UNUSED(projectIsLoaded)
}

void BackgroundParser::addDocument(const KUrl& url)
{
    kDebug() << "BackgroundParser::addDocument" << endl;
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);
    {
        Q_ASSERT(url.isValid());

        QMap<KUrl, bool>::const_iterator it = sdBgParserPrivate->m_documents.find(url);
        if (it == sdBgParserPrivate->m_documents.end() || (*it) == false) {
            kDebug() << "BackgroundParser::addDocument: queuing " << url << endl;
            sdBgParserPrivate->m_documents[url] = true;
        } else {
            kDebug() << "BackgroundParser::addDocument: is already queued: " << url << endl;
        }

        if (!sdBgParserPrivate->m_timer.isActive()) {
            sdBgParserPrivate->m_timer.start();
        }
    }
}

void BackgroundParser::addDocumentList(const KUrl::List &urls)
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    foreach (KUrl url, urls) {
        Q_ASSERT(url.isValid());

        sdBgParserPrivate->m_documents[url] = true;
    }

    if (!sdBgParserPrivate->m_timer.isActive()) {
        sdBgParserPrivate->m_timer.start();
    }
}

void BackgroundParser::removeDocument(const KUrl &url)
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    Q_ASSERT(url.isValid());

    sdBgParserPrivate->m_documents.remove(url);
}

void BackgroundParser::parseDocuments()
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    sdBgParserPrivate->parseDocumentsInternal();
}

void BackgroundParser::parseComplete(ThreadWeaver::Job* job)
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    if (ParseJob* parseJob = qobject_cast<ParseJob*>(job)) {
        kDebug() << "BackgroundParser: parsed " << parseJob->document() << endl;
        sdBgParserPrivate->m_parseJobs.remove(parseJob->document());

        parseJob->setBackgroundParser(0);

        // Use a delayed delete to make sure the weaver has finished up.
        // TODO: There has to be a better way to do this.
        QTimer::singleShot(100, parseJob, SLOT(deleteLater()));
    }
}

void BackgroundParser::suspend()
{
    sdBgParserPrivate->suspend();
}

void BackgroundParser::resume()
{
    sdBgParserPrivate->resume();
}

ParserDependencyPolicy* BackgroundParser::dependencyPolicy() const
{
    return &sdBgParserPrivate->m_dependencyPolicy;
}

ParseJob* BackgroundParser::parseJobForDocument(const KUrl& document) const
{
    QMutexLocker lock(&sdBgParserPrivate->m_mutex);

    if (sdBgParserPrivate->m_parseJobs.contains(document)) {
        return sdBgParserPrivate->m_parseJobs[document];
    }

    return 0;
}

}

#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2007 Kris Wong <kris.p.wong@gmail.com>
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
    BackgroundParserPrivate(BackgroundParser *parser, ILanguageController *languageController)
        :m_parser(parser), m_languageController(languageController)
    {
        m_timer.setSingleShot(true);
        m_delay = 500;
        m_threads = 1;

        ThreadWeaver::setDebugLevel(true, 1);

        QObject::connect(&m_timer, SIGNAL(timeout()), m_parser, SLOT(parseDocuments()));

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
        kDebug(9505) << "BackgroundParser::parseDocumentsInternal";
        // First create the jobs, then enqueue them, because they may
        // need to access each other for generating dependencies.
        QList<ParseJob*> jobs;

        for (QMap<KUrl, bool>::Iterator it = m_documents.begin();
             it != m_documents.end(); ++it)
        {
            // When a document is scheduled for parsing while it is being parsed, it will be parsed
            // again once the job finished, but not now.
            if (m_parseJobs.contains(it.key()) ) {
                kDebug(9505) << "skipping" << it.key() << "because it is already being parsed";
                continue;
            }

            kDebug(9505) << "adding document" << it.key();
            KUrl url = it.key();
            bool &p = it.value();
            if (p) {
                QList<ILanguage*> languages = m_languageController->languagesForUrl(url);
                foreach (ILanguage* language, languages) {
                    ParseJob* job = language->languageSupport()->createParseJob(url);
                    if (!job) {
                        continue; // Language part did not produce a valid ParseJob.
                    }

                    job->setBackgroundParser(m_parser);

                    QObject::connect(job, SIGNAL(done(ThreadWeaver::Job*)),
                                     m_parser, SLOT(parseComplete(ThreadWeaver::Job*)));
                    QObject::connect(job, SIGNAL(failed(ThreadWeaver::Job*)),
                                     m_parser, SLOT(parseComplete(ThreadWeaver::Job*)));

                    m_parseJobs.insert(url, job);
                    jobs.append(job);
                }

                p = false; // Don't parse for next time.
            }
        }

        // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

        foreach (ParseJob* job, jobs) {
            kDebug(9505) << "Enqueue" << job;
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

    BackgroundParser *m_parser;
    ILanguageController* m_languageController;

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


BackgroundParser::BackgroundParser(ILanguageController *languageController)
    : QObject(languageController), d(new BackgroundParserPrivate(this, languageController))
{
}

BackgroundParser::~BackgroundParser()
{
    delete d;
}

void BackgroundParser::clear(QObject* parent)
{
    QMutexLocker lock(&d->m_mutex);

    QHashIterator<KUrl, ParseJob*> it = d->m_parseJobs;
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

    d->loadSettings();
}

void BackgroundParser::saveSettings(bool projectIsLoaded)
{
    Q_UNUSED(projectIsLoaded)
}

void BackgroundParser::addDocument(const KUrl& url)
{
    kDebug(9505) << "BackgroundParser::addDocument" << url.prettyUrl();
    QMutexLocker lock(&d->m_mutex);
    {
        Q_ASSERT(url.isValid());

        QMap<KUrl, bool>::const_iterator it = d->m_documents.find(url);
        if (it == d->m_documents.end() || (*it) == false) {
            kDebug(9505) << "BackgroundParser::addDocument: queuing" << url;
            d->m_documents[url] = true;
        } else {
            kDebug(9505) << "BackgroundParser::addDocument: is already queued:" << url;
        }

        if (!d->m_timer.isActive()) {
            d->m_timer.start();
        }
    }
}

void BackgroundParser::addDocumentList(const KUrl::List &urls)
{
    QMutexLocker lock(&d->m_mutex);

    foreach (KUrl url, urls) {
        Q_ASSERT(url.isValid());

        d->m_documents[url] = true;
    }

    if (!d->m_timer.isActive()) {
        d->m_timer.start();
    }
}

void BackgroundParser::removeDocument(const KUrl &url)
{
    QMutexLocker lock(&d->m_mutex);

    Q_ASSERT(url.isValid());

    d->m_documents.remove(url);
}

void BackgroundParser::parseDocuments()
{
    QMutexLocker lock(&d->m_mutex);

    d->parseDocumentsInternal();
}

void BackgroundParser::parseComplete(ThreadWeaver::Job* job)
{
    QMutexLocker lock(&d->m_mutex);

    if (ParseJob* parseJob = qobject_cast<ParseJob*>(job)) {
        kDebug(9505) << "BackgroundParser: parsed" << parseJob->document();

        emit parseJobFinished(parseJob);

        d->m_parseJobs.remove(parseJob->document());

        parseJob->setBackgroundParser(0);

        // Use a delayed delete to make sure the weaver has finished up.
        // TODO: There has to be a better way to do this.
        QTimer::singleShot(100, parseJob, SLOT(deleteLater()));
    }
}

void BackgroundParser::suspend()
{
    d->suspend();
}

void BackgroundParser::resume()
{
    d->resume();
}

ParserDependencyPolicy* BackgroundParser::dependencyPolicy() const
{
    return &d->m_dependencyPolicy;
}

ParseJob* BackgroundParser::parseJobForDocument(const KUrl& document) const
{
    QMutexLocker lock(&d->m_mutex);

    if (d->m_parseJobs.contains(document)) {
        return d->m_parseJobs[document];
    }

    return 0;
}

}

#include "backgroundparser.moc"


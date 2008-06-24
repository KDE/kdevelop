/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2007 Kris Wong <kris.p.wong@gmail.com>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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
#include <klocale.h>

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
        m_doneParseJobs = 0;
        m_maxParseJobs = 0;

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

        qDeleteAll(m_delayedParseJobs.values());
    }

    // Non-mutex guarded functions, only call with m_mutex acquired.
    void parseDocumentsInternal()
    {
        kDebug(9505) << "BackgroundParser::parseDocumentsInternal";
        // First create the jobs, then enqueue them, because they may
        // need to access each other for generating dependencies.
        
        // Create delayed jobs, that is, jobs for documents which have been changed
        // by the user.
        QList<ParseJob*> jobs;
        QHashIterator<KUrl, DocumentChangeTracker*> it = m_delayedParseJobs;
        while (it.hasNext()) {
            ParseJob* job = createParseJob(it.next().key());
            if (job) {
                job->setChangedRanges(it.value()->changedRanges());
                jobs.append(job);
            } else {
                kWarning() << "No job created for url " << it.key();
            }
        }
        qDeleteAll(m_delayedParseJobs);
        m_delayedParseJobs.clear();

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
                ParseJob* job = createParseJob(url);
                if (job)
                    jobs.append(job);

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

        m_parser->updateProgressBar();

        //We don't hide the progress-bar in updateProgressBar, so it doesn't permanently flash when a document is reparsed again and again.
        if(m_doneParseJobs == m_maxParseJobs)
            emit m_parser->hideProgress();
    }

    ParseJob* createParseJob(const KUrl& url)
    {
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
            QObject::connect(job, SIGNAL(progress(KDevelop::ParseJob*, float, QString)),
                                m_parser, SLOT(parseProgress(KDevelop::ParseJob*, float, QString)), Qt::QueuedConnection);

            m_parseJobs.insert(url, job);

            ++m_maxParseJobs;

            // TODO more thinking required here to support multiple parse jobs per url (where multiple language plugins want to parse)
            return job;
        }

        return 0;
    }


    void loadSettings()
    {
        KConfigGroup config(KGlobal::config(), "Background Parser");

        m_delay = config.readEntry("Delay", 500);
        m_timer.setInterval(m_delay);
        m_threads = config.readEntry("Real Number of Threads", 1);
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
    QHash<KUrl, DocumentChangeTracker*> m_delayedParseJobs;

    QHash<KTextEditor::SmartRange*, KUrl> m_managedRanges;

    ThreadWeaver::Weaver m_weaver;
    ParserDependencyPolicy m_dependencyPolicy;

    QMutex m_mutex;

    int m_maxParseJobs;
    int m_doneParseJobs;
    QMap<KDevelop::ParseJob*, float> m_jobProgress;
};


BackgroundParser::BackgroundParser(ILanguageController *languageController)
    : QObject(languageController), d(new BackgroundParserPrivate(this, languageController))
{
}

BackgroundParser::~BackgroundParser()
{
    delete d;
}

QString BackgroundParser::statusName() const
{
    return i18n("Background Parser");
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

void BackgroundParser::parseProgress(KDevelop::ParseJob* job, float value, QString text)
{
    d->m_jobProgress[job] = value;
    updateProgressBar();
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
        kDebug(9505) << "BackgroundParser: parsed" << parseJob->document().str();

        emit parseJobFinished(parseJob);

        d->m_parseJobs.remove(parseJob->document().str());

        d->m_jobProgress.remove(parseJob);
        
        parseJob->setBackgroundParser(0);

		kDebug() << "Queueing job for deletion" << job->metaObject()->className() << "in thread" << QThread::currentThread();

		delete parseJob;

        ++d->m_doneParseJobs;
        updateProgressBar();

        //It may be needed to reparse some files that were changed during parsing. 
        if (!d->m_timer.isActive())
            d->m_timer.start();
    }
}

void BackgroundParser::suspend()
{
    d->suspend();

    emit hideProgress();
}

void BackgroundParser::resume()
{
    d->resume();

    updateProgressBar();
}

void BackgroundParser::updateProgressBar()
{
    if (d->m_doneParseJobs == d->m_maxParseJobs) {
        d->m_doneParseJobs = 0;
        d->m_maxParseJobs = 0;
    } else {
        float additionalProgress = 0;
        for(QMap<KDevelop::ParseJob*, float>::const_iterator it = d->m_jobProgress.begin(); it != d->m_jobProgress.end(); ++it)
            additionalProgress += *it;
        
        emit showProgress(0, d->m_maxParseJobs*1000, (additionalProgress + d->m_doneParseJobs)*1000);
    }
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

void BackgroundParser::setThreadCount(int threadCount)
{
    if (d->m_threads != threadCount) {
        d->m_threads = threadCount;
        d->m_weaver.setMaximumNumberOfThreads(d->m_threads);
    }
}

void BackgroundParser::setDelay(int miliseconds)
{
    if (d->m_delay != miliseconds) {
        d->m_delay = miliseconds;
        d->m_timer.setInterval(d->m_delay);
    }
}

void BackgroundParser::addManagedTopRange(const KUrl& document, KTextEditor::SmartRange* range)
{
    range->addWatcher(this);
    d->m_managedRanges.insert(range, document);
}

void BackgroundParser::removeManagedTopRange(KTextEditor::SmartRange* range)
{
    range->removeWatcher(this);
    d->m_managedRanges.remove(range);
}

void BackgroundParser::rangeContentsChanged(KTextEditor::SmartRange* range, KTextEditor::SmartRange* mostSpecificChild)
{
    QMutexLocker l(&d->m_mutex);

    // Smart mutex is already locked
    KUrl documentUrl = range->document()->url();

    if (d->m_parseJobs.contains(documentUrl)) {
        ParseJob* job = d->m_parseJobs[documentUrl];
        if (job->addChangedRange( mostSpecificChild ))
            // Success
            return;
    }

    // Initially I just created a new parse job here, but that causes a deadlock as the smart mutex is locked
    // So store the info in a class with just the changed ranges information...
    DocumentChangeTracker* newTracker = 0;
    if (d->m_delayedParseJobs.contains(documentUrl))
        newTracker = d->m_delayedParseJobs[documentUrl];

    if (!newTracker) {
        newTracker = new DocumentChangeTracker();
        d->m_delayedParseJobs.insert(documentUrl, newTracker);
    }

    newTracker->addChangedRange( mostSpecificChild );
    
    if (!d->m_timer.isActive())
        d->m_timer.start(d->m_delay);
}

}

#include "backgroundparser.moc"


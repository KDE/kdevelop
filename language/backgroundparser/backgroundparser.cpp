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
#include <QtCore/QWeakPointer>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocale.h>

#include <ktexteditor/document.h>

#include <threadweaver/State.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>
#include <threadweaver/DebuggingAids.h>

#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>

#include "../interfaces/ilanguagesupport.h"

#include "parsejob.h"
#include "parserdependencypolicy.h"
#include <editor/modificationrevisionset.h>
#include <interfaces/icore.h>
#include <qcoreapplication.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/isession.h>

const bool separateThreadForHighPriority = true;

namespace KDevelop
{

class BackgroundParserPrivate
{
public:
    BackgroundParserPrivate(BackgroundParser *parser, ILanguageController *languageController)
        :m_parser(parser), m_languageController(languageController), m_shuttingDown(false), m_mutex(QMutex::Recursive)
    {
        parser->d = this; //Set this so we can safely call back BackgroundParser from within loadSettings()

        m_timer.setSingleShot(true);
        m_delay = 500;
        m_threads = 1;
        m_doneParseJobs = 0;
        m_maxParseJobs = 0;
        m_neededPriority = BackgroundParser::WorstPriority;

        ThreadWeaver::setDebugLevel(true, 1);

        QObject::connect(&m_timer, SIGNAL(timeout()), m_parser, SLOT(parseDocuments()));
    }

    void startTimerThreadSafe() {
        QMetaObject::invokeMethod(m_parser, "startTimer", Qt::QueuedConnection);
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
        if(m_shuttingDown)
            return;
        // Create delayed jobs, that is, jobs for documents which have been changed
        // by the user.
        QList<ParseJob*> jobs;

        for (QMap<int, QSet<KUrl> >::Iterator it1 = m_documentsForPriority.begin();
             it1 != m_documentsForPriority.end(); ++it1 )
        {
            if(it1.key() > m_neededPriority)
                break; //The priority is not good enough to be processed right now

            for(QSet<KUrl>::Iterator it = it1.value().begin(); it != it1.value().end();) {
                //Only create parse-jobs for up to thread-count * 2 documents, so we don't fill the memory unnecessarily
                if(m_parseJobs.count() >= m_threads+1 || (m_parseJobs.count() >= m_threads && !separateThreadForHighPriority) )
                    break;

                if(m_parseJobs.count() >= m_threads && it1.key() > BackgroundParser::NormalPriority && !specialParseJob)
                    break; //The additional parsing thread is reserved for higher priority parsing

                // When a document is scheduled for parsing while it is being parsed, it will be parsed
                // again once the job finished, but not now.
                if (m_parseJobs.contains(*it) ) {
                    ++it;
                    continue;
                }

                kDebug(9505) << "creating parse-job" << *it << "new count of active parse-jobs:" << m_parseJobs.count() + 1;
                ParseJob* job = createParseJob(*it, m_documents[*it].features(), m_documents[*it].notifyWhenReady());

                if(m_parseJobs.count() == m_threads+1 && !specialParseJob)
                    specialParseJob = job; //This parse-job is allocated into the reserved thread

                if(job)
                    jobs.append(job);

                m_documents.remove(*it);
                it = it1.value().erase(it);
                --m_maxParseJobs; //We have added one when putting the document into m_documents
            }
        }

        // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

        foreach (ParseJob* job, jobs)
            m_weaver.enqueue(job);

        m_parser->updateProgressBar();

        //We don't hide the progress-bar in updateProgressBar, so it doesn't permanently flash when a document is reparsed again and again.
        if(m_doneParseJobs == m_maxParseJobs
            || (m_neededPriority == BackgroundParser::BestPriority && m_weaver.queueLength() == 0))
        {
            emit m_parser->hideProgress(m_parser);
        }
    }

    ParseJob* createParseJob(const KUrl& url, TopDUContext::Features features, QList<QWeakPointer<QObject> > notifyWhenReady)
    {
        QList<ILanguage*> languages = m_languageController->languagesForUrl(url);
        foreach (ILanguage* language, languages) {
            if(!language) {
                kWarning() << "got zero language for" << url;
                continue;
            }
            if(!language->languageSupport()) {
                kWarning() << "language has no language support assigned:" << language->name();
                continue;
            }
            ParseJob* job = language->languageSupport()->createParseJob(url);
            if (!job) {
                continue; // Language part did not produce a valid ParseJob.
            }

            job->setMinimumFeatures(features);
            job->setBackgroundParser(m_parser);
            job->setNotifyWhenReady(notifyWhenReady);
            job->setTracker(m_parser->trackerForUrl(IndexedString(url)));

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

        if(languages.isEmpty())
            kDebug() << "found no languages for url" << url;
        else
            kDebug() << "could not create parse-job for url" << url;

        //Notify that we failed
        typedef QWeakPointer<QObject> Notify;
        foreach(const Notify& n, notifyWhenReady)
            if(n)
                QMetaObject::invokeMethod(n.data(), "updateReady", Qt::QueuedConnection, Q_ARG(KDevelop::IndexedString, IndexedString(url)), Q_ARG(KDevelop::ReferencedTopDUContext, ReferencedTopDUContext()));

        return 0;
    }


    void loadSettings()
    {
        ///@todo re-load settings when they have been changed!
        Q_ASSERT(ICore::self()->activeSession());
        KConfigGroup config(ICore::self()->activeSession()->config(), "Background Parser");

        // stay backwards compatible
        KConfigGroup oldConfig(KGlobal::config(), "Background Parser");
#define BACKWARDS_COMPATIBLE_ENTRY(entry, default) \
config.readEntry(entry, oldConfig.readEntry(entry, default))

        m_delay = BACKWARDS_COMPATIBLE_ENTRY("Delay", 500);
        m_timer.setInterval(m_delay);
        m_threads = 0;
        m_parser->setThreadCount(BACKWARDS_COMPATIBLE_ENTRY("Number of Threads", 1));

        resume();

        if (BACKWARDS_COMPATIBLE_ENTRY("Enabled", true)) {
            m_parser->enableProcessing();
        } else {
            m_parser->disableProcessing();
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

    //Current parse-job that is executed in the additional thread
    QWeakPointer<ParseJob> specialParseJob;

    QTimer m_timer;
    int m_delay;
    int m_threads;
    
    bool m_shuttingDown;
    
    struct DocumentParseTarget {
        QWeakPointer<QObject> notifyWhenReady;
        int priority;
        TopDUContext::Features features;
        bool operator==(const DocumentParseTarget& rhs) const {
            return notifyWhenReady == rhs.notifyWhenReady && priority == rhs.priority && features == rhs.features;
        }
    };

    struct DocumentParsePlan {
        QList<DocumentParseTarget> targets;

        int priority() const {
            //Pick the best priority
            int ret = BackgroundParser::WorstPriority;
            foreach(const DocumentParseTarget &target, targets)
                if(target.priority < ret)
                    ret = target.priority;
            return ret;
        }

        TopDUContext::Features features() const {
            //Pick the best features
            TopDUContext::Features ret = (TopDUContext::Features)0;
            foreach(const DocumentParseTarget &target, targets)
                ret = (TopDUContext::Features) (ret | target.features);
            return ret;
        }

        QList<QWeakPointer<QObject> > notifyWhenReady() {
            QList<QWeakPointer<QObject> > ret;

            foreach(const DocumentParseTarget &target, targets)
                if(target.notifyWhenReady)
                    ret << target.notifyWhenReady;

            return ret;
        }
    };
    // A list of documents that are planned to be parsed, and their priority
    QMap<KUrl, DocumentParsePlan > m_documents;
    // The documents ordered by priority
    QMap<int, QSet<KUrl> > m_documentsForPriority;
    // Currently running parse jobs
    QHash<KUrl, ParseJob*> m_parseJobs;
    // A change tracker for each managed document
    QHash<IndexedString, DocumentChangeTracker*> m_managed;
    // The url for each managed document. Those may temporarily differ from the real url.
    QHash<KTextEditor::Document*, IndexedString> m_managedTextDocumentUrls;

    ThreadWeaver::Weaver m_weaver;
    ParserDependencyPolicy m_dependencyPolicy;

    QMutex m_mutex;

    int m_maxParseJobs;
    int m_doneParseJobs;
    QMap<KDevelop::ParseJob*, float> m_jobProgress;
    int m_neededPriority; //The minimum priority needed for processed jobs
};

BackgroundParser::BackgroundParser(ILanguageController *languageController)
    : QObject(languageController), d(new BackgroundParserPrivate(this, languageController))
{
    Q_ASSERT(ICore::self()->documentController());
    connect(ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), this, SLOT(documentLoaded(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentUrlChanged(KDevelop::IDocument*)), this, SLOT(documentUrlChanged(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), this, SLOT(documentClosed(KDevelop::IDocument*)));
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
}

void BackgroundParser::aboutToQuit()
{
    d->m_shuttingDown = true;
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

void BackgroundParser::loadSettings()
{
    d->loadSettings();
}

void BackgroundParser::parseProgress(KDevelop::ParseJob* job, float value, QString text)
{
    Q_UNUSED(text)
    d->m_jobProgress[job] = value;
    updateProgressBar();
}

void BackgroundParser::revertAllRequests(QObject* notifyWhenReady)
{
    QWeakPointer<QObject> p(notifyWhenReady);

    QMutexLocker lock(&d->m_mutex);
    for(QMap<KUrl, BackgroundParserPrivate::DocumentParsePlan >::iterator it = d->m_documents.begin(); it != d->m_documents.end(); ) {

        d->m_documentsForPriority[it.value().priority()].remove(it.key());

        int index = -1;
        for(int a = 0; a < (*it).targets.size(); ++a)
            if((*it).targets[a].notifyWhenReady.data() == notifyWhenReady)
                index = a;

        if(index != -1) {
            (*it).targets.removeAt(index);
            if((*it).targets.isEmpty()) {
                it = d->m_documents.erase(it);
                --d->m_maxParseJobs;

                continue;
            }
        }

        d->m_documentsForPriority[it.value().priority()].insert(it.key());
        ++it;
    }
}

void BackgroundParser::addDocument(const KUrl& url, TopDUContext::Features features, int priority, QObject* notifyWhenReady)
{
//     kDebug(9505) << "BackgroundParser::addDocument" << url.prettyUrl();
    QMutexLocker lock(&d->m_mutex);
    {
        Q_ASSERT(url.isValid());

        BackgroundParserPrivate::DocumentParseTarget target;
        target.priority = priority;
        target.features = features;
        target.notifyWhenReady = QWeakPointer<QObject>(notifyWhenReady);

        QMap<KUrl, BackgroundParserPrivate::DocumentParsePlan>::iterator it = d->m_documents.find(url);

        if (it != d->m_documents.end()) {
            //Update the stored plan

            d->m_documentsForPriority[it.value().priority()].remove(url);
            it.value().targets << target;
            d->m_documentsForPriority[it.value().priority()].insert(url);
        }else{
//             kDebug(9505) << "BackgroundParser::addDocument: queuing" << url;
            d->m_documents[url].targets << target;
            d->m_documentsForPriority[d->m_documents[url].priority()].insert(url);
            ++d->m_maxParseJobs; //So the progress-bar waits for this document
        }

        d->startTimerThreadSafe();
    }
}

void BackgroundParser::addDocumentList(const KUrl::List &urls, TopDUContext::Features features, int priority)
{
    foreach (const KUrl &url, urls)
        addDocument(url, features, priority);
}

void BackgroundParser::removeDocument(const KUrl &url, QObject* notifyWhenReady)
{
    QMutexLocker lock(&d->m_mutex);

    Q_ASSERT(url.isValid());

    if(d->m_documents.contains(url)) {
        
        d->m_documentsForPriority[d->m_documents[url].priority()].remove(url);
        
        foreach(BackgroundParserPrivate::DocumentParseTarget target, d->m_documents[url].targets)
            if(target.notifyWhenReady.data() == notifyWhenReady)
                d->m_documents[url].targets.removeAll(target);

        if(d->m_documents[url].targets.isEmpty()) {
            d->m_documents.remove(url);
            --d->m_maxParseJobs;
        }else{
            //Insert with an eventually different priority
            d->m_documentsForPriority[d->m_documents[url].priority()].insert(url);
        }
    }
}

void BackgroundParser::parseDocuments()
{
    QMutexLocker lock(&d->m_mutex);

    d->parseDocumentsInternal();
}

void BackgroundParser::parseComplete(ThreadWeaver::Job* job)
{
    if (ParseJob* parseJob = qobject_cast<ParseJob*>(job)) {

        emit parseJobFinished(parseJob);

        {
            {
                QMutexLocker lock(&d->m_mutex);
    
                d->m_parseJobs.remove(parseJob->document().str());
    
                d->m_jobProgress.remove(parseJob);
    
                parseJob->setBackgroundParser(0);
    
                ++d->m_doneParseJobs;
                updateProgressBar();
            }
            //Unlock the mutex before deleting the parse-job, because the parse-job
            //has a virtual destructor that may lock the duchain, leading to deadlocks
            delete parseJob;
        }
        //Continue creating more parse-jobs
        QMetaObject::invokeMethod(this, "parseDocuments", Qt::QueuedConnection);
    }
}

void BackgroundParser::disableProcessing()
{
    setNeededPriority(BestPriority);
}

void BackgroundParser::enableProcessing()
{
    setNeededPriority(WorstPriority);
}

bool BackgroundParser::isQueued(KUrl url) const {
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents.contains(url);
}

int BackgroundParser::queuedCount() const
{
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents.count();
}

void BackgroundParser::setNeededPriority(int priority)
{
    QMutexLocker lock(&d->m_mutex);
    d->m_neededPriority = priority;
    d->startTimerThreadSafe();
}

void BackgroundParser::suspend()
{
    d->suspend();

    emit hideProgress(this);
}

void BackgroundParser::resume()
{
    d->resume();

    updateProgressBar();
}

void BackgroundParser::updateProgressBar()
{
    if (d->m_doneParseJobs >= d->m_maxParseJobs) {
        if(d->m_doneParseJobs > d->m_maxParseJobs) {
            kDebug() << "m_doneParseJobs larger than m_maxParseJobs:" << d->m_doneParseJobs << d->m_maxParseJobs;
        }
        d->m_doneParseJobs = 0;
        d->m_maxParseJobs = 0;
    } else {
        float additionalProgress = 0;
        for(QMap<KDevelop::ParseJob*, float>::const_iterator it = d->m_jobProgress.constBegin(); it != d->m_jobProgress.constEnd(); ++it)
            additionalProgress += *it;

        emit showProgress(this, 0, d->m_maxParseJobs*1000, (additionalProgress + d->m_doneParseJobs)*1000);
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
        d->m_weaver.setMaximumNumberOfThreads(d->m_threads+1); //1 Additional thread for high-priority parsing
    }
}

void BackgroundParser::setDelay(int miliseconds)
{
    if (d->m_delay != miliseconds) {
        d->m_delay = miliseconds;
        d->m_timer.setInterval(d->m_delay);
    }
}

QList< IndexedString > BackgroundParser::managedDocuments()
{
    QMutexLocker l(&d->m_mutex);
    
    return d->m_managed.keys();
}

DocumentChangeTracker* BackgroundParser::trackerForUrl(const KDevelop::IndexedString& url) const
{
    QMutexLocker l(&d->m_mutex);
    
    QHash< IndexedString, DocumentChangeTracker* >::iterator it = d->m_managed.find(url);
    if(it != d->m_managed.end())
        return *it;
    else
        return 0;
}

void BackgroundParser::documentClosed ( IDocument* document )
{
    QMutexLocker l(&d->m_mutex);
    
    if(document->textDocument())
    {
        KTextEditor::Document* textDocument = document->textDocument();
        
        if(!d->m_managedTextDocumentUrls.contains(textDocument))
            return; // Probably the document had an invalid url, and thus it wasn't added to the background parser
        
        Q_ASSERT(d->m_managedTextDocumentUrls.contains(textDocument));
        
        IndexedString url(d->m_managedTextDocumentUrls[textDocument]);
        Q_ASSERT(d->m_managed.contains(url));
        
        kDebug() << "removing" << url.str() << "from background parser";
        delete d->m_managed[url];
        d->m_managedTextDocumentUrls.remove(textDocument);
        d->m_managed.remove(url);
    }
}

void BackgroundParser::documentLoaded( IDocument* document )
{
    QMutexLocker l(&d->m_mutex);
    if(document->textDocument() && document->textDocument()->url().isValid())
    {
        KTextEditor::Document* textDocument = document->textDocument();
        
        IndexedString url(document->url());
        // Some debugging because we had issues with this
        
        if(d->m_managed.contains(url) && d->m_managed[url]->document() == textDocument)
        {
            kDebug() << "Got redundant documentLoaded from" << document->url() << textDocument;
            return;
        }
        
        kDebug() << "Creating change tracker for " << document->url();
        
        
        Q_ASSERT(!d->m_managed.contains(url));
        Q_ASSERT(!d->m_managedTextDocumentUrls.contains(textDocument));
        
        d->m_managedTextDocumentUrls[textDocument] = url;
        d->m_managed.insert(url, new DocumentChangeTracker(textDocument));
    }else{
        kDebug() << "NOT creating change tracker for" << document->url();
    }
}

void BackgroundParser::documentUrlChanged(IDocument* document)
{
    documentClosed(document);
    
    // Only call documentLoaded if the file wasn't renamed to a filename that is already tracked.
    if(document->textDocument() && !d->m_managed.contains(IndexedString(document->textDocument()->url())))
        documentLoaded(document);
}

void BackgroundParser::startTimer() {
    d->m_timer.start(d->m_delay);
}

}

#include "backgroundparser.moc"


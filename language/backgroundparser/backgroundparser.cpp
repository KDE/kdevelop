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

#include <ThreadWeaver/State>
#include <ThreadWeaver/ThreadWeaver>
#include <ThreadWeaver/DebuggingAids>

#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>

#include "../interfaces/ilanguagesupport.h"

#include "parsejob.h"
#include <editor/modificationrevisionset.h>
#include <interfaces/icore.h>
#include <qcoreapplication.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>

const bool separateThreadForHighPriority = true;

/**
 * Elides string in @p path, e.g. "VEEERY/LONG/PATH" -> ".../LONG/PATH"
 * - probably much faster than QFontMetrics::elidedText()
 * - we dont need a widget context
 * - takes path separators into account
 *
 * @p width Maximum number of characters
 *
 * TODO: Move to kdevutil?
 */
static QString elidedPathLeft(const QString& path, int width)
{
    static const QChar separator = QDir::separator();
    static const QString placeholder = "...";

    if (path.size() <= width) {
        return path;
    }

    int start = (path.size() - width) + placeholder.size();
    int pos = path.indexOf(separator, start);
    if (pos == -1) {
        pos = start; // no separator => just cut off the path at the beginning
    }
    Q_ASSERT(path.size() - pos >= 0 && path.size() - pos <= width);

    QStringRef elidedText = path.rightRef(path.size() - pos);
    QString result = placeholder;
    result.append(elidedText);
    return result;
}

namespace {
/**
 * @return true if @p url is non-empty, valid and has a clean path, false otherwise.
 */
inline bool isValidURL(const KDevelop::IndexedString& url)
{
    if (url.isEmpty()) {
        return false;
    }
    KUrl original = url.toUrl();
    if (!original.isValid()) {
        return false;
    }
    KUrl cleaned = original;
    cleaned.cleanPath();
    return original == cleaned;
}
}

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
        QHashIterator<IndexedString, ThreadWeaver::QObjectDecorator*> it = m_parseJobs;
        while (it.hasNext()) {
            it.next();
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
        QList<ThreadWeaver::JobPointer> jobs;

        // Before starting a new job, first wait for all higher-priority ones to finish.
        // That way, parse job priorities can be used for dependency handling.
        int bestRunningPriority = BackgroundParser::WorstPriority;
        foreach (const ThreadWeaver::QObjectDecorator* decorator, m_parseJobs) {
            const ParseJob* parseJob = dynamic_cast<const ParseJob*>(decorator->job());
            Q_ASSERT(parseJob);
            if (parseJob->respectsSequentialProcessing() && parseJob->parsePriority() < bestRunningPriority) {
                bestRunningPriority = parseJob->parsePriority();
            }
        }

        bool done = false;
        for (QMap<int, QSet<IndexedString> >::Iterator it1 = m_documentsForPriority.begin();
             it1 != m_documentsForPriority.end(); ++it1 )
        {

            if(it1.key() > m_neededPriority)
                break; //The priority is not good enough to be processed right now

            for(QSet<IndexedString>::Iterator it = it1.value().begin(); it != it1.value().end();) {
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

                Q_ASSERT(m_documents.contains(*it));
                const DocumentParsePlan& parsePlan = m_documents[*it];
                // If the current job requires sequential processing, but not all jobs with a better priority have been
                // completed yet, it will not be created now.
                if (    parsePlan.sequentialProcessingFlags() & ParseJob::RequiresSequentialProcessing
                     && parsePlan.priority() > bestRunningPriority )
                {
                    ++it;
                    continue;
                }

                kDebug(9505) << "creating parse-job" << it->toUrl() << "new count of active parse-jobs:" << m_parseJobs.count() + 1;
                const QString elidedPathString = elidedPathLeft(it->toUrl().toLocalFile(), 70);
                emit m_parser->showMessage(m_parser, i18n("Parsing: %1", elidedPathString));

                ThreadWeaver::QObjectDecorator* decorator = createParseJob(*it, parsePlan.features(), parsePlan.notifyWhenReady(), parsePlan.priority());

                if(m_parseJobs.count() == m_threads+1 && !specialParseJob)
                    specialParseJob = decorator; //This parse-job is allocated into the reserved thread

                if (decorator) {
                    ParseJob* parseJob = dynamic_cast<ParseJob*>(decorator->job());
                    parseJob->setSequentialProcessingFlags(parsePlan.sequentialProcessingFlags());
                    jobs.append(ThreadWeaver::JobPointer(decorator));
                    // update the currently best processed priority, if the created job respects sequential processing
                    if (   parsePlan.sequentialProcessingFlags() & ParseJob::RespectsSequentialProcessing
                        && parsePlan.priority() < bestRunningPriority)
                    {
                        bestRunningPriority = parsePlan.priority();
                    }
                }

                // Remove all mentions of this document.
                foreach(const DocumentParseTarget& target, parsePlan.targets) {
                    if (target.priority != it1.key()) {
                        m_documentsForPriority[target.priority].remove(*it);
                    }
                }
                m_documents.remove(*it);
                it = it1.value().erase(it);
                --m_maxParseJobs; //We have added one when putting the document into m_documents

                if(!m_documents.isEmpty())
                {
                    // Only try creating one parse-job at a time, else we might iterate through thousands of files
                    // without finding a language-support, and block the UI for a long time.
                    // If there are more documents to parse, instantly re-try.
                    QMetaObject::invokeMethod(m_parser, "parseDocuments", Qt::QueuedConnection);
                    done = true;
                    break;
                }
            }
            if ( done ) break;
        }

        // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

        foreach (ThreadWeaver::JobPointer job, jobs)
            m_weaver.enqueue(job);

        m_parser->updateProgressBar();

        //We don't hide the progress-bar in updateProgressBar, so it doesn't permanently flash when a document is reparsed again and again.
        if(m_doneParseJobs == m_maxParseJobs
            || (m_neededPriority == BackgroundParser::BestPriority && m_weaver.queueLength() == 0))
        {
            emit m_parser->hideProgress(m_parser);
        }
    }

    ThreadWeaver::QObjectDecorator* createParseJob(const IndexedString& url, TopDUContext::Features features, const QList<QWeakPointer<QObject> >& notifyWhenReady, int priority = 0)
    {
        ///FIXME: use IndexedString in the other APIs as well! Esp. for createParseJob!
        KUrl kUrl = url.toUrl();
        QList<ILanguage*> languages = m_languageController->languagesForUrl(kUrl);
        foreach (ILanguage* language, languages) {
            if(!language) {
                kWarning() << "got zero language for" << kUrl;
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

            job->setParsePriority(priority);
            job->setMinimumFeatures(features);
            job->setNotifyWhenReady(notifyWhenReady);

            ThreadWeaver::QObjectDecorator* decorator = new ThreadWeaver::QObjectDecorator(job);

            QObject::connect(decorator, SIGNAL(done(ThreadWeaver::JobPointer)),
                             m_parser, SLOT(parseComplete(ThreadWeaver::JobPointer)));
            QObject::connect(decorator, SIGNAL(failed(ThreadWeaver::JobPointer)),
                             m_parser, SLOT(parseComplete(ThreadWeaver::JobPointer)));
            QObject::connect(job, SIGNAL(progress(KDevelop::ParseJob*,float,QString)),
                             m_parser, SLOT(parseProgress(KDevelop::ParseJob*,float,QString)), Qt::QueuedConnection);

            m_parseJobs.insert(url, decorator);

            ++m_maxParseJobs;

            // TODO more thinking required here to support multiple parse jobs per url (where multiple language plugins want to parse)
            return decorator;
        }

        if(languages.isEmpty())
            kDebug() << "found no languages for url" << kUrl;
        else
            kDebug() << "could not create parse-job for url" << kUrl;

        //Notify that we failed
        typedef QWeakPointer<QObject> Notify;
        foreach(const Notify& n, notifyWhenReady)
            if(n)
                QMetaObject::invokeMethod(n.data(), "updateReady", Qt::QueuedConnection, Q_ARG(KDevelop::IndexedString, url), Q_ARG(KDevelop::ReferencedTopDUContext, ReferencedTopDUContext()));

        return nullptr;
    }


    void loadSettings()
    {
        ///@todo re-load settings when they have been changed!
        Q_ASSERT(ICore::self()->activeSession());
        KConfigGroup config(ICore::self()->activeSession()->config(), "Background Parser");

        // stay backwards compatible
        KConfigGroup oldConfig(KSharedConfig::openConfig(), "Background Parser");
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
        bool s = m_weaver.state()->stateId() == ThreadWeaver::Suspended ||
                 m_weaver.state()->stateId() == ThreadWeaver::Suspending;

        if (s) { // Already suspending
            return;
        }

        m_timer.stop();
        m_weaver.suspend();
    }

    void resume()
    {
        bool s = m_weaver.state()->stateId() == ThreadWeaver::Suspended ||
                 m_weaver.state()->stateId() == ThreadWeaver::Suspending;

        if (m_timer.isActive() && !s) { // Not suspending
            return;
        }

        m_timer.start(m_delay);
        m_weaver.resume();
    }

    BackgroundParser *m_parser;
    ILanguageController* m_languageController;

    //Current parse-job that is executed in the additional thread
    QWeakPointer<ThreadWeaver::JobInterface> specialParseJob;

    QTimer m_timer;
    int m_delay;
    int m_threads;

    bool m_shuttingDown;
    
    struct DocumentParseTarget {
        QWeakPointer<QObject> notifyWhenReady;
        int priority;
        TopDUContext::Features features;
        ParseJob::SequentialProcessingFlags sequentialProcessingFlags;
        bool operator==(const DocumentParseTarget& rhs) const {
            return notifyWhenReady == rhs.notifyWhenReady && priority == rhs.priority && features == rhs.features;
        }
    };

    struct DocumentParsePlan {
        QSet<DocumentParseTarget> targets;

        ParseJob::SequentialProcessingFlags sequentialProcessingFlags() const {
            //Pick the strictest possible flags
            ParseJob::SequentialProcessingFlags ret = ParseJob::IgnoresSequentialProcessing;
            foreach(const DocumentParseTarget &target, targets) {
                ret |= target.sequentialProcessingFlags;
            }
            return ret;
        }

        int priority() const {
            //Pick the best priority
            int ret = BackgroundParser::WorstPriority;
            foreach(const DocumentParseTarget &target, targets) {
                if(target.priority < ret) {
                    ret = target.priority;
                }
            }
            return ret;
        }

        TopDUContext::Features features() const {
            //Pick the best features
            TopDUContext::Features ret = (TopDUContext::Features)0;
            foreach(const DocumentParseTarget &target, targets) {
                ret = (TopDUContext::Features) (ret | target.features);
            }
            return ret;
        }

        QList<QWeakPointer<QObject> > notifyWhenReady() const {
            QList<QWeakPointer<QObject> > ret;

            foreach(const DocumentParseTarget &target, targets)
                if(target.notifyWhenReady)
                    ret << target.notifyWhenReady;

            return ret;
        }
    };
    // A list of documents that are planned to be parsed, and their priority
    QHash<IndexedString, DocumentParsePlan > m_documents;
    // The documents ordered by priority
    QMap<int, QSet<IndexedString> > m_documentsForPriority;
    // Currently running parse jobs
    QHash<IndexedString, ThreadWeaver::QObjectDecorator*> m_parseJobs;
    // A change tracker for each managed document
    QHash<IndexedString, DocumentChangeTracker*> m_managed;
    // The url for each managed document. Those may temporarily differ from the real url.
    QHash<KTextEditor::Document*, IndexedString> m_managedTextDocumentUrls;
    // Projects currently in progress of loading
    QSet<IProject*> m_loadingProjects;

    ThreadWeaver::Queue m_weaver;

    QMutex m_mutex;

    int m_maxParseJobs;
    int m_doneParseJobs;
    QHash<KDevelop::ParseJob*, float> m_jobProgress;
    int m_neededPriority; //The minimum priority needed for processed jobs
};

inline uint qHash(const BackgroundParserPrivate::DocumentParseTarget& target) {
    return target.features * 7 + target.priority * 13 + target.sequentialProcessingFlags * 17 
                               + reinterpret_cast<size_t>(target.notifyWhenReady.data());
};

BackgroundParser::BackgroundParser(ILanguageController *languageController)
    : QObject(languageController), d(new BackgroundParserPrivate(this, languageController))
{
    Q_ASSERT(ICore::self()->documentController());
    connect(ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), this, SLOT(documentLoaded(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentUrlChanged(KDevelop::IDocument*)), this, SLOT(documentUrlChanged(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), this, SLOT(documentClosed(KDevelop::IDocument*)));
    connect(ICore::self(), SIGNAL(aboutToShutdown()), this, SLOT(aboutToQuit()));

    bool connected = QObject::connect(ICore::self()->projectController(),
                                      SIGNAL(projectAboutToBeOpened(KDevelop::IProject*)),
                                      this, SLOT(projectAboutToBeOpened(KDevelop::IProject*)));
    Q_ASSERT(connected);
    connected = QObject::connect(ICore::self()->projectController(),
                                 SIGNAL(projectOpened(KDevelop::IProject*)),
                                 this, SLOT(projectOpened(KDevelop::IProject*)));
    Q_ASSERT(connected);
    connected = QObject::connect(ICore::self()->projectController(),
                                 SIGNAL(projectOpeningAborted(KDevelop::IProject*)),
                                 this, SLOT(projectOpeningAborted(KDevelop::IProject*)));
    Q_ASSERT(connected);
    Q_UNUSED(connected);
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
    QMutexLocker lock(&d->m_mutex);
    for(QHash<IndexedString, BackgroundParserPrivate::DocumentParsePlan >::iterator it = d->m_documents.begin(); it != d->m_documents.end(); ) {

        d->m_documentsForPriority[it.value().priority()].remove(it.key());

        foreach ( const BackgroundParserPrivate::DocumentParseTarget& target, (*it).targets ) {
            if ( target.notifyWhenReady.data() == notifyWhenReady ) {
                (*it).targets.remove(target);
            }
        }

        if((*it).targets.isEmpty()) {
            it = d->m_documents.erase(it);
            --d->m_maxParseJobs;

            continue;
        }

        d->m_documentsForPriority[it.value().priority()].insert(it.key());
        ++it;
    }
}

void BackgroundParser::addDocument(const IndexedString& url, TopDUContext::Features features, int priority, QObject* notifyWhenReady, ParseJob::SequentialProcessingFlags flags)
{
//     kDebug(9505) << "BackgroundParser::addDocument" << url.toUrl();
    Q_ASSERT(isValidURL(url));
    QMutexLocker lock(&d->m_mutex);
    {
        BackgroundParserPrivate::DocumentParseTarget target;
        target.priority = priority;
        target.features = features;
        target.sequentialProcessingFlags = flags;
        target.notifyWhenReady = QWeakPointer<QObject>(notifyWhenReady);

        QHash<IndexedString, BackgroundParserPrivate::DocumentParsePlan>::iterator it = d->m_documents.find(url);

        if (it != d->m_documents.end()) {
            //Update the stored plan

            d->m_documentsForPriority[it.value().priority()].remove(url);
            it.value().targets << target;
            d->m_documentsForPriority[it.value().priority()].insert(url);
        }else{
//             kDebug(9505) << "BackgroundParser::addDocument: queuing" << cleanedUrl;
            d->m_documents[url].targets << target;
            d->m_documentsForPriority[d->m_documents[url].priority()].insert(url);
            ++d->m_maxParseJobs; //So the progress-bar waits for this document
        }

        d->startTimerThreadSafe();
    }
}

void BackgroundParser::removeDocument(const IndexedString& url, QObject* notifyWhenReady)
{
    Q_ASSERT(isValidURL(url));

    QMutexLocker lock(&d->m_mutex);

    if(d->m_documents.contains(url)) {

        d->m_documentsForPriority[d->m_documents[url].priority()].remove(url);

        foreach(const BackgroundParserPrivate::DocumentParseTarget& target, d->m_documents[url].targets) {
            if(target.notifyWhenReady.data() == notifyWhenReady) {
                d->m_documents[url].targets.remove(target);
            }
        }

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
    if (!d->m_loadingProjects.empty()) {
        startTimer();
        return;
    }
    QMutexLocker lock(&d->m_mutex);

    d->parseDocumentsInternal();
}

void BackgroundParser::parseComplete(const ThreadWeaver::JobPointer& job)
{
    auto decorator = dynamic_cast<ThreadWeaver::QObjectDecorator*>(job.data());
    Q_ASSERT(decorator);
    ParseJob* parseJob = dynamic_cast<ParseJob*>(decorator->job());
    Q_ASSERT(parseJob);
    emit parseJobFinished(parseJob);

    {
        QMutexLocker lock(&d->m_mutex);

        d->m_parseJobs.remove(parseJob->document());

        d->m_jobProgress.remove(parseJob);

        ++d->m_doneParseJobs;
        updateProgressBar();
    }

    //Continue creating more parse-jobs
    QMetaObject::invokeMethod(this, "parseDocuments", Qt::QueuedConnection);
}

void BackgroundParser::disableProcessing()
{
    setNeededPriority(BestPriority);
}

void BackgroundParser::enableProcessing()
{
    setNeededPriority(WorstPriority);
}

int BackgroundParser::priorityForDocument(const IndexedString& url) const
{
    Q_ASSERT(isValidURL(url));
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents[url].priority();
}

bool BackgroundParser::isQueued(const IndexedString& url) const
{
    Q_ASSERT(isValidURL(url));
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents.contains(url);
}

int BackgroundParser::queuedCount() const
{
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents.count();
}

bool BackgroundParser::isIdle() const
{
    QMutexLocker lock(&d->m_mutex);
    return d->m_documents.isEmpty() && d->m_weaver.isIdle();
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
        for(QHash<KDevelop::ParseJob*, float>::const_iterator it = d->m_jobProgress.constBegin(); it != d->m_jobProgress.constEnd(); ++it)
            additionalProgress += *it;

        emit showProgress(this, 0, d->m_maxParseJobs*1000, (additionalProgress + d->m_doneParseJobs)*1000);
    }
}

ParseJob* BackgroundParser::parseJobForDocument(const IndexedString& document) const
{
    Q_ASSERT(isValidURL(document));

    QMutexLocker lock(&d->m_mutex);
    auto decorator = d->m_parseJobs.value(document);
    return decorator ? dynamic_cast<ParseJob*>(decorator->job()) : nullptr;
}

void BackgroundParser::setThreadCount(int threadCount)
{
    if (d->m_threads != threadCount) {
        d->m_threads = threadCount;
        d->m_weaver.setMaximumNumberOfThreads(d->m_threads+1); //1 Additional thread for high-priority parsing
    }
}

int BackgroundParser::threadCount() const
{
    return d->m_threads;
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
    if (url.isEmpty()) {
        // this happens e.g. when setting the final location of a problem that is not
        // yet associated with a top ctx.
        return 0;
    }
    Q_ASSERT(isValidURL(url));

    QMutexLocker l(&d->m_mutex);
    return d->m_managed.value(url, 0);
}

void BackgroundParser::documentClosed(IDocument* document)
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

void BackgroundParser::projectAboutToBeOpened(IProject* project)
{
    d->m_loadingProjects.insert(project);
}

void BackgroundParser::projectOpened(IProject* project)
{
    d->m_loadingProjects.remove(project);
}

void BackgroundParser::projectOpeningAborted(IProject* project)
{
    d->m_loadingProjects.remove(project);
}

}

Q_DECLARE_TYPEINFO(KDevelop::BackgroundParserPrivate::DocumentParseTarget, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::BackgroundParserPrivate::DocumentParsePlan, Q_MOVABLE_TYPE);

#include "backgroundparser.moc"


/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Kris Wong <kris.p.wong@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BACKGROUNDPARSER_H
#define KDEVPLATFORM_BACKGROUNDPARSER_H

#include <language/languageexport.h>
#include <interfaces/istatus.h>
#include <language/duchain/topducontext.h>
#include <language/interfaces/ilanguagesupport.h>
#include "parsejob.h"

namespace KDevelop {
class DocumentChangeTracker;

class IDocument;
class IProject;
class ILanguageController;
class ParserDependencyPolicy;

/**
 * This class handles the creation of parse jobs for given file URLs.
 *
 * For performance reasons you must always use clean, canonical URLs. If you do not do that,
 * issues might arise (and the debug build will assert).
 */
class KDEVPLATFORMLANGUAGE_EXPORT BackgroundParser
    : public QObject
    , public IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    explicit BackgroundParser(ILanguageController* languageController);
    ~BackgroundParser() override;

    QString statusName() const override;

    enum {
        BestPriority = -10000,  ///Best possible job-priority. No jobs should actually have this.
        NormalPriority = 0,     ///Standard job-priority. This priority is used for parse-jobs caused by document-editing/opening.
        ///There is an additional parsing-thread reserved for jobs with this and better priority, to improve responsiveness.
        InitialParsePriority = 10000, ///Priority used when adding file on project loading
        WorstPriority = 100000  ///Worst possible job-priority.
    };

    /**
     * Queries the background parser as to whether there is currently
     * a parse job for @p document, and if so, returns it.
     *
     * This may not contain all of the parse jobs that are intended
     * unless you call in from your job's ThreadWeaver::Job::aboutToBeQueued()
     * function.
     */
    ParseJob* parseJobForDocument(const IndexedString& document) const;

    /**
     * Set how many ThreadWeaver threads the background parser should set up and use.
     */
    void setThreadCount(int threadCount);

    /**
     * Return how many ThreadWeaver threads the background parser should set up and use.
     */
    int threadCount() const;

    /**
     * Set the delay in milliseconds before the background parser starts parsing.
     */
    void setDelay(int milliseconds);

    /**
     * Returns all documents that were added through addManagedTopRange. This is typically the currently
     * open documents.
     */
    QList<IndexedString> managedDocuments();

    /**
     * Returns the tracker for the given url if the document is being tracked, else returns zero.
     * This function is thread-safe, but the returned object also isn't, so you must not use it
     * when you're in a background thread without the foreground lock acquired.
     * */
    DocumentChangeTracker* trackerForUrl(const IndexedString& url) const;

    bool waitForIdle() const;

    /// Returns true if the given url is queued for parsing
    bool isQueued(const IndexedString& url) const;

    /// Retrieve the current priority for the given URL.
    /// You need to check whether @param url is queued before calling this function.
    int priorityForDocument(const IndexedString& url) const;

    /// Returns the number of queued jobs (not yet running nor submitted to ThreadWeaver)
    int queuedCount() const;

    /// Returns true if there are no jobs running nor queued anywhere
    bool isIdle() const;

Q_SIGNALS:
    /**
     * Emitted whenever a document parse-job has finished.
     * The job contains the du-chain(if one was created) etc.
     *
     * The job is deleted after this signal has been emitted.  Receivers should not hold
     * references to it.
     *
     * Note that if you want to be get updated for all DUChain updates, use
     * DUChain::updateReady instead, as a single ParseJob may update multiple
     * DUChain top contexts.
     *
     * @sa DUChain::updateReady
     */
    void parseJobFinished(KDevelop::ParseJob* job);

    // Implementations of IStatus signals
    void clearMessage(KDevelop::IStatus*) override;
    void showMessage(KDevelop::IStatus*, const QString& message, int timeout = 0) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;
    void showErrorMessage(const QString&, int) override;

public Q_SLOTS:
    /**
     * Aborts all parse jobs
     */
    void abortAllJobs();

    /**
     * Suspends execution of the background parser
     *
     * NOTE: Only the Core should call this
     */
    void suspend();

    /**
     * Resumes execution of the background parser
     *
     * NOTE: Only the Core should call this
     */
    void resume();

    /// Reverts all requests that were made for the given notification-target.
    /// priorities and requested features will be reverted as well.
    void revertAllRequests(QObject* notifyWhenReady);

    /**
     * Queues up the @p url to be parsed.
     * @p features The minimum features that should be computed for this top-context
     * @p priority A value that manages the order of parsing. Documents with lowest priority are parsed first.
     * @param notifyWhenReady An optional pointer to a QObject that should contain a slot
     *                        "void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext)".
     *                        The notification is guaranteed to be called once for each call to addDocument. The given top-context
     *                        may be invalid if the update failed.
     * @param flags Flags indicating how the document should be treated in the queue
     * @param delay_ms The delay in milliseconds to add the job with, or one of the values of the
     *                 ILanguageSupport::ReparseDelaySpecialValues enum.
     */
    void addDocument(const IndexedString& url,
                     TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts,
                     int priority = 0,
                     QObject* notifyWhenReady = nullptr,
                     ParseJob::SequentialProcessingFlags flags = ParseJob::IgnoresSequentialProcessing,
                     int delay_ms = ILanguageSupport::DefaultDelay);

    /**
     * try to add a listener to an existing document
     * @see addDocument(...) for parameter description
     * @returns true if the document existed and a listener was added
     */
    [[nodiscard]]
    bool addListenerToDocumentIfExist(const IndexedString& url,
                     TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts,
                     int priority = 0,
                     QObject* notifyWhenReady = nullptr,
                     ParseJob::SequentialProcessingFlags flags = ParseJob::IgnoresSequentialProcessing,
                     int delay_ms = ILanguageSupport::DefaultDelay);

    /**
     * Removes the @p url that is registered for the given notification from the url.
     *
     * @param notifyWhenReady Notifier the document was added with.
     */
    void removeDocument(const IndexedString& url, QObject* notifyWhenReady = nullptr);

    /**
     * Forces the current queue to be parsed.
     */
    void parseDocuments();

    void updateProgressData();

    /// Disables processing for all jobs that have a worse priority than @p priority
    /// This can be used to temporarily limit the processing to only the most important jobs.
    /// To only enable processing for important jobs, call setNeededPriority(0).
    /// This should only be used to temporarily alter the processing. A progress-bar
    /// will still be shown for the not yet processed jobs.
    void setNeededPriority(int priority);
    /// Disables all processing of new jobs, equivalent to setNeededPriority(BestPriority)
    void disableProcessing();
    /// Enables all processing of new jobs, equivalent to setNeededPriority(WorstPriority)
    void enableProcessing();

    void documentClosed(KDevelop::IDocument*);
    void documentLoaded(KDevelop::IDocument*);
    void documentUrlChanged(KDevelop::IDocument*);

    void loadSettings();

protected Q_SLOTS:
    void parseComplete(const ThreadWeaver::JobPointer& job);
    void parseProgress(KDevelop::ParseJob*, float value, const QString& text);
    void startTimer(int delay);
    void aboutToQuit();
    void updateProgressBar();

private:
    friend class BackgroundParserPrivate;
    class BackgroundParserPrivate* d_ptr;
    Q_DECLARE_PRIVATE(BackgroundParser)

private Q_SLOTS:
    /// Tracking of projects in state of loading.
    void projectAboutToBeOpened(KDevelop::IProject* project);
    void projectOpened(KDevelop::IProject* project);
    void projectOpeningAborted(KDevelop::IProject* project);
};
}
#endif

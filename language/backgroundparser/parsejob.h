/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_PARSEJOB_H
#define KDEVPLATFORM_PARSEJOB_H

#include <QtCore/QWeakPointer>
#include <KDE/KUrl>

#include <ThreadWeaver/Sequence>

#include "../duchain/indexedstring.h"
#include <language/duchain/topducontext.h>
#include <language/editor/modificationrevision.h>

namespace ThreadWeaver {
class QObjectDecorator;
}

namespace KDevelop
{
class ControlFlowGraph;
class DataAccessRepository;
class BackgroundParser;
class TopDUContext;
class ReferencedTopDUContext;

/**
 * The base class for background parser jobs.
 *
 * In your language plugin, don't forget to use acquire an UrlParseLock before starting to the actual parsing.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ParseJob : public QObject, public ThreadWeaver::Sequence
{
    Q_OBJECT

public:
    explicit ParseJob( const IndexedString &url, ILanguageSupport* languageSupport );
    /**
     * _No_ mutexes/locks are allowed to be locked when this object is destroyed (except for optionally the foreground lock)
     * */
    virtual ~ParseJob();

    /**
     * @return the language support that created this parse job.
     */
    ILanguageSupport* languageSupport() const;

    struct Contents {
        // Modification-time of the read content
        ModificationRevision modification;
        // The contents in utf-8 format
        QByteArray contents;
    };

    enum SequentialProcessingFlag {
        IgnoresSequentialProcessing = 0,
        RequiresSequentialProcessing = 1,
        RespectsSequentialProcessing = 2,
        FullSequentialProcessing = 3
    };
    Q_DECLARE_FLAGS(SequentialProcessingFlags, SequentialProcessingFlag);

    ///Sets the priority of this parse job. This is just for the purpose of
    ///reading it later, and does not affect the actual behaviour in any way.
    void setParsePriority(int priority);
    ///Get the priority of this parse job.
    ///Other than priority(), this will give you the "KDevelop-priority" of the job,
    ///not the QThread one (which is always zero).
    int parsePriority() const;

    /**
     * _No_ mutexes/locks are allowed to be locked when this is called (except for optionally the foreground lock)
     *
     * Locks the document revision so that mapping from/to the revision in the editor using MovingInterface will be possible.
     *
     * Returns an invalid pointer if the call succeeds, and a valid one if the reading fails.
     * */
    KDevelop::ProblemPointer readContents();

    /**
     * After reading the contents, you can call this to retrieve it.
     * */
    const Contents& contents() const;

    /**
     * Translates the given context from its previous revision to the revision that has
     * been retrieved during readContents(). The top-context meta-data will be updated
     * with the revision.
     *
     * This can be done after reading the context before updating, so
     * that the correct ranges are matched onto each other during the update.
     *
     * _No_ mutexes/locks are allowed to be locked when this is called (except for optionally the foreground lock)
     */
    void translateDUChainToRevision(TopDUContext* context);

    /**
     * Query whether this job is needed to be waited for when trying to process a job with a lower priority.
     **/
    bool respectsSequentialProcessing() const;

    /**
     * Query whether this job requires all higher-priority jobs to finish before being processed itself.
     **/
    bool requiresSequentialProcessing() const;

    void setSequentialProcessingFlags(SequentialProcessingFlags flags);

    /// \returns the indexed url of the document to be parsed.
    Q_SCRIPTABLE KDevelop::IndexedString document() const;

    /**
    * Sets a list of QObjects that should contain a slot
    * "void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext)".
    * The notification is guaranteed to be called once the parse-job finishes, from within its destructor.
    * The given top-context may be invalid if the update failed.
    */
    Q_SCRIPTABLE void setNotifyWhenReady(const QList<QWeakPointer<QObject> >& notify);

    /// Sets the du-context that was created by this parse-job
    Q_SCRIPTABLE virtual void setDuChain(ReferencedTopDUContext duChain);
    /// Returns the set du-context, or zero of none was set.
    Q_SCRIPTABLE virtual ReferencedTopDUContext duChain() const;

    /// Overridden to allow jobs to determine if they've been requested to abort
    Q_SCRIPTABLE virtual void requestAbort();
    /// Determine if the job has been requested to abort
    Q_SCRIPTABLE bool abortRequested() const;
    /// Sets success to false, causing failed() to be emitted
    Q_SCRIPTABLE void abortJob();

    /// Overridden to convey whether the job succeeded or not.
    Q_SCRIPTABLE virtual bool success() const;

    /// Set the minimum features the resulting top-context should have
    Q_SCRIPTABLE void setMinimumFeatures(TopDUContext::Features features);

    /// Minimum set of features the resulting top-context should have
    Q_SCRIPTABLE TopDUContext::Features minimumFeatures() const;

    /// Allows statically specifying an amount of features required for an url.
    /// These features will automatically be or'ed with the minimumFeatures() returned
    /// by any ParseJob with the given url.
    /// Since this causes some additional complixity in update-checking, minimum features should not
    /// be set permanently.
    static void setStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features);

    /// Must be called exactly once for each call to setStaticMinimumFeatures, with the same features.
    static void unsetStaticMinimumFeatures(const IndexedString& url, TopDUContext::Features features);

    /// Returns the statically set minimum features for  the given url, or zero.
    static TopDUContext::Features staticMinimumFeatures(const IndexedString& url);

    /// Returns whether there is minimum features set up for some url
    static bool hasStaticMinimumFeatures();
    
    ///Returns a structure containing information about data accesses in the parsed file.
    /// It's up to the caller to remove the returned instance
    virtual KDevelop::DataAccessRepository* dataAccessInformation();
    
    ///Returns a control flow graph for the code in the parsed file.
    /// It's up to the caller to remove the returned instance
    virtual KDevelop::ControlFlowGraph* controlFlowGraph();

    ThreadWeaver::QObjectDecorator* decorator() const;

Q_SIGNALS:
    /**Can be used to give progress feedback to the background-parser. @param value should be between 0 and 1, where 0 = 0% and 1 = 100%
     * @param text may be a text that describes the current state of parsing
     * Do not trigger this too often, for performance reasons. */
    void progress(KDevelop::ParseJob*, float value, const QString& text);

protected:
    /**
     * Checks whether there is already an up to date context available for the
     * current document. If so, it returns true and ensures that the document
     * is highlighted properly. Otherwise returns false.
     *
     * NOTE: This should be called while holding an URLParseLock for the
     * current document.
     *
     * @param languageString The unique string identifying your language.
     * This must be the same as you assign to the DUChain's environment file.
     *
     * @return True if an update is required, false if the job can return early.
     */
    bool isUpdateRequired(const IndexedString& languageString);

    /**
     * Trigger an update to the code highlighting of the current file based
     * on the DUChain set in setDuChain.
     *
     * If the file for this parse job is not opened in an editor or if the language
     * support does not return a code highlighter, this will do nothing.
     *
     * NOTE: No DUChain lock should be held when you call this.
     */
    void highlightDUChain();

    /**
     * Returns whether there is a tracker for the current document.
     */
    bool hasTracker() const;

private:
    class ParseJobPrivate* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ParseJob::SequentialProcessingFlags);

}
Q_DECLARE_METATYPE(KDevelop::ParseJob*);

#endif

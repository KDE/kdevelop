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

#ifndef KDEVPARSEJOB_H
#define KDEVPARSEJOB_H

#include <KUrl>

#include <JobSequence.h>

#include "language/duchain/indexedstring.h"
#include "language/backgroundparser/documentchangetracker.h"

namespace KDevelop
{
class BackgroundParser;
class TopDUContext;

/**
 * The base class for background parser jobs.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ParseJob : public ThreadWeaver::JobSequence, public DocumentChangeTracker
{
    Q_OBJECT
public:
    ParseJob( const KUrl &url, QObject *parent );
    virtual ~ParseJob();

    BackgroundParser* backgroundParser() const;
    void setBackgroundParser(BackgroundParser* parser);

    virtual int priority() const;

    /**
     * Determine whether the editor can provide the contents of the document or not.
     * Once this is called, the editor integrator saves the revision token, and no changes will
     * be made to the changedRanges().
     * You can then just call KTextEditor::SmartRange::text() on each of the changedRanges().
     * Or, you can parse the whole document, the text of which is available from contentsFromEditor().
     */
    bool contentsAvailableFromEditor();

    /// Retrieve the contents of the file from the currently open editor.
    /// Ensure it is loaded by calling editorLoaded() first.
    /// The editor integrator seamlessly saves the revision token and applies it
    QString contentsFromEditor();

    /// Returns the revision token issued by the document's smart interface,
    /// or -1 if there was a problem.
    int revisionToken() const;

    /// \returns the indexed url of the document to be parsed.
    KDevelop::IndexedString document() const;

    /// Sets the du-context that was created by this parse-job
    virtual void setDuChain(TopDUContext* duChain);
    /// Returns the set du-context, or zero of none was set.
    virtual TopDUContext* duChain() const;

    /// Overriden to allow jobs to determine if they've been requested to abort
    virtual void requestAbort();
    /// Determine if the job has been requested to abort
    bool abortRequested() const;
    /// Sets success to false, causing failed() to be emitted
    void abortJob();

    /// Overridden to convey whether the job succeeded or not.
    virtual bool success() const;

    /// Overridden to set the DependencyPolicy on subjobs.
    virtual void addJob(Job* job);

    /**
     * Attempt to add \a dependency as a dependency of \a actualDependee, which must
     * be a subjob of this job, or null (in which case, the dependency is added
     * to this job).  If a circular dependency is detected, the dependency will
     * not be added and the method will return false.
     */
    bool addDependency(ParseJob* dependency, ThreadWeaver::Job* actualDependee = 0);

Q_SIGNALS:
    /**Can be used to give progress feedback to the background-parser. @param value should be between 0 and 1, where 0 = 0% and 1 = 100%
     * @param text may be a text that describes the current state of parsing
     * Do not trigger this too often, for performance reasons. */
    void progress(KDevelop::ParseJob*, float value, QString text);

private:
    class ParseJobPrivate* const d;
};

}
#endif


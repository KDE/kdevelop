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

#ifndef KDEVBACKGROUNDPARSER_H
#define KDEVBACKGROUNDPARSER_H

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QMutex>
#include <QtCore/QHash>
#include <QtCore/QPointer>

#include <KUrl>

#include <KTextEditor/SmartRangeWatcher>

#include "../languageexport.h"
#include <interfaces/istatus.h>

class QMutex;

namespace ThreadWeaver
{
class Weaver;
class Job;
}

namespace KDevelop
{

class ILanguageController;
class ParseJob;
class ParserDependencyPolicy;

class KDEVPLATFORMLANGUAGE_EXPORT BackgroundParser : public QObject, public IStatus, public KTextEditor::SmartRangeWatcher
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IStatus )

public:
    BackgroundParser(ILanguageController *languageController);
    ~BackgroundParser();

    virtual QString statusName() const;

    /**
     * Abort or dequeue all current running jobs with the specified @p parent.
     */
    void clear(QObject* parent);

    /**
     * Queries the background parser as to whether there is currently
     * a parse job for @p document, and if so, returns it.
     *
     * This may not contain all of the parse jobs that are intended
     * unless you call in from your job's ThreadWeaver::Job::aboutToBeQueued()
     * function.
     */
    ParseJob* parseJobForDocument(const KUrl& document) const;

    /**
     * The dependency policy which applies to all jobs (it is applied automatically).
     */
    ParserDependencyPolicy* dependencyPolicy() const;

    /**
     * Set how many ThreadWeaver threads the background parser should set up and use.
     */
    void setThreadCount(int threadCount);

    /**
     * Set the delay in miliseconds before the background parser starts parsing.
     */
    void setDelay(int miliseconds);

    /**
     * Inform the background parser that \a document has a given top smart \a range.
     *
     * This will be watched for modifications and background jobs scheduled accordingly.
     */
    void addManagedTopRange(const KUrl& document, KTextEditor::SmartRange* range);

    /**
     * Remove an associated top \a range from modification watching.
     */
    void removeManagedTopRange(KTextEditor::SmartRange* range);

Q_SIGNALS:
    /** 
	 * Emitted whenever a document parse-job has finished. 
	 * The job contains the du-chain(if one was created) etc.
	 *
	 * The job is deleted after this signal has been emitted.  Receivers should not hold
	 * references to it.
	 */
    void parseJobFinished(KDevelop::ParseJob* job);

    // Implementations of IStatus signals
    void clearMessage();
    void showMessage(const QString & message, int timeout = 0);
    void hideProgress();
    void showProgress(int minimum, int maximum, int value);

public Q_SLOTS:

    /**
     * Suspends execution of the background parser
     */
    void suspend();

    /**
     * Resumes execution of the background parser
     */
    void resume();

    /**
     * Queues up the @p url to be parsed.
     */
    void addDocument(const KUrl& url);

    /**
     * Queues up the list of @p urls to be parsed.
     */
    void addDocumentList(const KUrl::List& urls);

    /**
     * Removes the @p url from the queue.
     */
    void removeDocument(const KUrl& url);

    /**
     * Forces the current queue to be parsed.
     */
    void parseDocuments();

    void updateProgressBar();

protected:
    void loadSettings(bool projectIsLoaded);
    void saveSettings(bool projectIsLoaded);

protected Q_SLOTS:
    void parseComplete(ThreadWeaver::Job *job);
    void parseProgress(KDevelop::ParseJob*, float value, QString text);

protected:
    // Receive changed notifications
    using KTextEditor::SmartRangeWatcher::rangeContentsChanged;
    virtual void rangeContentsChanged(KTextEditor::SmartRange* range, KTextEditor::SmartRange* mostSpecificChild);

private:
    friend class BackgroundParserPrivate;
    class BackgroundParserPrivate *d;
};

}
#endif


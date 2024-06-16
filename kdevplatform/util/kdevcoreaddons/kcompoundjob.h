/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOUNDJOB_H
#define KCOMPOUNDJOB_H

#include <util/utilexport.h>

#include <KJob>

#include <QList>

namespace KDevCoreAddons
{
class KCompoundJobPrivate;
/**
 * @class KCompoundJob kcompoundjob.h KCompoundJob
 *
 * The base class for all jobs able to be composed of one
 * or more subjobs.
 */
class KDEVPLATFORMUTIL_EXPORT KCompoundJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new KCompoundJob object.
     *
     * @param parent the parent QObject
     */
    explicit KCompoundJob(QObject *parent = nullptr);

    /**
     * Destroys a KCompoundJob object.
     */
    ~KCompoundJob() override;

protected:
    /**
     * Add a job that has to be finished before a result
     * is emitted. This has obviously to be called before
     * the finished() signal has been emitted by the job.
     *
     * Note that the compound job takes ownership of @p job
     *
     * @param job the subjob to add
     * @return true if the job has been added correctly, false otherwise
     */
    virtual bool addSubjob(KJob *job);

    /**
     * Mark a sub job as being done.
     *
     * The ownership of @p job is passed on to the caller.
     *
     * @param job the subjob to remove
     * @return true if the job has been removed correctly, false otherwise
     */
    virtual bool removeSubjob(KJob *job);

    /**
     * Checks if this job has subjobs running.
     *
     * @return true if we still have subjobs running, false otherwise
     */
    bool hasSubjobs() const;

    /**
     * Retrieves the list of the subjobs.
     *
     * @return the full list of sub jobs
     */
    const QList<KJob *> &subjobs() const;

    /**
     * Clears the list of subjobs.
     *
     * Note that this will *not* delete the subjobs.
     * Ownership of the subjobs is passed on to the caller.
     */
    virtual void clearSubjobs();

protected Q_SLOTS:
    /**
     * Called whenever a subjob finishes.
     * Default implementation checks for errors and propagates
     * to parent job, and in all cases it calls removeSubjob.
     *
     * @param job the subjob
     */
    virtual void subjobFinished(KJob *job);

    /**
     * Forward signal from subjob.
     *
     * @param job the subjob
     * @param message the info message
     * @see infoMessage()
     */
    virtual void subjobInfoMessage(KJob *job, const QString &message);

protected:
    std::unique_ptr<KCompoundJobPrivate> const d_ptr;
    KCompoundJob(KCompoundJobPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(KCompoundJob)
};

} // namespace KDevCoreAddons

#endif

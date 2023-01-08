/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSEQUENTIALCOMPOUNDJOB_H
#define KSEQUENTIALCOMPOUNDJOB_H

#include "kcompoundjob.h"

#include <util/utilexport.h>

namespace KDevCoreAddons
{
class KSequentialCompoundJobPrivate;
/**
 * @class KSequentialCompoundJob ksequentialcompoundjob.h KSequentialCompoundJob
 *
 * A compound job that executes its subjobs one by one in order and finishes
 * when the last subjob finishes. Killing is propagated to subjobs. State
 * information, overall progress and errors are propagated from subjobs.
 *
 * The public API does not allow to add subjobs, so KSequentialCompoundJob is
 * usable only as a base class. addSubjob() is protected because it should be
 * called before start(). A derived class that adds subjobs itself can keep
 * addSubjob() protected. KSimpleSequentialCompoundJob makes addSubjob() public
 * and can be used on its own.
 */
class KDEVPLATFORMUTIL_EXPORT KSequentialCompoundJob : public KCompoundJob
{
    Q_OBJECT
public:
    /**
     * Creates a new KSequentialCompoundJob object.
     *
     * @param parent the parent QObject
     */
    explicit KSequentialCompoundJob(QObject *parent = nullptr);

    /**
     * Destroys a KSequentialCompoundJob object.
     */
    ~KSequentialCompoundJob() override;

    /**
     * Configures whether this compound job finishes as soon as
     * a subjob finishes with error.
     *
     * By default, unless @p abort is set to @c false, when a subjob
     * finishes with error, a compound job ignores any remaining subjobs
     * and finishes with the subjob's error.
     */
    void setAbortOnSubjobError(bool abort);

    /**
     * Starts running subjobs beginning with the first subjob in the list
     */
    void start() override;

protected Q_SLOTS:
    /**
     * @warning The default implementations of removeSubjob() and clearSubjobs()
     * are inherited from KCompoundJob. They simply deregister subjobs and never
     * finish the compound job or start the next subjob in the list. They also
     * do not adjust progress data. As a result, if these functions are called,
     * the compound job's progress will likely never reach 100%.
     *
     * @sa addSubjob()
     */

    /**
     * This slot is connected to each subjob's percentChanged() signal.
     *
     * The default implementation calculates total percent value assuming equal
     * time is spent in each subjob and passes the value to KJob::emitPercent().
     */
    virtual void subjobPercentChanged(KJob *job, unsigned long percent);

    void subjobFinished(KJob *job) override;

protected:
    /**
     * Adds a subjob that will run once all preceding subjobs finish
     *
     * If possible, all subjobs should be added before calling start().
     * If a subjob is added after the compound job's percent() becomes
     * greater than zero, the overall progress can jump back.
     *
     * @see KCompoundJob::addSubjob()
     */
    bool addSubjob(KJob *job) override;

    bool doKill() override;

    KSequentialCompoundJob(KSequentialCompoundJobPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(KSequentialCompoundJob)
};

} // namespace KDevCoreAddons

#endif

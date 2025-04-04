/*
    This file was partly taken from KDevelop's cvs plugin
    SPDX-FileCopyrightText: 2002-2003 Christian Loose <christian.loose@hamburg.de>
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>

    Adapted for DVCS
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#ifndef KDEVPLATFORM_DVCS_JOB_H
#define KDEVPLATFORM_DVCS_JOB_H

#include <vcs/vcsexport.h>
#include <vcs/vcsjob.h>

#include <KProcess>

#include <QStringList>
#include <QVariant>

class QDir;

namespace KDevelop
{
class DVcsJobPrivate;

/**
 * This class is capable of running our dvcs commands. 
 * Most of all DVcsJob are created in DVCS executors, but executed in DistributedVersionControlPlugin or
 * any managers like BranchManager.
 * @note Connect to KJob::result(KJob*) to be notified when the job finished.
 * 
 * How to create DVcsJob:
 * @code
 * DVcsJob* job = new DVcsJob(vcsplugin);
 * 
 * job->setDirectory(workDir);
 * *job << "git-rev-parse";
 * foreach(const QString &arg, args) // *job << args can be used instead!
 *     *job << arg;
 * return job;
 * 
 * return error_cmd(i18n("could not create the job"));
 * @endcode
 * 
 * Usage example 1:
 * @code
 * VcsJob* j = add(QList<QUrl>() << a << b << c, IBasicVersionControl::Recursive);
 * DVcsJob* job = qobject_cast<DVCSjob*>(j);
 * connect(job, SIGNAL(result(KJob*)),
 *         this, SIGNAL(jobFinished(KJob*)));
 * ICore::self()->runController()->registerJob(job);
 * @endcode
 * 
 * Usage example 2, asynchronous:
 * @code
 * DVcsJob* branchJob = d->branch(repo, baseBranch, newBranch);
 * 
 * if (job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded)
 *     return true;
 * else
 *     //something, maybe even just
 *     return false
 * @endcode
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class KDEVPLATFORMVCS_EXPORT DVcsJob : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    explicit DVcsJob(const QDir& workingDir, KDevelop::IPlugin* parent=nullptr, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    ~DVcsJob() override;

    /**
     * Returns current working directory.
     */
    QDir directory() const;

    /**
     * Call this method to set command to execute and its arguments.
     * @note Don't forget <<"one two"; is not the same as <<"one"<<"two"; Use one word(command, arg) per one QString!
     */
    DVcsJob& operator<<(const QString& arg);

    /**
     * Overloaded convenience function.
     * @param arg command or argument as utf8-encoded string
     * @see operator<<(const QString& arg).
     */
    DVcsJob& operator<<(const char* arg);

    /**
     * Overloaded convenience function.
     * @see operator<<(const QString& arg).
     */
    DVcsJob& operator<<(const QStringList& args);
    
    /**
     * Overloaded operator << for url's, can be used to pass files and
     * makes arguments absolute to the process working directory
     * 
     * Override if you need to treat paths before adding them as parameters.
     */
    virtual DVcsJob& operator<<(const QUrl& arg);
    
    /**
     * @see operator<<(const QUrl& arg).
     */
    DVcsJob& operator<<(const QList<QUrl>& args);

    /**
     * Call this method to start this job.
     * @note Default communication mode is KProcess::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    void start() override;

    /**
     * In some cases it's needed to specify the communication mode between the
     * process and the job object. This is for instance done for the "git status"
     * command. If stdout and stderr are processed as separate streams, their signals
     * do not always get emitted in correct order by KProcess, which will lead to a
     * screwed up output.
     * @note Default communication mode is KProcess::SeparateChannels.
     */
    void setCommunicationMode(KProcess::OutputChannelMode comm);

    /**
     * @return The command that is executed when calling start().
     */
    QStringList dvcsCommand() const;

    /**
     * @return The whole output of the job as a string. (Might fail on binary data)
     */
    QString output() const;

    /**
     * @return The whole binary output of the job
     */
    QByteArray rawOutput() const;

    /**
     * @return The whole binary stderr output of the job.
     */
    QByteArray errorOutput() const;

    /**
     * Ignore a non-zero exit code depending on @p ignore.
     */
    void setIgnoreError(bool ignore);

    // Begin:  KDevelop::VcsJob

    /** 
     * Sets executions results.
     * In most cases this method is used by IDVCSexecutor
     * @see fetchResults()
     */
    virtual void setResults(const QVariant &res);

    /**
     * Returns execution results stored in QVariant.
     * Mostly used in vcscommitdialog.
     * @see setResults(const QVariant &res)
     */
    QVariant fetchResults() override;

    /**
     * Returns JobStatus
     * @see KDevelop::VcsJob::JobStatus
     */
    KDevelop::VcsJob::JobStatus status() const override;

    /**
     * Returns pointer to IPlugin (which was used to create a job).
     */
    KDevelop::IPlugin* vcsPlugin() const override;
    // End:  KDevelop::VcsJob
    
    KProcess* process() const;
    
    void displayOutput(const QString& output);

public Q_SLOTS:
    /**
     * Cancel slot.
     */
    void cancel();

Q_SIGNALS:
    void readyForParsing(KDevelop::DVcsJob *job);

protected Q_SLOTS:
    virtual void slotProcessError( QProcess::ProcessError );

private Q_SLOTS:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout();

protected:
    bool doKill() override;

private:
    void jobIsReady();

private:
    const QScopedPointer<class DVcsJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DVcsJob)
};

/**
 * @return a VCS job that outputs a given error message and finishes
 *
 * @todo The returned job finishes without errors, so higher-level code mistakenly believes that
 *       the operation has succeeded. Replace this function with a special-purpose class VcsErrorJob
 *       that inherits VcsJob, outputs a specified error message (or does not output anything,
 *       relies on the RunController to report its error instead) and finishes with an error.
 */
[[nodiscard]] KDEVPLATFORMVCS_EXPORT DVcsJob*
makeVcsErrorJob(const QString& errorMessage, IPlugin* parent,
                OutputJob::OutputJobVerbosity verbosity = OutputJob::Verbose);
}

#endif

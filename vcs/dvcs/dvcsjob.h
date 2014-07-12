/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/


#ifndef KDEVPLATFORM_DVCS_JOB_H
#define KDEVPLATFORM_DVCS_JOB_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <KDE/KProcess>

#include <vcs/vcsexport.h>
#include "../vcsjob.h"

class QDir;
struct DVcsJobPrivate;

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
 * VcsJob* j = add(KUrl::List() << a << b << c, IBasicVersionControl::Recursive);
 * DVcsJob* job = qobject_cast<DVCSjob*>(j);
 * connect(job, SIGNAL(result(KJob*) ),
 *         this, SIGNAL(jobFinished(KJob*) ));
 * ICore::self()->runController()->registerJob(job);
 * @endcode
 * 
 * Usage example 2, asyunchronous:
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

namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT DVcsJob : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    DVcsJob(const QDir& workingDir, KDevelop::IPlugin* parent=0, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    virtual ~DVcsJob();

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
     * Override if you need to treat paths beffore adding them as parameters.
     */
    virtual DVcsJob& operator<<(const KUrl& arg);
    
    /**
     * @see operator<<(const KUrl& arg).
     */
    DVcsJob& operator<<(const QList<KUrl>& args);

    /**
     * Call this method to start this job.
     * @note Default communication mode is KProcess::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    virtual void start();

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
    virtual QVariant fetchResults();

    /**
     * Returns JobStatus
     * @see KDevelop::VcsJob::JobStatus
     */
    virtual KDevelop::VcsJob::JobStatus status() const;

    /**
     * Returns pointer to IPlugin (which was used to create a job).
     */
    virtual KDevelop::IPlugin* vcsPlugin() const;
    // End:  KDevelop::VcsJob
    
    KProcess *process();
    
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
    virtual bool doKill();

private:
    void jobIsReady();
    DVcsJobPrivate* const d;
};

}

#endif

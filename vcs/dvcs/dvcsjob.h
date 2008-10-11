/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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


#ifndef DVCS_JOB_H
#define DVCS_JOB_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <KDE/KProcess>

#include "../vcsexport.h"
#include "../vcsjob.h"

class DVCSjobPrivate;

/**
 * This class is capable of running our dvcs commands. 
 * Most of all DVCSjobs are created in DVCS executors, but executed in DistributedVersionControlPlugin or 
 * any managers like BranchManager.
 * @note Connect to Kjob::result(KJob*) to be notified when the job finished.
 * 
 * How to create DVCSjob:
 * @code
 * DVCSjob* job = new DVCSjob(vcsplugin);
 * if (job)
 * {
 *     job->setDirectory(workDir);
 *     *job << "git-rev-parse";
 *     foreach(const QString &arg, args) // *job << args can be used instead!
 *     *job << arg;
 *     return job;
 * }
 * if (job) delete job;
 * return NULL;
 * @endcode
 * 
 * Usage example 1:
 * @code
 * VcsJob* j = add(DistributedVersionControlPlugin::d->m_ctxUrlList, IBasicVersionControl::Recursive);
 * DVCSjob* job = dynamic_cast<DVCSjob*>(j);
 * if (job) {
 *     connect(job, SIGNAL(result(KJob*) ),
 *             this, SIGNAL(jobFinished(KJob*) ));
 *     job->start();
 * }
 * @endcode
 * 
 * Usage example 2:
 * @code
 * DVCSjob *branchJob = d->branch(repo, baseBranch, newBranch);
 * DVCSjob* job = gitRevParse(dirPath.path(), QStringList(QString("--is-inside-work-tree")));
 * if (job)
 * {
 *     job->exec();
 *     if (job->status() == KDevelop::VcsJob::JobSucceeded)
 *         return true;
 *     else
 *     //something, mabe even just
 *         return false
 * }
 * @endcode
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class KDEVPLATFORMVCS_EXPORT DVCSjob : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    DVCSjob(KDevelop::IPlugin* parent);
    virtual ~DVCSjob();

    /**
     * Call this method to clear the job (for example, before setting another job).
     */
    void clear();

    /**
     * It's not used in any DVCS plugin.
     */
    void setServer(const QString& server);

    /**
     * Sets working directory.
     * @param directory Should contain only absolute path. Relative path or "" (working dir) are depricated and will make job failed.
     * @note In DVCS plugins directory variable is used to get relative pathes.
     */
    void setDirectory(const QString& directory);

    /**
     * Sets standart Input file.
     */
    void setStandardInputFile(const QString &fileName);

    /**
     * Returns current working directory.
     */
    QString getDirectory();

    /**
     * Call this method to set command to execute and its arguments.
     * @note Don't forget <<"one two"; is not the same as <<"one"<<"two"; Use one word(command, arg) per one QString!
     */
    DVCSjob& operator<<(const QString& arg);

    /**
     * Overloaded convinience function.
     * @see operator<<(const QString& arg).
     */
    DVCSjob& operator<<(const char* arg);

    /**
     * Overloaded convinience function.
     * @see operator<<(const QString& arg).
     */
    DVCSjob& operator<<(const QStringList& args);

    /**
     * Call this mehod to start this job.
     * @note Default communiaction mode is KProcess::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    virtual void start();

    /**
     * In some cases it's needed to specify the communisation mode between the
     * process and the job object. This is for instance done for the "git status"
     * command. If stdout and stderr are processed as separate streams their signals
     * do not always get emmited in correct order by KProcess. Which will lead to a
     * screwed up output.
     * @note Default communiaction mode is KProcess::SeparateChannels.
     */
    void setCommunicationMode(KProcess::OutputChannelMode comm);

    /**
     * @return The command that is executed when calling start().
     */
    QString dvcsCommand() const;

    /**
     * @return The whole output of the job.
     */
    QString output() const;

    // Begin:  KDevelop::VcsJob

    /** 
     * Sets executions reults.
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
     * Sets exit status (d->failed variable).
     * Since only executors can parse the job to set result, they can connect parsers to readyForParsing(DVCSjob) using
     * Qt::DirectConnection to set the result. For example git-status can return exit status 1 
     * if you don't set exit status in your parser then you will have JobFailes in status() result.
     * @note First result is set in slotProcessExited() or slotProcessError().
     */
    virtual void setExitStatus(const bool exitStatus);

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
    
    KProcess *getChildproc();

public Q_SLOTS:
    /**
     * Cancel slot.
     */
    void cancel();

    /**
     * Returns if the job is running.
     */
    bool isRunning() const;

signals:
    void readyForParsing(DVCSjob *job);

private Q_SLOTS:
    void slotProcessError( QProcess::ProcessError );
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout(const QStringList&);
    void slotReceivedStderr(const QStringList&);

private:
    void jobIsReady();
    DVCSjobPrivate* const d;
    QVariant results;
};

#endif

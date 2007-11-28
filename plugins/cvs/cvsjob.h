/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSJOB_H
#define CVSJOB_H


#include <QStringList>
#include <KProcess>

#include "vcsjob.h"

/**
 * This class is capable of running our cvs commands
 * Connect to Kjob::result(KJob*) to be notified when the job finished.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsJob : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    CvsJob(KDevelop::IPlugin* parent);
    virtual ~CvsJob();

    void clear();
    void setRSH(const QString& rsh);
    void setServer(const QString& server);
    void setDirectory(const QString& directory);

    CvsJob& operator<<(const QString& arg);
    CvsJob& operator<<(const char* arg);
    CvsJob& operator<<(const QStringList& args);

    /**
     * Call this mehod to start this job.
     * @note Default communiaction mode is KProcess::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    virtual void start();

    /**
     * In some cases it's needed to specify the communisation mode between the
     * process and the job object. This is for instance done for the "cvs status"
     * command. If stdout and stderr are processed as separate streams their signals
     * do not always get emmited in correct order by KProcess. Which will lead to a
     * screwed up output.
     * @note Default communiaction mode is KProcess::SeparateChannels.
     */
    void setCommunicationMode(KProcess::OutputChannelMode comm);

    /**
     * @return The command that is executed when calling start()
     */
    QString cvsCommand() const;

    /**
     * @return The whole output of the job
     */
    QString output() const;

    // Begin:  KDevelop::VcsJob
    virtual QVariant fetchResults();
    virtual KDevelop::VcsJob::JobStatus status() const;
    virtual KDevelop::IPlugin* vcsPlugin() const;
    // End:  KDevelop::VcsJob

public slots:
    void cancel();
    bool isRunning() const;

private slots:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout(const QStringList&);
    void slotReceivedStderr(const QStringList&);

private:
    struct Private;
    Private* d;
};

#endif

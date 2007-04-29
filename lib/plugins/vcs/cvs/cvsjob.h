/***************************************************************************
 *   This file was partly taken from cervisia's cvsservice                 *
 *   Copyright (C) 2002-2003 Christian Loose <christian.loose@hamburg.de>  *
 *                                                                         *
 *   Adapted for KDevelop                                                  *
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
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
#include <KJob>
#include <K3Process>

/**
 * This class is capable of running our cvs commands 
 * Connect to Kjob::result(KJob*) to be notified when the job finished.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsJob : public KJob
{
    Q_OBJECT
public:
    CvsJob(QObject* parent);
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
     * @note Default communiaction mode is K3Process::AllOutput.
     * @see Use setCommunicationMode() to override the default communication mode.
     */
    virtual void start();

    /**
     * In some cases it's needed to specify the communisation mode between the 
     * process and the job object. This is for instance done for the "cvs status"
     * command. If stdout and stderr are processed as separate streams their signals
     * do not always get emmited in correct order by K3Process. Which will lead to a 
     * screwed up ouput.
     * @note Default communiaction mode is K3Process::AllOutput.
     */
    void setCommunicationMode(K3Process::Communication comm);

    /**
     * @return The command that is executed when calling start()
     */
    QString cvsCommand() const;

    /**
     * @return The whole output of the job
     */
    QString output() const;

public slots:
    void cancel();
    bool isRunning() const;

private slots:
    void slotProcessExited(K3Process* proc);
    void slotReceivedStdout(K3Process* proc, char* buffer, int buflen);
    void slotReceivedStderr(K3Process* proc, char* buffer, int buflen);

private:
    struct Private;
    Private* d;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

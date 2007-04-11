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

#include <QString>
#include <QStringList>
#include <KJob>

class K3Process;

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
     */
    virtual void start();

    /**
     * @return The command that is executed when calling start()
     */
    QString cvsCommand() const;

    /**
     * @return The whole output of the job
     */
    QStringList output() const;

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

/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2002-2003 Christian Loose <christian.loose@hamburg.de>      *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
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


#ifndef GITJOB_H
#define GITJOB_H


#include <QStringList>
#include <KProcess>

#include "vcsjob.h"

/**
 * This class is capable of running our git commands
 * Connect to Kjob::result(KJob*) to be notified when the job finished.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class GitJob : public KDevelop::VcsJob
{
    Q_OBJECT
    public:
        GitJob(KDevelop::IPlugin* parent);
        virtual ~GitJob();

        void clear();
        void setServer(const QString& server);
        void setDirectory(const QString& directory);

        QString getDirectory();

        GitJob& operator<<(const QString& arg);
        GitJob& operator<<(const char* arg);
        GitJob& operator<<(const QStringList& args);

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
         * @return The command that is executed when calling start()
     */
        QString gitCommand() const;

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
        void slotProcessError( QProcess::ProcessError );
        void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
        void slotReceivedStdout(const QStringList&);
        void slotReceivedStderr(const QStringList&);

    private:
        struct Private;
        Private* d;
};

#endif

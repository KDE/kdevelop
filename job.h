/* This file is part of KDevelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef JOB_H
#define JOB_H

#include <QProcess>
#include <QTcpSocket>
#include <QFileInfo>
#include <QTimer>
#include <QBuffer>
#include <QUrl>

#include <outputview/outputjob.h>

#include "iparser.h"

class KJob;
class KConfigGroup;
class QXmlInputSource;
class QXmlSimpleReader;
class QTcpServer;
class QTcpSocket;
class QBuffer;

namespace KDevelop
{
class ProcessLineMaker;
class ILaunchConfiguration;
class OutputModel;
class LaunchConfiguration;
}

namespace cppcheck
{
class Parser;

class Job : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    struct Parameters
    {
        QString parameters;
        int viewMode;
        bool checkStyle;
        bool checkPerformance;
        bool checkPortability;
        bool checkInformation;
        bool checkUnusedFunction;
        bool checkMissingInclude;
        QString path;
        QString executable;
    };

    Job(const Parameters &params, QObject* parent = nullptr);
    ~Job() override;
    KDevelop::OutputModel* model();
    void start() override;
    bool doKill() override;

    QVector<KDevelop::IProblem::Ptr> problems() const;

signals:
    void jobFinished();

private slots:

    void readyReadStandardError();
    void readyReadStandardOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

protected:
    typedef QString   t_cppcheck_cfg_argarray[][3];

    void beforeStart(); // called before launching the process
    void processStarted(); // called after the process has been launched
    void processEnded(); // called when the process ended

    QStringList buildCommandLine() const;


protected:

    QProcess* m_process;
    QUrl m_workingDir;
    int m_pid;

    cppcheck::Parser* m_parser;

    KDevelop::ProcessLineMaker* m_applicationOutput;
    KDevelop::ILaunchConfiguration* m_launchcfg;

    // The cppcheck output file
    QFile* m_file;
    bool      m_killed;
    QBuffer* string_device;
    QString stdout_output;
    QString stderr_output;
    Parameters m_parameters;

private:
};


/**
 * It permits to call an process to do a post treatment on the cppcheck output
 */
class KProcessOutputToParser : public QObject
{
    Q_OBJECT
public:
    KProcessOutputToParser(Parser* inst);
    ~KProcessOutputToParser() override;

    bool execute(QString execPath, QStringList args);

private slots:
    void  newDataFromStdOut();
    void  processEnded(int returnCode, QProcess::ExitStatus status);

private:
    QProcess*  m_process;
    QIODevice* m_device;
    Parser*    m_parser;

};
}
#endif

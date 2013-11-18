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
#include <KLocale>
#include <KUrl>
#include <QFileInfo>
#include <QTimer>
#include <QBuffer>

#include <outputview/outputjob.h>

#include "iparser.h"

class KJob;
class KProcess;
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
class Plugin;
class Parser;

class Job : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    Job(cppcheck::Plugin* inst, bool allFiles, QObject* parent = 0);
    ~Job();
    cppcheck::Plugin* plugin() const;
    KDevelop::OutputModel* model();
    void start();
    bool doKill();

signals:
    void updateTabText(cppcheck::Model*, const QString& text);
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

    void addToolArgs(QStringList& args, KConfigGroup& cfg) const;

    void processModeArgs(QStringList& out,
                         const t_cppcheck_cfg_argarray mode_args,
                         int mode_args_count,
                         KConfigGroup& cfg) const;

    QStringList buildCommandLine() const;


protected:

    KProcess* m_process;
    KUrl m_workingDir;
    int m_pid;

    cppcheck::Model* m_model;
    cppcheck::Parser* m_parser;

    KDevelop::ProcessLineMaker* m_applicationOutput;
    KDevelop::ILaunchConfiguration* m_launchcfg;
    cppcheck::Plugin* m_plugin;

    // The cppcheck output file
    QFile* m_file;
    bool      m_killed;
    bool allFiles;
    QBuffer* string_device;
    QString stdout_output;
    QString stderr_output;

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
    ~KProcessOutputToParser();
    int execute(QString execPath, QStringList args);

private slots:
    void  newDataFromStdOut();
    void  processEnded(int returnCode, QProcess::ExitStatus status);

private:
    KProcess*  m_process;
    QIODevice* m_device;
    Parser*    m_parser;

};
}
#endif

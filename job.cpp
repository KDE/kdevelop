/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include <KShell>
#include <KMessageBox>
#include <QApplication>
#include <KLocalizedString>
#include <QRegularExpression>

#include "clangtidyparser.h"
#include "debug.h"
#include "job.h"
#include <QFile>

namespace ClangTidy
{

Job::Job ( const Parameters &params, QObject* parent )
    : KDevelop::OutputExecuteJob ( parent ), m_parameters ( params )
{
    setJobName ( i18n ( "clang-tidy output" ) );
    mustDumpConfig = ! ( params.dump.isEmpty() );

    setCapabilities ( KJob::Killable );
    if ( !mustDumpConfig ) {
        setStandardToolView ( KDevelop::IOutputView::TestView );
        setBehaviours ( KDevelop::IOutputView::AutoScroll );
        setProperties ( KDevelop::OutputExecuteJob::JobProperty::DisplayStdout );
        setProperties ( KDevelop::OutputExecuteJob::JobProperty::DisplayStderr );
        setProperties ( KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput );
    }

    *this << params.executable;

    if(params.overrideConfigFile.isEmpty()) *this << QString ( "--checks=%1" ).arg ( params.checks ); else *this << 
params.overrideConfigFile;
    *this << QString ( "--export-fixes=%1.%2" ).arg ( params.filePath ).arg ( "yaml" );
    *this << QString ( "-p=%1" ).arg ( params.buildDir );
    *this << QString ( "%1" ).arg ( params.filePath );

    if ( !params.headerFilter.isEmpty() ) {
        *this << params.headerFilter;
    }
    if ( !params.additionals.isEmpty() ) {
        *this << params.additionals;
    }
    if ( !params.checkSysHeaders.isEmpty() ) {
        *this << params.checkSysHeaders;
    }
    if ( mustDumpConfig ) {
        *this << params.dump;
    }


    qCDebug ( KDEV_CLANGTIDY ) << "checking path" << params.filePath;
}

Job::~Job()
{
    doKill();
}

void Job::processStdoutLines ( const QStringList& lines )
{
    if ( !mustDumpConfig ) {
        m_standardOutput << lines;
    } else {
        QFile file ( m_parameters.projectRootDir + "/.clang-tidy" );
        file.open ( QIODevice::WriteOnly );
        QTextStream os ( &file );
        os << lines.join ( '\n' );
    }
}

void Job::processStderrLines ( const QStringList& lines )
{
    static const auto xmlStartRegex = QRegularExpression ( "\\s*<" );

    for ( const QString & line : lines ) {
        // unfortunately sometime clangtidy send non-XML messages to stderr.
        // For example, if we pass '-I /missing_include_dir' to the argument list,
        // then stderr output will contains such line (tested on clangtidy 1.72):
        //
        // (information) Couldn't find path given by -I '/missing_include_dir'
        //
        // Therefore we must 'move' such messages to m_standardOutput.

        if ( line.indexOf ( xmlStartRegex ) != -1 ) { // the line contains XML
            m_xmlOutput << line;
        } else {
            m_standardOutput << line;
        }
    }
}

void Job::postProcessStdout ( const QStringList& lines )
{
    processStdoutLines ( lines );

    KDevelop::OutputExecuteJob::postProcessStdout ( lines );
}

void Job::postProcessStderr ( const QStringList& lines )
{
    processStderrLines ( lines );

    KDevelop::OutputExecuteJob::postProcessStderr ( lines );
}

void Job::start()
{
    m_standardOutput.clear();
    m_xmlOutput.clear();

    qCDebug ( KDEV_CLANGTIDY ) << "executing:" << commandLine().join ( ' ' );

    KDevelop::OutputExecuteJob::start();
}

QVector<KDevelop::IProblem::Ptr> Job::problems() const
{
    return m_problems;
}

void Job::childProcessError ( QProcess::ProcessError e )
{
    QString message;

    switch ( e ) {
    case QProcess::FailedToStart:
        message = i18n ( "Failed to start Clangtidy from %1.", commandLine() [0] );
        break;

    case QProcess::Crashed:
        message = i18n ( "Clangtidy crashed." );
        break;

    case QProcess::Timedout:
        message = i18n ( "Clangtidy process timed out." );
        break;

    case QProcess::WriteError:
        message = i18n ( "Write to Clangtidy process failed." );
        break;

    case QProcess::ReadError:
        message = i18n ( "Read from Clangtidy process failed." );
        break;

    case QProcess::UnknownError:
        // current clangtidy errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if ( !message.isEmpty() ) {
        KMessageBox::error ( qApp->activeWindow(), message, i18n ( "Clangtidy Error" ) );
    }

    KDevelop::OutputExecuteJob::childProcessError ( e );
}

void Job::childProcessExited ( int exitCode, QProcess::ExitStatus exitStatus )
{
    qCDebug ( KDEV_CLANGTIDY ) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    if ( exitCode != 0 ) {
        qCDebug ( KDEV_CLANGTIDY ) << "clangtidy failed, standard output: ";
        qCDebug ( KDEV_CLANGTIDY ) << m_standardOutput.join ( '\n' );
        qCDebug ( KDEV_CLANGTIDY ) << "clangtidy failed, XML output: ";
        qCDebug ( KDEV_CLANGTIDY ) << m_xmlOutput.join ( '\n' );
    } else {
        ClangtidyParser parser;
        parser.addData ( m_xmlOutput.join ( '\n' ) );
        parser.parse();
        m_problems = parser.problems();
    }

    KDevelop::OutputExecuteJob::childProcessExited ( exitCode, exitStatus );
}

}

/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2008 by Hamish Rodda                                        *
*   rodda@kde.org                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepjob.h"
#include "grepoutputmodel.h"

#include <QWhatsThis>
#include <QList>
#include <QRegExp>
#include <QKeySequence>

#include <kprocess.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <ioutputview.h>
#include <iplugincontroller.h>
#include <iuicontroller.h>
#include <kparts/mainwindow.h>
#include <iproject.h>
#include <projectmodel.h>
#include <processlinemaker.h>

#include "grepviewplugin.h"
#include "grepoutputdelegate.h"

using namespace KDevelop;

GrepJob::GrepJob( GrepViewPlugin* parent )
    : OutputJob( parent )
{
    setCapabilities(Killable);
}

void GrepJob::start()
{
    KProcess *catProc=0, *findProc=0, *grepProc=0, *sedProc=0, *xargsProc=0;
    QList<KProcess*> validProcs;

    // waba: code below breaks on filenames containing a ',' !!!
//     QStringList filelist = QString::split(",", filesString);
    QStringList filelist = filesString.split(",");
    KTemporaryFile* tempFile = 0;
    if (useProjectFilesFlag)
    {
        if (project)
        {
            tempFile = new KTemporaryFile();
            tempFile->setSuffix(".grep.tmp");
            //This is ok, the tempfile is deleted when the last process
            //finished running, see the end of this function
            tempFile->setAutoRemove( false );

            QList<ProjectFileItem*> fileItems = project->files();
            KUrl::List projectFiles;
            foreach( ProjectFileItem *_item, fileItems )
            {
                projectFiles << _item->url();
            }
            if (!projectFiles.isEmpty())
            {
                QList<QRegExp> regExpList;

                if (!filelist.isEmpty())
                {
                    for (QStringList::Iterator it = filelist.begin(); it != filelist.end(); ++it)
                        regExpList.append(QRegExp(*it, Qt::CaseSensitive, QRegExp::Wildcard));
                }


                if (tempFile->open())
                {
                    QTextStream out(tempFile);
                    for (QList<KUrl>::Iterator it = projectFiles.begin(); it != projectFiles.end(); ++it)
                    {
                        // parent directory check.
                        if( recursiveFlag && ! directory.isParentOf( *it ) ) continue;
                        if( !recursiveFlag &&
                            (*it).upUrl().equals(directory, KUrl::CompareWithoutTrailingSlash))
                            continue;

                        // filelist check
                        bool matchOne = regExpList.count() == 0;
                        for (QList<QRegExp>::Iterator it2 = regExpList.begin(); it2 != regExpList.end() && !matchOne; ++it2)
                            matchOne = (*it2).exactMatch( (*it).toLocalFile() );

                        // in case of files containing space, convert to \ so that
                        // xargs can correctly treat that.
                        if (matchOne)
                            out << (*it).toLocalFile().replace(' ', "\\ ") << endl;
                    }

//                     tempFile.close();
                }
                else
                {
                    setError(TemporaryFileError);
                    setErrorText(i18n("Unable to create a temporary file for search."));
                    delete tempFile;
                    return emitResult();
                }
            }

            QStringList catCmd;
            catCmd << tempFile->fileName().replace(' ', "\\ ");
            catProc = new KProcess(this);
            catProc->setProgram( "cat", catCmd );
            catProc->setOutputChannelMode( KProcess::SeparateChannels );
            validProcs << catProc;
        }
    }
    else
    {
        QStringList files;
        if (!filelist.isEmpty())
        {
            QStringList::Iterator it(filelist.begin());
            files << *it;
            ++it;
            for(; it!=filelist.end(); ++it )
                files << "-o" << "-name" << *it;
        }
        QStringList findCmd;
        findCmd << directory.path();
        if (!recursiveFlag)
            findCmd << "-maxdepth" << "1";

        findCmd << "-name" << files;
        findCmd << "-follow";

        kDebug(9001) << "findCmd :" << findCmd;
        findProc = new KProcess(this);
        findProc->setProgram( "find", findCmd );
        findProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << findProc;
    }

    QStringList grepCmd;
    QString excludestring = excludeString;
    QStringList excludelist = excludestring.split(",", QString::SkipEmptyParts);
    if (!excludelist.isEmpty())
    {
        QStringList::Iterator it(excludelist.begin());
        grepCmd << "-v";
        for (; it != excludelist.end(); ++it)
            grepCmd << "-e" << *it;

        grepProc = new KProcess(this);
        grepProc->setProgram( "grep", grepCmd );
        grepProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << grepProc;
    }

    QStringList sedCmd;
    if (!useProjectFilesFlag)
    {
        // quote spaces in filenames going to xargs
        sedCmd << "s/ /\\\\\\ /g";
        sedProc = new KProcess(this);
        sedProc->setProgram( "sed", sedCmd );
        sedProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << sedProc;
    }

    QStringList xargsCmd;
#ifndef USE_SOLARIS
    xargsCmd << "egrep" << "-H" << "-n" << "-s";
#else
    // -H reported as not being available on Solaris,
    // but we're buggy without it on Linux.
    xargsCmd << "egrep" << "-n";
#endif

    if (!caseSensitiveFlag)
    {
        xargsCmd << "-i";
    }
    xargsCmd << "-e";

    QString pattern = templateString;
    if (regexpFlag)
        pattern.replace(QRegExp("%s"), patternString);
    else
        pattern.replace(QRegExp("%s"), escape( patternString ) );
//     command += KShellProcess::quote(pattern);
//     xargsCmd += quote(pattern); // quote isn't needed now.
    xargsCmd << pattern;

    xargsProc = new KProcess(this);
    xargsProc->setProgram( "xargs", xargsCmd );
    xargsProc->setOutputChannelMode( KProcess::SeparateChannels );
    validProcs << xargsProc;

    if( validProcs.count() > 1 )
        validProcs[0]->setStandardOutputProcess( validProcs[1] );
    if( validProcs.count() > 2 )
        validProcs[1]->setStandardOutputProcess( validProcs[2] );
    if( validProcs.count() > 3 )
        validProcs[2]->setStandardOutputProcess( validProcs[3] );

    setToolTitle(i18n("Find in Files"));
    setToolIcon(KIcon("edit-find"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setTitle(patternString);
    setBehaviours( KDevelop::IOutputView::AutoScroll | KDevelop::IOutputView::AllowUserClose );

    setModel(new GrepOutputModel(plugin()), KDevelop::IOutputView::TakeOwnership);
    setDelegate(plugin()->delegate());
    startOutput();
    
    m_lineMaker = new KDevelop::ProcessLineMaker( xargsProc );

    // Delete the tempfile when xargs process is destroyed
    if( tempFile )
        connect( xargsProc, SIGNAL(destroyed(QObject*)), tempFile, SLOT(deleteLater()) );

    connect( m_lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
             model(), SLOT(appendOutputs(const QStringList&)) );
    connect( m_lineMaker, SIGNAL(receivedStderrLines( const QStringList& )),
             model(), SLOT(appendErrors(const QStringList&)) );
    connect( xargsProc, SIGNAL(finished(int, QProcess::ExitStatus)),
             this, SLOT(slotFinished()) );
    connect( xargsProc, SIGNAL(error( QProcess::ProcessError )),
             this, SLOT(slotError(QProcess::ProcessError)) );

    // At first line, print out actual command invocation as if it was run via shell.
    QString printCmd;
    foreach( KProcess *_proc, validProcs )
    {
        printCmd += _proc->program().join(" ") + " | ";
    }
    printCmd.chop(3); // chop last '|'
    model()->appendRow( new QStandardItem(printCmd) );

    // start processes
    foreach( KProcess *_proc, validProcs )
    {
        _proc->start();
    }
}

QString GrepJob::escape(const QString &str)
{
    QString escaped("[]{}()\\^$?.+-*|");
    QString res;

    for (int i=0; i < str.length(); ++i)
    {
        if (escaped.indexOf(str[i]) != -1)
            res += "\\";
        res += str[i];
    }

    return res;
}


void GrepJob::slotFinished()
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    
    emitResult();
}

void GrepJob::slotError(QProcess::ProcessError error)
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();

    foreach(KProcess* proc, m_processes)
    {
        proc->kill();
    }
    
    setError(UserDefinedError);
    // TODO more informative error codes
    switch (error) {
        case QProcess::FailedToStart:
            setErrorText(i18n("Process failed to start."));
            break;
        case QProcess::Crashed:
            setErrorText(i18n("Process crashed."));
            break;
        case QProcess::Timedout:
            setErrorText(i18n("Process timed out."));
            break;
        case QProcess::WriteError:
            setErrorText(i18n("Error while writing to process."));
            break;
        case QProcess::ReadError:
            setErrorText(i18n("Error while reading from process."));
            break;
        case QProcess::UnknownError:
            setErrorText(i18n("Unknown process error."));
            break;
    }
    
    emitResult();
}

GrepViewPlugin* GrepJob::plugin() const
{
    return const_cast<GrepViewPlugin*>(static_cast<const GrepViewPlugin*>(parent()));
}

GrepOutputModel* GrepJob::model() const
{
    return static_cast<GrepOutputModel*>(OutputJob::model());
}

bool GrepJob::doKill()
{
    foreach (KProcess* p, m_processes)
        p->close();
    
    return true;
}

#include "grepjob.moc"

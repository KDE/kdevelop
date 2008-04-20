/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepviewplugin.h"
#include "grepdialog.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"

#include <QWhatsThis>
#include <QList>
#include <QSignalMapper>
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
#include <idocument.h>
#include <idocumentcontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <projectmodel.h>
#include <processlinemaker.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(GrepViewFactory, registerPlugin<GrepViewPlugin>(); )
K_EXPORT_PLUGIN(GrepViewFactory("kdevgrepview"))

GrepViewPlugin::GrepViewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( GrepViewFactory::componentData(), parent )
    , m_projectForActiveFile(0), m_view(0)
{
    setXMLFile("kdevgrepview.rc");

//     connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//              this, SLOT(stopButtonClicked(KDevPlugin*)) );

    finishedmapper = new QSignalMapper(this);
    connect(finishedmapper, SIGNAL(mapped(int)), this, SLOT(procFinished(int)));
    failedmapper = new QSignalMapper(this);
    connect(failedmapper, SIGNAL(mapped(int)), this, SLOT(procFailed(int)));

    m_grepdlg = new GrepDialog( this );
    connect( m_grepdlg, SIGNAL(search()), this, SLOT(searchActivated()) );


    KAction *action = actionCollection()->addAction("edit_grep");
    action->setText(i18n("Find in Fi&les..."));
    action->setShortcut( QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F) );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotGrep()));
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("<b>Find in files</b><p>"
            "Opens the 'Find in files' dialog. There you "
            "can enter a regular expression which is then "
            "searched for within all files in the directories "
            "you specify. Matches will be displayed, you "
            "can switch to a match directly.</p>") );

    KDevelop::IPlugin* plugin =
            core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    Q_ASSERT( plugin );

    m_view = plugin->extension<KDevelop::IOutputView>();
    Q_ASSERT( m_view );
    connect( plugin, SIGNAL(outputRemoved(int, int)), this, SLOT(cleanupForView(int, int)));

}

GrepViewPlugin::~GrepViewPlugin()
{
    delete m_grepdlg;
}

void GrepViewPlugin::slotGrep()
{
    QString contextString = currentSelectedWord();
    if( contextString.isEmpty() )
        contextString = currentWord();
    showDialogWithPattern( contextString );

}

void GrepViewPlugin::showDialogWithPattern(const QString& p)
{
    // Before anything, this removes line feeds from the
    // beginning and the end.
    QString pattern = p;
    int len = pattern.length();
    if (len > 0 && pattern[0] == '\n')
    {
        pattern.remove(0, 1);
        len--;
    }
    if (len > 0 && pattern[len-1] == '\n')
        pattern.truncate(len-1);
    m_grepdlg->setPattern( pattern );

    m_grepdlg->enableButtonOk( !pattern.isEmpty() );

    KUrl currentUrl;
    KDevelop::IDocument *document = core()->documentController()->activeDocument();
    if( document )
    {
        currentUrl = document->url();
    }
    if( currentUrl.isValid() )
    {
        KDevelop::IProject *proj =
                core()->projectController()->findProjectForUrl( currentUrl );
        if( proj )
        {
            m_grepdlg->setEnableProjectBox(! proj->files().isEmpty() );
            m_projectForActiveFile = proj;
        }
        else
        {
            m_grepdlg->setEnableProjectBox(false);
        }
    }

    m_grepdlg->show();
}

void GrepViewPlugin::searchActivated()
{
//     m_grepdlg->hide();

    KProcess *catProc=0, *findProc=0, *grepProc=0, *sedProc=0, *xargsProc=0;
    QList<KProcess*> validProcs;

    // waba: code below breaks on filenames containing a ',' !!!
//     QStringList filelist = QString::split(",", m_grepdlg->filesString());
    QStringList filelist = m_grepdlg->filesString().split(",");
    KTemporaryFile* tempFile = 0;
    if (m_grepdlg->useProjectFilesFlag())
    {
        if (m_projectForActiveFile)
        {
            tempFile = new KTemporaryFile();
            tempFile->setSuffix(".grep.tmp");
            //This is ok, the tempfile is deleted when the last process
            //finished running, see the end of this function
            tempFile->setAutoRemove( false );

            QList<ProjectFileItem*> fileItems = m_projectForActiveFile->files();
            KUrl::List projectFiles;
            foreach( ProjectFileItem *_item, fileItems )
            {
                projectFiles << _item->url();
            }
            if (!projectFiles.isEmpty())
            {
                KUrl dir = m_grepdlg->directory();
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
                        if( m_grepdlg->recursiveFlag() && ! dir.isParentOf( *it ) ) continue;
                        if( !m_grepdlg->recursiveFlag() &&
                            (*it).upUrl().equals(dir, KUrl::CompareWithoutTrailingSlash))
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
                    KMessageBox::error(0, i18n("Unable to create a temporary file for search."));
                    delete tempFile;
                    return;
                }
            }

            QStringList catCmd;
            catCmd << tempFile->fileName().replace(' ', "\\ ");
            catProc = new KProcess(this);
            connect( catProc, SIGNAL(error(QProcess::ProcessError)), catProc, SLOT(deleteLater()));
            connect( catProc, SIGNAL(finished(int, QProcess::ExitStatus)), catProc, SLOT(deleteLater()));
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
        findCmd << m_grepdlg->directory().path();
        if (!m_grepdlg->recursiveFlag())
            findCmd << "-maxdepth" << "1";

        findCmd << "-name" << files;
        findCmd << "-follow";

        kDebug(9001) << "findCmd :" << findCmd;
        findProc = new KProcess(this);

        connect( findProc, SIGNAL(error(QProcess::ProcessError)), findProc, SLOT(deleteLater()));
        connect( findProc, SIGNAL(finished(int, QProcess::ExitStatus)), findProc, SLOT(deleteLater()));
        findProc->setProgram( "find", findCmd );
        findProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << findProc;
    }

    QStringList grepCmd;
    QString excludestring = m_grepdlg->excludeString();
    QStringList excludelist = excludestring.split(",", QString::SkipEmptyParts);
    if (!excludelist.isEmpty())
    {
        QStringList::Iterator it(excludelist.begin());
        grepCmd << "-v";
        for (; it != excludelist.end(); ++it)
            grepCmd << "-e" << *it;

        grepProc = new KProcess(this);
        connect( grepProc, SIGNAL(error(QProcess::ProcessError)), grepProc, SLOT(deleteLater()));
        connect( grepProc, SIGNAL(finished(int, QProcess::ExitStatus)), grepProc, SLOT(deleteLater()));
        grepProc->setProgram( "grep", grepCmd );
        grepProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << grepProc;
    }

    QStringList sedCmd;
    if (!m_grepdlg->useProjectFilesFlag())
    {
        // quote spaces in filenames going to xargs
        sedCmd << "s/ /\\\\\\ /g";
        sedProc = new KProcess(this);
        connect( sedProc, SIGNAL(error(QProcess::ProcessError)), sedProc, SLOT(deleteLater()));
        connect( sedProc, SIGNAL(finished(int, QProcess::ExitStatus)), sedProc, SLOT(deleteLater()));
        sedProc->setProgram( "sed", sedCmd );
        sedProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << sedProc;
    }

    QStringList xargsCmd;
#ifndef USE_SOLARIS
//     xargsCmd += " egrep -H -n -s ";
    xargsCmd << "egrep" << "-H" << "-n" << "-s";
#else
    // -H reported as not being available on Solaris,
    // but we're buggy without it on Linux.
//     xargsCmd += " egrep -n ";
    xargsCmd << "egrep" << "-n";
#endif

    if (!m_grepdlg->caseSensitiveFlag())
    {
        xargsCmd << "-i";
    }
    xargsCmd << "-e";

    m_lastPattern = m_grepdlg->patternString();
    QString pattern = m_grepdlg->templateString();
    if (m_grepdlg->regexpFlag())
        pattern.replace(QRegExp("%s"), m_grepdlg->patternString());
    else
        pattern.replace(QRegExp("%s"), escape( m_grepdlg->patternString() ) );
//     command += KShellProcess::quote(pattern);
//     xargsCmd += quote(pattern); // quote isn't needed now.
    xargsCmd << pattern;

    xargsProc = new KProcess(this);

    connect( xargsProc, SIGNAL(error(QProcess::ProcessError)), xargsProc, SLOT(deleteLater()));
    connect( xargsProc, SIGNAL(finished(int, QProcess::ExitStatus)), xargsProc, SLOT(deleteLater()));
    xargsProc->setProgram( "xargs", xargsCmd );
    xargsProc->setOutputChannelMode( KProcess::SeparateChannels );
    validProcs << xargsProc;

    if( validProcs.count() > 1 )
        validProcs[0]->setStandardOutputProcess( validProcs[1] );
    if( validProcs.count() > 2 )
        validProcs[1]->setStandardOutputProcess( validProcs[2] );
    if( validProcs.count() > 3 )
        validProcs[2]->setStandardOutputProcess( validProcs[3] );

    int toolviewid = m_view->registerToolView( i18n("Find in Files"), KDevelop::IOutputView::HistoryView );
    int id = m_view->registerOutputInToolView( toolviewid, m_grepdlg->patternString(), ( KDevelop::IOutputView::AutoScroll | KDevelop::IOutputView::AllowUserClose ) );

    models[id] = new GrepOutputModel(this);
    delegates[id] = new GrepOutputDelegate(this);
    lineMakers[id] = new KDevelop::ProcessLineMaker( xargsProc );

    // Delete the tempfile when xargs process is destroyed
    if( tempFile )
        connect( xargsProc, SIGNAL(destroyed(QObject*)), tempFile, SLOT(deleteLater()) );

    connect( lineMakers[id], SIGNAL(receivedStdoutLines( const QStringList& ) ),
             models[id], SLOT(appendOutputs(const QStringList&)) );
    connect( lineMakers[id], SIGNAL(receivedStderrLines( const QStringList& )),
             models[id], SLOT(appendErrors(const QStringList&)) );
    connect( xargsProc, SIGNAL(finished(int, QProcess::ExitStatus)),
             finishedmapper, SLOT(map()) );
    connect( xargsProc, SIGNAL(error( QProcess::ProcessError )),
             failedmapper, SLOT(map()) );

    m_view->setModel( id, models[id] );
    m_view->setDelegate( id, delegates[id] );

    // At first line, print out actual command invocation as if it was run via shell.
    QString printCmd;
    foreach( KProcess *_proc, validProcs )
    {
        printCmd += _proc->program().join(" ") + " | ";
    }
    printCmd.chop(3); // chop last '|'
    models[id]->appendRow( new QStandardItem(printCmd) );

    // start process. GrepviewProcess is self-deleted.
    foreach( KProcess *_proc, validProcs )
    {
        _proc->start();
    }
}

QString GrepViewPlugin::escape(const QString &str)
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


// from kdeveditorutil.cpp in kdev3.4, plus porting
QString GrepViewPlugin::currentWord()
{
    KDevelop::IDocument *doc =
             core()->documentController()->activeDocument();

    if( !doc ) return QString();

    KTextEditor::Document* ktedoc = doc->textDocument();

    if( !ktedoc ) return QString();

    KTextEditor::View *view = ktedoc->activeView();
    if( !view ) return QString();

    KTextEditor::Cursor cursor = view->cursorPositionVirtual();
    int line = cursor.line();
    int col  = cursor.column();
    QString linestr = ktedoc->line(line);

    int startPos = qMax( qMin( col, linestr.length()-1 ), 0 );
    int endPos = startPos;
    while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' || linestr[startPos] == '~') )
        startPos--;
    while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
        endPos++;

    return ( ( startPos == endPos ) ? QString() : linestr.mid( startPos+1, endPos-startPos-1 ) );
}

QString GrepViewPlugin::currentSelectedWord()
{
    KDevelop::IDocument *doc =
             core()->documentController()->activeDocument();

    if( !doc ) return QString();

    KTextEditor::Document* ktedoc = doc->textDocument();

    if( !ktedoc ) return QString();

    KTextEditor::View *view = ktedoc->activeView();
    if( !view ) return QString();
    return view->selectionText();
}

void GrepViewPlugin::procFinished( int id )
{
    if( models.contains( id ) )
    {
        lineMakers[id]->flushBuffers();
        models[id]->slotCompleted();
    }
}

void GrepViewPlugin::procFailed( int id )
{
    if( models.contains( id ) )
    {
        lineMakers[id]->flushBuffers();
        models[id]->slotCompleted();
        foreach(KProcess* proc, processes[id])
        {
            proc->kill();
            proc->deleteLater();
        }
    }
}

void GrepViewPlugin::cleanupForView(int, int id )
{
    if( models.contains(id) )
    {
        delegates[id]->deleteLater();
        models[id]->deleteLater();
        lineMakers[id]->deleteLater();
        foreach(KProcess* proc, processes[id])
        {
            proc->kill();
            proc->deleteLater();
        }
    }
}
#include "grepviewplugin.moc"

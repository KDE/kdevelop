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

#include "grepviewpart.h"
#include "grepdlg.h"
#include "grepoutputmodel.h"
#include "grepviewprocess.h"

#include <qwhatsthis.h>
#include <QtDesigner/QExtensionFactory>
#include <QList>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QKeySequence>

#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kmessagebox.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>
#include <kparts/partmanager.h>

#include <icore.h>
#include <ioutputview.h>
#include <iplugincontroller.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <projectmodel.h>
#include <commandexecutor.h>
#include <processlinemaker.h>

using namespace KDevelop;

typedef KGenericFactory<GrepViewPart> GrepViewFactory;
K_EXPORT_COMPONENT_FACTORY( kdevgrepview,
                            GrepViewFactory( "kdevgrepview" )  )

GrepViewPart::GrepViewPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin( GrepViewFactory::componentData(), parent )
    , m_view(0), m_projectForActiveFile(0)
{
    setXMLFile("kdevgrepview.rc");

//     connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//              this, SLOT(stopButtonClicked(KDevPlugin*)) );

    m_grepdlg = new GrepDialog( this, 0 );
    connect( m_grepdlg, SIGNAL(searchClicked()), this, SLOT(searchActivated()) );

//     m_widget = new GrepViewWidget(this);
//     m_widget->setIcon(SmallIcon("grep"));
//     m_widget->setCaption(i18n("Grep Output"));
//     QWhatsThis::add(m_widget, i18n("<b>Find in files</b><p>"
//                                 "This window contains the output of a grep "
//                                 "command. Clicking on an item in the list "
//                                 "will automatically open the corresponding "
//                                 "source file and set the cursor to the line "
//                                 "with the match."));

//     mainWindow()->embedOutputView(m_widget, i18n("Find in Files"), i18n("Output of the grep command"));

    QAction *action;

    action = actionCollection()->addAction("edit_grep");
    action->setText(i18n("Find in Fi&les..."));
    action->setShortcut( QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F) );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotGrep()));
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("<b>Find in files</b><p>"
            "Opens the 'Find in files' dialog. There you "
            "can enter a regular expression which is then "
            "searched for within all files in the directories "
            "you specify. Matches will be displayed, you "
            "can switch to a match directly.") );

    KDevelop::IPlugin* plugin =
            core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    Q_ASSERT( plugin );

    m_view = plugin->extension<KDevelop::IOutputView>();
    Q_ASSERT( m_view );

}


GrepViewPart::~GrepViewPart()
{
    delete m_grepdlg;
}

void GrepViewPart::slotGrep()
{
    QString contextString = currentSelectedWord();
    if( contextString.isEmpty() )
        contextString = currentWord();
    showDialogWithPattern( contextString );

}

void GrepViewPart::showDialogWithPattern(QString pattern)
{
    // Before anything, this removes line feeds from the
    // beginning and the end.
    int len = pattern.length();
    if (len > 0 && pattern[0] == '\n')
    {
        pattern.remove(0, 1);
        len--;
    }
    if (len > 0 && pattern[len-1] == '\n')
        pattern.truncate(len-1);
    m_grepdlg->setPattern( pattern );

    // Determine if we have a list of project files
    // dukju ahn: is this feature really useful?
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

void GrepViewPart::searchActivated()
{
    GrepviewProcess *catProc=0, *findProc=0, *grepProc=0, *sedProc=0, *xargsProc=0;
    QList<GrepviewProcess*> validProcs;

    // waba: code below breaks on filenames containing a ',' !!!
//     QStringList filelist = QString::split(",", m_grepdlg->filesString());
    QStringList filelist = m_grepdlg->filesString().split(",");

    if (m_grepdlg->useProjectFilesFlag())
    {
        if (m_projectForActiveFile)
        {
            KUrl tmpFilePath;
            QList<ProjectFileItem*> fileItems = m_projectForActiveFile->files();
            KUrl::List projectFiles;
            foreach( ProjectFileItem *_item, fileItems )
            {
                projectFiles << _item->url();
            }
            if (!projectFiles.isEmpty())
            {
                tmpFilePath = m_projectForActiveFile->folder();
                tmpFilePath.addPath(".grep.tmp");
                KUrl dir(m_grepdlg->directoryString());
                QList<QRegExp> regExpList;

                if (!filelist.isEmpty())
                {
                    for (QStringList::Iterator it = filelist.begin(); it != filelist.end(); ++it)
                        regExpList.append(QRegExp(*it, Qt::CaseSensitive, QRegExp::Wildcard));
                }

                m_tempFile.setFileName(tmpFilePath.toLocalFile());
                if (m_tempFile.open(QIODevice::WriteOnly))
                {
                    QTextStream out(&m_tempFile);
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

                    m_tempFile.close();
                }
                else
                {
                    KMessageBox::error(0, i18n("Unable to create a temporary file for search."));
                    return;
                }
            }

            QStringList catCmd;
            catCmd << tmpFilePath.toLocalFile().replace(' ', "\\ ");
            catProc = new GrepviewProcess(this);
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
        findCmd << m_grepdlg->directoryString();
        if (!m_grepdlg->recursiveFlag())
            findCmd << "-maxdepth" << "1";

        // these cause error, and doesn't needed in qt4
//         QString filePattern;
//         filePattern = "\\( -name ";
//         filePattern += files;
//         filePattern += " \\)";
        findCmd << "-name" << files;
        findCmd << "-follow";

//         if (m_grepdlg->noFindErrorsFlag()) // PORT specify it by channel mode.
//             findCmd += " 2>/dev/null";

        kDebug() << "findCmd :" << findCmd << endl;
        findProc = new GrepviewProcess(this);
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

        grepProc = new GrepviewProcess(this);
        grepProc->setProgram( "grep", grepCmd );
        grepProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << grepProc;
    }

    QStringList sedCmd;
    if (!m_grepdlg->useProjectFilesFlag())
    {
        // quote spaces in filenames going to xargs
        sedCmd << "s/ /\\\\\\ /g";
        sedProc = new GrepviewProcess(this);
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

    xargsProc = new GrepviewProcess(this);
    xargsProc->setProgram( "xargs", xargsCmd );
    xargsProc->setOutputChannelMode( KProcess::SeparateChannels );
    validProcs << xargsProc;

    if( validProcs.count() > 1 )
        validProcs[0]->setStandardOutputProcess( validProcs[1] );
    if( validProcs.count() > 2 )
        validProcs[1]->setStandardOutputProcess( validProcs[2] );
    if( validProcs.count() > 3 )
        validProcs[2]->setStandardOutputProcess( validProcs[3] );

    GrepOutputModel *model = new GrepOutputModel(this);
    ProcessLineMaker *lineMaker = new ProcessLineMaker( xargsProc );

    // needed because processlinemaker is not a child of KProcess.
    connect( xargsProc, SIGNAL(destroyed(QObject*)), lineMaker, SLOT(deleteLater()) );

    connect( lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
             model, SLOT(appendOutputs(const QStringList&)) );
    connect( lineMaker, SIGNAL(receivedStderrLines( const QStringList& )),
             model, SLOT(appendErrors(const QStringList&)) );
    connect( xargsProc, SIGNAL(finished(int, QProcess::ExitStatus)),
             model, SLOT(slotCompleted()) );
    connect( xargsProc, SIGNAL(error( QProcess::ProcessError )),
             model, SLOT(slotFailed()) );

    int id = m_view->registerView( m_grepdlg->patternString() );
    m_view->setModel( id, model );

    // At first line, print out actual command invokation as if it was run via shell.
    QString printCmd;
    foreach( GrepviewProcess *_proc, validProcs )
    {
        printCmd += _proc->program().join(" ") + " | ";
    }
    printCmd.chop(3); // chop last '|'
    model->appendRow( new QStandardItem(printCmd) );

    // start process. GrepviewProcess is self-deleted.
    foreach( GrepviewProcess *_proc, validProcs )
    {
        _proc->start();
    }
}

QString GrepViewPart::escape(const QString &str)
{
    QString escaped("[]{}()\\^$?.+-*|");
    QString res;

    for (uint i=0; i < str.length(); ++i)
    {
        if (escaped.indexOf(str[i]) != -1)
            res += "\\";
        res += str[i];
    }

    return res;
}


// from kdeveditorutil.cpp in kdev3.4, plus porting
QString GrepViewPart::currentWord()
{
    KTextEditor::Document *doc =
            dynamic_cast<KTextEditor::Document*>( core()->partManager()->activePart() );

    if( !doc ) return QString();

    KTextEditor::View *view = doc->activeView();
    if( !view ) return QString();

    KTextEditor::Cursor cursor = view->cursorPositionVirtual();
    int line = cursor.line();
    int col  = cursor.column();
    QString linestr = doc->line(line);

    int startPos = qMax( qMin( col, linestr.length()-1 ), 0 );
    int endPos = startPos;
    while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' || linestr[startPos] == '~') )
        startPos--;
    while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
        endPos++;

    return ( ( startPos == endPos ) ? QString::null : linestr.mid( startPos+1, endPos-startPos-1 ) );
}

QString GrepViewPart::currentSelectedWord()
{
    KTextEditor::Document *doc =
            dynamic_cast<KTextEditor::Document*>( core()->partManager()->activePart() );

    if( !doc ) return QString();

    KTextEditor::View *view = doc->activeView();
    if( !view ) return QString();
    return view->selectionText();
}



#include "grepviewpart.moc"

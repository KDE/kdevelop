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

// #include <qpopupmenu.h>
// #include <qvbox.h>
#include <qwhatsthis.h>
#include <QtDesigner/QExtensionFactory>
#include <QList>

#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kstringhandler.h>
#include <ktexteditor/document.h>

#include <icore.h>
#include <ioutputview.h>
#include <iplugincontroller.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <commandexecutor.h>
#include <processlinemaker.h>

using namespace KDevelop;

typedef KGenericFactory<GrepViewPart> GrepViewFactory;
K_EXPORT_COMPONENT_FACTORY( kdevgrepview,
                            GrepViewFactory( "kdevgrepview" )  )

// class GrepViewWidgetFactory: public KDevelop::IToolViewFactory{
// public:
//     GrepViewWidgetFactory(GrepViewPart *part): m_part(part) {}
//     virtual QWidget* create(QWidget *parent = 0)
//     {
//         return new GrepViewWidget(m_part, parent);
//     }
//     virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
//     {
//         return Qt::BottomDockWidgetArea;
//     }
// private:
//     GrepViewPart *m_part;
// };

GrepViewPart::GrepViewPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin( GrepViewFactory::componentData(), parent )
    , m_view(0)
{
    setXMLFile("kdevgrepview.rc");
//     d->m_factory = new GrepViewFactory(this);
//     core()->uiController()->addToolView("GrepView", d->m_factory);

    // TODO treat these
//     connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//              this, SLOT(stopButtonClicked(KDevPlugin*)) );
//     connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
//     connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
//     connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
//              this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

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
//     action->setShortcut( QKeySequence(Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotGrep()));
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("<b>Find in files</b><p>"
            "Opens the 'Find in files' dialog. There you "
            "can enter a regular expression which is then "
            "searched for within all files in the directories "
            "you specify. Matches will be displayed, you "
            "can switch to a match directly.") );

//     KAction *action;
//
//     action = new KAction(i18n("Find in Fi&les..."), "grep", CTRL+ALT+Key_F,
//                         this, SLOT(slotGrep()),
//                         actionCollection(), "edit_grep");
//     action->setToolTip( i18n("Search for expressions over several files") );
//     action->setWhatsThis( i18n("<b>Find in files</b><p>"
//                             "Opens the 'Find in files' dialog. There you "
//                             "can enter a regular expression which is then "
//                             "searched for within all files in the directories "
//                             "you specify. Matches will be displayed, you "
//                             "can switch to a match directly.") );

    KDevelop::IPlugin* plugin =
            core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    Q_ASSERT( plugin );

    m_view = plugin->extension<KDevelop::IOutputView>();
    Q_ASSERT( m_view );

}


GrepViewPart::~GrepViewPart()
{
}

/*
void GrepViewPart::stopButtonClicked(KDevPlugin* which)
{
    if ( which != 0 && which != this )
        return;
    kdDebug(9001) << "GrepViewPart::stopButtonClicked()" << endl;
    m_widget->killJob( SIGHUP );
}


void GrepViewPart::projectOpened()
{
    kdDebug(9001) << "GrepViewPart::projectOpened()" << endl;
    m_widget->projectChanged(project());
}


void GrepViewPart::projectClosed()
{
    m_widget->projectChanged(0);
}*/

/*
void GrepViewPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug(9001) << "context in grepview" << endl;
    if (!context->hasType( Context::EditorContext ))
        return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString ident = econtext->currentWord();
    if (!ident.isEmpty()) {
        m_popupstr = ident;
        QString squeezed = KStringHandler::csqueeze(ident, 30);
        int id = popup->insertItem( i18n("Grep: %1").arg(squeezed),
                        this, SLOT(slotContextGrep()) );
        popup->setWhatsThis(id, i18n("<b>Grep</b><p>Opens the find in files dialog "
                            "and sets the pattern to the text under the cursor."));
        popup->insertSeparator();
    }
}*/


void _playGround()
{
//         // test debug
//     QString command( "/home/kdev/httpsrep | grep Makefile" );
//     KDevelop::CommandExecutor *executor = new KDevelop::CommandExecutor( "find", false );
//     QStringList finalArgs = command.split(" ");
//
//     executor->setArguments( finalArgs );
//
//     // prepare model
//     GrepOutputModel *model = new GrepOutputModel(this);
//     model->appendRow( new QStandardItem(command) );
//     connect( executor, SIGNAL(receivedStandardOutput( const QStringList& )),
//              model, SLOT(appendOutputs( const QStringList & )) );
//     connect( executor, SIGNAL(receivedStandardError( const QStringList& )),
//              model, SLOT(appendErrors( const QStringList& )) );
//     connect( executor, SIGNAL(failed()), model, SLOT(slotFailed()) );
//     connect( executor, SIGNAL(completed()), model, SLOT(slotCompleted()) );
//     int id = m_view->registerView( "/home/kdev/httpsrep" );
//     m_view->setModel( id, model );
//
//     executor->start();
//
//     test debug 2 -- WORKED!
//     KProcess *findProc = new KProcess();
//     findProc->setOutputChannelMode( KProcess::SeparateChannels );
//     KProcess *grepProc = new KProcess();
//     grepProc->setOutputChannelMode( KProcess::SeparateChannels );
//     findProc->setStandardOutputProcess( grepProc );
//
//     *findProc << "find" << "/home/kdev/httpsrep";
//     *grepProc << "grep" << "Makefile";
//
//     GrepOutputModel *model = new GrepOutputModel(this);
//
//     ProcessLineMaker *lineMaker = new ProcessLineMaker( grepProc );
//     connect( lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
//              model, SLOT(appendOutputs(const QStringList&)) );
//     connect( lineMaker, SIGNAL(receivedStderrLines( const QStringList& )),
//              model, SLOT(appendErrors(const QStringList&)) );
//     connect( grepProc, SIGNAL(finished(int, QProcess::ExitStatus)),
//              model, SLOT(slotCompleted()) );
//     connect( grepProc, SIGNAL(error( QProcess::ProcessError )),
//              model, SLOT(slotFailed()) );
//
//     int id = m_view->registerView( "/home/kdev/httpsrep" );
//     m_view->setModel( id, model );
//
//     grepProc->start();
//     findProc->start();
//
//     test debug 3 - xargs -- WORK
//     KProcess *findProc = new KProcess;
//     findProc->setOutputChannelMode( KProcess::SeparateChannels );
//     KProcess *xargsProc = new KProcess;
//     xargsProc->setOutputChannelMode( KProcess::SeparateChannels );
//     findProc->setStandardOutputProcess( xargsProc );
//     QString findCmd;
//     findCmd += QString("/home/kdev/httpsrep ");
// //     findCmd += "-name ";
// //     findCmd += "*.c ";
//
//     findProc->setProgram( "find", findCmd.split(' ', QString::SkipEmptyParts));
//     QString xargsCmd = "grep main";
//     xargsProc->setProgram( "xargs", xargsCmd.split(' ', QString::SkipEmptyParts) );
//
//     GrepOutputModel *model = new GrepOutputModel(this);
//
//     ProcessLineMaker *lineMaker = new ProcessLineMaker( xargsProc );
//     connect( lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
//              model, SLOT(appendOutputs(const QStringList&)) );
//     connect( lineMaker, SIGNAL(receivedStderrLines( const QStringList& )),
//              model, SLOT(appendErrors(const QStringList&)) );
//     connect( xargsProc, SIGNAL(finished(int, QProcess::ExitStatus)),
//              model, SLOT(slotCompleted()) );
//     connect( xargsProc, SIGNAL(error( QProcess::ProcessError )),
//              model, SLOT(slotFailed()) );
//
//     int id = m_view->registerView( "/home/kdev/httpsrep" );
//     m_view->setModel( id, model );
//
//     findProc->start();
//     xargsProc->start();

//     test debug 4 -- not work
    KProcess *findProc = new KProcess;
    findProc->setOutputChannelMode( KProcess::SeparateChannels );
    QString findCmd;
    findCmd = QString("/home/kdev/httpsrep-other/ ");
//     findCmd += " \\( -name ";
//     findCmd += quote("*.c");
//     findCmd += " -o -name ";
//     findCmd += quote("*.h");
//     findCmd += " \\) ";
    findCmd += " -print -follow ";

    kDebug() << findCmd << endl;

    findProc->setProgram( "find", findCmd.split(' ', QString::SkipEmptyParts ) );

    GrepOutputModel *model = new GrepOutputModel(0);

    ProcessLineMaker *lineMaker = new ProcessLineMaker( findProc );
    QObject::connect( lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
             model, SLOT(appendOutputs(const QStringList&)) );
    QObject::connect( lineMaker, SIGNAL(receivedStderrLines( const QStringList& )),
             model, SLOT(appendErrors(const QStringList&)) );
    QObject::connect( findProc, SIGNAL(finished(int, QProcess::ExitStatus)),
             model, SLOT(slotCompleted()) );
    QObject::connect( findProc, SIGNAL(error( QProcess::ProcessError )),
             model, SLOT(slotFailed()) );

//     int id = m_view->registerView( "/home/kdev/httpsrep" );
//     m_view->setModel( id, model );
    findProc->start();
}

void GrepViewPart::slotGrep()
{
// TODO PORT THIS
//     QString contextString = KDevEditorUtil::currentSelection( dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) );
//     if ( contextString.isEmpty() )
//     {
//         contextString = KDevEditorUtil::currentWord( dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) );
//     }
//
    this->showDialogWithPattern( QString() );


//     _playGround();
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
        }
        else
        {
            m_grepdlg->setEnableProjectBox(false);
        }
    }

    m_grepdlg->show();
}

/*
void GrepViewPart::slotContextGrep()
{
    if ( !m_widget->isRunning() )
    {
        m_widget->showDialogWithPattern(m_popupstr);
    }
}*/

void GrepViewPart::searchActivated()
{
//     if ( m_grepdlg->keepOutputFlag() )
//         slotKeepOutput();

//     m_tabWidget->showPage( m_curOutput );

//     m_curOutput->setLastFileName("");
//     m_curOutput->setMatchCount( 0 );

    QString /*command, */files;
    KProcess *findProc=0, *grepProc=0, *sedProc=0, *xargsProc=0;
    QList<KProcess*> validProcs;
    QString /*findCmd, */grepCmd, sedCmd, xargsCmd;

    // waba: code below breaks on filenames containing a ',' !!!
//     QStringList filelist = QString::split(",", m_grepdlg->filesString());
    QStringList filelist = m_grepdlg->filesString().split(",");

    if (m_grepdlg->useProjectFilesFlag())
    {
//         KDevProject *openProject = m_part->project();
//         if (openProject)
//         {
//             QString tmpFilePath;
//             QStringList projectFiles = openProject->allFiles();
//             if (!projectFiles.isEmpty())
//             {
//                 tmpFilePath = openProject->projectDirectory() + QChar(QDir::separator()) + ".grep.tmp";
//                 QString dir = m_grepdlg->directoryString(), file;
//                 QValueList<QRegExp> regExpList;
//
//                 if (dir.endsWith(QChar(QDir::separator())))
//                     dir.truncate(dir.length() - 1);
//
//                 if (!filelist.isEmpty())
//                 {
//                     for (QStringList::Iterator it = filelist.begin(); it != filelist.end(); ++it)
//                         regExpList.append(QRegExp(*it, true, true));
//                 }
//
//                 m_tempFile.setName(tmpFilePath);
//                 if (m_tempFile.open(IO_WriteOnly))
//                 {
//                     QTextStream out(&m_tempFile);
//                     for (QStringList::Iterator it = projectFiles.begin(); it != projectFiles.end(); ++it)
//                     {
//                         file = QDir::cleanDirPath(openProject->projectDirectory() + QChar(QDir::separator()) + *it);
//
//                         QFileInfo info(file);
//                         if (m_grepdlg->recursiveFlag() && !info.dirPath(true).startsWith(dir)) continue;
//                         if (!m_grepdlg->recursiveFlag() && info.dirPath(true) != dir) continue;
//
//                         bool matchOne = regExpList.count() == 0;
//                         for (QValueList<QRegExp>::Iterator it2 = regExpList.begin(); it2 != regExpList.end() && !matchOne; ++it2)
//                             matchOne = (*it2).exactMatch(file);
//
//                         if (matchOne)
//                             out << KShellProcess::quote(file) + "\n";
//                     }
//
//                     m_tempFile.close();
//                 }
//                 else
//                 {
//                     KMessageBox::error(this, i18n("Unable to create a temporary file for search."));
//                     return;
//                 }
//             }
//
//             command << "cat";
//             command << tmpFilePath.replace(' ', "\\ ");
//         }
    }
    else
    {
#if 0 // TODO PORT seems that quote doesn't work properly. Should be fixed later
        if (!filelist.isEmpty())
        {
            QStringList::Iterator it(filelist.begin());
            files = quote(*it);
            ++it;
            for(; it!=filelist.end(); ++it )
                files += " -o -name " + quote(*it) + " ";
        }
#endif
//         findCmd += KShellProcess::quote(m_grepdlg->directoryString());
//         findCmd += quote( m_grepdlg->directoryString() ); // debug : check
        QStringList findCmd;
        findCmd << m_grepdlg->directoryString();
        if (!m_grepdlg->recursiveFlag())
            findCmd << "-maxdepth" << "1";

#if 0 // TODO PORT seems that quote doesn't work properly. Should be fixed later
        QString filePattern;
        filePattern = "\\( -name ";
        filePattern += files;
        filePattern += " \\)";

        findCmd << filePattern.split(' ', QString::SkipEmptyParts );
#endif
        findCmd << "-print0" << "-follow";
//         if (m_grepdlg->noFindErrorsFlag()) // PORT specify it by channel mode.
//             findCmd += " 2>/dev/null";
        kDebug() << "findCmd :" << findCmd << endl;
        findProc = new KProcess(this);
        findProc->setProgram( "find", findCmd/*.split(' ', QString::SkipEmptyParts)*/ );
        findProc->setOutputChannelMode( KProcess::SeparateChannels );
        validProcs << findProc;
    }

    // TODO PORT handle quote correctly. -- temporarily disabled.
//     QStringList excludelist = m_grepdlg->excludeString().split(",");
//     if (!excludelist.isEmpty())
//     {
//         QStringList::Iterator it(excludelist.begin());
// //         command += "| grep -v ";
//         grepCmd += "-v ";
//         for (; it != excludelist.end(); ++it)
// //             command += "-e " + KShellProcess::quote(*it) + " ";
//             grepCmd += "-e " + quote(*it) + " ";
//
//         grepProc = new KProcess(this);
//         grepProc->setProgram( "grep", grepCmd.split(' ', QString::SkipEmptyParts) );
//         validProcs << grepProc;
//     }

    // PORTING NOTE dukju ahn: why perform artificial quote in filename containing space?
    // By using -0 option at xarg and -print0 option at find, filename space can be treated.
//     if (!m_grepdlg->useProjectFilesFlag())
//     {
//         // quote spaces in filenames going to xargs
// //         command += "| sed \"s/ /\\\\\\ /g\" ";
//         sedCmd += "\"s/ /\\\\\\ /g\"";
//         sedProc = new KProcess(this);
//         sedProc->setProgram( "sed", QStringList() << sedCmd );
//         validProcs << sedProc;
//     }


#ifndef USE_SOLARIS
    xargsCmd += " -0 egrep -H -n -s ";
#else
    // -H reported as not being available on Solaris,
    // but we're buggy without it on Linux.
    xargsCmd += " egrep -n ";
#endif

    if (!m_grepdlg->caseSensitiveFlag())
    {
        xargsCmd += "-i ";
    }
    xargsCmd += " -e ";

    m_lastPattern = m_grepdlg->patternString();
    QString pattern = m_grepdlg->templateString();
    if (m_grepdlg->regexpFlag())
        pattern.replace(QRegExp("%s"), m_grepdlg->patternString());
    else
        pattern.replace(QRegExp("%s"), escape( m_grepdlg->patternString() ) );
//     command += KShellProcess::quote(pattern);
//     xargsCmd += quote(pattern);
    xargsCmd += pattern; // TODO PORT why quote doesn't work?

    xargsProc = new KProcess(this);
    xargsProc->setProgram( "xargs", xargsCmd.split(' ', QString::SkipEmptyParts) );
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


    foreach( KProcess *_proc, validProcs )
    {
        _proc->start();
    }
    kDebug() << "grepCmd :" << grepCmd.split(' ',QString::SkipEmptyParts) << endl;
    kDebug() << "sedCmd :" << sedCmd << endl;
    kDebug() << "xargsCmd :" << xargsCmd.split(' ',QString::SkipEmptyParts) << endl;
    //TODO memory free when completed.
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

// from K3ShellProcess::quote()
QString GrepViewPart::quote(const QString &arg)
{
    QChar q('\'');
    return QString(arg).replace(q, "'\\''").prepend(q).append(q);
}

#include "grepviewpart.moc"

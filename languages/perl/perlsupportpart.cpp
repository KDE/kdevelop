/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "perlsupportpart.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <qregexp.h>
#include <codemodel.h>
#include <qprogressbar.h>
#include <kstatusbar.h>
#include "kdevmainwindow.h"

#include <kprocess.h>
#include <stdlib.h>
#include <unistd.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevappfrontend.h"
//#include "classstore.h"
//#include "parsedclass.h"
//#include "parsedmethod.h"
//#include "parsedscript.h"
#include "domutil.h"
//#include "programmingbycontract.h"

typedef KGenericFactory<PerlSupportPart> PerlSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevperlsupport, PerlSupportFactory( "kdevperlsupport" ) );

PerlSupportPart::PerlSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport("PerlSupport", "perl", parent, name ? name : "PerlSupportPart")
{
    setInstance(PerlSupportFactory::instance());

    setXMLFile("kdevperlsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

    KAction *action;

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setStatusText( i18n("Runs the Perl program") );

    action = new KAction( i18n("Execute String..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setStatusText( i18n("Executes a string as Perl code") );

    action = new KAction( i18n("Start Perl Interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setStatusText( i18n("Starts the Perl interpreter without a program") );

    action = new KAction( i18n("Find Perl Function Documentation..."), 0,
                          this, SLOT(slotPerldocFunction()),
                          actionCollection(), "help_perldocfunction" );
    action->setStatusText( i18n("Show the documentation page of a Perl function") );

    action = new KAction( i18n("Find Perl FAQ Entry..."), 0,
                          this, SLOT(slotPerldocFAQ()),
                          actionCollection(), "help_perldocfaq" );
    action->setStatusText( i18n("Show the FAQ entry for a keyword") );

    //perl parser for codemodel
    m_parser = new  perlparser(core(),codeModel(),interpreter());
}


PerlSupportPart::~PerlSupportPart()
{}


void PerlSupportPart::projectOpened()
{
    kdDebug(9007) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PerlSupportPart::projectClosed()
{
}

void PerlSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString extension = fi.extension();
    if (extension == "pl" || extension == "pm") {
        kdDebug(9016) << "maybe " << fileName << endl;
        if( codeModel()->hasFile(fileName) ){
          emit aboutToRemoveSourceInfo( fileName );
          codeModel()->removeFile( codeModel()->fileByName(fileName) );
        }
        m_parser->parse(fileName);
    }
}

void PerlSupportPart::addedFilesToProject(const QStringList &fileList)
{
	kdDebug(9016) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		maybeParse(project()->projectDirectory() + "/" + ( *it ) );
	}
	emit updatedSourceInfo();
}


void PerlSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    kdDebug(9016) << "removedFilesFromProject()" << endl;
    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
     QString fileName = project()->projectDirectory() + "/" + ( *it );
     if( codeModel()->hasFile(fileName) ){
      emit aboutToRemoveSourceInfo( fileName );
      codeModel()->removeFile( codeModel()->fileByName(fileName) );
     }
   }
  emit updatedSourceInfo();
}


void PerlSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9016) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName);
        m_parser->parseUseFiles();
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PerlSupportPart::features()
{
    return KDevLanguageSupport::Features(Classes | Functions | Variables | Namespaces | Scripts | NewClass | AddMethod | AddAttribute | NewScript);
//    return Functions;
}

QString PerlSupportPart::interpreter()
{
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevperlsupport/run/interpreter");
    if (prog.isEmpty())
        prog = "perl";

    return prog;
}


void PerlSupportPart::startApplication(const QString &program)
{
    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevperlsupport/run/terminal");
    appFrontend()->startAppCommand(QString::QString(), program, inTerminal);
}


void PerlSupportPart::slotExecute()
{
    QString program =  project()->mainProgram();
    QString cmd = interpreter() + " " + program;
    startApplication(cmd);
}


void PerlSupportPart::slotStartInterpreter()
{
    startApplication(interpreter());
}


void PerlSupportPart::slotExecuteString()
{
    bool ok;
    QString cmd = KLineEditDlg::getText(i18n("String to execute"), QString::null, &ok, 0);
    if (ok) {
        cmd.prepend("'");
        cmd.append("'");
        startApplication(cmd);
    }
}


void PerlSupportPart::slotPerldocFunction()
{
    bool ok;
    QString key = KLineEditDlg::getText(i18n("Show Perl documentation for function:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "perldoc:functions/";
        url += key;
        partController()->showDocument(KURL(url));
    }
}


void PerlSupportPart::slotPerldocFAQ()
{
    bool ok;
    QString key = KLineEditDlg::getText(i18n("Show FAQ entry for keyword:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "perldoc:faq/";
        url += key;
        partController()->showDocument(KURL(url));
    }
}
KMimeType::List PerlSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "application/x-perl" );
    if( mime )
	list << mime;
    return list;
}
void PerlSupportPart::initialParse()
{
    kdDebug(9016) << "initialParse()" << endl;

    if (project()) {
        //copy from cpp support : give user some feedback
        mainWindow()->statusBar()->message( i18n("Updating...") );
        kapp->processEvents( );

        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allFiles();
        m_parser->initialParse();

        //progress bar
        QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
        bar->setMinimumWidth( 120 );
        bar->setCenterIndicator( true );
        mainWindow( )->statusBar( )->addWidget( bar );
        bar->show( );
        int n = 0;
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9016) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
            //update progress bar
            bar->setProgress( n++ );
            if( (n%5) == 0 )
              kapp->processEvents();
        }

        m_parser->parseUseFiles();


        emit updatedSourceInfo();
        //remove progressbar
        mainWindow( )->statusBar( )->removeWidget( bar );
        delete bar;
        kapp->restoreOverrideCursor();
        mainWindow()->statusBar()->message( i18n("Done") );

    } else {
        kdDebug(9016) << "No project" << endl;
    }
}


#include "perlsupportpart.moc"

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
#include <kdevgenericfactory.h>
#include <kinputdialog.h>
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
#include "kdevplugininfo.h"
#include "kdevappfrontend.h"
//#include "classstore.h"
//#include "parsedclass.h"
//#include "parsedmethod.h"
//#include "parsedscript.h"
#include "domutil.h"
//#include "programmingbycontract.h"

typedef KDevGenericFactory<PerlSupportPart> PerlSupportFactory;
static const KDevPluginInfo data("kdevperlsupport");
K_EXPORT_COMPONENT_FACTORY( libkdevperlsupport, PerlSupportFactory( data ) )

PerlSupportPart::PerlSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(&data, parent, name ? name : "PerlSupportPart")
{
    setInstance(PerlSupportFactory::instance());

    setXMLFile("kdevperlsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );

    KAction *action;

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setToolTip( i18n("Runs the Perl program") );

    action = new KAction( i18n("Execute String..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setToolTip( i18n("Executes a string as Perl code") );

    action = new KAction( i18n("Start Perl Interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setToolTip( i18n("Starts the Perl interpreter without a program") );

    action = new KAction( i18n("Find Perl Function Documentation..."), 0,
                          this, SLOT(slotPerldocFunction()),
                          actionCollection(), "help_perldocfunction" );
    action->setToolTip( i18n("Show the documentation page of a Perl function") );

    action = new KAction( i18n("Find Perl FAQ Entry..."), 0,
                          this, SLOT(slotPerldocFAQ()),
                          actionCollection(), "help_perldocfaq" );
    action->setToolTip( i18n("Show the FAQ entry for a keyword") );

    //perl parser for codemodel
    m_parser = new  perlparser(core(),codeModel(),interpreter());
}


PerlSupportPart::~PerlSupportPart()
{
  if (project())
      projectClosed();

  delete m_parser;
  m_parser=0;
}


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
        removeWithReference(fileName);
        m_parser->parse(fileName);
	emit addedSourceInfo( fileName);
    }
}

void PerlSupportPart::addedFilesToProject(const QStringList &fileList)
{
	kdDebug(9016) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		maybeParse(project()->projectDirectory() + "/" + ( *it ));
	}
}


void PerlSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    kdDebug(9016) << "removedFilesFromProject()" << endl;
    QStringList::ConstIterator it;
    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
     QString fileName = project()->projectDirectory() + "/" + ( *it );
     removeWithReference(fileName);
   }
  emit updatedSourceInfo();
}


void PerlSupportPart::savedFile(const KURL &fileName)
{
    Q_UNUSED( fileName.path() );
#if 0  // not needed anymore
    kdDebug(9016) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
#endif
}


KDevLanguageSupport::Features PerlSupportPart::features()
{
    return KDevLanguageSupport::Features(Classes | Functions | Variables | Namespaces | /*Scripts | */NewClass | AddMethod | AddAttribute /*| NewScript*/);
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
    if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
        appFrontend->startAppCommand(QString::QString(), program, inTerminal);
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
    QString cmd = KInputDialog::getText(i18n("String to Execute"), i18n("String to execute:"), QString::null, &ok, 0);
    if (ok) {
        cmd.prepend("'");
        cmd.append("'");
        startApplication(cmd);
    }
}


void PerlSupportPart::slotPerldocFunction()
{
    bool ok;
    QString key = KInputDialog::getText(i18n("Show Perl Documentation"), i18n("Show Perl documentation for function:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "perldoc:functions/";
        url += key;
        partController()->showDocument(KURL(url));
    }
}


void PerlSupportPart::slotPerldocFAQ()
{
    bool ok;
    QString key = KInputDialog::getText(i18n("Show FAQ Entry"), i18n("Show FAQ entry for keyword:"), "", &ok, 0);
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
//            kdDebug(9016) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
            //update progress bar
            bar->setProgress( n++ );
            if( (n%5) == 0 )
              kapp->processEvents();
        }
        parseUseFiles();
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

void PerlSupportPart::removeWithReference( const QString & fileName )
{
    kdDebug(9016) << "remove with references: " << fileName << endl;
    //m_timestamp.remove( fileName );
    if( !codeModel()->hasFile(fileName) )
        return;

    emit aboutToRemoveSourceInfo( fileName );
    codeModel()->removeFile( codeModel()->fileByName(fileName) );
}

void PerlSupportPart::parseUseFiles()
{
 kdDebug(9016) << "parse addional libs" << endl;
 return;
 QString filename;
 QStringList m_usefiles = m_parser->UseFiles();

 //parse addional use files
 for (QStringList::Iterator it = m_usefiles.begin(); it != m_usefiles.end() ;++it)
 {
        filename = m_parser->findLib(*it);
       //if something found , parse it
       if (!filename.isEmpty()) {
            //kdDebug(9016) << "found " << filename << endl;
            maybeParse(filename);
       }
 }
}

#include "perlsupportpart.moc"

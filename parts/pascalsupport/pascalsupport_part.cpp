/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <fstream>

#include <qdir.h>
#include <qtimer.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstatusbar.h>

#include <antlr/ASTFactory.hpp>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"

#include "pascalsupport_part.h"
#include "problemreporter.h"
#include "PascalLexer.hpp"
#include "PascalParser.hpp"
#include "PascalStoreWalker.hpp"

struct PascalSupportPartData{
    ProblemReporter* problemReporter;

    PascalSupportPartData()
        : problemReporter( 0 )
        {}
};

typedef KGenericFactory<PascalSupportPart> PascalSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpascalsupport, PascalSupportFactory( "kdevpascalsupport" ) );

PascalSupportPart::PascalSupportPart(QObject *parent, const char *name, const QStringList &)
  : KDevLanguageSupport("PascalSupport", "pascal", parent, name ? name : "KDevPascalSupport" ),
    d( new PascalSupportPartData() )
{
    setInstance(PascalSupportFactory::instance());
    setXMLFile("kdevpascalsupport.rc");

    d->problemReporter = new ProblemReporter( this );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             d->problemReporter, SLOT(configWidget(KDialogBase*)) );

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    mainWindow()->embedOutputView( d->problemReporter, i18n("Problems"), i18n("problem reporter") );
}

PascalSupportPart::~PascalSupportPart()
{
    mainWindow()->removeView( d->problemReporter );
    delete( d->problemReporter );

    delete( d );
}

PascalSupportPart::Features PascalSupportPart::features()
{
    return Features(Classes | Structs | Functions | Variables | Declarations);
}

QStringList PascalSupportPart::fileFilters()
{
    return QStringList::split(",", "*.p,*.pp,*.pas,*.dpr");
}

void PascalSupportPart::projectOpened()
{
    connect(project(), SIGNAL(addedFilesToProject(const QStringList &)),
        this, SLOT(addedFilesToProject(const QStringList &)));
    connect(project(), SIGNAL(removedFilesFromProject(const QStringList &)),
        this, SLOT(removedFilesFromProject(const QStringList &)));
    connect(project(), SIGNAL(projectCompiled()),
        this, SLOT(slotProjectCompiled()) );

    QDir::setCurrent(project()->projectDirectory());
    m_projectFileList = project()->allFiles();
    m_projectClosed = false;

    QTimer::singleShot(0, this, SLOT(initialParse()));
}

void PascalSupportPart::projectClosed()
{
    m_projectClosed = true;
}

void PascalSupportPart::configWidget(KDialogBase *dlg)
{
    return;
}

void PascalSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    return;
}

void PascalSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    return;
}

void PascalSupportPart::savedFile(const QString &fileName)
{
    return;
}

void PascalSupportPart::addedFilesToProject(const QStringList &fileList)
{
    return;
}

void PascalSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    return;
}

void PascalSupportPart::slotProjectCompiled()
{
    return;
}

void PascalSupportPart::initialParse( )
{
    kdDebug(9013) << "------------------------------------------> initialParse()" << endl;

    if (project())
    {
        kapp->setOverrideCursor(waitCursor);

        // TODO: Progress indicator!

        QStringList files = project()->allFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it){
            QString fn = project()->projectDirectory() + "/" + *it;
            maybeParse( fn );
            kapp->processEvents( 500 );
        }

        emit updatedSourceInfo();

        kapp->restoreOverrideCursor();
        mainWindow()->statusBar()->message( i18n("Found 1 problem", "Found %n problems", d->problemReporter->childCount()) );
    }
}

void PascalSupportPart::maybeParse( const QString & fileName )
{
    kdDebug(9013) << "Maybe parse: " << fileName << endl;

    if( !fileFilters().contains( QFileInfo( fileName ).extension() ) )
        return;

    mainWindow()->statusBar()->message( i18n("Parsing file: %1").arg(fileName) );
    parse( fileName );
}

void PascalSupportPart::parse( const QString & fileName )
{
    kdDebug(9013) << "PascalSupportPart::parse() -- " << fileName << endl;

    std::ifstream stream( fileName );
    QCString _fn = fileName.utf8();
    std::string fn( _fn.data() );

    PascalLexer lexer( stream );
    lexer.setFilename( fn );
    lexer.setProblemReporter( d->problemReporter );

    PascalParser parser( lexer );
    parser.setFilename( fn );
    parser.setProblemReporter( d->problemReporter );

    try{
        antlr::ASTFactory my_factory( "PascalAST", PascalAST::factory );
        parser.initializeASTFactory(my_factory);
        parser.setASTFactory( &my_factory );

        lexer.resetErrors();
        parser.resetErrors();

        parser.program();
        int errors = lexer.numberOfErrors() + parser.numberOfErrors();
        RefPascalAST ast = parser.getAST();

        if( errors == 0 && ast != antlr::nullAST ){
            kdDebug(9013) << "-------------------> start StoreWalker" << endl;
            PascalStoreWalker walker;
            walker.setFileName( fileName );
            walker.setClassStore( classStore() );
            walker.program( ast );
        }

    } catch( antlr::ANTLRException& ex ){
        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        d->problemReporter->reportError( ex.getMessage(),
                                         fileName,
                                         lexer.getLine(),
                                         lexer.getColumn() );
    }
}

#include "pascalsupport_part.moc"

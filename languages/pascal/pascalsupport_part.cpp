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
#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstatusbar.h>

#include <antlr/ASTFactory.hpp>

#include "catalog.h"
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>

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

typedef KDevGenericFactory<PascalSupportPart> PascalSupportFactory;
static const KAboutData data("kdevpascalsupport", I18N_NOOP("Language"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevpascalsupport, PascalSupportFactory( &data ) )

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
    connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    mainWindow()->embedOutputView( d->problemReporter, i18n("Problems"), i18n("problem reporter") );
    QWhatsThis::add(d->problemReporter, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
        "It displays errors reported by a language parser."));
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
    Q_UNUSED( dlg );
    return;
}

void PascalSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    Q_UNUSED( dlg );
    return;
}

void PascalSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    Q_UNUSED( popup );
    Q_UNUSED( context );
    return;
}

void PascalSupportPart::savedFile(const KURL &fileName)
{
    maybeParse(fileName.path());
    emit updatedSourceInfo();
}

void PascalSupportPart::addedFilesToProject(const QStringList &fileList)
{
    for (QStringList::ConstIterator it = fileList.begin(); it != fileList.end() ;++it)
    {
        QString fn = project()->projectDirectory() + "/" + *it;
        maybeParse( fn );
        kapp->processEvents( 500 );
        emit addedSourceInfo(fn);
    }
}

void PascalSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    for (QStringList::ConstIterator it = fileList.begin(); it != fileList.end() ;++it)
    {
        QString fn = project()->projectDirectory() + "/" + *it;

        emit aboutToRemoveSourceInfo(fn);
        codeModel()->removeFile( codeModel()->fileByName(fn) );
    }
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

        /// @todo Progress indicator!

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

    KMimeType::Ptr mime = KMimeType::findByURL( KURL( fileName ) );
    if( !mime || mime->name() != "text/x-pascal" )
	return;

    mainWindow()->statusBar()->message( i18n("Parsing file: %1").arg(fileName) );
    parse( fileName );
}

void PascalSupportPart::parse( const QString & fileName )
{
    kdDebug(9013) << "PascalSupportPart::parse() -- " << fileName << endl;

    std::ifstream stream( QFile::encodeName( fileName ).data() );
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

        parser.compilationUnit();
        int errors = lexer.numberOfErrors() + parser.numberOfErrors();
        RefPascalAST ast = parser.getAST();

        if( errors == 0 && ast != antlr::nullAST ){
            kdDebug(9013) << "-------------------> start StoreWalker" << endl;
/*            PascalStoreWalker walker;
            walker.setFileName( fileName );
            walker.setCodeModel( codeModel() );
            walker.compilationUnit( ast );*/
        }

    } catch( antlr::ANTLRException& ex ){
        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        d->problemReporter->reportError( ex.getMessage().c_str(),
                                         fileName,
                                         lexer.getLine(),
                                         lexer.getColumn() );
    }
}

KMimeType::List PascalSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "text/x-pascal" );
    if( mime )
	list << mime;
    return list;
}

QString PascalSupportPart::formatTag( const Tag & inputTag )
{
    Tag tag = inputTag;

    switch( tag.kind() )
    {
        case Tag::Kind_Namespace:
            return QString::fromLatin1("unit ") + tag.name();

        case Tag::Kind_Class:
            return QString::fromLatin1("class ") + tag.name();

        case Tag::Kind_Function:
        case Tag::Kind_FunctionDeclaration:
        {
            return tag.name() + "()";
        }
        break;

        case Tag::Kind_Variable:
        case Tag::Kind_VariableDeclaration:
        {
            return QString::fromLatin1("var ") + tag.name();
        }
        break;
    }
    return tag.name();
}

QString PascalSupportPart::formatModelItem( const CodeModelItem * item, bool shortDescription )
{
    if (item->isFunction() || item->isFunctionDefinition() )
    {
        const FunctionModel *model = static_cast<const FunctionModel*>(item);
        QString function;
        QString args;
        ArgumentList argumentList = model->argumentList();
        for (ArgumentList::const_iterator it = argumentList.begin(); it != argumentList.end(); ++it)
        {
            args.isEmpty() ? args += "" : args += ", " ;
            args += formatModelItem((*it).data());
        }

        function += model->name() + "(" + args + ")";

        if( !shortDescription )
            function += (model->isVirtual() ? QString("virtual; ") : QString("") ) + model->resultType() + " ";

        return function;
    }
    else if (item->isVariable())
    {
        const VariableModel *model = static_cast<const VariableModel*>(item);
        if( shortDescription )
            return model->name();
        return model->name() + ": " + model->type();
    }
    else if (item->isArgument())
    {
        const ArgumentModel *model = static_cast<const ArgumentModel*>(item);
        QString arg;
        arg += model->name();
        arg += ": " + model->type();
        if( !shortDescription )
            arg += model->defaultValue().isEmpty() ? QString("") : QString(" = ") + model->defaultValue();
        return arg.stripWhiteSpace();
    }
    else
        return KDevLanguageSupport::formatModelItem( item, shortDescription );
}

#include "pascalsupport_part.moc"

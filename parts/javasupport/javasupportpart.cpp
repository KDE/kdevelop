
#include <qfileinfo.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kgenericfactory.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstatusbar.h>
#include <kdialogbase.h>

#include <fstream>
#include <strstream>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "classstore.h"
#include "addclass.h"
#include "javasupportpart.h"
#include "problemreporter.h"
#include "configproblemreporter.h"

#include "JavaLexer.hpp"
#include "JavaRecognizer.hpp"
#include "JavaStoreWalker.hpp"
#include "JavaAST.hpp"


typedef KGenericFactory<JavaSupportPart> JavaSupportPartFactory;

K_EXPORT_COMPONENT_FACTORY(libkdevjavasupport, JavaSupportPartFactory("kdevjavasupport"));


struct JavaSupportPartData{
    ProblemReporter* problemReporter;

    JavaSupportPartData()
        : problemReporter( 0 )
        {}
};

JavaSupportPart::JavaSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(parent, name ? name : "JavaSupportPart"), d( new JavaSupportPartData() )
{
    setInstance(JavaSupportPartFactory::instance());

    d->problemReporter = new ProblemReporter( this );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             d->problemReporter, SLOT(configWidget(KDialogBase*)) );


    setXMLFile("javasupportpart.rc");

    connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
    connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));

    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

    mainWindow()->embedOutputView( d->problemReporter, i18n("Problems"), i18n("problem reporter") );

    connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

    // a small hack (robe)
    //classStore()->globalScope()->setName( "(default packages)" );
    //classStore()->addScope( classStore()->globalScope() );
    //classStore()->globalScope()->setName( QString::null );
}


JavaSupportPart::~JavaSupportPart()
{
    mainWindow()->removeView( d->problemReporter );
    delete( d->problemReporter );

    delete( d );
    d = 0;
}


KDevLanguageSupport::Features JavaSupportPart::features()
{
    return KDevLanguageSupport::Features(Classes | NewClass);
}


QStringList JavaSupportPart::fileFilters()
{
    QStringList r;
    r << "*.java";
    return r;
}


void JavaSupportPart::projectOpened()
{
    connect(project(), SIGNAL(addedFilesToProject(const QStringList &)),
            this, SLOT(addedFilesToProject(const QStringList &)));
    connect(project(), SIGNAL(removedFilesFromProject(const QStringList &)),
            this, SLOT(removedFilesFromProject(const QStringList &)));

    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void JavaSupportPart::projectClosed()
{
}


void JavaSupportPart::initialParse()
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

QStringList JavaSupportPart::fileExtensions()
{
    return QStringList() << "java";
}

void JavaSupportPart::maybeParse(const QString &fileName)
{
    kdDebug(9013) << "Maybe parse: " << fileName << endl;

    if( !fileExtensions().contains( QFileInfo( fileName ).extension() ) )
        return;

    mainWindow()->statusBar()->message( i18n("Parsing file: %1").arg(fileName) );
    parse( fileName );
}


void JavaSupportPart::addedFilesToProject(const QStringList &fileList)
{
	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString path = project()->projectDirectory() + "/" + ( *it );
		maybeParse( path );
	}

	emit updatedSourceInfo();
}


void JavaSupportPart::removedFilesFromProject(const QStringList &fileList)
{
	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		kdDebug(9013) << "JavaSupportPart::removedFileFromProject() -- " << ( *it ) << endl;
		QString path = project()->projectDirectory() + "/" + ( *it );
		classStore()->removeWithReferences(path);
	}

	emit updatedSourceInfo();
}


void JavaSupportPart::parse(const QString &fileName)
{
    kdDebug(9013) << "JavaSupportPart::parse() -- " << fileName << endl;

    std::ifstream stream( fileName );
    QCString _fn = fileName.utf8();
    std::string fn( _fn.data() );

    JavaLexer lexer( stream );
    lexer.setFilename( fn );
    lexer.setProblemReporter( d->problemReporter );

    JavaRecognizer parser( lexer );
    parser.setFilename( fn );
    parser.setProblemReporter( d->problemReporter );

    try{
        parser.setASTNodeFactory( JavaAST::factory );
        lexer.resetErrors();
        parser.resetErrors();

        parser.compilationUnit();
        int errors = lexer.numberOfErrors() + parser.numberOfErrors();
        RefJavaAST ast = parser.getAST();

        if( errors == 0 && ast != antlr::nullAST ){
	    kdDebug(9013) << "-------------------> start StoreWalker" << endl;
            JavaStoreWalker walker;
            walker.setFileName( fileName );
            walker.setClassStore( classStore() );
            walker.compilationUnit( ast );
        }

    } catch( antlr::ANTLRException& ex ){
        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        d->problemReporter->reportError( ex.what(),
                                         fileName,
                                         lexer.getLine(),
                                         lexer.getColumn() );
    }
}

void JavaSupportPart::parseContents( const QString& contents, const QString& fileName )
{
    kdDebug() << "JavaSupportPart::parseContents() -- " << fileName << endl;


    QCString _fn = QFile::encodeName(fileName);
    std::string fn( _fn.data() );

    QCString text = contents.utf8();
    std::istrstream stream( text );

    JavaLexer lexer( stream );
    lexer.setFilename( fn );
    lexer.setProblemReporter( d->problemReporter );

    JavaRecognizer parser( lexer );
    parser.setFilename( fn );
    parser.setProblemReporter( d->problemReporter );

    try{
        lexer.resetErrors();
        parser.resetErrors();

        parser.compilationUnit();
        int errors = lexer.numberOfErrors() + parser.numberOfErrors();

    } catch( antlr::ANTLRException& ex ){
        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        d->problemReporter->reportError( ex.what(),
                                         fileName,
                                         lexer.getLine(),
                                         lexer.getColumn() );
    }
}



void JavaSupportPart::addClass()
{
    static AddClassInfo info;
    kdDebug() << "New JAVA class" << endl;

    AddClass ac;

    info.className = "org.kde.koala.TestClass";
    info.projectDir = project()->projectDirectory();
    info.sourceDir = "src";

    ac.setInfo(info);

    QStringList baseClasses = classStore()->getSortedClassNameList();
    baseClasses.prepend("java.lang.Object");
    ac.setBaseClasses(baseClasses);

    if (ac.showDialog())
    {
        info = ac.info();

        if (ac.generate())
            project()->addFile ( info.javaFileName() );
    }
}

void JavaSupportPart::savedFile( const QString& fileName )
{
    kdDebug() << "JavaSupportPart::savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse( fileName );
        emit updatedSourceInfo();
    }
}

#include "javasupportpart.moc"

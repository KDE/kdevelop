
#include <qfileinfo.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qprogressbar.h>
#include <qwhatsthis.h>

#include <kgenericfactory.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kdialogbase.h>
#include <kiconloader.h>

#include <fstream>
#include <sstream>

#include "kdevgenericfactory.h"
#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "codemodel.h"
#include "adasupportpart.h"
#include "problemreporter.h"
#include "backgroundparser.h"

#include "AdaLexer.hpp"
#include "AdaParser.hpp"
#include "AdaStoreWalker.hpp"
#include "AdaAST.hpp"

#include <kdevplugininfo.h>

enum { KDEV_DB_VERSION = 6 };
enum { KDEV_PCS_VERSION = 6 };

typedef KDevGenericFactory<AdaSupportPart> AdaSupportPartFactory;

static const KDevPluginInfo data("kdevadasupport");
K_EXPORT_COMPONENT_FACTORY (libkdevadasupport, AdaSupportPartFactory (data))


struct AdaSupportPartData {
    ProblemReporter* problemReporter;

    AdaSupportPartData () : problemReporter (0) {}
};

AdaSupportPart::AdaSupportPart (QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport (&data, parent, name ? name : "AdaSupportPart"), d (new AdaSupportPartData())
{
    setInstance (AdaSupportPartFactory::instance ());

    d->problemReporter = new ProblemReporter (this);
//    connect (core (), SIGNAL (configWidget (KDialogBase*)),
//             d->problemReporter, SLOT (configWidget (KDialogBase*)));
    d->problemReporter->setIcon( SmallIcon("info") );
    mainWindow( )->embedOutputView( d->problemReporter, i18n("Problems"), i18n("Problem reporter"));
    QWhatsThis::add(d->problemReporter, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
        "It displays errors reported by a language parser."));

    setXMLFile ("adasupportpart.rc");

    connect (core (), SIGNAL (projectOpened ()), this, SLOT (projectOpened ()));
    connect (core (), SIGNAL (projectClosed ()), this, SLOT (projectClosed ()));

    connect (partController (), SIGNAL (savedFile (const KURL&)),
             this, SLOT (savedFile (const KURL&)));

//    connect (core (), SIGNAL (configWidget (KDialogBase*)), this, SLOT (configWidget (KDialogBase*)));
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             d->problemReporter, SLOT(configWidget(KDialogBase*)) );

    // a small hack (robe)
    //classStore ()->globalScope ()->setName ("(default packages)");
    //classStore ()->addScope (classStore ()->globalScope ());
    //classStore ()->globalScope ()->setName (QString::null);
}


AdaSupportPart::~AdaSupportPart ()
{
    mainWindow ()->removeView (d->problemReporter);
    delete (d->problemReporter);
    d->problemReporter = 0;

    delete (d);
    d = 0;
}


KDevLanguageSupport::Features AdaSupportPart::features ()
{
    return KDevLanguageSupport::Features
        (  // TBD: Classes |
	 Functions | Namespaces);
}

void AdaSupportPart::projectOpened ()
{
    connect (project (), SIGNAL (addedFilesToProject (const QStringList &)),
            this, SLOT (addedFilesToProject (const QStringList &)));
    connect (project (), SIGNAL (removedFilesFromProject (const QStringList &)),
            this, SLOT (removedFilesFromProject (const QStringList &)));
    connect( project( ), SIGNAL( changedFilesInProject( const QStringList & ) ),
             this, SLOT( changedFilesInProject( const QStringList & ) ) );

    QTimer::singleShot (0, this, SLOT (initialParse ()));
}


void AdaSupportPart::projectClosed ()
{
    saveProjectSourceInfo();
}


void AdaSupportPart::initialParse ()
{
    kdDebug () << "------------------------------------------> initialParse ()" << endl;

    if (project ())
    {
        mainWindow()->statusBar()->message( i18n("Updating...") );
        kapp->processEvents( );
        kapp->setOverrideCursor (waitCursor);

        int n = 0;
        QStringList files = project ()->allFiles ();

        QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
        bar->setMinimumWidth( 120 );
        bar->setCenterIndicator( true );
        mainWindow( )->statusBar( )->addWidget( bar );
        bar->show( );

        for (QStringList::Iterator it = files.begin (); it != files.end (); ++it) {
            bar->setProgress( n++ );

            QString fn = project ()->projectDirectory () + "/" + *it;
            maybeParse (fn);
            kapp->processEvents (500);
        }

        emit updatedSourceInfo();

        mainWindow( )->statusBar( )->removeWidget( bar );
        delete bar;

        kapp->restoreOverrideCursor ();
        mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );
/*        mainWindow ()->statusBar ()->message
            (i18n ("Found 1 problem", "Found %n problems", d->problemReporter->childCount ()));*/
    }
}

QStringList AdaSupportPart::fileExtensions ()
{
    return QStringList () << "ads" << "adb";
}

void AdaSupportPart::maybeParse (const QString &fileName)
{
    kdDebug () << "AdaSupportPart::maybeParse: " << fileName << endl;

    if (!fileExtensions ().contains (QFileInfo (fileName).extension ()))
        return;

//    mainWindow ()->statusBar ()->message (i18n ("Parsing file: %1").arg (fileName));
    parse (fileName);
}


void AdaSupportPart::addedFilesToProject (const QStringList &fileList)
{
        QStringList::ConstIterator it;

        for (it = fileList.begin (); it != fileList.end (); ++it)
        {
            QString path = project ()->projectDirectory () + "/" + (*it);
            maybeParse (path);
            emit addedSourceInfo( path );
        }
}


void AdaSupportPart::removedFilesFromProject (const QStringList &fileList)
{
        QStringList::ConstIterator it;

        for (it = fileList.begin (); it != fileList.end (); ++it)
        {
            kdDebug () << "AdaSupportPart::removedFileFromProject () -- " << (*it) << endl;
            QString path = project ()->projectDirectory () + "/" + (*it);

            if( codeModel()->hasFile(path) )
            {
                emit aboutToRemoveSourceInfo( path );
                codeModel()->removeFile( codeModel()->fileByName(path) );
            }
        }

//        emit updatedSourceInfo();
}


void AdaSupportPart::parse (const QString &fileName)
{
    kdDebug () << "AdaSupportPart::parse () -- " << fileName << endl;

    std::ifstream stream (QFile::encodeName( fileName ).data());
    QCString _fn = fileName.utf8 ();
    std::string fn (_fn.data ());

    AdaLexer lexer (stream);
    lexer.setFilename (fn);
    lexer.setProblemReporter (d->problemReporter);

    AdaParser parser (lexer);
    parser.setFilename (fn);
    parser.setProblemReporter (d->problemReporter);

    // make an ast factory
    antlr::ASTFactory ast_factory;
    // initialize and put it in the parser...
    parser.initializeASTFactory (ast_factory);
    parser.setASTFactory (&ast_factory);
    // parser.setASTNodeType ("RefAdaAST");

    try {
        // old: parser.setASTNodeFactory (AdaAST::factory);
        lexer.resetErrors ();
        parser.resetErrors ();

        parser.compilation_unit ();
        int errors = lexer.numberOfErrors () + parser.numberOfErrors ();
        RefAdaAST ast = parser.getAST ();

        if (errors == 0 && ast != antlr::nullAST) {
            kdDebug () << "-------------------> start StoreWalker" << endl;
            AdaStoreWalker walker;
            walker.setFileName (fileName);
            walker.setCodeModel (codeModel ());
            walker.compilation_unit (ast);
        }
    } catch (antlr::ANTLRException& ex) {
        kdDebug () << "*exception*: " << ex.toString ().c_str () << endl;
        d->problemReporter->reportError (QString::fromLatin1( ex.getMessage ().c_str() ),
                                         fileName,
                                         lexer.getLine (),
                                         lexer.getColumn ());
    }
}

void AdaSupportPart::parseContents (const QString& contents, const QString& fileName)
{
    kdDebug () << "AdaSupportPart::parseContents () -- " << fileName << endl;

    QCString _fn = QFile::encodeName (fileName);
    std::string fn (_fn.data ());

    QCString text = contents.utf8 ();
    std::istringstream stream ((const char *)text);

    AdaLexer lexer (stream);
    lexer.setFilename (fn);
    lexer.setProblemReporter (d->problemReporter);

    AdaParser parser (lexer);
    parser.setFilename (fn);
    parser.setProblemReporter (d->problemReporter);

    try {
        lexer.resetErrors ();
        parser.resetErrors ();

        parser.compilation_unit ();
        int errors = lexer.numberOfErrors () + parser.numberOfErrors ();
        Q_UNUSED( errors );

    } catch (antlr::ANTLRException& ex) {
        kdDebug () << "*exception*: " << ex.toString ().c_str () << endl;
        d->problemReporter->reportError (QString::fromLatin1( ex.getMessage().c_str() ),
                                         fileName,
                                         lexer.getLine (),
                                         lexer.getColumn ());
    }
}



void AdaSupportPart::savedFile (const KURL& fileName)
{
    kdDebug () << "AdaSupportPart::savedFile ()" << endl;

    if (project ()->allFiles ().contains (fileName.path().mid (project ()->projectDirectory ().length () + 1))) {
        maybeParse (fileName.path());
        emit updatedSourceInfo();
    }
}

KMimeType::List AdaSupportPart::mimeTypes( )
{
    KMimeType::List list;

    list << KMimeType::mimeType( "text/x-adasrc" );

    return list;
}

//@todo adymo: implement source info loading and saving
//hint: check javasupport for an example
//      and modify initialParse() method
void AdaSupportPart::saveProjectSourceInfo( )
{
/*    const FileList fileList = codeModel()->fileList();

    if( !project() || fileList.isEmpty() )
        return;

    QFile f( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" );
    if( !f.open( IO_WriteOnly ) )
        return;

    QDataStream stream( &f );
    QMap<QString, Q_ULONG> offsets;

    QString pcs( "PCS" );
    stream << pcs << KDEV_PCS_VERSION;

    stream << int( fileList.size() );
    for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it ){
        const FileDom dom = (*it);
#if QT_VERSION >= 0x030100
        stream << dom->name() << m_timestamp[ dom->name() ].toTime_t();
#else
        stream << dom->name() << toTime_t(m_timestamp[ dom->name() ]);
#endif
        offsets.insert( dom->name(), stream.device()->at() );
        stream << (Q_ULONG)0; // dummy offset
    }

    for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it ){
        const FileDom dom = (*it);
        int offset = stream.device()->at();

        dom->write( stream );

        int end = stream.device()->at();

        stream.device()->at( offsets[dom->name()] );
        stream << offset;
        stream.device()->at( end );
    }*/
}

void AdaSupportPart::changedFilesInProject( const QStringList & fileList )
{
    QStringList files = fileList;

    for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
        QString path = project ()->projectDirectory () + "/" + *it ;

        maybeParse( path );
        emit addedSourceInfo( path );
    }
}


#include "adasupportpart.moc"


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
#include "adasupportpart.h"
#include "problemreporter.h"

#include "AdaLexer.hpp"
#include "AdaParser.hpp"
// #include "AdaStoreWalker.hpp"
#include "AdaAST.hpp"


typedef KGenericFactory<AdaSupportPart> AdaSupportPartFactory;

K_EXPORT_COMPONENT_FACTORY (libkdevadasupport, AdaSupportPartFactory ("kdevadasupport"));


struct AdaSupportPartData {
    ProblemReporter* problemReporter;

    AdaSupportPartData () : problemReporter (0) {}
};

AdaSupportPart::AdaSupportPart (QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport ("AdaSupport", "ada", parent, name ? name : "AdaSupportPart"), d (new AdaSupportPartData())
{
    setInstance (AdaSupportPartFactory::instance ());

    d->problemReporter = new ProblemReporter (this);
    connect (core (), SIGNAL (configWidget (KDialogBase*)),
             d->problemReporter, SLOT (configWidget (KDialogBase*)));


    setXMLFile ("adasupportpart.rc");

    connect (core (), SIGNAL (projectOpened ()), this, SLOT (projectOpened ()));
    connect (core (), SIGNAL (projectClosed ()), this, SLOT (projectClosed ()));

    connect (partController (), SIGNAL (savedFile (const QString&)),
             this, SLOT (savedFile (const QString&)));

    mainWindow ()->embedOutputView (d->problemReporter, i18n ("Problems"), i18n ("problem reporter"));

    connect (core (), SIGNAL (configWidget (KDialogBase*)), this, SLOT (configWidget (KDialogBase*)));

    // a small hack (robe)
    //classStore ()->globalScope ()->setName ("(default packages)");
    //classStore ()->addScope (classStore ()->globalScope ());
    //classStore ()->globalScope ()->setName (QString::null);
}


AdaSupportPart::~AdaSupportPart ()
{
    mainWindow ()->removeView (d->problemReporter);
    delete (d->problemReporter);

    delete (d);
    d = 0;
}


KDevLanguageSupport::Features AdaSupportPart::features ()
{
    return KDevLanguageSupport::Features
        (Classes | Structs | Functions | Variables | Namespaces | Declarations);
}


QStringList AdaSupportPart::fileFilters ()
{
    QStringList r;
    r << "*.ads" << "*.adb";
    return r;
}


void AdaSupportPart::projectOpened ()
{
    connect (project (), SIGNAL (addedFilesToProject (const QStringList &)),
            this, SLOT (addedFilesToProject (const QStringList &)));
    connect (project (), SIGNAL (removedFilesFromProject (const QStringList &)),
            this, SLOT (removedFilesFromProject (const QStringList &)));

    QTimer::singleShot (0, this, SLOT (initialParse ()));
}


void AdaSupportPart::projectClosed ()
{
}


void AdaSupportPart::initialParse ()
{
    kdDebug () << "------------------------------------------> initialParse ()" << endl;

    if (project ())
    {
        kapp->setOverrideCursor (waitCursor);

        // TODO: Progress indicator!

        QStringList files = project ()->allFiles ();
        for (QStringList::Iterator it = files.begin (); it != files.end (); ++it) {
            QString fn = project ()->projectDirectory () + "/" + *it;
            maybeParse (fn);
            kapp->processEvents (500);
        }

        emit updatedSourceInfo ();

        kapp->restoreOverrideCursor ();
        mainWindow ()->statusBar ()->message
            (i18n ("Found 1 problem", "Found %n problems", d->problemReporter->childCount ()));
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

    mainWindow ()->statusBar ()->message (i18n ("Parsing file: %1").arg (fileName));
    parse (fileName);
}


void AdaSupportPart::addedFilesToProject (const QStringList &fileList)
{
        QStringList::ConstIterator it;

        for (it = fileList.begin (); it != fileList.end (); ++it)
        {
            QString path = project ()->projectDirectory () + "/" + (*it);
            maybeParse (path);
        }

        emit updatedSourceInfo ();
}


void AdaSupportPart::removedFilesFromProject (const QStringList &fileList)
{
        QStringList::ConstIterator it;

        for (it = fileList.begin (); it != fileList.end (); ++it)
        {
            kdDebug () << "AdaSupportPart::removedFileFromProject () -- " << (*it) << endl;
            QString path = project ()->projectDirectory () + "/" + (*it);
            classStore ()->removeWithReferences (path);
        }

        emit updatedSourceInfo ();
}


void AdaSupportPart::parse (const QString &fileName)
{
    kdDebug () << "AdaSupportPart::parse () -- " << fileName << endl;

    std::ifstream stream (fileName);
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
/* TBD
            AdaStoreWalker walker;
            walker.setFileName (fileName);
            walker.setClassStore (classStore ());
            walker.compilation_unit (ast);
 */
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
    std::istrstream stream (text);

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

    } catch (antlr::ANTLRException& ex) {
        kdDebug () << "*exception*: " << ex.toString ().c_str () << endl;
        d->problemReporter->reportError (QString::fromLatin1( ex.getMessage().c_str() ),
                                         fileName,
                                         lexer.getLine (),
                                         lexer.getColumn ());
    }
}



void AdaSupportPart::savedFile (const QString& fileName)
{
    kdDebug () << "AdaSupportPart::savedFile ()" << endl;

    if (project ()->allFiles ().contains (fileName.mid (project ()->projectDirectory ().length () + 1))) {
        maybeParse (fileName);
        emit updatedSourceInfo ();
    }
}

#include "adasupportpart.moc"

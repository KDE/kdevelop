/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002-2003 by Roberto Raggi                              *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cppsupportpart.h"
#include "cppsupport_events.h"
#include "problemreporter.h"
#include "implementmethodsdialog.h"
#include "backgroundparser.h"
#include "store_walker.h"
#include "ast.h"
#include "ast_utils.h"
#include "realtime_classbrowser.h"
#include "cppnewclassdlg.h"
#include "cppcodecompletion.h"
#include "ccconfigwidget.h"
#include "subclassingdlg.h"
#include "addmethoddialog.h"
#include "addattributedialog.h"
#include "KDevCppSupportIface.h"
#include "cppsupportfactory.h"
#include "classgeneratorconfig.h"
#include "catalog.h"
#include "cpp_tags.h"
#include "kdevdriver.h"

#include <qheader.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qdom.h>
#include <qfileinfo.h>
#include <qguardedptr.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qvbox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
#  include <ktexteditor/texthintinterface.h>
# else
#  include <kde30x_texthintinterface.h>
# endif
#else
#  include <kde30x_texthintinterface.h>
#endif

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <classstore.h>
#include <kdevpartcontroller.h>
#include <kdevmakefrontend.h>
#include <kdevcoderepository.h>

#include <parsedclass.h>
#include <parsedattribute.h>
#include <parsedmethod.h>
#include <domutil.h>
#include <config.h>

#if defined(__GLIBC__)
#include <malloc.h>
void showMemUsage()
{
    struct mallinfo mi = mallinfo();
    kdDebug(9007) << "Mem usage: " << mi.uordblks << endl;
}
#else
void showMemUsage()
{}
#endif

enum { PCS_VERSION = 1 };

class CppDriver: public KDevDriver
{
public:
    CppDriver( CppSupportPart* cppSupport )
	: KDevDriver( cppSupport )
    {
    }
    
    void fileParsed( const QString& fileName )
    {
	kdDebug(9007) << "-----> file " << fileName << " parsed!" << endl;
	TranslationUnitAST::Node ast = takeTranslationUnit( fileName );
	cppSupport()->classStore()->removeWithReferences( fileName );
	StoreWalker walker( fileName, cppSupport()->classStore() );
	walker.parseTranslationUnit( ast.get() );
	remove( fileName );
    }
};

CppSupportPart::CppSupportPart(QObject *parent, const char *name, const QStringList &args)
    : KDevLanguageSupport("CppSupport", "cpp", parent, name ? name : "KDevCppSupport"),
      m_activeSelection( 0 ), m_activeEditor( 0 ),
      m_activeViewCursor( 0 ), m_projectClosed( true ), m_valid( false )
{
    setInstance(CppSupportFactory::instance());

    setXMLFile("kdevcppsupport.rc");

    m_catalogList.setAutoDelete( true );
    m_backgroundParser = 0;
    setupCatalog();

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)));
    connect( partController(), SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(partRemoved(KParts::Part*)));

    m_problemReporter = new ProblemReporter( this );
    mainWindow( )->embedOutputView( m_problemReporter, i18n("Problems"), i18n("problem reporter"));

#ifdef ENABLE_FILE_STRUCTURE
    m_structureView = new KListView();
    QFont f = m_structureView->font();
    f.setPointSize( 8 );
    m_structureView->setFont( f );
    m_structureView->setSorting( 0 );
    m_structureView->addColumn( "" );
    m_structureView->header()->hide();
    mainWindow()->embedSelectViewRight( m_structureView, i18n("File Structure"), i18n("Show the structure for the current source unit") );
    connect( m_structureView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotNodeSelected(QListViewItem*)) );
    connect( m_structureView, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(slotNodeSelected(QListViewItem*)) );
#endif

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             m_problemReporter, SLOT(configWidget(KDialogBase*)) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );

    KAction *action;

    action = new KAction(i18n("Switch Header/Implementation"), SHIFT+Key_F12,
                         this, SLOT(slotSwitchHeader()),
                         actionCollection(), "edit_switchheader");
    action->setStatusText( i18n("Switch between header and implementation files") );
    action->setWhatsThis( i18n("Switch between header and implementation files\n\n"
                               "If you are currently looking at a header file, this "
                               "brings you to the corresponding implementation file. "
                               "If you are looking at an implementation file (.cpp etc.), "
                               "this brings you to the corresponding header file.") );
    action->setEnabled(false);

    action = new KAction(i18n("Complete Text"), CTRL+Key_Space,
                         this, SLOT(slotCompleteText()),
                         actionCollection(), "edit_complete_text");
    action->setStatusText( i18n("Complete current expression") );
    action->setWhatsThis( i18n("Complete current expression") );
    action->setEnabled(false);

    action = new KAction(i18n("Make Member"), "makermember", Key_F2,
                         this, SLOT(slotMakeMember()),
                         actionCollection(), "edit_make_member");

    action = new KAction(i18n("New Class..."), "classnew", 0,
                         this, SLOT(slotNewClass()),
                         actionCollection(), "project_newclass");
    action->setStatusText( i18n("Generate a new class") );
    action->setWhatsThis( i18n("Generate a new class") );

    m_pCompletion  = 0;

    withcpp = false;
    if ( args.count() == 1 && args[ 0 ] == "Cpp" )
        withcpp = true;

    // daniel
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    m_bEnableCC = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/codecompletion/enablecc", true );


    QDomElement element = projectDom( )->documentElement( )
                          .namedItem( "cppsupportpart" ).toElement( )
                          .namedItem( "codecompletion" ).toElement( );
    new KDevCppSupportIface( this );
    //(void) dcopClient();
}


CppSupportPart::~CppSupportPart()
{
    if( m_backgroundParser ){
	//    while( m_backgroundParser->filesInQueue() > 0 )
	//       m_backgroundParser->isEmpty().wait();
	//m_backgroundParser->reparse();
	m_backgroundParser->close();
	m_backgroundParser->wait();
	delete m_backgroundParser;
	m_backgroundParser = 0;
    }

    QPtrListIterator<Catalog> it( m_catalogList );
    while( Catalog* catalog = it.current() ){
        ++it;
        codeRepository()->unregisterCatalog( catalog );
    }

    mainWindow( )->removeView( m_problemReporter );
#ifdef ENABLE_FILE_STRUCTURE
    mainWindow()->removeView( m_structureView );
#endif

    delete m_pCompletion;
#ifdef ENABLE_FILE_STRUCTURE
    delete m_structureView;
#endif
    delete m_problemReporter;
}

void CppSupportPart::customEvent( QCustomEvent* ev )
{
    kdDebug(9007) << "CppSupportPart::customEvent()" << endl;
    
    if( ev->type() == int(Event_FileParsed) ){
	FileParsedEvent* event = (FileParsedEvent*) ev;
	QString fileName = event->fileName();
	
	kdDebug(9007) << "----------> file " << fileName << " parsed" << endl; 

        if( m_problemReporter ){
	    m_problemReporter->removeAllErrors( fileName );

	    QValueList<Problem> problems = event->problems();
	    QValueList<Problem>::ConstIterator it = problems.begin();
	    while( it != problems.end() ){
	        const Problem& p = *it++;
	        m_problemReporter->reportError( p.text(), fileName, p.line(), p.column() );
	    }

	    m_backgroundParser->lock();
	    if( TranslationUnitAST* ast = m_backgroundParser->translationUnit(fileName) ){
		classStore()->removeWithReferences( fileName );
		StoreWalker walker( fileName, classStore() );
		walker.parseTranslationUnit( ast );
		
#ifdef ENABLE_FILE_STRUCTURE
		if( fileName == m_activeFileName ){
		    RTClassBrowser b( fileName, m_structureView );
		    b.parseTranslationUnit( ast );
		}
#endif
		
	    }
	    m_backgroundParser->unlock();	    
	}	
    }
}

// daniel
void CppSupportPart::projectConfigWidget( KDialogBase* dlg )
{
    QVBox* vbox = dlg->addVBoxPage( i18n( "C++ Specific" ) );
    CCConfigWidget* w = new CCConfigWidget( this, vbox );
    connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );

    connect( w, SIGNAL( enableCodeCompletion( bool ) ),
             this, SLOT( slotEnableCodeCompletion( bool ) ) );

}

void CppSupportPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("C++ New Class Generator"));
  ClassGeneratorConfig *w = new ClassGeneratorConfig(vbox, "classgenerator config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(storeConfig()));
}

void
CppSupportPart::slotEnableCodeCompletion( bool setEnable )
{
    kdDebug( 9007 ) << "slotEnableCodeCompletion" << endl;

    if( m_pCompletion )
        m_pCompletion->setEnabled( setEnable );
}

void CppSupportPart::activePartChanged(KParts::Part *part)
{
    kdDebug(9032) << "CppSupportPart::activePartChanged()" << endl;

    bool enabled = false;

#ifdef ENABLE_FILE_STRUCTURE
    m_structureView->clear();
#endif

    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(part);
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
    m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
    m_activeViewCursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() ) : 0;

    m_activeFileName = QString::null;

    if (doc) {
	m_activeFileName = doc->url().path();
        QFileInfo fi(doc->url().path());
        QString ext = fi.extension();
        if (fileExtensions().contains(ext))
            enabled = true;
    }

    actionCollection()->action("edit_switchheader")->setEnabled(enabled);
    actionCollection()->action("edit_complete_text")->setEnabled(enabled);
    actionCollection()->action("edit_make_member")->setEnabled(enabled);

    if( !part )
	return;

    KTextEditor::View* view = dynamic_cast<KTextEditor::View*>( part->widget() );
    if( !view )
	return;

    KTextEditor::TextHintInterface* textHintIface = dynamic_cast<KTextEditor::TextHintInterface*>( view );
    if( !textHintIface )
	return;

    connect( view, SIGNAL(needTextHint(int,int,QString&)),
	     this, SLOT(slotNeedTextHint(int,int,QString&)) );

    textHintIface->enableTextHints( 1000 );
}


void
CppSupportPart::projectOpened( )
{
    kdDebug( 9007 ) << "projectOpened( )" << endl;

    connect( project( ), SIGNAL( addedFilesToProject( const QStringList & ) ),
             this, SLOT( addedFilesToProject( const QStringList & ) ) );
    connect( project( ), SIGNAL( removedFilesFromProject( const QStringList &) ),
             this, SLOT( removedFilesFromProject( const QStringList & ) ) );
    connect( project( ), SIGNAL( changedFilesInProject( const QStringList & ) ),
             this, SLOT( changedFilesInProject( const QStringList & ) ) );
    connect( project(), SIGNAL(projectCompiled()),
	     this, SLOT(slotProjectCompiled()) );

    QDir::setCurrent( project()->projectDirectory() );

    m_timestamp.clear();
    m_pCompletion = new CppCodeCompletion( this );
    m_projectClosed = false;

    m_backgroundParser = new BackgroundParser( this, &m_eventConsumed );
    m_backgroundParser->start();

    QTimer::singleShot( 500, this, SLOT( initialParse( ) ) );
}


void
CppSupportPart::projectClosed( )
{
    kdDebug( 9007 ) << "projectClosed( )" << endl;
    
    if( m_backgroundParser )
	m_backgroundParser->removeAllFiles();

    delete m_pCompletion;
    m_pCompletion = 0;
    m_projectClosed = true;
}


static QString findHeader(const QStringList &list, const QString &header)
{
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QString s = *it;
        int pos = s.findRev('.');
        if (pos != -1)
            s = s.left(pos) + ".h";
        if (s.right(header.length()) == header)
            return s;
    }

    return QString::null;
}


void CppSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType("editor"))
        return;

    popup->insertSeparator();
    popup->insertItem( i18n( "Switch Header/Implementation"),
        this, SLOT( slotSwitchHeader() ) );

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString str = econtext->currentLine();
    if (str.isEmpty())
        return;

    QRegExp re("[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*");
    if (!re.exactMatch(str))
        return;

    QString popupstr = re.cap(1);
    QStringList projectFileList = project()->allFiles();
    m_contextFileName = findHeader(projectFileList, popupstr);
    if (m_contextFileName.isEmpty())
        return;

    popup->insertItem( i18n("Goto Include File: %1").arg(popupstr),
                       this, SLOT(slotGotoIncludeFile()) );
}


// Makes sure that header files come first
static QStringList reorder(const QStringList &list)
{
    QStringList headers, others;

    QStringList headerExtensions = QStringList::split(",", "h,H,hh,hxx,hpp,tlh");

    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        if (headerExtensions.contains(QFileInfo(*it).extension()))
            headers << (*it);
        else
            others << (*it);

    return headers + others;
}




void CppSupportPart::addedFilesToProject(const QStringList &fileList)
{
    QStringList::ConstIterator it;
    QDir d( project()->projectDirectory() );

    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
	QFileInfo fileInfo( d, *it );
	kdDebug(9007) << "addedFilesToProject(): " << fileInfo.absFilePath() << endl;

	// changed - daniel
	QString path = fileInfo.absFilePath();
	maybeParse( path );

	//partController()->editDocument ( KURL ( path ) );
    }

    emit updatedSourceInfo();
}


void CppSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    QStringList::ConstIterator it;
    QDir d( project()->projectDirectory() );

    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
	QFileInfo fileInfo( d, *it );
	kdDebug(9007) << "removedFilesFromProject(): " << fileInfo.absFilePath() << endl;

	QString path = fileInfo.absFilePath();
	classStore()->removeWithReferences(path);
	m_backgroundParser->removeFile( path );
    }

    emit updatedSourceInfo();
}

void CppSupportPart::changedFilesInProject( const QStringList & fileList )
{
    QStringList::ConstIterator it;
    QDir d( project()->projectDirectory() );

    for ( it = fileList.begin(); it != fileList.end(); ++it )
    {
        QFileInfo fileInfo( d, *it );
        kdDebug(9007) << "changedFilesInProject() " << fileInfo.absFilePath() << endl;
        maybeParse( fileInfo.absFilePath() );
    }
    emit updatedSourceInfo();
}

void CppSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9007) << "savedFile(): " << fileName.mid ( project()->projectDirectory().length() + 1 ) << endl;

    QStringList projectFileList = project()->allFiles();
    if (projectFileList.contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
	// changed - daniel
	maybeParse( fileName );
	emit updatedSourceInfo();
    }
}

QString CppSupportPart::findSourceFile()
{
    QFileInfo fi( m_activeFileName );
    QString path = fi.filePath();
    QString ext = fi.extension();
    QString base = path.left( path.length() - ext.length() );
    QStringList candidates;

    if (ext == "h" || ext == "H" || ext == "hh" || ext == "hxx" || ext == "hpp" || ext == "tlh") {
        candidates << (base + "c");
        candidates << (base + "cc");
        candidates << (base + "cpp");
        candidates << (base + "c++");
        candidates << (base + "cxx");
        candidates << (base + "C");
        candidates << (base + "m");
        candidates << (base + "mm");
        candidates << (base + "M");
	candidates << (base + "inl");
    }

    QStringList::ConstIterator it;
    for (it = candidates.begin(); it != candidates.end(); ++it) {
        kdDebug(9007) << "Trying " << (*it) << endl;
        if (QFileInfo(*it).exists()) {
            return *it;
        }
    }

    return m_activeFileName;
}


void CppSupportPart::slotSwitchHeader()
{
    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(partController()->activePart());
    if (!doc)
      return;

    QFileInfo fi(doc->url().path());
    QString path = fi.filePath();
    QString ext = fi.extension();
    QString base = path.left(path.length()-ext.length());
    kdDebug(9007) << "base: " << base << ", ext: " << ext << endl;
    QStringList candidates;
    if (ext == "h" || ext == "H" || ext == "hh" || ext == "hxx" || ext == "hpp" || ext == "tlh") {
        candidates << (base + "c");
        candidates << (base + "cc");
        candidates << (base + "cpp");
        candidates << (base + "c++");
        candidates << (base + "cxx");
        candidates << (base + "C");
        candidates << (base + "m");
        candidates << (base + "mm");
        candidates << (base + "M");
	candidates << (base + "inl");
    } else if (QStringList::split(',', "c,cc,cpp,c++,cxx,C,m,mm,M,inl").contains(ext)) {
        candidates << (base + "h");
        candidates << (base + "H");
        candidates << (base + "hh");
        candidates << (base + "hxx");
        candidates << (base + "hpp");
        candidates << (base + "tlh");
    }

    QStringList::ConstIterator it;
    for (it = candidates.begin(); it != candidates.end(); ++it) {
        kdDebug(9007) << "Trying " << (*it) << endl;
        if (QFileInfo(*it).exists()) {
            partController()->editDocument(*it);
            return;
        }
    }
}

void CppSupportPart::slotGotoIncludeFile()
{
    if (!m_contextFileName.isEmpty())
        partController()->editDocument(m_contextFileName, 0);

}

KDevLanguageSupport::Features CppSupportPart::features()
{
    if (withcpp)
        return Features(Classes | Structs | Functions | Variables | Namespaces | Declarations
                        | Signals | Slots | AddMethod | AddAttribute);
    else
        return Features (Structs | Functions | Variables | Declarations);
}

QString CppSupportPart::formatClassName(const QString &name)
{
    QString res = name;
    res.replace(QRegExp("\\."), "::");
    return res;
}

QString CppSupportPart::unformatClassName(const QString &name)
{
    QString res = name;
    res.replace(QRegExp("::"), ".");
    return res;
}

QStringList CppSupportPart::fileExtensions() const
{
    if (withcpp)
        return QStringList::split(",", "c,C,cc,cpp,c++,cxx,m,mm,M,h,H,hh,hxx,hpp,inl,tlh,diff,ui.h");
    else
        return QStringList::split(",", "c,h");
}

void CppSupportPart::slotNewClass()
{
    CppNewClassDialog dlg(this);
    dlg.exec();
}

void CppSupportPart::addMethod(const QString &className)
{
    ParsedClass* pc = classStore()->getClassByName( className );
    if (!pc) {
	QMessageBox::critical(0,i18n("Error"),i18n("Please select a class!"));
	return;
    }

    AddMethodDialog dlg( this, pc, mainWindow()->main() );
    dlg.exec();
}

void CppSupportPart::addAttribute(const QString &className)
{
    ParsedClass *pc = classStore()->getClassByName(className);

    if (!pc) {
	QMessageBox::critical(0,i18n("Error"),i18n("Please select a class!"));
	return;
    }

    AddAttributeDialog dlg( this, pc, mainWindow()->main() );
    dlg.exec();
}

void CppSupportPart::slotCompleteText()
{
    m_pCompletion->completeText();
}

/**
 * parsing stuff for project persistant classstore and code completion
 */
void
CppSupportPart::initialParse( )
{
    // For debugging
    if( !project( ) ){
        // messagebox ?
        kdDebug( 9007 ) << "No project" << endl;
        return;
    }

    parseProject( );
    emit updatedSourceInfo();
    m_valid = true;
    return;
}

bool
CppSupportPart::parseProject( )
{
    //QLabel* label = new QLabel( "", mainWindow( )->statusBar( ) );
    //label->setMinimumWidth( 600 );
    //mainWindow( )->statusBar( )->addWidget( label );
    //label->show( );

    mainWindow()->statusBar()->message( i18n("Updating...") );

    kapp->processEvents( );
    kapp->setOverrideCursor( waitCursor );

    QStringList files = reorder( modifiedFileList() );

    QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
    bar->setMinimumWidth( 120 );
    bar->setCenterIndicator( true );
    mainWindow( )->statusBar( )->addWidget( bar );
    bar->show( );

    int n = 0;
    QDir d( project()->projectDirectory() );

    CppDriver driver( this );
    
    for( QStringList::Iterator it = files.begin( ); it != files.end( ); ++it ) {
        bar->setProgress( n++ );
	QFileInfo fileInfo( d, *it );

        if( fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() ){
            QString absFilePath = fileInfo.absFilePath();
            //kdDebug(9007) << "parse file" << absFilePath << endl;

            driver.parseFile( absFilePath );

	    if( (n%5) == 0 )
	        kapp->processEvents();
        }

	if( m_projectClosed ){
	    kdDebug(9007) << "ABORT" << endl;
            kapp->restoreOverrideCursor( );
	    return false;
	}
    }

    kdDebug( 9007 ) << "updating sourceinfo" << endl;
    emit updatedSourceInfo();

    mainWindow( )->statusBar( )->removeWidget( bar );
    delete bar;
    //mainWindow( )->statusBar( )->removeWidget( label );
    //delete label;

    kapp->restoreOverrideCursor( );
    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    return true;
}

void
CppSupportPart::maybeParse( const QString& fileName )
{
    if( !fileExtensions( ).contains( QFileInfo( fileName ).extension( ) ) )
        return;

    QFileInfo fileInfo( fileName );
    QDateTime t = fileInfo.lastModified();

    if( !fileInfo.exists() ){
	classStore()->removeWithReferences( fileName );
	return;
    }

    QMap<QString, QDateTime>::Iterator it = m_timestamp.find( fileName );
    if( it != m_timestamp.end() && *it == t ){
	return;
    }

    m_timestamp[ fileName ] = t;

    m_backgroundParser->addFile( fileName );
}

void CppSupportPart::implementVirtualMethods( const QString& className )
{
    ParsedClass *pc = classStore()->getClassByName(className);

    if (!pc) {
	QMessageBox::critical(0,i18n("Error"),i18n("Please select a class!"));
	return;
    }

    ImplementMethodsDialog dlg( 0, "implementMethodsDlg" );
    dlg.setPart( this );
    if( !dlg.implementMethods(pc) )
      return;

    KMessageBox::sorry( 0, i18n("Not implemented yet ;)"), i18n("Sorry") );
}

void CppSupportPart::slotNeedTextHint( int line, int column, QString& textHint )
{
    if( 1 || !m_activeEditor )
	return;

    // sync
    //while( m_backgroundParser->filesInQueue() > 0 )
    //     m_backgroundParser->isEmpty().wait();

    m_backgroundParser->lock();
    TranslationUnitAST* ast = m_backgroundParser->translationUnit( m_activeFileName );
    AST* node = 0;
    if( ast && (node = findNodeAt(ast, line, column)) ){

	while( node && node->nodeType() != NodeType_FunctionDefinition )
	    node = node->parent();

	if( node ){
	    int startLine, startColumn;
	    int endLine, endColumn;
	    node->getStartPosition( &startLine, &startColumn );
	    node->getEndPosition( &endLine, &endColumn );

	    if( !node->text().isNull() )
	        textHint = node->text();
	    else
	        textHint = m_activeEditor->textLine( startLine ).simplifyWhiteSpace();
	}
    }
    m_backgroundParser->unlock();
}

void CppSupportPart::slotNodeSelected( QListViewItem* item )
{
    if( !item || !m_activeSelection || !m_activeViewCursor)
	return;

    m_activeSelection->setSelection( item->text(1).toInt(), item->text(2).toInt(),
				     item->text(3).toInt(), item->text(4).toInt() );
    m_activeViewCursor->setCursorPositionReal(item->text(1).toInt(), item->text(2).toInt());
}

void CppSupportPart::slotMakeMember()
{
    if( !m_activeViewCursor || !m_valid )
        return;

    // sync
    while( m_backgroundParser->filesInQueue() > 0 )
         m_backgroundParser->isEmpty().wait();

    QString text;

    m_backgroundParser->lock();
    TranslationUnitAST* translationUnit = m_backgroundParser->translationUnit( m_activeFileName );
    if( translationUnit ){
        unsigned int line, column;
	m_activeViewCursor->cursorPositionReal( &line, &column );

        AST* currentNode = findNodeAt( translationUnit, line, column );
	DeclaratorAST* declarator = 0;
	while( currentNode && currentNode->nodeType() != NodeType_SimpleDeclaration ){
	    if( currentNode->nodeType() == NodeType_Declarator )
	        declarator = (DeclaratorAST*) currentNode;
	    currentNode = currentNode->parent();
	}
	SimpleDeclarationAST* decl = currentNode ? (SimpleDeclarationAST*) currentNode : 0;
	if( decl && decl->initDeclaratorList() && !declarator ){
	    InitDeclaratorAST* i = decl->initDeclaratorList()->initDeclaratorList().at( 0 );
	    if( i )
	       declarator = i->declarator();
	}

	if( decl && declarator && declarator->parameterDeclarationClause() ){

	    QStringList scope;
	    scopeOfNode( decl, scope );

	    QString scopeStr = scope.join( "::" );
	    if( !scopeStr.isEmpty() )
	        scopeStr += "::";

	    QString declStr = declaratorToString( declarator, scopeStr ).simplifyWhiteSpace();

	    text += "\n\n";
	    QString type = typeSpecToString( decl->typeSpec() );
	    text += type;
	    if( !type.isNull() )
		text +=  + " ";

	    text += declStr + "\n{\n}";
	}

	m_backgroundParser->unlock();

	QString implFile = findSourceFile();

	if( !text.isEmpty() && !implFile.isEmpty() ){
	    partController()->editDocument( implFile );
	    kapp->processEvents( 500 );

            // sync
            while( m_backgroundParser->filesInQueue() > 0 )
                m_backgroundParser->isEmpty().wait();

	}

	m_backgroundParser->lock();
	translationUnit = m_backgroundParser->translationUnit( m_activeFileName );
	int atLine, atColumn;
	if( translationUnit ){
	    translationUnit->getEndPosition( &atLine, &atColumn );
	} else {
	    atLine = m_activeEditor->numLines() - 1;
	    atColumn = 0;
	}

	if( m_activeEditor )
	    m_activeEditor->insertText( atLine, atColumn, text );
	if( m_activeViewCursor )
	    m_activeViewCursor->setCursorPositionReal( atLine+3, 1 );
    }
    m_backgroundParser->unlock();
}

QStringList CppSupportPart::subclassWidget(const QString& formName)
{
    QStringList newFileNames;
    SubclassingDlg *dlg = new SubclassingDlg(this, formName, newFileNames);
    dlg->exec();
    return newFileNames;
}

QStringList CppSupportPart::updateWidget(const QString& formName, const QString& fileName)
{
    QStringList dummy;
    SubclassingDlg *dlg = new SubclassingDlg(this, formName, fileName, dummy);
    dlg->exec();
    return dummy;
}

void CppSupportPart::partRemoved( KParts::Part* part )
{
    kdDebug(9032) << "CppSupportPart::partRemoved()" << endl;

    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>( part );
    if( doc ){
	QString fileName = doc->url().path();
	if( !fileName.isEmpty() )
	    m_backgroundParser->removeFile( fileName );
    }
}

void CppSupportPart::slotProjectCompiled()
{
    kdDebug(9007) << "CppSupportPart::slotProjectCompiled()" << endl;
    parseProject();
}

QStringList CppSupportPart::modifiedFileList()
{
    QStringList lst;

    QStringList fileList = project()->allFiles();
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
	QString fileName = *it;
	++it;

	QFileInfo fileInfo( project()->projectDirectory(), fileName );

	if( !fileExtensions().contains(fileInfo.extension()) )
	    continue;

	QDateTime t = fileInfo.lastModified();
	QMap<QString, QDateTime>::Iterator dictIt = m_timestamp.find( fileInfo.absFilePath() );
	if( fileInfo.exists() && dictIt != m_timestamp.end() && *dictIt == t )
	    continue;

	lst << fileName;
    }

    return lst;
}

KTextEditor::Document * CppSupportPart::findDocument( const KURL & url )
{
    if( !partController()->parts() )
        return 0;

    QPtrList<KParts::Part> parts( *partController()->parts() );
    QPtrListIterator<KParts::Part> it( parts );
    while( KParts::Part* part = it.current() ){
        KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
	if( doc && doc->url() == url )
	    return doc;
        ++it;
    }

    return 0;
}

void CppSupportPart::setupCatalog( )
{
    kdDebug(9007) << "CppSupportPart::setupCatalog()" << endl;

    QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName";

    KStandardDirs *dirs = CppSupportFactory::instance()->dirs();
    QStringList pcsList = dirs->findAllResources( "pcs", "*.db", false, true );
    QStringList pcsIdxList = dirs->findAllResources( "pcs", "*.idx", false, true );

    if( pcsList.size() && pcsVersion() < PCS_VERSION ){
        QStringList l = pcsList + pcsIdxList;
        int rtn = KMessageBox::questionYesNoList( 0, i18n("Persistant class store will be disabled!! You have a wrong version of pcs installed.\nRemove old pcs files?"), l, i18n("C++ Support") );
        if( rtn == KMessageBox::Yes ){
            QStringList::Iterator it = l.begin();
            while( it != l.end() ){
                QFile::remove( *it );
                ++it;
            }
            // TODO: regenerate the pcs list
            pcsList.clear();
        } else {
            return;
        }
    }

    QStringList::Iterator it = pcsList.begin();
    while( it != pcsList.end() ){
        Catalog* catalog = new Catalog();
        catalog->open( *it );
        ++it;

        for( QStringList::Iterator idxIt=indexList.begin(); idxIt!=indexList.end(); ++idxIt )
            catalog->addIndex( (*idxIt).utf8() );

        m_catalogList.append( catalog );
        codeRepository()->registerCatalog( catalog );
    }

    setPcsVersion( PCS_VERSION );
}

KMimeType::List CppSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime;

    mime = KMimeType::mimeType( "text/x-csrc" );
    if( mime )
	list << mime;

    mime = KMimeType::mimeType( "text/x-chdr" );
    if( mime )
	list << mime;

    mime = KMimeType::mimeType( "text/x-c++src" );
    if( mime )
	list << mime;

    mime = KMimeType::mimeType( "text/x-c++hdr" );
    if( mime )
	list << mime;

    return list;
}

void CppSupportPart::emitFileParsed( const QString & fileName )
{
    emit fileParsed( fileName );
}

int CppSupportPart::pcsVersion()
{
    KConfig* config = CppSupportFactory::instance()->config();
    KConfigGroupSaver cgs( config, "PCS" );
    return config->readNumEntry( "Version", 0 );
}

void CppSupportPart::setPcsVersion( int version )
{
    KConfig* config = CppSupportFactory::instance()->config();
    KConfigGroupSaver cgs( config, "PCS" );
    config->writeEntry( "Version", version );
    config->sync();
}

QString CppSupportPart::formatTag( const Tag & inputTag )
{
    Tag tag = inputTag;

    switch( tag.kind() )
    {
        case Tag::Kind_Namespace:
            return QString::fromLatin1("namespace ") + tag.name();

        case Tag::Kind_Class:
            return QString::fromLatin1("class ") + tag.name();

        case Tag::Kind_Function:
        case Tag::Kind_FunctionDeclaration:
        {
            CppFunction<Tag> tagInfo( tag );
            return tagInfo.name() + "( " + tagInfo.arguments().join(", ") + " ) : " + tagInfo.type();
        }
        break;

        case Tag::Kind_Variable:
        case Tag::Kind_VariableDeclaration:
        {
            CppVariable<Tag> tagInfo( tag );
            return tagInfo.name() + " : " + tagInfo.type();
        }
        break;
    }
    return tag.name();
}

#include "cppsupportpart.moc"

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

#include "javasupportpart.h"
#include "javasupport_events.h"
#include "problemreporter.h"
#include "backgroundparser.h"
#include "KDevJavaSupportIface.h"
#include "javasupportfactory.h"
#include "catalog.h"
#include "kdevdriver.h"
#include "javasupport_utils.h"

#include "JavaStoreWalker.hpp"
#include "JavaAST.hpp"

#include <qheader.h>
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
#include <kmessagebox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/clipboardinterface.h>

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
#include <kdevpartcontroller.h>
#include <kdevmakefrontend.h>
#include <kdevcoderepository.h>

#include <domutil.h>
#include <urlutil.h>
#include <config.h>

enum { KDEV_DB_VERSION = 7 };
enum { KDEV_PCS_VERSION = 8 };

class JavaDriver: public KDevDriver
{
public:
    JavaDriver( JavaSupportPart* javaSupport )
	: KDevDriver( javaSupport )
    {
    }

    void fileParsed( const QString& fileName )
    {
	//kdDebug(9013) << "-----> file " << fileName << " parsed!" << endl;
	RefJavaAST ast = takeTranslationUnit( fileName );

        if( javaSupport()->problemReporter() ){
	    javaSupport()->problemReporter()->removeAllProblems( fileName );

	    QValueList<Problem> pl = problems( fileName );
	    QValueList<Problem>::ConstIterator it = pl.begin();
	    while( it != pl.end() ){
	        const Problem& p = *it++;
	        javaSupport()->problemReporter()->reportProblem( fileName, p );
	    }
	}

	if( javaSupport()->codeModel()->hasFile(fileName) ){
	    FileDom file = javaSupport()->codeModel()->fileByName( fileName );
	    javaSupport()->removeWithReferences( fileName );
	}

	FileDom file = javaSupport()->codeModel()->create<FileModel>();
	file->setName( fileName );
	JavaStoreWalker walker;
	walker.setFile( file );
	walker.setCodeModel( javaSupport()->codeModel() );
	walker.compilationUnit( ast );
	javaSupport()->codeModel()->addFile( file );

	remove( fileName );
    }
};

JavaSupportPart::JavaSupportPart(QObject *parent, const char *name, const QStringList &/*args*/)
    : KDevLanguageSupport(JavaSupportFactory::info(), parent, name ? name : "KDevJavaSupport"),
      m_activeDocument( 0 ), m_activeView( 0 ), m_activeSelection( 0 ), m_activeEditor( 0 ),
      m_activeViewCursor( 0 ), m_projectClosed( true ), m_valid( false )
{
    setInstance(JavaSupportFactory::instance());

    m_driver = new JavaDriver( this );

    setXMLFile( "kdevjavasupport.rc" );

    m_catalogList.setAutoDelete( true );
    setupCatalog();

    m_backgroundParser = new BackgroundParser( this, &m_eventConsumed );
    m_backgroundParser->start();

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)));
    connect( partController(), SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(partRemoved(KParts::Part*)));

    m_problemReporter = new ProblemReporter( this, 0, "problemReporterWidget" );
    m_problemReporter->setIcon( SmallIcon("info") );
    mainWindow( )->embedOutputView( m_problemReporter, i18n("Problems"), i18n("Problem reporter"));

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             m_problemReporter, SLOT(configWidget(KDialogBase*)) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );

    KAction *action;

    action = new KAction(i18n("New Class..."), "classnew", 0,
                         this, SLOT(slotNewClass()),
                         actionCollection(), "project_newclass");
    action->setToolTip( i18n("Generate a new class") );
    action->setWhatsThis( i18n("<b>New Class</b>Generates a new class.<p>") );

    // daniel
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    new KDevJavaSupportIface( this );
    //(void) dcopClient();
}


JavaSupportPart::~JavaSupportPart()
{
    delete( m_driver );
    m_driver = 0;

    if( m_backgroundParser ){
	m_backgroundParser->close();
	m_backgroundParser->wait();
	delete m_backgroundParser;
	m_backgroundParser = 0;
    }

    codeRepository()->setMainCatalog( 0 );

    QPtrListIterator<Catalog> it( m_catalogList );
    while( Catalog* catalog = it.current() ){
        ++it;
        codeRepository()->unregisterCatalog( catalog );
    }

    mainWindow( )->removeView( m_problemReporter );

    delete m_problemReporter;
    m_problemReporter = 0;
}

void JavaSupportPart::customEvent( QCustomEvent* ev )
{
    //kdDebug(9013) << "JavaSupportPart::customEvent()" << endl;

    if( ev->type() == int(Event_FileParsed) ){
	FileParsedEvent* event = (FileParsedEvent*) ev;
	QString fileName = event->fileName();

        if( m_problemReporter ){
	    m_problemReporter->removeAllProblems( fileName );

	    bool hasErrors = false;
	    QValueList<Problem> problems = event->problems();
	    QValueList<Problem>::ConstIterator it = problems.begin();
	    while( it != problems.end() ){
	        const Problem& p = *it++;
		if( p.level() == Problem::Level_Error )
		    hasErrors = true;

	        m_problemReporter->reportProblem( fileName, p );
	    }

	    m_backgroundParser->lock();
	    if( RefJavaAST ast = m_backgroundParser->translationUnit(fileName) ){

		if( !hasErrors ){
		    if( codeModel()->hasFile(fileName) ){
			FileDom file = codeModel()->fileByName( fileName );
			removeWithReferences( fileName );
		    }

		    FileDom file = codeModel()->create<FileModel>();
		    file->setName( fileName );
		    JavaStoreWalker walker;
		    walker.setFile( file );
		    walker.setCodeModel( codeModel() );

		    walker.compilationUnit( ast );
		    codeModel()->addFile( file );

		    emit addedSourceInfo( fileName );
		}
	    }
	    m_backgroundParser->unlock();
	}
	emit fileParsed( fileName );
    }
}

void JavaSupportPart::projectConfigWidget( KDialogBase* /*dlg*/ )
{
}

void JavaSupportPart::configWidget(KDialogBase */*dlg*/)
{
}

void JavaSupportPart::activePartChanged(KParts::Part *part)
{
    kdDebug(9032) << "JavaSupportPart::activePartChanged()" << endl;

    bool enabled = false;

    m_activeDocument = dynamic_cast<KTextEditor::Document*>( part );
    m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
    m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
    m_activeViewCursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>( m_activeView ) : 0;

    m_activeFileName = QString::null;

    if (m_activeDocument) {
	m_activeFileName = URLUtil::canonicalPath( m_activeDocument->url().path() );
        QFileInfo fi( m_activeFileName );
        QString ext = fi.extension();
        if (fileExtensions().contains(ext))
            enabled = true;
    }

    if( !part )
	return;

    if( !m_activeView )
	return;

#if 0
    KTextEditor::TextHintInterface* textHintIface = dynamic_cast<KTextEditor::TextHintInterface*>( m_activeView );
    if( !textHintIface )
	return;

    connect( view, SIGNAL(needTextHint(int,int,QString&)),
	     this, SLOT(slotNeedTextHint(int,int,QString&)) );

    textHintIface->enableTextHints( 1000 );
#endif
}


void JavaSupportPart::projectOpened( )
{
    kdDebug( 9013 ) << "projectOpened( )" << endl;

    m_projectDirectory = URLUtil::canonicalPath( project()->projectDirectory() );

    connect( project( ), SIGNAL( addedFilesToProject( const QStringList & ) ),
             this, SLOT( addedFilesToProject( const QStringList & ) ) );
    connect( project( ), SIGNAL( removedFilesFromProject( const QStringList &) ),
             this, SLOT( removedFilesFromProject( const QStringList & ) ) );
    connect( project( ), SIGNAL( changedFilesInProject( const QStringList & ) ),
             this, SLOT( changedFilesInProject( const QStringList & ) ) );
    connect( project(), SIGNAL(projectCompiled()),
	     this, SLOT(slotProjectCompiled()) );

    QDir::setCurrent( m_projectDirectory );

    m_timestamp.clear();

    m_projectClosed = false;

    QTimer::singleShot( 500, this, SLOT( initialParse( ) ) );
}


void JavaSupportPart::projectClosed( )
{
    kdDebug( 9013 ) << "projectClosed( )" << endl;

    saveProjectSourceInfo();

    if( m_backgroundParser )
	m_backgroundParser->removeAllFiles();

    m_projectClosed = true;
}

void JavaSupportPart::contextMenu(QPopupMenu */*popup*/, const Context *context)
{
    m_activeClass = 0;
    m_activeFunction = 0;
    m_activeVariable = 0;

    if( context->hasType(Context::EditorContext) ){
        // nothing!
    } else if( context->hasType(Context::CodeModelItemContext) ){
	const CodeModelItemContext* mcontext = static_cast<const CodeModelItemContext*>( context );

	if( mcontext->item()->isClass() ){
	    m_activeClass = (ClassModel*) mcontext->item();
	} else if( mcontext->item()->isFunction() ){
	    m_activeFunction = (FunctionModel*) mcontext->item();
	}
    }
}

void JavaSupportPart::addedFilesToProject(const QStringList &fileList)
{
    QStringList files = fileList;

    for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
	QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + (*it) );

	maybeParse( path );
	emit addedSourceInfo( path );
    }
}

void JavaSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    for ( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
    {
	QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );

	removeWithReferences( path );
	m_backgroundParser->removeFile( path );
    }
}

void JavaSupportPart::changedFilesInProject( const QStringList & fileList )
{
    QStringList files = fileList;

    for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
	QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );

	maybeParse( path );
	emit addedSourceInfo( path );
    }
}

void JavaSupportPart::savedFile(const KURL &fileName)
{
    Q_UNUSED( fileName.path() );

#if 0  // not needed anymore
    kdDebug(9013) << "savedFile(): " << fileName.mid ( m_projectDirectory.length() + 1 ) << endl;

    QStringList projectFileList = project()->allFiles();
    if (projectFileList.contains(fileName.mid ( m_projectDirectory.length() + 1 ))) {
	maybeParse( fileName );
	emit addedSourceInfo( fileName );
    }
#endif
}

QString JavaSupportPart::findSourceFile()
{
    QFileInfo fi( m_activeFileName );
    QString path = fi.filePath();
    QString ext = fi.extension();
    QString base = path.left( path.length() - ext.length() );
    QStringList candidates;

    if (ext == "h" || ext == "H" || ext == "hh" || ext == "hxx" || ext == "hpp" || ext == "tlh") {
        candidates << (base + "c");
        candidates << (base + "cc");
        candidates << (base + "java");
        candidates << (base + "java");
        candidates << (base + "cxx");
        candidates << (base + "C");
        candidates << (base + "m");
        candidates << (base + "mm");
        candidates << (base + "M");
	candidates << (base + "inl");
    }

    QStringList::ConstIterator it;
    for (it = candidates.begin(); it != candidates.end(); ++it) {
        kdDebug(9013) << "Trying " << (*it) << endl;
        if (QFileInfo(*it).exists()) {
            return *it;
        }
    }

    return m_activeFileName;
}

KDevLanguageSupport::Features JavaSupportPart::features()
{
    return Features( Classes | Functions | Variables );
}

QString JavaSupportPart::formatClassName(const QString &name)
{
    return name;
}

QString JavaSupportPart::unformatClassName(const QString &name)
{
    return name;
}

QStringList JavaSupportPart::fileExtensions() const
{
    return QStringList::split(",", "java");
}

void JavaSupportPart::slotNewClass()
{
}

void JavaSupportPart::addMethod( ClassDom /*klass*/ )
{
}

void JavaSupportPart::addAttribute( ClassDom /*klass*/ )
{
}

void JavaSupportPart::initialParse( )
{
    // For debugging
    if( !project( ) ){
        // messagebox ?
        kdDebug( 9013 ) << "No project" << endl;
        return;
    }

    parseProject( );
    emit updatedSourceInfo();
    m_valid = true;
    return;
}

#if QT_VERSION < 0x030100
// Taken from qt-3.2/tools/qdatetime.java/QDateTime::toTime_t() and modified for normal function
uint toTime_t(QDateTime t)
{
    tm brokenDown;
    brokenDown.tm_sec = t.time().second();
    brokenDown.tm_min = t.time().minute();
    brokenDown.tm_hour = t.time().hour();
    brokenDown.tm_mday = t.date().day();
    brokenDown.tm_mon = t.date().month() - 1;
    brokenDown.tm_year = t.date().year() - 1900;
    brokenDown.tm_isdst = -1;
    int secsSince1Jan1970UTC = (int) mktime( &brokenDown );
    if ( secsSince1Jan1970UTC < -1 )
    secsSince1Jan1970UTC = -1;
    return (uint) secsSince1Jan1970UTC;
}
#endif

bool
JavaSupportPart::parseProject( )
{
    //QLabel* label = new QLabel( "", mainWindow( )->statusBar( ) );
    //label->setMinimumWidth( 600 );
    //mainWindow( )->statusBar( )->addWidget( label );
    //label->show( );

    mainWindow()->statusBar()->message( i18n("Updating...") );

    kapp->processEvents( );
    kapp->setOverrideCursor( waitCursor );

    QStringList files = modifiedFileList();

    QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
    bar->setMinimumWidth( 120 );
    bar->setCenterIndicator( true );
    mainWindow( )->statusBar( )->addWidget( bar );
    bar->show( );

    QDir d( m_projectDirectory );

    QDataStream stream;
    QMap< QString, QPair<uint, Q_LONG> > pcs;

    QFile f( project()->projectDirectory() + "/" + project()->projectName().lower() + ".kdevelop" + ".pcs" );
    if( f.open(IO_ReadOnly) ){
	stream.setDevice( &f );

	QString sig;
	int pcs_version = 0;
	stream >> sig >> pcs_version;
	if( sig == "PCS" && pcs_version == KDEV_PCS_VERSION ){

	    int numFiles = 0;
	    stream >> numFiles;

	    for( int i=0; i<numFiles; ++i ){
		QString fn;
		uint ts;
		Q_LONG offset;

		stream >> fn >> ts >> offset;
		pcs[ fn ] = qMakePair( ts, offset );
	    }
	}
    }

    int n = 0;
    for( QStringList::Iterator it = files.begin( ); it != files.end( ); ++it ) {
        bar->setProgress( n++ );
	QFileInfo fileInfo( d, *it );

        if( fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() ){
            QString absFilePath = URLUtil::canonicalPath( fileInfo.absFilePath() );
	    kdDebug(9013) << "parse file: " << absFilePath << endl;

	    if( (n%5) == 0 ){
	        kapp->processEvents();

		if( m_projectClosed ){
		    delete( bar );
		    return false;
		}
	    }

	    if( isValidSource(absFilePath) ){
		QDateTime t = fileInfo.lastModified();
		if( m_timestamp.contains(absFilePath) && m_timestamp[absFilePath] == t )
		    continue;

#if QT_VERSION >= 0x030100
		if( pcs.contains(absFilePath) && t.toTime_t() == pcs[absFilePath].first ){
#else
		if( pcs.contains(absFilePath) && toTime_t(t) == pcs[absFilePath].first ){
#endif
		    stream.device()->at( pcs[absFilePath].second );
		    FileDom file = codeModel()->create<FileModel>();
		    file->read( stream );
		    codeModel()->addFile( file );
		} else {
		    m_driver->parseFile( absFilePath );
		}

		m_timestamp[ absFilePath ] = t;
	    }
        }

	if( m_projectClosed ){
	    kdDebug(9013) << "ABORT" << endl;
            kapp->restoreOverrideCursor( );
	    return false;
	}
    }

    kdDebug( 9013 ) << "updating sourceinfo" << endl;
    emit updatedSourceInfo();

    mainWindow( )->statusBar( )->removeWidget( bar );
    delete bar;
    //mainWindow( )->statusBar( )->removeWidget( label );
    //delete label;

    kapp->restoreOverrideCursor( );
    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    return true;
}

void JavaSupportPart::maybeParse( const QString& fileName )
{
    if( !isValidSource(fileName) )
        return;

    QFileInfo fileInfo( fileName );
    QString path = URLUtil::canonicalPath( fileName );
    QDateTime t = fileInfo.lastModified();

    if( !fileInfo.exists() ){
	removeWithReferences( path );
	return;
    }

    QMap<QString, QDateTime>::Iterator it = m_timestamp.find( path );
    if( it != m_timestamp.end() && *it == t ){
	return;
    }

    m_timestamp[ path ] = t;
    m_driver->parseFile( path );
}

void JavaSupportPart::slotNeedTextHint( int /*line*/, int /*column*/, QString& /*textHint*/ )
{
}

QStringList JavaSupportPart::subclassWidget(const QString& /*formName*/)
{
    QStringList newFileNames;
    return newFileNames;
}

QStringList JavaSupportPart::updateWidget(const QString& /*formName*/, const QString& /*fileName*/)
{
    QStringList dummy;
    return dummy;
}

void JavaSupportPart::partRemoved( KParts::Part* part )
{
    kdDebug(9032) << "JavaSupportPart::partRemoved()" << endl;

    if( KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part ) ){

	QString fileName = doc->url().path();
	if( fileName.isEmpty() )
	    return;

	QString canonicalFileName = URLUtil::canonicalPath( fileName );
	m_backgroundParser->removeFile( canonicalFileName );
	m_backgroundParser->addFile( canonicalFileName, true );
    }
}

void JavaSupportPart::slotProjectCompiled()
{
    kdDebug(9013) << "JavaSupportPart::slotProjectCompiled()" << endl;
    parseProject();
}

QStringList JavaSupportPart::modifiedFileList()
{
    QStringList lst;

    QStringList fileList = project()->allFiles();
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
	QString fileName = *it;
	++it;

	QFileInfo fileInfo( m_projectDirectory, fileName );

	if( !fileExtensions().contains(fileInfo.extension()) )
	    continue;

	QDateTime t = fileInfo.lastModified();
	QString path = URLUtil::canonicalPath( fileInfo.absFilePath() );
	QMap<QString, QDateTime>::Iterator dictIt = m_timestamp.find( path );
	if( fileInfo.exists() && dictIt != m_timestamp.end() && *dictIt == t )
	    continue;

	lst << fileName;
    }

    return lst;
}

KTextEditor::Document * JavaSupportPart::findDocument( const KURL & url )
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

void JavaSupportPart::setupCatalog( )
{
    kdDebug(9013) << "JavaSupportPart::setupCatalog()" << endl;

    QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName";

    KStandardDirs *dirs = JavaSupportFactory::instance()->dirs();
    QStringList pcsList = dirs->findAllResources( "pcs", "*.db", false, true );
    QStringList pcsIdxList = dirs->findAllResources( "pcs", "*.idx", false, true );

    if( pcsList.size() && pcsVersion() < KDEV_DB_VERSION ){
        QStringList l = pcsList + pcsIdxList;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,4,0)
        int rtn = KMessageBox::questionYesNoList( 0, i18n("Persistant class store will be disabled: you have a wrong version of pcs installed.\nRemove old pcs files?"), l, i18n("Java Support"), KStdGuiItem::remove(), i18n("Keep Them") );
#else
        int rtn = KMessageBox::questionYesNoList( 0, i18n("Persistant class store will be disabled: you have a wrong version of pcs installed.\nRemove old pcs files?"), l, i18n("Java Support"), KGuiItem( i18n( "&Remove" ), "editdelete", i18n( "Remove item(s)" )), i18n("Keep Them") );
#endif
        if( rtn == KMessageBox::Yes ){
            QStringList::Iterator it = l.begin();
            while( it != l.end() ){
                QFile::remove( *it );
                ++it;
            }
            // @todo regenerate the pcs list
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

    setPcsVersion( KDEV_DB_VERSION );
}

KMimeType::List JavaSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime;

    mime = KMimeType::mimeType( "text/x-java" );
    if( mime )
	list << mime;

    return list;
}

int JavaSupportPart::pcsVersion()
{
    KConfig* config = JavaSupportFactory::instance()->config();
    KConfigGroupSaver cgs( config, "PCS" );
    return config->readNumEntry( "Version", 0 );
}

void JavaSupportPart::setPcsVersion( int version )
{
    KConfig* config = JavaSupportFactory::instance()->config();
    KConfigGroupSaver cgs( config, "PCS" );
    config->writeEntry( "Version", version );
    config->sync();
}

QString JavaSupportPart::formatTag( const Tag & /*inputTag*/ )
{
    return QString::null;
}

void JavaSupportPart::removeWithReferences( const QString & fileName )
{
    kdDebug(9013) << "remove with references: " << fileName << endl;
    m_timestamp.remove( fileName );
    if( !codeModel()->hasFile(fileName) )
        return;

    emit aboutToRemoveSourceInfo( fileName );

    codeModel()->removeFile( codeModel()->fileByName(fileName) );
}

bool JavaSupportPart::isValidSource( const QString& fileName ) const
{
    QFileInfo fileInfo( fileName );
    return fileExtensions().contains( fileInfo.extension() ) && !QFile::exists(fileInfo.dirPath(true) + "/.kdev_ignore");
}

QString JavaSupportPart::formatModelItem( const CodeModelItem *item, bool shortDescription )
{
    if (item->isFunction())
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
	if( !shortDescription )
            function += model->resultType() + " ";

	function += model->name() + "(" + args + ")" +
            (model->isAbstract() ? QString(" = 0") : QString("") );

        return function;
    }
    else if (item->isVariable())
    {
        const VariableModel *model = static_cast<const VariableModel*>(item);
	if( shortDescription )
	    return model->name();
        return model->type() + " " + model->name();
    }
    else if (item->isArgument())
    {
        const ArgumentModel *model = static_cast<const ArgumentModel*>(item);
	QString arg;
	if( !shortDescription )
	    arg += model->type() + " ";
	arg += model->name();
	if( !shortDescription )
	    arg += model->defaultValue().isEmpty() ? QString("") : QString(" = ") + model->defaultValue();
	return arg.stripWhiteSpace();
    }
    else
        return KDevLanguageSupport::formatModelItem( item, shortDescription );
}

void JavaSupportPart::addClass( )
{
    slotNewClass();
}

void JavaSupportPart::saveProjectSourceInfo( )
{
    const FileList fileList = codeModel()->fileList();

    if( !project() || fileList.isEmpty() )
	return;

    QFile f( project()->projectDirectory() + "/" + project()->projectName().lower() + ".kdevelop" + ".pcs" );
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
    }
}

#include "javasupportpart.moc"

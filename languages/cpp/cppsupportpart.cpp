/***************************************************************************
*   Copyright (C) 1999 by Jonas Nordin                                    *
*   jonas.nordin@syncom.se                                                *
*   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*   Copyright (C) 2002-2003 by Roberto Raggi                              *
*   roberto@kdevelop.org                                                  *
*   Copyright (C) 2003-2004 by Alexander Dymo                             *
*   adymo@mksat.net                                                       *
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
#include "backgroundparser.h"
#include "store_walker.h"
#include "ast.h"
#include "ast_utils.h"
#include "cppcodecompletion.h"
#include "ccconfigwidget.h"
#include "KDevCppSupportIface.h"
#include "cppsupportfactory.h"
#include "catalog.h"
#include "cpp_tags.h"
#include "kdevdriver.h"
#include "cppcodecompletionconfig.h"
#include "cppsplitheadersourceconfig.h"
#include "tag_creator.h"
#include "cppsupport_utils.h"
#include "classgeneratorconfig.h"
#include "urlutil.h"
#include "creategettersetterconfiguration.h"
#include "kdevsourceformatter.h"
#include "kdevcreatefile.h" 
#include "qtbuildconfig.h"
// wizards
#include "cppnewclassdlg.h"
#include "subclassingdlg.h"
#include "addmethoddialog.h"
#include "addattributedialog.h"
#include "creategettersetterdialog.h" 
// designer integration
#include "qtdesignercppintegration.h"
#include "cppimplementationwidget.h"

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
#include <kmessagebox.h>
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
#include <ktexteditor/texthintinterface.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>
#include <kdevmakefrontend.h>
#include <kdevcoderepository.h>
#include <codemodel_utils.h>
#include <kdevplugininfo.h>

#include <domutil.h>
#include <config.h>

enum { KDEV_DB_VERSION = 6 };
enum { KDEV_PCS_VERSION = 5 };

QStringList CppSupportPart::m_sourceMimeTypes = QStringList() << "text/x-csrc" << "text/x-c++src";
QStringList CppSupportPart::m_headerMimeTypes = QStringList() << "text/x-chdr" << "text/x-c++hdr";

QStringList CppSupportPart::m_sourceExtensions = QStringList::split( ",", "c,C,cc,cpp,c++,cxx,m,mm,M" );
QStringList CppSupportPart::m_headerExtensions = QStringList::split( ",", "h,H,hh,hxx,hpp,inl,tlh,diff,ui.h" );

class CppDriver: public KDevDriver
{
public:
	CppDriver( CppSupportPart* cppSupport ) : KDevDriver( cppSupport )
	{}

	void fileParsed( const QString& fileName )
	{
		//kdDebug(9007) << "-----> file " << fileName << " parsed!" << endl;
		TranslationUnitAST::Node ast = takeTranslationUnit( fileName );
		
		if ( cppSupport() ->problemReporter() )
		{
			cppSupport() ->problemReporter() ->removeAllProblems( fileName );
			
			QValueList<Problem> pl = problems( fileName );
			QValueList<Problem>::ConstIterator it = pl.begin();
			while ( it != pl.end() )
			{
				const Problem & p = *it++;
				cppSupport() ->problemReporter() ->reportProblem( fileName, p );
			}
		}
		
		StoreWalker walker( fileName, cppSupport() ->codeModel() );
		
		if ( cppSupport() ->codeModel() ->hasFile( fileName ) )
		{
			FileDom file = cppSupport() ->codeModel() ->fileByName( fileName );
			cppSupport() ->removeWithReferences( fileName );
		}
		
		walker.parseTranslationUnit( ast.get() );
		cppSupport() ->codeModel() ->addFile( walker.file() );
		remove
			( fileName );
	}
};

CppSupportPart::CppSupportPart( QObject *parent, const char *name, const QStringList &args )
: KDevLanguageSupport( CppSupportFactory::info(), parent, name ? name : "KDevCppSupport" ),
m_activeDocument( 0 ), m_activeView( 0 ), m_activeSelection( 0 ), m_activeEditor( 0 ),
m_activeViewCursor( 0 ), m_projectClosed( true ), m_valid( false )
{
	setInstance( CppSupportFactory::instance() );
	
	m_pCompletionConfig = new CppCodeCompletionConfig( this, projectDom() );
	m_pSplitHeaderSourceConfig = new CppSplitHeaderSourceConfig( this, projectDom() );
	m_pCreateGetterSetterConfiguration = new CreateGetterSetterConfiguration( this );	connect( m_pSplitHeaderSourceConfig, SIGNAL( stored() ), 
		this, SLOT( splitHeaderSourceConfigStored() ) );
	connect( m_pCompletionConfig, SIGNAL( stored() ), 
	         this, SLOT( codeCompletionConfigStored() ) );
	m_qtBuildConfig = new QtBuildConfig( this, projectDom() );
	
	m_driver = new CppDriver( this );
	m_problemReporter = 0;
	
	m_functionHintTimer = new QTimer( this );
	//    connect( m_functionHintTimer, SIGNAL(timeout()), this, SLOT(slotFunctionHint()) );
	
	setXMLFile( "kdevcppsupport.rc" );
	
	m_catalogList.setAutoDelete( true );
	
	connect( core(), SIGNAL( projectOpened() ), this, SLOT( projectOpened() ) );
	connect( core(), SIGNAL( projectClosed() ), this, SLOT( projectClosed() ) );
	connect( core(), SIGNAL( languageChanged() ), this, SLOT( projectOpened() ) );
	connect( partController(), SIGNAL( savedFile( const KURL& ) ),
	         this, SLOT( savedFile( const KURL& ) ) );
	connect( core(), SIGNAL( contextMenu( QPopupMenu *, const Context * ) ),
	         this, SLOT( contextMenu( QPopupMenu *, const Context * ) ) );
	connect( partController(), SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( activePartChanged( KParts::Part* ) ) );
	connect( partController(), SIGNAL( partRemoved( KParts::Part* ) ),
	         this, SLOT( partRemoved( KParts::Part* ) ) );
	
	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
	         this, SLOT( configWidget( KDialogBase* ) ) );
	
	KAction *action;
	
	action = new KAction( i18n( "Switch Header/Implementation" ), SHIFT + Key_F12,
	                      this, SLOT( slotSwitchHeader() ),
	                      actionCollection(), "edit_switchheader" );
	action->setToolTip( i18n( "Switch between header and implementation files" ) );
	action->setWhatsThis( i18n( "<b>Switch Header/Implementation</b><p>"
	                            "If you are currently looking at a header file, this "
	                            "brings you to the corresponding implementation file. "
	                            "If you are looking at an implementation file (.cpp etc.), "
	                            "this brings you to the corresponding header file." ) );
	action->setEnabled( false );
	
	action = new KAction( i18n( "Complete Text" ), CTRL + Key_Space,
	                      this, SLOT( slotCompleteText() ),
	                      actionCollection(), "edit_complete_text" );
	action->setToolTip( i18n( "Complete current expression" ) );
	action->setWhatsThis( i18n( "<b>Complete Text</p><p>Completes current expression using "
	                            "memory class store for the current project and persistant class stores "
	                            "for external libraries." ) );
	action->setEnabled( false );
	
	m_createGetterSetterAction = new KAction( i18n( "Create Accessor Methods" ), 0,
	                                          this, SLOT( slotCreateAccessMethods() ), actionCollection(),
	                                          "edit_create_getter_setter" );
	
	action = new KAction( i18n( "Make Member" ), 0, Key_F2,
	                      this, SLOT( slotMakeMember() ),
	                      actionCollection(), "edit_make_member" );
	action->setToolTip( i18n( "Make member" ) );
	action->setWhatsThis( i18n( "<b>Make member</b><p>Creates a class member function in implementation file "
	                            "based on the member declaration at the current line." ) );
	action->plug( new QWidget() );
	
	action = new KAction( i18n( "New Class..." ), "classnew", 0,
	                      this, SLOT( slotNewClass() ),
	                      actionCollection(), "project_newclass" );
	action->setToolTip( i18n( "Generate a new class" ) );
	action->setWhatsThis( i18n( "<b>New Class</b><p>Calls the <b>New Class</b> wizard." ) );
	
	m_pCompletion = 0;
	
	withcpp = false;
	if ( args.count() == 1 && args[ 0 ] == "Cpp" )
		withcpp = true;
	
	// daniel
	connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
	         SLOT( projectConfigWidget( KDialogBase* ) ) );
	
	new KDevCppSupportIface( this );
	//(void) dcopClient();
}


CppSupportPart::~CppSupportPart()
{
	if ( !m_projectClosed )
		projectClosed();
	
	delete( m_driver );
	m_driver = 0;
	
	if ( m_backgroundParser )
	{
		m_backgroundParser->close();
		//	m_backgroundParser->wait();
		delete m_backgroundParser;
		m_backgroundParser = 0;
	}
	
	codeRepository() ->setMainCatalog( 0 );
	
	QPtrListIterator<Catalog> it( m_catalogList );
	while ( Catalog * catalog = it.current() )
	{
		++it;
		codeRepository() ->unregisterCatalog( catalog );
	}
	
	mainWindow( ) ->removeView( m_problemReporter );
	
	delete m_pCompletion;
	delete m_problemReporter;
	
	m_pCompletion = 0;
	m_problemReporter = 0;
	
	delete _jd;
	_jd = 0;
	
	kdDebug( 9007 ) << k_funcinfo << endl;
}

void CppSupportPart::customEvent( QCustomEvent* ev )
{
	kdDebug( 9007 ) << "CppSupportPart::customEvent(" << ev->type() << ")" << endl;
	
	QTime t;
	t.start();
	
	if ( ev->type() == int( Event_FileParsed ) )
	{
		FileParsedEvent * event = ( FileParsedEvent* ) ev;
		QString fileName = event->fileName();
		if ( m_problemReporter )
		{
			m_problemReporter->removeAllProblems( fileName );
			
			bool hasErrors = false;
			QValueList<Problem> problems = event->problems();
			QValueList<Problem>::ConstIterator it = problems.begin();
			while ( it != problems.end() )
			{
				const Problem & p = *it++;
				if ( p.level() == Problem::Level_Error )
					hasErrors = true;
				
				m_problemReporter->reportProblem( fileName, p );
			}
			recomputeCodeModel( fileName );
			//QTimer::singleShot( 0, this, SLOT(recomputeCodeModel()) );
		}
		
		emit fileParsed( fileName );
	}
}

void CppSupportPart::projectConfigWidget( KDialogBase* dlg )
{
	QVBox * vbox = 0;
	
	vbox = dlg->addVBoxPage( i18n( "C++ Specific" ), i18n( "C++ Specific" ),
	                         BarIcon( info() ->icon(), KIcon::SizeMedium ) );
	CCConfigWidget* w = new CCConfigWidget( this, vbox );
	connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );
}

void CppSupportPart::configWidget( KDialogBase *dlg )
{
	QVBox * vbox = dlg->addVBoxPage( i18n( "C++ Class Generator" ), i18n( "C++ Class Generator" ),
	                                 BarIcon( info() ->icon(), KIcon::SizeMedium ) );
	ClassGeneratorConfig *w = new ClassGeneratorConfig( vbox, "classgenerator config widget" );
	connect( dlg, SIGNAL( okClicked() ), w, SLOT( storeConfig() ) );
}

void CppSupportPart::activePartChanged( KParts::Part *part )
{
	kdDebug( 9032 ) << "CppSupportPart::activePartChanged()" << endl;
	
	bool enabled = false;
	
	m_functionHintTimer->stop();
	
	if ( m_activeView )
	{
		disconnect( m_activeView, SIGNAL( cursorPositionChanged() ), this, SLOT( slotCursorPositionChanged() ) );
	}
	
	m_activeDocument = dynamic_cast<KTextEditor::Document*>( part );
	m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;
	m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
	m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
	m_activeViewCursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>( m_activeView ) : 0;
	
	m_activeFileName = QString::null;
	
	if ( m_activeDocument )
	{
		m_activeFileName = URLUtil::canonicalPath( m_activeDocument->url().path() );
		QFileInfo fi( m_activeFileName );
		QString ext = fi.extension();
		if ( isSource( m_activeFileName ) || isHeader( m_activeFileName ) )
			enabled = true;
	}
	
	actionCollection() ->action( "edit_switchheader" ) ->setEnabled( enabled );
	actionCollection() ->action( "edit_complete_text" ) ->setEnabled( enabled );
	actionCollection() ->action( "edit_make_member" ) ->setEnabled( enabled );
	
	if ( !part )
		return ;
	
	if ( !m_activeView )
		return ;
	
	if ( m_activeViewCursor )
	{
		connect( m_activeView, SIGNAL( cursorPositionChanged() ),
		         this, SLOT( slotCursorPositionChanged() ) );
	}
	
#if 0
	KTextEditor::TextHintInterface* textHintIface = dynamic_cast<KTextEditor::TextHintInterface*>( m_activeView );
	if ( !textHintIface )
		return ;
	
	connect( view, SIGNAL( needTextHint( int, int, QString& ) ),
	         this, SLOT( slotNeedTextHint( int, int, QString& ) ) );
	
	textHintIface->enableTextHints( 1000 );
#endif
}


void CppSupportPart::projectOpened( )
{
	kdDebug( 9007 ) << "projectOpened( )" << endl;
	
	m_backgroundParser = new BackgroundParser( this, &m_eventConsumed );
	m_backgroundParser->start();
	
	// setup the driver
	QString conf_file_name = specialHeaderName();
	if ( QFile::exists( conf_file_name ) )
		m_driver->parseFile( conf_file_name, true );
	
	m_projectDirectory = URLUtil::canonicalPath( project() ->projectDirectory() );
	m_projectFileList = project() ->allFiles();
	
	setupCatalog();
	
	m_problemReporter = new ProblemReporter( this );
	m_problemReporter->setIcon( SmallIcon( "info" ) );
	m_problemReporter->setCaption( i18n( "Problem Reporter" ) );
	mainWindow( ) ->embedOutputView( m_problemReporter, i18n( "Problems" ), i18n( "Problem reporter" ) );
	
	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
	         m_problemReporter, SLOT( configWidget( KDialogBase* ) ) );
	
	connect( project( ), SIGNAL( addedFilesToProject( const QStringList & ) ),
	         this, SLOT( addedFilesToProject( const QStringList & ) ) );
	connect( project( ), SIGNAL( removedFilesFromProject( const QStringList & ) ),
	         this, SLOT( removedFilesFromProject( const QStringList & ) ) );
	connect( project( ), SIGNAL( changedFilesInProject( const QStringList & ) ),
	         this, SLOT( changedFilesInProject( const QStringList & ) ) );
	connect( project(), SIGNAL( projectCompiled() ),
	         this, SLOT( slotProjectCompiled() ) );
	
	QDir::setCurrent( m_projectDirectory );
	
	m_timestamp.clear();
	
	m_pCompletion = new CppCodeCompletion( this );
	m_projectClosed = false;
	
	QTimer::singleShot( 500, this, SLOT( initialParse( ) ) );
}


void CppSupportPart::projectClosed( )
{
	kdDebug( 9007 ) << "projectClosed( )" << endl;
	
	QStringList enabledPCSs;
	QValueList<Catalog*> catalogs = codeRepository() ->registeredCatalogs();
	for ( QValueList<Catalog*>::Iterator it = catalogs.begin(); it != catalogs.end(); ++it )
	{
		Catalog* c = *it;
		if ( c->enabled() )
			enabledPCSs.push_back( QFileInfo( c->dbName() ).baseName() );
	}
	DomUtil::writeListEntry( *project() ->projectDom(), "kdevcppsupport/references", "pcs", enabledPCSs );
	
	for ( QMap<KInterfaceDesigner::DesignerType, KDevDesignerIntegration*>::const_iterator it = m_designers.begin();
	      it != m_designers.end(); ++it )
	{
		kdDebug() << "calling save settings fro designer integration" << endl;
		it.data() ->saveSettings( *project() ->projectDom(), "kdevcppsupport/designerintegration" );
	}
	
	saveProjectSourceInfo();
	
	m_pCompletionConfig->store();
	
	delete _jd;
	_jd = 0;
	
	delete m_pCompletion;
	m_pCompletion = 0;
	m_projectClosed = true;
}


QString CppSupportPart::findHeader( const QStringList &list, const QString &header )
{
	QStringList::ConstIterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		QString s = *it;
		int pos = s.findRev( '.' );
		if ( pos != -1 )
			s = s.left( pos ) + ".h";
		if ( s.right( header.length() ) == header )
			return s;
	}
	
	return QString::null;
}


void CppSupportPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	m_activeClass = 0;
	m_activeFunction = 0;
	m_activeVariable = 0;
	m_curAttribute = 0;
	m_curClass = 0;
	
	if ( context->hasType( Context::EditorContext ) )
	{
		int id;
		
		id = popup->insertItem( "Switch header/implementation", this, SLOT( slotSwitchHeader() ) );
		popup->setWhatsThis( id, i18n( "<b>Switch Header/Implementation</b><p>"
		                               "If you are currently looking at a header file, this "
		                               "brings you to the corresponding implementation file. "
		                               "If you are looking at an implementation file (.cpp etc.), "
		                               "this brings you to the corresponding header file." ) );
		
		// 	CodeModelItemContext
		if ( context->type() == Context::EditorContext )
		{
			m_curClass = currentClass();
			if ( m_curClass != 0 )
			{
				m_curAttribute = currentAttribute( m_curClass );
				if ( m_curAttribute != 0 )
					m_createGetterSetterAction->plug( popup );
			}
		}
		
		QString text;
		int atline, atcol;
		MakeMemberHelper( text, atline, atcol );
		if ( !text.isEmpty() )
		{
			id = popup->insertItem( i18n( "Make Member" ), this, SLOT( slotMakeMember() ) );
			popup->setWhatsThis( id, i18n( "<b>Make member</b><p>Creates a class member function in implementation file "
			                               "based on the member declaration at the current line." ) );
		}
		
		kdDebug( 9007 ) << "======> code model has the file: " << m_activeFileName << " = " << codeModel() ->hasFile( m_activeFileName ) << endl;
		
		bool showContextMenuExplosion = false;
		KConfig *config = CppSupportFactory::instance() ->config();
		if ( config )
		{
			config->setGroup( "General" );
			showContextMenuExplosion = config->readBoolEntry( "ShowContextMenuExplosion", false );
		}
		
		if ( showContextMenuExplosion && codeModel() ->hasFile( m_activeFileName ) )
		{
			//kdDebug() << "CppSupportPart::contextMenu 1" << endl;
			QString candidate;
			if ( isSource( m_activeFileName ) )
				candidate = sourceOrHeaderCandidate();
			else
				candidate = m_activeFileName;
			
			unsigned int curLine = 0, curCol = 0;
			if ( m_activeViewCursor != 0 )
				m_activeViewCursor->cursorPosition( &curLine, &curCol );
			
			//kdDebug() << "CppSupportPart::contextMenu 2: candidate: " << candidate << endl;
			
			if ( !candidate.isEmpty() && codeModel() ->hasFile( candidate ) )
			{
				QPopupMenu * m2 = new QPopupMenu( popup );
				id = popup->insertItem( i18n( "Go to Declaration" ), m2 );
				popup->setWhatsThis( id, i18n( "<b>Go to declaration</b><p>Provides a menu to select available function declarations "
				                               "in the current file and in the corresponding header (if the current file is an implementation) or source (if the current file is a header) file." ) );
				
				FileDom file2 = codeModel() ->fileByName( candidate );
				//kdDebug() << "CppSupportPart::contextMenu 3: " << file2->name() << endl;
				
				FunctionList functionList2 = CodeModelUtils::allFunctions( file2 );
				for ( FunctionList::ConstIterator it = functionList2.begin(); it != functionList2.end(); ++it )
				{
					QString text = ( *it ) ->scope().join( "::" );
					//kdDebug() << "CppSupportPart::contextMenu 3 text: " << text << endl;
					if ( !text.isEmpty() )
					{
						text += "::";
					}
					text += formatModelItem( *it, true );
					text = text.replace( QString::fromLatin1( "&" ), QString::fromLatin1( "&&" ) );
					int id = m2->insertItem( text, this, SLOT( gotoDeclarationLine( int ) ) );
					int line, column;
					( *it ) ->getStartPosition( &line, &column );
					m2->setItemParameter( id, line );
				}
				
				if ( m2->count() == 0 )
				{
					popup->removeItem( id );
				}
				//kdDebug() << "CppSupportPart::contextMenu 4" << endl;
			}
			
			QString candidate1;
			if ( isHeader( m_activeFileName ) )
			{
				candidate1 = sourceOrHeaderCandidate();
			}
			else
			{
				candidate1 = m_activeFileName;
			}
			//kdDebug() << "CppSupportPart::go to definition in " << candidate1 << endl;
			if ( codeModel() ->hasFile( candidate1 ) )
			{
				QPopupMenu * m = new QPopupMenu( popup );
				id = popup->insertItem( i18n( "Go to Definition" ), m );
				popup->setWhatsThis( id, i18n( "<b>Go to definition</b><p>Provides a menu to select available function definitions "
				                               "in the current file and in the corresponding header (if the current file is an implementation) or source (if the current file is a header) file." ) );
				
				const FileDom file = codeModel() ->fileByName( candidate1 );
				const FunctionDefinitionList functionDefinitionList = CodeModelUtils::allFunctionDefinitionsDetailed( file ).functionList;
				for ( FunctionDefinitionList::ConstIterator it = functionDefinitionList.begin(); it != functionDefinitionList.end(); ++it )
				{
					QString text = ( *it ) ->scope().join( "::" );
					if ( !text.isEmpty() )
					{
						text += "::";
					}
					text += formatModelItem( *it, true );
					text = text.replace( QString::fromLatin1( "&" ), QString::fromLatin1( "&&" ) );
					int id = m->insertItem( text, this, SLOT( gotoLine( int ) ) );
					int line, column;
					( *it ) ->getStartPosition( &line, &column );
					m->setItemParameter( id, line );
				}
				if ( m->count() == 0 )
				{
					popup->removeItem( id );
				}
				
			}
		}

		const EditorContext *econtext = static_cast<const EditorContext*>( context );
		QString str = econtext->currentLine();
		if ( str.isEmpty() )
			return ;
		
		QRegExp re( "[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*" );
		if ( !re.exactMatch( str ) )
			return ;
		
		QString popupstr = re.cap( 1 );
		m_contextFileName = findHeader( m_projectFileList, popupstr );
		if ( m_contextFileName.isEmpty() )
			return ;
		
		id = popup->insertItem( i18n( "Goto Include File: %1" ).arg( popupstr ), this, SLOT( slotGotoIncludeFile() ) );
		popup->setWhatsThis( id, i18n( "<b>Goto include file</b><p>Opens an include file under the cursor position." ) );
		
	}
	else if ( context->hasType( Context::CodeModelItemContext ) )
	{
		const CodeModelItemContext * mcontext = static_cast<const CodeModelItemContext*>( context );
		
		if ( mcontext->item() ->isClass() )
		{
			m_activeClass = ( ClassModel* ) mcontext->item();
			int id = popup->insertItem( i18n( "Extract Interface..." ), this, SLOT( slotExtractInterface() ) );
			popup->setWhatsThis( id, i18n( "<b>Extract interface</b><p>Extracts interface from the selected class and creates a new class with this interface. "
			                               "No implementation code is extracted and no implementation code is created." ) );
		}
		else if ( mcontext->item() ->isFunction() )
		{
			m_activeFunction = ( FunctionModel* ) mcontext->item();
		}
	}
	else if ( context->hasType( Context::FileContext ) )
	{
		const FileContext * fc = static_cast<const FileContext*>( context );
		//this is a .ui file and only selection contains only one such file
		KURL url = fc->urls().first();
		kdDebug() << "file context with " << url.path() << endl;
		if ( url.fileName().endsWith( ".ui" ) )
		{
			m_contextFileName = url.path();
			int id = popup->insertItem( i18n( "Create or Select Implementation..." ), this, SLOT( slotCreateSubclass() ) );
			popup->setWhatsThis( id, i18n( "<b>Create or select implementation</b><p>Creates or selects a subclass of selected form for use with integrated KDevDesigner." ) );
		}
	}
}


// Makes sure that header files come first
QStringList CppSupportPart::reorder( const QStringList &list )
{
	QStringList headers, others;
	
	QStringList headerExtensions = QStringList::split( ",", "h,H,hh,hxx,hpp,tlh" );
	
	QStringList::ConstIterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		QString fileName = *it;
		if ( headerExtensions.contains( QFileInfo( *it ).extension() ) )
			headers << ( *it );
		else
			others << ( *it );
	}
	
	return headers + others;
}

void CppSupportPart::addedFilesToProject( const QStringList &fileList )
{
	m_projectFileList = project() ->allFiles();
	QStringList files = reorder( fileList );
	
	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
	{
		QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + ( *it ) );
		
		maybeParse( path );
		emit addedSourceInfo( path );
	}
}

void CppSupportPart::removedFilesFromProject( const QStringList &fileList )
{
	m_projectFileList = project() ->allFiles();
	for ( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );
		kdDebug( 9007 ) << "=====================> remove file: " << path << endl;
		
		removeWithReferences( path );
		m_backgroundParser->removeFile( path );
	}
}

void CppSupportPart::changedFilesInProject( const QStringList & fileList )
{
	QStringList files = reorder( fileList );
	
	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
	{
		QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );
		
		maybeParse( path );
		emit addedSourceInfo( path );
	}
}

void CppSupportPart::savedFile( const KURL &fileName )
{
	Q_UNUSED( fileName.path() );

#if 0  // not needed anymore

	kdDebug( 9007 ) << "savedFile(): " << fileName.mid ( m_projectDirectory.length() + 1 ) << endl;

	if ( m_projectFileList.contains( fileName.mid ( m_projectDirectory.length() + 1 ) ) )
	{
		maybeParse( fileName );
		emit addedSourceInfo( fileName );
	}
#endif
}

QString CppSupportPart::findSourceFile()
{
	// get the path of the currently active document
	QFileInfo fi( m_activeFileName );
	QString path = fi.filePath();
	QString ext = fi.extension();
	// extract the base path (full path without '.' and extension)
	QString base = path.left( path.length() - ext.length() - 1 );
	QStringList candidates;
	
	if ( QStringList::split( ',', "h,H,hh,hxx,hpp,tlh" ).contains( ext ) )
	{
		candidates << ( base + ".c" );
		candidates << ( base + ".cc" );
		candidates << ( base + ".cpp" );
		candidates << ( base + ".c++" );
		candidates << ( base + ".cxx" );
		candidates << ( base + ".C" );
		candidates << ( base + ".m" );
		candidates << ( base + ".mm" );
		candidates << ( base + ".M" );
		candidates << ( base + ".inl" );
		candidates << ( base + "_impl.h" );
	}
	
	QStringList::ConstIterator it;
	for ( it = candidates.begin(); it != candidates.end(); ++it )
	{
		kdDebug( 9007 ) << "Trying " << ( *it ) << endl;
		if ( QFileInfo( *it ).exists() )
		{
			return * it;
		}
	}
	return m_activeFileName;
}

QString CppSupportPart::sourceOrHeaderCandidate( const KURL &url )
{
	QString urlPath;
	if ( url.isEmpty() )
	{
		KTextEditor::Document * doc = 
			dynamic_cast<KTextEditor::Document*>( partController() ->activePart() );
		if ( !doc )
			return QString::null;
		urlPath = doc->url().path();
	}
	else
	{
		urlPath = url.path();
	}
	
	// get the path of the currently active document
	QFileInfo fi( urlPath );
	QString path = fi.filePath();
	// extract the exension
	QString ext = fi.extension();
	if ( ext.isEmpty() )
		return QString::null;
	// extract the base path (full path without '.' and extension)
	QString base = path.left( path.length() - ext.length() - 1 );
	kdDebug( 9007 ) << "base: " << base << ", ext: " << ext << endl;
	// depending on the current extension assemble a list of
	// candidate files to look for
	QStringList candidates;
	// special case for template classes created by the new class dialog
	if ( path.endsWith( "_impl.h" ) )
	{
		QString headerpath = path;
		headerpath.replace( "_impl.h", ".h" );
		candidates << headerpath;
	}
	// if file is a header file search for implementation file
	else if ( QStringList::split( ',', "h,H,hh,hxx,hpp,tlh" ).contains( ext ) )
	{
		candidates << ( base + ".c" );
		candidates << ( base + ".cc" );
		candidates << ( base + ".cpp" );
		candidates << ( base + ".c++" );
		candidates << ( base + ".cxx" );
		candidates << ( base + ".C" );
		candidates << ( base + ".m" );
		candidates << ( base + ".mm" );
		candidates << ( base + ".M" );
		candidates << ( base + ".inl" );
		candidates << ( base + "_impl.h" );
	}
	// if file is an implementation file, search for header file
	else if ( QStringList::split( ',', "c,cc,cpp,c++,cxx,C,m,mm,M,inl" ).contains( ext ) )
	{
		candidates << ( base + ".h" );
		candidates << ( base + ".H" );
		candidates << ( base + ".hh" );
		candidates << ( base + ".hxx" );
		candidates << ( base + ".hpp" );
		candidates << ( base + ".tlh" );
	}
	// search for files from the assembled candidate lists, return the first
	// candidate file that actually exists or QString::null if nothing is found.
	QStringList::ConstIterator it;
	for ( it = candidates.begin(); it != candidates.end(); ++it )
	{
		kdDebug( 9007 ) << "Trying " << ( *it ) << endl;
		if ( QFileInfo( *it ).exists() )
		{
			return * it;
		}
	}
	return QString::null;
}

void CppSupportPart::slotSwitchHeader( bool scrollOnly )
{
	QString candidate = sourceOrHeaderCandidate();
	if ( candidate == QString::null )
		return ;
	
	bool attemptMatch = true;
	KConfig *config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "General" );
		attemptMatch = config->readBoolEntry( "SwitchShouldMatch", true );
	}

	// ok, both files exist. Do the codemodel have them?
	if ( codeModel() ->hasFile( m_activeFileName ) && codeModel()->hasFile( candidate ) &&
	     m_activeViewCursor && attemptMatch )
	{
		unsigned int currentline, column;
		m_activeViewCursor->cursorPosition( &currentline, &column );
		
		if ( isHeader( m_activeFileName ) )
		{
			// we're in the header file, let's find the current function declaration
			FileDom header = codeModel() ->fileByName( m_activeFileName );
			FunctionList functionList = CodeModelUtils::allFunctionsDetailed( header ).functionList;
			
			FunctionList::ConstIterator it_decl = functionList.begin();
			while ( it_decl != functionList.end() )
			{
				int startline, column, endLine, endColumn;
				( *it_decl ) ->getStartPosition( &startline, &column );
				( *it_decl ) ->getEndPosition( &endLine, &endColumn );
				if ( (int)currentline >= startline && (int)currentline <= endLine )
				{
					// found it. can we find a matching defintion?
					FileDom source = codeModel() ->fileByName( candidate );
					FunctionDefinitionList functionDefList = CodeModelUtils::allFunctionDefinitionsDetailed( source ).functionList;
					for ( FunctionDefinitionList::ConstIterator it_def = functionDefList.begin();
					      it_def != functionDefList.end(); ++it_def )
					{
						if ( CodeModelUtils::compareDeclarationToDefinition( *it_decl, *it_def ) )
						{
							// found the declaration, let's jump!
							int line, column;
							( *it_def ) ->getStartPosition( &line, &column );
							KURL url;
							url.setPath( candidate );
							if ( scrollOnly )
								partController() ->scrollToLineColumn( url, line );
							else if ( !splitHeaderSourceConfig()->splitEnabled() )
								partController() ->editDocument( url, line );
							else
								partController() ->splitCurrentDocument( url, line );
							return ;
						}
					}
					break;
				}
				++it_decl;
			}
		}
		else if ( isSource( m_activeFileName ) )
		{
			// we're in the source file, let's find the current function definition
			FileDom header = codeModel() ->fileByName( m_activeFileName );
			FunctionDefinitionList functionDefList = CodeModelUtils::allFunctionDefinitionsDetailed( header ).functionList;
			
			FunctionDefinitionList::ConstIterator it_def = functionDefList.begin();
			while ( it_def != functionDefList.end() )
			{
				int startline, column, endLine, endColumn;
				( *it_def ) ->getStartPosition( &startline, &column );
				( *it_def ) ->getEndPosition( &endLine, &endColumn );
				if ( (int)currentline >= startline && (int)currentline <= endLine )
				{
					// found it. can we find a matching declaration?
					FileDom source = codeModel() ->fileByName( candidate );
					FunctionList functionList = CodeModelUtils::allFunctionsDetailed( source ).functionList;
					for ( FunctionList::ConstIterator it_decl = functionList.begin();
					      it_decl != functionList.end(); ++it_decl )
					{
						if ( CodeModelUtils::compareDeclarationToDefinition( *it_decl, *it_def ) )
						{
							// found the declaration, let's jump!
							int line, column;
							( *it_decl ) ->getStartPosition( &line, &column );
							KURL url;
							url.setPath( candidate );
							if ( scrollOnly )
								partController() ->scrollToLineColumn( url, line );
							else if ( !splitHeaderSourceConfig()->splitEnabled() )
								partController() ->editDocument( url, line );
							else
								partController() ->splitCurrentDocument( url, line );
							return ;
						}
					}
					break;
				}
				++it_def;
			}
		}
	}
	
	// last chance
	KURL url;
	url.setPath( candidate );
	
	if ( scrollOnly )
		return;
	else if ( !splitHeaderSourceConfig()->splitEnabled() )
		partController() ->editDocument( url );
	else
		partController() ->splitCurrentDocument( url );
}

void CppSupportPart::slotGotoIncludeFile()
{
	if ( !m_contextFileName.isEmpty() )
		partController() ->editDocument( KURL( m_contextFileName ), 0 );
	
}

KDevLanguageSupport::Features CppSupportPart::features()
{
	if ( withcpp )
		return Features( Classes | Structs | Functions | Variables | Namespaces | Declarations
		                 | Signals | Slots | AddMethod | AddAttribute | NewClass | CreateAccessMethods );
	else
		return Features ( Structs | Functions | Variables | Declarations );
}

QString CppSupportPart::formatClassName( const QString &name )
{
	QString n = name;
	return n.replace( ".", "::" );
}

QString CppSupportPart::unformatClassName( const QString &name )
{
	QString n = name;
	return n.replace( "::", "." );
}

bool CppSupportPart::shouldSplitDocument(const KURL &url)
{
	if ( !splitHeaderSourceConfig()->splitEnabled() )
		return false;
	
	KURL::List list = partController()->openURLs();
	KURL::List::ConstIterator it = list.begin();
	while ( it != list.end() )
	{
		QString candidate = sourceOrHeaderCandidate( ( *it ) );
		if ( candidate.isEmpty() )
			continue;
		
		KURL urlCandidate;
		urlCandidate.setPath( candidate );
		if ( url == urlCandidate )
		{
			// It is already open, so switch to it so 
			// our split view will open with it
			partController() ->editDocument( ( *it ) );
			return true;
		}
		++it;
	}
	return false;
}

Qt::Orientation CppSupportPart::splitOrientation() const
{
	QString o = splitHeaderSourceConfig()->orientation();
	if ( o == "Vertical" )
		return Qt::Vertical;
	else
		return Qt::Horizontal;
}

void CppSupportPart::slotNewClass()
{
	CppNewClassDialog dlg( this );
	dlg.exec();
}

void CppSupportPart::addMethod( ClassDom klass )
{
	if ( !klass )
	{
		KMessageBox::error( 0, i18n( "Please select a class." ), i18n( "Error" ) );
		return ;
	}
	
	AddMethodDialog dlg( this, klass, mainWindow() ->main() );
	dlg.exec();
}

void CppSupportPart::addAttribute( ClassDom klass )
{
	if ( !klass )
	{
		KMessageBox::error( 0, i18n( "Please select a class." ), i18n( "Error" ) );
		return ;
	}
	
	AddAttributeDialog dlg( this, klass, mainWindow() ->main() );
	dlg.exec();
}

void CppSupportPart::slotCompleteText()
{
	if ( !m_pCompletion )
		return ;
	m_pCompletion->completeText( true );
}

/**
 * parsing stuff for project persistant classstore and code completion
 */
void CppSupportPart::initialParse( )
{
	// For debugging
	if ( !project( ) )
	{
		// messagebox ?
		kdDebug( 9007 ) << "No project" << endl;
		return ;
	}
	
	parseProject( );
	m_valid = true;
	return ;
}

bool CppSupportPart::parseProject( bool force )
{
	kdDebug( 9007 ) << "CppSupportPart::parseProject 1" << endl;
	mainWindow() ->statusBar() ->message( i18n( "Updating..." ) );
	
	kapp->setOverrideCursor( waitCursor );
	
	_jd = new JobData;
	_jd->file.setName( project() ->projectDirectory() + "/" + project() ->projectName() + ".pcs" );
	
	QString skip_file_name = project() ->projectDirectory() + "/" + project() ->projectName() + ".ignore_pcs";
	
	if ( !force && !QFile::exists( skip_file_name ) && _jd->file.open( IO_ReadOnly ) )
	{
		_jd->stream.setDevice( &( _jd->file ) );
		
		createIgnorePCSFile();
		
		QString sig;
		int pcs_version = 0;
		_jd->stream >> sig >> pcs_version;
		if ( sig == "PCS" && pcs_version == KDEV_PCS_VERSION )
		{
			
			int numFiles = 0;
			_jd->stream >> numFiles;
			
			for ( int i = 0; i < numFiles; ++i )
			{
				QString fn;
				uint ts;
				uint offset;
				
				_jd->stream >> fn >> ts >> offset;
				_jd->pcs[ fn ] = qMakePair( ts, offset );
			}
		}
	}
	kdDebug( 9007 ) << "CppSupportPart::parseProject 2" << endl;
	
	_jd->files = reorder( modifiedFileList() );
	kdDebug( 9007 ) << "CppSupportPart::parseProject 3" << endl;
	
	QProgressBar* bar = new QProgressBar( _jd->files.count( ), mainWindow( ) ->statusBar( ) );
	bar->setMinimumWidth( 120 );
	bar->setCenterIndicator( true );
	mainWindow( ) ->statusBar( ) ->addWidget( bar );
	bar->show( );
	kdDebug( 9007 ) << "CppSupportPart::parseProject 4" << endl;
	
	_jd->progressBar = bar;
	_jd->dir.setPath( m_projectDirectory );
	_jd->it = _jd->files.begin();
	
	kdDebug( 9007 ) << "CppSupportPart::parseProject 5" << endl;
	QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );
	
	return true;
}

void CppSupportPart::slotParseFiles()
{
	// NOTE: The checking for m_projectClosed is actually (currently) not needed.
	// When the project is closed, the language support plugin is destroyed
	// and as a consequence, the timer job signal never arrives at this method

	if ( !m_projectClosed && _jd->it != _jd->files.end() )
	{
		_jd->progressBar->setProgress( _jd->progressBar->progress() + 1 );
		
		QFileInfo fileInfo( _jd->dir, *( _jd->it ) );
		
		if ( fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() )
		{
			QString absFilePath = URLUtil::canonicalPath( fileInfo.absFilePath() );
			
			if ( isValidSource( absFilePath ) )
			{
				QDateTime t = fileInfo.lastModified();
				if ( ! ( m_timestamp.contains( absFilePath ) && m_timestamp[ absFilePath ] == t ) )
				{
					if ( _jd->pcs.contains( absFilePath ) && t.toTime_t() == _jd->pcs[ absFilePath ].first )
					{
						_jd->stream.device() ->at( _jd->pcs[ absFilePath ].second );
						FileDom file = codeModel() ->create<FileModel>();
						file->read( _jd->stream );
						codeModel() ->addFile( file );
					}
					else
					{
						kdDebug( 9007 ) << "newly parsing: " << absFilePath << endl;
						m_driver->parseFile( absFilePath );
					}
					
					m_timestamp[ absFilePath ] = t;
				}
			}
		}
		
		++( _jd->it );
		QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );
	}
	else // finished or interrupted
	{
		kapp->restoreOverrideCursor( );
		mainWindow( ) ->statusBar( ) ->removeWidget( _jd->progressBar );
		
		if ( !m_projectClosed )
		{
			kdDebug( 9007 ) << "updating sourceinfo" << endl;
			emit updatedSourceInfo();
			mainWindow( ) ->statusBar( ) ->message( i18n( "Done" ), 2000 );
			QFile::remove
				( project() ->projectDirectory() + "/" + project() ->projectName() + ".ignore_pcs" );
		}
		else
		{
			kdDebug( 9007 ) << "ABORT" << endl;
		}
		
		delete _jd;
		_jd = 0;
	}
}

void CppSupportPart::maybeParse( const QString& fileName )
{
	if ( !isValidSource( fileName ) )
		return ;
	
	QFileInfo fileInfo( fileName );
	QString path = URLUtil::canonicalPath( fileName );
	QDateTime t = fileInfo.lastModified();
	
	if ( !fileInfo.exists() )
	{
		removeWithReferences( path );
		return ;
	}
	
	QMap<QString, QDateTime>::Iterator it = m_timestamp.find( path );
	if ( it != m_timestamp.end() && *it == t )
	{
		return ;
	}
	
	m_timestamp[ path ] = t;
	m_driver->parseFile( path );
}

void CppSupportPart::slotNeedTextHint( int line, int column, QString& textHint )
{
	if ( 1 || !m_activeEditor )
		return ;
	
	m_backgroundParser->lock();
	TranslationUnitAST* ast = m_backgroundParser->translationUnit( m_activeFileName );
	AST* node = 0;
	if ( ast && ( node = findNodeAt( ast, line, column ) ) )
	{
		
		while ( node && node->nodeType() != NodeType_FunctionDefinition )
			node = node->parent();
		
		if ( node )
		{
			int startLine, startColumn;
			int endLine, endColumn;
			node->getStartPosition( &startLine, &startColumn );
			node->getEndPosition( &endLine, &endColumn );
			
			if ( !node->text().isNull() )
				textHint = node->text();
			else
				textHint = m_activeEditor->textLine( startLine ).simplifyWhiteSpace();
		}
	}
	m_backgroundParser->unlock();
}

void CppSupportPart::MakeMemberHelper( QString& text, int& atLine, int& atColumn )
{
	if ( !m_activeViewCursor || !m_valid )
		return ;

	m_backgroundParser->lock();
	TranslationUnitAST* translationUnit = m_backgroundParser->translationUnit( m_activeFileName );
	if ( translationUnit )
	{
		unsigned int line, column;
		m_activeViewCursor->cursorPositionReal( &line, &column );
		
		AST* currentNode = findNodeAt( translationUnit, line, column );
		DeclaratorAST* declarator = 0;
		while ( currentNode && currentNode->nodeType() != NodeType_SimpleDeclaration )
		{
			if ( currentNode->nodeType() == NodeType_Declarator )
				declarator = ( DeclaratorAST* ) currentNode;
			currentNode = currentNode->parent();
		}
		SimpleDeclarationAST* decl = currentNode ? ( SimpleDeclarationAST* ) currentNode : 0;
		if ( decl && decl->initDeclaratorList() && !declarator )
		{
			InitDeclaratorAST * i = decl->initDeclaratorList() ->initDeclaratorList().at( 0 );
			if ( i )
				declarator = i->declarator();
		}
		
		if ( decl && declarator && declarator->parameterDeclarationClause() )
		{
			
			QStringList scope;
			scopeOfNode( decl, scope );
			
			QString scopeStr = scope.join( "::" );
			if ( !scopeStr.isEmpty() )
				scopeStr += "::";
			
			QString declStr = declaratorToString( declarator, scopeStr ).simplifyWhiteSpace();
			if ( declarator->exceptionSpecification() )
			{
				declStr += QString::fromLatin1( " throw( " );
				QPtrList<AST> l = declarator->exceptionSpecification() ->nodeList();
				QPtrListIterator<AST> type_it( l );
				while ( type_it.current() )
				{
					declStr += type_it.current() ->text();
					++type_it;
					
					if ( type_it.current() )
						declStr += QString::fromLatin1( ", " );
				}
				
				declStr += QString::fromLatin1( " )" );
			}
			
			text += "\n\n";
			QString type = typeSpecToString( decl->typeSpec() );
			text += type;
			if ( !type.isNull() )
				text += + " ";
			
			text += declStr + "\n{\n}";
		}
		
		m_backgroundParser->unlock();
		
		QString implFile = findSourceFile();
		
		m_backgroundParser->lock();
		translationUnit = m_backgroundParser->translationUnit( implFile );
		if ( translationUnit )
			translationUnit->getEndPosition( &atLine, &atColumn );
		else
		{
			atLine = -2;
			atColumn = 0;
		}
		kdDebug() << "at line in mm: " << atLine << endl;
	}
	m_backgroundParser->unlock();
}

void CppSupportPart::slotMakeMember()
{
	QString text;
	int atColumn, atLine;
	MakeMemberHelper( text, atLine, atColumn );
	
	if ( !text.isEmpty() )
	{
		QString implFile = findSourceFile();
		
		if ( !implFile.isEmpty() )
		{
			partController() ->editDocument( KURL( implFile ) );
			kapp->processEvents( 500 );
		}
		if ( atLine == -2 )
			atLine = m_activeEditor->numLines() - 1;
		
		m_backgroundParser->lock ()
			;
		
		kdDebug() << "at line in mm: " << atLine << " atCol: " << atColumn << endl;
		kdDebug() << "text: " << text << endl;
		if ( m_activeEditor )
			m_activeEditor->insertText( atLine, atColumn, text );
		if ( m_activeViewCursor )
			m_activeViewCursor->setCursorPositionReal( atLine + 3, 1 );
		
		m_backgroundParser->unlock();
	}
}

QStringList CppSupportPart::subclassWidget( const QString& formName )
{
	QStringList newFileNames;
	SubclassingDlg *dlg = new SubclassingDlg( this, formName, newFileNames );
	dlg->exec();
	return newFileNames;
}

QStringList CppSupportPart::updateWidget( const QString& formName, const QString& fileName )
{
	QStringList dummy;
	SubclassingDlg *dlg = new SubclassingDlg( this, formName, fileName, dummy );
	dlg->exec();
	return dummy;
}

void CppSupportPart::partRemoved( KParts::Part* part )
{
	kdDebug( 9032 ) << "CppSupportPart::partRemoved()" << endl;
	
	if ( KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part ) )
	{
		
		QString fileName = doc->url().path();
		if ( !isValidSource( fileName ) )
			return ;
		
		QString canonicalFileName = URLUtil::canonicalPath( fileName );
		m_backgroundParser->removeFile( canonicalFileName );
		m_backgroundParser->addFile( canonicalFileName, true );
	}
}

void CppSupportPart::slotProjectCompiled()
{
	kdDebug( 9007 ) << "CppSupportPart::slotProjectCompiled()" << endl;
	parseProject();
}

QStringList CppSupportPart::modifiedFileList()
{
	QStringList lst;
	
	QStringList fileList = m_projectFileList;
	QStringList::Iterator it = fileList.begin();
	while ( it != fileList.end() )
	{
		QString fileName = *it;
		++it;
		
		QFileInfo fileInfo( m_projectDirectory, fileName );
		QString path = URLUtil::canonicalPath( fileInfo.absFilePath() );
		
		if ( !( isSource( path ) || isHeader( path ) ) )
			continue;
		
		QDateTime t = fileInfo.lastModified();
		
		QMap<QString, QDateTime>::Iterator dictIt = m_timestamp.find( path );
		if ( fileInfo.exists() && dictIt != m_timestamp.end() && *dictIt == t )
			continue;
		
		lst << fileName;
	}
	
	return lst;
}

KTextEditor::Document * CppSupportPart::findDocument( const KURL & url )
{
	if ( !partController() ->parts() )
		return 0;
	
	QPtrList<KParts::Part> parts( *partController() ->parts() );
	QPtrListIterator<KParts::Part> it( parts );
	while ( KParts::Part * part = it.current() )
	{
		KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part );
		if ( doc && doc->url() == url )
			return doc;
		++it;
	}
	
	return 0;
}

void CppSupportPart::setupCatalog( )
{
	kdDebug( 9007 ) << "CppSupportPart::setupCatalog()" << endl;

	KStandardDirs *dirs = CppSupportFactory::instance() ->dirs();
	QStringList pcsList = dirs->findAllResources( "pcs", "*.db", false, true );
	QStringList pcsIdxList = dirs->findAllResources( "pcs", "*.idx", false, true );

	QStringList enabledPCSs;
	if ( DomUtil::elementByPath( *project() ->projectDom(), "kdevcppsupport/references" ).isNull() )
	{
		for ( QStringList::Iterator it = pcsList.begin(); it != pcsList.end(); ++it )
		{
			enabledPCSs.push_back( QFileInfo( *it ).baseName() );
		}
	}
	else
	{
		enabledPCSs = DomUtil::readListEntry( *project() ->projectDom(), "kdevcppsupport/references", "pcs" );
	}

	QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName" << "prefix";

	if ( pcsList.size() && pcsVersion() < KDEV_DB_VERSION )
	{
		QStringList l = pcsList + pcsIdxList;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
		int rtn = KMessageBox::questionYesNoList( 0, i18n( "Persistant class store will be disabled: you have a wrong version of pcs installed.\nRemove old pcs files?" ), l, i18n( "C++ Support" ), KStdGuiItem::del(), KStdGuiItem::cancel() );
#else
		int rtn = KMessageBox::questionYesNoList( 0, i18n( "Persistant class store will be disabled: you have a wrong version of pcs installed.\nRemove old pcs files?" ), l, i18n( "C++ Support" ), KGuiItem( i18n( "&Delete" ), "editdelete", i18n( "Delete item(s)" )), KStdGuiItem::cancel() );
#endif

		if ( rtn == KMessageBox::Yes )
		{
			QStringList::Iterator it = l.begin();
			while ( it != l.end() )
			{
				QFile::remove
					( *it );
				++it;
			}
			// @todo regenerate the pcs list
			pcsList.clear();
		}
		else
		{
			return ;
		}
	}

	QStringList::Iterator it = pcsList.begin();
	while ( it != pcsList.end() )
	{
		Catalog * catalog = new Catalog();
		catalog->open( *it );
		catalog->setEnabled( enabledPCSs.contains( QFileInfo( *it ).baseName() ) );
		++it;

		for ( QStringList::Iterator idxIt = indexList.begin(); idxIt != indexList.end(); ++idxIt )
			catalog->addIndex( ( *idxIt ).utf8() );

		m_catalogList.append( catalog );
		codeRepository() ->registerCatalog( catalog );
	}

	setPcsVersion( KDEV_DB_VERSION );
}

KMimeType::List CppSupportPart::mimeTypes( )
{
	QStringList mimeList;
	mimeList += m_headerMimeTypes;
	mimeList += m_sourceMimeTypes;

	KMimeType::List list;
	for ( QStringList::Iterator it = mimeList.begin(); it != mimeList.end(); ++it )
	{
		if ( KMimeType::Ptr mime = KMimeType::mimeType( *it ) )
			list << mime;
	}

	return list;
}

int CppSupportPart::pcsVersion()
{
	KConfig * config = CppSupportFactory::instance() ->config();
	KConfigGroupSaver cgs( config, "PCS" );
	return config->readNumEntry( "Version", 0 );
}

void CppSupportPart::setPcsVersion( int version )
{
	KConfig * config = CppSupportFactory::instance() ->config();
	KConfigGroupSaver cgs( config, "PCS" );
	config->writeEntry( "Version", version );
	config->sync();
}

QString CppSupportPart::formatTag( const Tag & inputTag )
{
	Tag tag = inputTag;
	
	switch ( tag.kind() )
	{
	case Tag::Kind_Namespace:
		return QString::fromLatin1( "namespace " ) + tag.name();
		
	case Tag::Kind_Class:
		return QString::fromLatin1( "class " ) + tag.name();
		
	case Tag::Kind_Function:
	case Tag::Kind_FunctionDeclaration:
		{
			CppFunction<Tag> tagInfo( tag );
			return tagInfo.name() + "( " + tagInfo.arguments().join( ", " ) + " ) : " + tagInfo.type();
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

void CppSupportPart::codeCompletionConfigStored( )
{
	partController() ->setActivePart( partController() ->activePart() );
}

void CppSupportPart::splitHeaderSourceConfigStored( )
{
	QString o = splitHeaderSourceConfig()->orientation();
	if ( o == "Vertical" )
		emit splitOrientationChanged( Qt::Vertical );
	else if ( o == "Horizontal" )
		emit splitOrientationChanged( Qt::Horizontal );
}

void CppSupportPart::removeWithReferences( const QString & fileName )
{
kdDebug( 9007 ) << "remove with references: " << fileName << endl;
	m_timestamp.remove( fileName );
	if ( !codeModel() ->hasFile( fileName ) )
		return ;
	
	emit aboutToRemoveSourceInfo( fileName );
	
	codeModel() ->removeFile( codeModel() ->fileByName( fileName ) );
}

bool CppSupportPart::isValidSource( const QString& fileName ) const
{
	QFileInfo fileInfo( fileName );
	QString path = URLUtil::canonicalPath( fileInfo.absFilePath() );
	
	return project() && project() ->isProjectFile( path )
		&& ( isSource( path ) || isHeader( path ) )
		&& !QFile::exists( fileInfo.dirPath( true ) + "/.kdev_ignore" );
}

QString CppSupportPart::formatModelItem( const CodeModelItem *item, bool shortDescription )
{
	if ( item->isFunction() || item->isFunctionDefinition() )
	{
		const FunctionModel * model = static_cast<const FunctionModel*>( item );
		QString function;
		QString args;
		ArgumentList argumentList = model->argumentList();
		for ( ArgumentList::const_iterator it = argumentList.begin(); it != argumentList.end(); ++it )
		{
			args.isEmpty() ? args += "" : args += ", " ;
			args += formatModelItem( ( *it ).data() );
		}
		if ( !shortDescription )
			function += ( model->isVirtual() ? QString( "virtual " ) : QString( "" ) ) + model->resultType() + " ";
		
		function += model->name() + "(" + args + ")" + ( model->isConstant() ? QString( " const" ) : QString( "" ) ) +
			( model->isAbstract() ? QString( " = 0" ) : QString( "" ) );
		
		return function;
	}
	else if ( item->isVariable() )
	{
		const VariableModel * model = static_cast<const VariableModel*>( item );
		if ( shortDescription )
			return model->name();
		return model->type() + " " + model->name();
	}
	else if ( item->isArgument() )
	{
		const ArgumentModel * model = static_cast<const ArgumentModel*>( item );
		QString arg;
		if ( !shortDescription )
			arg += model->type() + " ";
		arg += model->name();
		if ( !shortDescription )
			arg += model->defaultValue().isEmpty() ? QString( "" ) : QString( " = " ) + model->defaultValue();
		return arg.stripWhiteSpace();
	}
	else
		return KDevLanguageSupport::formatModelItem( item, shortDescription );
}

void CppSupportPart::addClass()
{
	slotNewClass();
}

void CppSupportPart::saveProjectSourceInfo()
{
	const FileList fileList = codeModel() ->fileList();
	
	if ( !project() || fileList.isEmpty() )
		return ;
	
	QFile f( project() ->projectDirectory() + "/" + project() ->projectName() + ".pcs" );
	if ( !f.open( IO_WriteOnly ) )
		return ;
	
	createIgnorePCSFile();
	
	QDataStream stream( &f );
	QMap<QString, uint> offsets;
	
	QString pcs( "PCS" );
	stream << pcs << KDEV_PCS_VERSION;
	
	stream << int( fileList.size() );
	for ( FileList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		const FileDom dom = ( *it );
		stream << dom->name() << m_timestamp[ dom->name() ].toTime_t();
		offsets.insert( dom->name(), stream.device() ->at() );
		stream << ( uint ) 0; // dummy offset
	}
	
	for ( FileList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		const FileDom dom = ( *it );
		int offset = stream.device() ->at();
		
		dom->write( stream );
		
		int end = stream.device() ->at();
		
		stream.device() ->at( offsets[ dom->name() ] );
		stream << offset;
		stream.device() ->at( end );
	}
	
	QString skip_file_name = project() ->projectDirectory() + "/" + project() ->projectName() + ".ignore_pcs";
	QFile::remove
		( skip_file_name );
}

QString CppSupportPart::extractInterface( const ClassDom& klass )
{
	QString txt;
	QTextStream stream( &txt, IO_WriteOnly );
	
	QString name = klass->name() + "Interface";
	QString ind;
	ind.fill( QChar( ' ' ), 4 );
	
	stream
		<< "class " << name << "\n"
		<< "{" << "\n"
		<< "public:" << "\n"
		<< ind << name << "() {}" << "\n"
		<< ind << "virtual ~" << name << "() {}" << "\n"
		<< "\n";
	
	const FunctionList functionList = klass->functionList();
	for ( FunctionList::ConstIterator it = functionList.begin(); it != functionList.end(); ++it )
	{
		const FunctionDom& fun = *it;
		
		if ( !fun->isVirtual() || fun->name().startsWith( "~" ) )
			continue;
		
		stream << ind << formatModelItem( fun );
		if ( !fun->isAbstract() )
			stream << " = 0";
		
		stream << ";\n";
	}
	
	stream
		<< "\n"
		<< "private:" << "\n"
		<< ind << name << "( const " << name << "& source );" << "\n"
		<< ind << "void operator = ( const " << name << "& source );" << "\n"
		<< "};" << "\n\n";
	
	return txt;
}

void CppSupportPart::slotExtractInterface( )
{
	if ( !m_activeClass )
		return ;
	
	QFileInfo fileInfo( m_activeClass->fileName() );
	QString ifaceFileName = fileInfo.dirPath( true ) + "/" + m_activeClass->name().lower() + "_interface.h";
	if ( QFile::exists( ifaceFileName ) )
	{
		KMessageBox::error( mainWindow() ->main(), i18n( "File %1 already exists" ).arg( ifaceFileName ),
		                    i18n( "C++ Support" ) );
	}
	else
	{
		QString text = extractInterface( m_activeClass );
		
		QFile f( ifaceFileName );
		if ( f.open( IO_WriteOnly ) )
		{
			QTextStream stream( &f );
			stream
				<< "#ifndef __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n"
				<< "#define __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n"
				<< "\n"
				<< extractInterface( m_activeClass )
				<< "\n"
				<< "#endif // __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n";
			f.close();
			
			project() ->addFile( ifaceFileName );
		}
	}
	
	m_activeClass = 0;
}

void CppSupportPart::gotoLine( int line )
{
	if ( isHeader( m_activeFileName ) )
	{
		KURL url;
		url.setPath( sourceOrHeaderCandidate() );
		partController() ->editDocument( url, line );
	}
	else
		m_activeViewCursor->setCursorPositionReal( line, 0 );
}

void CppSupportPart::recomputeCodeModel( const QString& fileName )
{
	if ( codeModel() ->hasFile( fileName ) )
	{
		FileDom file = codeModel() ->fileByName( fileName );
		removeWithReferences( fileName );
	}
	
	m_backgroundParser->lock();
	if ( TranslationUnitAST * ast = m_backgroundParser->translationUnit( fileName ) )
	{
		
		if ( true /*!hasErrors*/ )
		{
			StoreWalker walker( fileName, codeModel() );
			walker.parseTranslationUnit( ast );
			codeModel() ->addFile( walker.file() );
			emit addedSourceInfo( fileName );
		}
	}
	m_backgroundParser->unlock();
}

void CppSupportPart::emitFileParsed( )
{
	emit fileParsed( m_activeFileName );
}

bool CppSupportPart::isHeader( const QString& fileName ) const
{
	KMimeType::Ptr ptr = KMimeType::findByPath( fileName );
	if ( ptr && m_headerMimeTypes.contains( ptr->name() ) )
		return true;
	
	return m_headerExtensions.contains( QFileInfo( fileName ).extension() );
}

bool CppSupportPart::isSource( const QString& fileName ) const
{
	KMimeType::Ptr ptr = KMimeType::findByPath( fileName );
	if ( ptr && m_sourceMimeTypes.contains( ptr->name() ) )
		return true;
	
	return m_sourceExtensions.contains( QFileInfo( fileName ).extension() );
}

void CppSupportPart::gotoDeclarationLine( int line )
{
	if ( isHeader( m_activeFileName ) )
		m_activeViewCursor->setCursorPositionReal( line, 0 );
	else
	{
		KURL url;
		url.setPath( sourceOrHeaderCandidate() );
		partController() ->editDocument( url, line );
	}
}

void CppSupportPart::removeCatalog( const QString & dbName )
{
	if ( !QFile::exists( dbName ) )
		return ;
	
	QValueList<Catalog*> catalogs = codeRepository() ->registeredCatalogs();
	Catalog* c = 0;
	for ( QValueList<Catalog*>::Iterator it = catalogs.begin(); it != catalogs.end(); ++it )
	{
		if ( ( *it ) ->dbName() == dbName )
		{
			c = *it;
			break;
		}
	}
	
	if ( c )
	{
		codeRepository() ->unregisterCatalog( c );
		m_catalogList.remove( c );
	}
	
	QFileInfo fileInfo( dbName );
	QDir dir( fileInfo.dir( true ) );
	QStringList fileList = dir.entryList( fileInfo.baseName() + "*.idx" );
	for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString idxName = fileInfo.dirPath( true ) + "/" + *it;
	kdDebug( 9007 ) << "=========> remove db index: " << idxName << endl;
		dir.remove( *it );
	}
	
	dir.remove( fileInfo.fileName() );
}

void CppSupportPart::addCatalog( Catalog * catalog )
{
	m_catalogList.append( catalog );
	codeRepository() ->registerCatalog( catalog );
}

FunctionDefinitionDom CppSupportPart::functionDefinitionAt( int line, int column )
{
	if ( !codeModel() ->hasFile( m_activeFileName ) )
		return FunctionDefinitionDom();
	
	FileDom file = codeModel() ->fileByName( m_activeFileName );
	return functionDefinitionAt( model_cast<NamespaceDom>( file ), line, column );
}

FunctionDefinitionDom CppSupportPart::currentFunctionDefinition( )
{
	if ( !this->m_activeViewCursor )
		return FunctionDefinitionDom();
	
	unsigned int line, column;
	this->m_activeViewCursor->cursorPositionReal( &line, &column );
	return functionDefinitionAt( line, column );
}

FunctionDefinitionDom CppSupportPart::functionDefinitionAt( NamespaceDom ns, int line, int column )
{
	NamespaceList namespaceList = ns->namespaceList();
	for ( NamespaceList::Iterator it = namespaceList.begin();
	      it != namespaceList.end(); ++it )
	{
		if ( FunctionDefinitionDom def = functionDefinitionAt( *it, line, column ) )
			return def;
	}
	
	ClassList classList = ns->classList();
	for ( ClassList::Iterator it = classList.begin();
	      it != classList.end(); ++it )
	{
		if ( FunctionDefinitionDom def = functionDefinitionAt( *it, line, column ) )
			return def;
	}
	
	FunctionDefinitionList functionDefinitionList = ns->functionDefinitionList();
	for ( FunctionDefinitionList::Iterator it = functionDefinitionList.begin();
	      it != functionDefinitionList.end(); ++it )
	{
		if ( FunctionDefinitionDom def = functionDefinitionAt( *it, line, column ) )
			return def;
	}
	
	return FunctionDefinitionDom();
}

FunctionDefinitionDom CppSupportPart::functionDefinitionAt( ClassDom klass, int line, int column )
{
	ClassList classList = klass->classList();
	for ( ClassList::Iterator it = classList.begin();
	      it != classList.end(); ++it )
	{
		if ( FunctionDefinitionDom def = functionDefinitionAt( *it, line, column ) )
			return def;
	}
	
	FunctionDefinitionList functionDefinitionList = klass->functionDefinitionList();
	for ( FunctionDefinitionList::Iterator it = functionDefinitionList.begin();
	      it != functionDefinitionList.end(); ++it )
	{
		if ( FunctionDefinitionDom def = functionDefinitionAt( *it, line, column ) )
			return def;
	}
	
	return FunctionDefinitionDom();
}

FunctionDefinitionDom CppSupportPart::functionDefinitionAt( FunctionDefinitionDom fun, int line, int column )
{
	int startLine, startColumn;
	int endLine, endColumn;
	
	fun->getStartPosition( &startLine, &startColumn );
	fun->getEndPosition( &endLine, &endColumn );
	
	if ( ! ( line >= startLine && line <= endLine ) )
		return FunctionDefinitionDom();
	
	if ( line == startLine && column < startColumn )
		return FunctionDefinitionDom();
	
	if ( line == endLine && column > endColumn )
		return FunctionDefinitionDom();
	
	return fun;
}

void CppSupportPart::slotCursorPositionChanged()
{
	//    m_functionHintTimer->changeInterval( 1000 );
	if ( splitHeaderSourceConfig()->splitEnabled() 
	     && splitHeaderSourceConfig()->autoSync() )
		slotSwitchHeader( true );
}

void CppSupportPart::slotFunctionHint( )
{
	kdDebug( 9007 ) << "=======> compute current function definition" << endl;
	m_functionHintTimer->stop();
	if ( FunctionDefinitionDom fun = currentFunctionDefinition() )
	{
		QStringList scope = fun->scope();
		QString funName = scope.join( "::" );
		if ( !funName.isEmpty() )
			funName += "::";
		
		funName += formatModelItem( fun, true );
		
		mainWindow() ->statusBar() ->message( funName, 2000 );
	}
}

void CppSupportPart::createIgnorePCSFile( )
{
	static QCString skip_me( "ignore me\n" );
	
	QString skip_file_name = project() ->projectDirectory() + "/" + project() ->projectName() + ".ignore_pcs";
	QFile skip_pcs_file( skip_file_name );
	if ( skip_pcs_file.open( IO_WriteOnly ) )
	{
		skip_pcs_file.writeBlock( skip_me );
		skip_pcs_file.close();
	}
}

QString CppSupportPart::specialHeaderName( bool local ) const
{
	if ( local )
		return ::locateLocal( "data", "kdevcppsupport/configuration", CppSupportFactory::instance() );
	
	return ::locate( "data", "kdevcppsupport/configuration", CppSupportFactory::instance() );
}

void CppSupportPart::updateParserConfiguration()
{
	m_backgroundParser->updateParserConfiguration();
	
	QString conf_file_name = specialHeaderName();
	m_driver->removeAllMacrosInFile( conf_file_name );
	
	m_driver->parseFile( conf_file_name, true );
	
	parseProject( true );
}

KDevDesignerIntegration * CppSupportPart::designer( KInterfaceDesigner::DesignerType type )
{
	KDevDesignerIntegration * des = 0;
	switch ( type )
	{
	case KInterfaceDesigner::Glade:
	case KInterfaceDesigner::QtDesigner:
		des = m_designers[ type ];
		if ( des == 0 )
		{
			CppImplementationWidget * impl = new CppImplementationWidget( this );
			des = new QtDesignerCppIntegration( this, impl );
			des->loadSettings( *project() ->projectDom(), "kdevcppsupport/designerintegration" );
			m_designers[ type ] = des;
		}
		break;
	}
	return des;
}

void CppSupportPart::slotCreateSubclass()
{
	QFileInfo fi( m_contextFileName );
	if ( fi.extension( false ) != "ui" )
		return ;
	QtDesignerCppIntegration *des = dynamic_cast<QtDesignerCppIntegration*>( designer( KInterfaceDesigner::QtDesigner ) );
	if ( des )
		des->selectImplementation( m_contextFileName );
}

void CppSupportPart::addMethod( ClassDom aClass, const QString& name, const QString type,
                                const QString& parameters, CodeModelItem::Access accessType,
                                bool isConst, bool isInline, bool isVirtual, bool isPureVirtual,
                                const QString& implementation )
{
	partController() ->editDocument( KURL( aClass->fileName() ) );
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
	if ( !editIface )
	{
		/// @fixme show messagebox
		return ;
	}
	QString declarationString = type + " " + name + "(" + parameters + ")" + ( isConst ? " const" : "" );

	KDevSourceFormatter* sourceFormatter = extension<KDevSourceFormatter>( "KDevelop/SourceFormatter" );

	QString finalDeclaration = ( ( isVirtual || isPureVirtual ) ? "\nvirtual " : "\n" + declarationString +
	                             ( isPureVirtual ? " = 0 " : "" ) +
	                             ( isInline ? "\n{\n" + implementation + "\n}\n" : ";" ) );

	if ( sourceFormatter != 0 )
		finalDeclaration = sourceFormatter->formatSource( finalDeclaration );

	QString indentString = "\t";

	if ( sourceFormatter != 0 )
		indentString = sourceFormatter->indentString();

	editIface->insertText( findInsertionLineMethod( aClass, accessType ), 0,
	                       finalDeclaration.replace( "\n", "\n\t" ) + "\n" );

	backgroundParser() ->addFile( aClass->fileName() );
	if ( isInline || isPureVirtual )
		return ;

	QString definitionString = "\n" + declarationString + "\n{\n" + implementation + "\n}\n";

	if ( sourceFormatter != 0 )
		definitionString = sourceFormatter->formatSource( definitionString );

	QFileInfo info( aClass->fileName() );
	QString implementationFile = info.dirPath( true ) + "/" + info.baseName() + ".cpp" ;
	QFileInfo fileInfo( implementationFile );
	KDevCreateFile* createFileSupport = extension<KDevCreateFile>( "KDevelop/CreateFile" );
	if ( !QFile::exists( fileInfo.absFilePath() ) && createFileSupport != 0 )
		createFileSupport->createNewFile( fileInfo.extension(), fileInfo.dirPath( true ), fileInfo.baseName() );

	partController() ->editDocument( KURL( implementationFile ) );
	editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
	if ( !editIface )
		return ; //@fixme errorverdoedelung

	editIface->insertLine( editIface->numLines(), QString::fromLatin1( "" ) );
	editIface->insertText( editIface->numLines() - 1, 0, definitionString );
	backgroundParser() ->addFile( implementationFile );
}

ClassDom CppSupportPart::currentClass( ) const
{
	FileDom file = codeModel() ->fileByName( m_activeFileName );
	if ( file == 0 || m_activeViewCursor == 0 )
		return 0;

	unsigned int curLine, curCol;
	m_activeViewCursor->cursorPosition( &curLine, &curCol );

	return CodeModelUtils::findClassByPosition( file, curLine, curCol );
}

VariableDom CppSupportPart::currentAttribute( ClassDom curClass ) const
{
	if ( m_activeViewCursor == 0 || curClass == 0 )
		return 0;
	
	unsigned int line, col;
	m_activeViewCursor->cursorPosition( &line, &col );
	
	VariableList vars = curClass->variableList();
	
	for ( VariableList::iterator i = vars.begin(); i != vars.end(); ++i )
	{
		int startLine, startCol;
		( *i ) ->getStartPosition( &startLine, &startCol );
		if ( startLine < (int)line || ( startLine == (int)line && startCol <= (int)col ) )
		{
			int endLine, endCol;
			( *i ) ->getEndPosition( &endLine, &endCol );
			if ( endLine > (int)line || ( endLine == (int)line && endCol >= (int)col ) )
				return * i;
		}
	}
	return 0;
}

void CppSupportPart::slotCreateAccessMethods( )
{
	if ( m_curAttribute == 0 || m_curClass == 0 )
		return ;

	CreateGetterSetterDialog dlg ( this, m_curClass, m_curAttribute );
	dlg.exec();
}

int CppSupportPart::findInsertionLineMethod( ClassDom aClass, CodeModelItem::Access access )
{
	int line, column;
	aClass->getEndPosition( &line, &column );

	int point = CodeModelUtils::findLastMethodLine( aClass, access );

	if ( point == -1 )
	{
		KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
		if ( !editIface )
			return -1;

		editIface->insertLine( line - 1, CodeModelUtils::accessSpecifierToString( access ) + ":\n" );
		return line;
	}

	return point + 1;
}

int CppSupportPart::findInsertionLineVariable( ClassDom aClass, CodeModelItem::Access access )
{
	int line, column;
	aClass->getEndPosition( &line, &column );
	
	int point = CodeModelUtils::findLastVariableLine( aClass, access );
	
	if ( point == -1 )
	{
		KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
		if ( !editIface )
			return -1;
		
		editIface->insertLine( line - 1, CodeModelUtils::accessSpecifierToString( access ) + ":\n" );
		return line;
	}
	
	return point;
}

void CppSupportPart::createAccessMethods( ClassDom theClass, VariableDom theVariable )
{
	m_curClass = theClass;
	m_curAttribute = theVariable;
	
	slotCreateAccessMethods();
}

#include "cppsupportpart.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;

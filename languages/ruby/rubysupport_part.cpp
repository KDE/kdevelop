#include "rubysupport_part.h"
#include "rubyconfigwidget.h"
#include "domutil.h"

#include "qtdesignerrubyintegration.h"
#include "rubyimplementationwidget.h"

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"
#include "kdevplugininfo.h"
#include "kdevshellwidget.h"
#include "kdevquickopen.h"

#include <qdir.h>
#include <qwhatsthis.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <klibloader.h>

#include <ktexteditor/viewcursorinterface.h>

#include <codemodel_utils.h>

typedef KDevGenericFactory<RubySupportPart> RubySupportFactory;
static const KDevPluginInfo data("kdevrubysupport");
K_EXPORT_COMPONENT_FACTORY( libkdevrubysupport, RubySupportFactory( data ) )

RubySupportPart::RubySupportPart(QObject *parent, const char *name, const QStringList& )
  : KDevLanguageSupport (&data, parent, name ? name : "RubySupportPart" )
{
  setInstance(RubySupportFactory::instance());
  setXMLFile("kdevrubysupport.rc");

  KAction *action;
  action = new KAction( i18n("&Run"), "exec", SHIFT + Key_F9,
                        this, SLOT(slotRun()),
                        actionCollection(), "build_execute" );
  action->setToolTip(i18n("Run"));
  action->setWhatsThis(i18n("<b>Run</b><p>Starts an application."));
  action->setIcon("ruby_run.png");

  action = new KAction( i18n("Run Test Under Cursor"), "exec", ALT + Key_F9,
                        this, SLOT(slotRunTestUnderCursor()),
                        actionCollection(), "build_execute_test_function" );
  action->setToolTip(i18n("Run Test Under Cursor"));
  action->setWhatsThis(i18n("<b>Run Test Under Cursor</b><p>Runs the function under the cursor as test."));

  action = new KAction( i18n("Launch Browser"), "network", 0, this, SLOT(slotBrowse()), actionCollection(), "build_launch_browser" );
  action->setToolTip(i18n("Launch Browser"));
  action->setWhatsThis(i18n("<b>Launch Browser</b><p>Opens a web browser pointing to the Ruby Rails server") );

  action = new KAction( i18n("Switch To Controller"), 0, CTRL+ALT+Key_1, this, SLOT(slotSwitchToController()), actionCollection(), "switch_to_controller" );
  action = new KAction( i18n("Switch To Model"), 0, CTRL+ALT+Key_2, this, SLOT(slotSwitchToModel()), actionCollection(), "switch_to_model" );
  action = new KAction( i18n("Switch To View"), 0, CTRL+ALT+Key_3, this, SLOT(slotSwitchToView()), actionCollection(), "switch_to_view" );
  action = new KAction( i18n("Switch To Test"), 0, CTRL+ALT+Key_4, this, SLOT(slotSwitchToTest()), actionCollection(), "switch_to_test" );

  kdDebug() << "Creating RubySupportPart" << endl;

  m_shellWidget = new KDevShellWidget( 0, "irb console");
  m_shellWidget->setIcon( SmallIcon("ruby_config.png", KIcon::SizeMedium, KIcon::DefaultState, RubySupportPart::instance()));
  m_shellWidget->setCaption(i18n("Ruby Shell"));
  mainWindow()->embedOutputView( m_shellWidget, i18n("Ruby Shell"), i18n("Ruby Shell"));
  mainWindow()->raiseView( m_shellWidget );

  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
        this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
  connect( partController(), SIGNAL(savedFile(const KURL&)),
  	this, SLOT(savedFile(const KURL&)) );
  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
        this, SLOT(projectConfigWidget(KDialogBase*)) );
}


RubySupportPart::~RubySupportPart()
{
    if ( m_shellWidget )
        mainWindow()->removeView( m_shellWidget );
    delete m_shellWidget;
}


void RubySupportPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Ruby"), i18n("Ruby"), BarIcon("ruby_config.png", KIcon::SizeMedium, KIcon::DefaultState, RubySupportPart::instance()));
    RubyConfigWidget *w = new RubyConfigWidget(*projectDom(), (QWidget *)vbox, "ruby config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void RubySupportPart::projectOpened()
{
  kdDebug() << "projectOpened()" << endl;

  QStrList l;
  l.append( shell().latin1() ) ;
  m_shellWidget->setShell( shell().latin1(), l );
  m_shellWidget->activate();
  m_shellWidget->setAutoReactivateOnClose( true );

  connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
  	this, SLOT(addedFilesToProject(const QStringList &)) );
  connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
  	this, SLOT(removedFilesFromProject(const QStringList &)) );

  QFileInfo program(mainProgram());

  // If it's a Rails project, create the project files if they're missing
  if (mainProgram().endsWith("script/server")) {
    QString cmd;
    QFileInfo server(project()->projectDirectory() + "/script/server");
    if (! server.exists()) {
      cmd += "rails " + project()->projectDirectory();
      if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
        appFrontend->startAppCommand(project()->projectDirectory(), cmd, false);
    }
  }

  // We want to parse only after all components have been
  // properly initialized
  QTimer::singleShot(0, this, SLOT(initialParse()));
}

void RubySupportPart::maybeParse(const QString fileName)
{
  QFileInfo fi(fileName);

  if (fi.extension() == "rb") {
    if (codeModel()->hasFile(fileName)) {
      emit aboutToRemoveSourceInfo(fileName);
      codeModel()->removeFile(codeModel()->fileByName(fileName));
    }

    parse(fileName);
    emit addedSourceInfo( fileName );
  }
}

void RubySupportPart::initialParse()
{
  kdDebug() << "initialParse()" << endl;

  if (project()) {
    kapp->setOverrideCursor(waitCursor);
    QStringList files = project()->allFiles();
    for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
      kdDebug() << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
      maybeParse(project()->projectDirectory() + "/" + *it);
    }

    emit updatedSourceInfo();
    kapp->restoreOverrideCursor();
  } else {
    kdDebug() << "No project" << endl;
  }
}

void RubySupportPart::addedFilesToProject(const QStringList &fileList)
{
  kdDebug() << "addedFilesToProject()" << endl;

  QStringList::ConstIterator it;

  for ( it = fileList.begin(); it != fileList.end(); ++it )
  {
    QString fileName = project()->projectDirectory() + "/" + ( *it );
    maybeParse( fileName );
  }
}

void RubySupportPart::removedFilesFromProject(const QStringList &fileList)
{
  kdDebug() << "removedFilesFromProject()" << endl;

  QStringList::ConstIterator it;

  for ( it = fileList.begin(); it != fileList.end(); ++it )
  {
    QString fileName = project()->projectDirectory() + "/" + ( *it );

    if( codeModel()->hasFile(fileName) ){
      emit aboutToRemoveSourceInfo( fileName );
      codeModel()->removeFile( codeModel()->fileByName(fileName) );
    }
  }
}

void RubySupportPart::savedFile(const KURL &fileName)
{
  kdDebug() << "savedFile()" << endl;

  if (project()->allFiles().contains(fileName.path().mid ( project()->projectDirectory().length() + 1 ))) {
    maybeParse(fileName.path());
    emit addedSourceInfo( fileName.path() );
  }
}

KDevLanguageSupport::Features RubySupportPart::features()
{
  return Features(Classes | Functions | Variables | Declarations | Signals | Slots);
}

void RubySupportPart::parse(const QString &fileName)
{
  QFile f(QFile::encodeName(fileName));
  if (!f.open(IO_ReadOnly))
    return;
  QTextStream stream(&f);

	QRegExp classre("^\\s*(class|module)\\s+([A-Z][A-Za-z0-9_]+::)*([A-Z][A-Za-z0-9_]+)\\s*(<\\s*([A-Z][A-Za-z0-9_:]+))?$");
  QRegExp methodre("^(\\s*)def\\s+(([A-Z][A-Za-z0-9_:]+|self)\\.)?([A-Za-z0-9_]+[!?=]?|\\[\\]=?|\\*\\*||\\-|[!~+*/%&|><^]|>>|<<||<=>|<=|>=|==|===|!=|=~|!~).*$");
  QRegExp accessre("^\\s*(private|protected|public)\\s*((:([A-Za-z0-9_]+[!?=]?|\\[\\]=?|\\*\\*||\\-|[!~+*/%&|><^]|>>|<<||<=>|<=|>=|==|===|!=|=~|!~),?\\s*)*)$");
  QRegExp attr_accessorre("^\\s*(attr_accessor|attr_reader|attr_writer)\\s*((:([A-Za-z0-9_]+),?\\s*)*)$");
  QRegExp symbolre(":([^,]+),?");
  QRegExp line_contre(",\\s*$");
  QRegExp slot_signalre("^\\s*(slots|signals|k_dcop|k_dcop_signals)\\s*(('[^)]+\\)',?\\s*)*)$");
  QRegExp memberre("'([A-Za-z0-9_ &*]+\\s)?([A-Za-z0-9_]+)\\([^)]*\\)',?");
  QRegExp begin_commentre("^*=begin");
  QRegExp end_commentre("^*=end");
  QRegExp variablere("(@@?[A-Za-z0-9_]+)\\s*=\\s*((?:([A-Za-z0-9_:.]+)\\.new)|[\\[\"'%:/\\?\\{]|%r|<<|true|false|^\\?|0[0-7]+|[-+]?0b[01]+|[-+]?0x[1-9a-fA-F]+|[-+]?[0-9_\\.e]+|nil)?");
  QRegExp endre("^(\\s*)end\\s*$");

  FileDom m_file = codeModel()->create<FileModel>();
  m_file->setName(fileName);

  ClassDom lastClass;
  FunctionDom lastMethod;
  QString lastMethodIndentation;
  int lastAccess = CodeModelItem::Public;
  QString rawline;
  QCString line;
  int lineNo = 0;

  while (!stream.atEnd()) {
    rawline = stream.readLine();
    line = rawline.stripWhiteSpace().local8Bit();
    if (classre.search(line) != -1) {
      if (m_file->hasClass(classre.cap(3))) {
        lastClass = m_file->classByName( classre.cap(3) )[ 0 ];
	  } else {
        lastClass = codeModel()->create<ClassModel>();
        lastClass->setName(classre.cap(3));
        lastClass->setFileName( fileName );
        lastClass->setStartPosition( lineNo, 0 );
        m_file->addClass( lastClass );
	  }

      QString parent = classre.cap(5);
      if (!parent.isEmpty())
      {
        kdDebug() << "Add parent " << parent << endl;
        lastClass->addBaseClass( parent );
      }

	  lastAccess = CodeModelItem::Public;
    } else if (methodre.search(line) != -1) {
      FunctionDom methodDecl;
      if ( lastClass != 0 && lastClass->hasFunction( methodre.cap(4) ) ) {
        FunctionList methods = lastClass->functionByName( methodre.cap(4) );
	    methodDecl = methods[0];
	  } else {
        methodDecl = codeModel()->create<FunctionModel>();
        methodDecl->setFileName( fileName );
        methodDecl->setStartPosition( lineNo, 0 );
        methodDecl->setName(methodre.cap(4));
	  }
      FunctionDefinitionDom method = codeModel()->create<FunctionDefinitionModel>();
      method->setName(methodre.cap(4));
      kdDebug() << "Add method: " << method->name() << endl;
      method->setFileName( fileName );
      method->setStartPosition( lineNo, 0 );
	  if (methodDecl->name() == "initialize") {
	    // Ruby constructors are alway private
	    methodDecl->setAccess( CodeModelItem::Private );
	  } else {
	    methodDecl->setAccess( lastAccess );
	  }
	  if (methodre.cap(2) != "") {
	    // A ruby class/singleton method of the form <classname>.<methodname>
	  	methodDecl->setStatic( true );
	  }

    lastMethodIndentation = methodre.cap(1);

	  lastMethod = method;

      if (lastClass != 0) {
		QStringList scope( lastClass->name() );
		method->setScope( scope );
		methodDecl->setScope( scope );
        if( !lastClass->hasFunction(methodDecl->name()) ) {
          lastClass->addFunction( methodDecl );
		}
        if( !lastClass->hasFunctionDefinition(method->name()) ) {
          lastClass->addFunctionDefinition( method );
		}
      } else if( !m_file->hasFunctionDefinition(method->name()) ){
        m_file->addFunction( methodDecl );
        m_file->addFunctionDefinition( method );
        lastClass = 0;
      }
    } else if (endre.search(line) != -1 && lastMethod != 0 && endre.cap(1) == lastMethodIndentation ) {
        int endCol, endLine;
        lastMethod->getEndPosition(&endCol, &endLine);
        if (endLine == 0) {
            //hack to set end position of the previous method to the line
            //where its corresponding "end" is found
            //there's an assumption that method's "def" statement will have the same
            //indentation level as method's "end"
            lastMethod->setEndPosition(lineNo, 0);
        }
    }
    else if (accessre.search(line) != -1 && lastClass != 0) {
	  int currentAccess = lastAccess;
	  if (accessre.cap(1) == "public") {
	    currentAccess = CodeModelItem::Public;
	  } else if (accessre.cap(1) == "protected") {
	    currentAccess = CodeModelItem::Protected;
	  } else if (accessre.cap(1) == "private") {
	    currentAccess = CodeModelItem::Private;
	  }

	  if (accessre.cap(2) == "") {
	  	lastAccess = currentAccess;
	  } else {
		QString symbolList( accessre.cap(2) );
        int pos = 0;

        while ( pos >= 0 ) {
          pos = symbolre.search( symbolList, pos );
		  if (pos == -1) {
			if (line_contre.search(line) != -1) {
              rawline = stream.readLine();
			  if (!stream.atEnd()) {
                line = rawline.stripWhiteSpace().local8Bit();
                ++lineNo;
			    symbolList = line;
			    pos = 0;
			  }
			}
		  } else {
            if ( lastClass->hasFunction( symbolre.cap(1) ) ) {
              FunctionList methods = lastClass->functionByName( symbolre.cap(1) );
			  methods[0]->setAccess( currentAccess );
			}
            pos += symbolre.matchedLength();
          }
        }
	  }
    } else if (slot_signalre.search(line) != -1 && lastClass != 0) {
      QString memberList( slot_signalre.cap(2) );
      int pos = 0;

      while ( pos >= 0 ) {
        pos = memberre.search( memberList, pos );
		if (pos == -1) {
	      if (line_contre.search(line) != -1) {
            rawline = stream.readLine();
			if (!stream.atEnd()) {
              line = rawline.stripWhiteSpace().local8Bit();
              ++lineNo;
			  memberList = line;
			  pos = 0;
			}
		  }
		} else {
          FunctionDom method;
          if ( lastClass->hasFunction( memberre.cap(2) ) ) {
            FunctionList methods = lastClass->functionByName( memberre.cap(2) );
	        method = methods[0];
	      } else {
            method = codeModel()->create<FunctionModel>();
		  }
		  QStringList scope( lastClass->name() );
		  method->setScope( scope );
          method->setName(memberre.cap(2));
          method->setFileName( fileName );
          method->setStartPosition( lineNo, 0 );

		  if (slot_signalre.cap(1) == "slots" || slot_signalre.cap(1) == "k_dcop") {
		    method->setSlot( true );
		  } else {
		    method->setSignal( true );
		  }
          if ( !lastClass->hasFunction(method->name()) ) {
            lastClass->addFunction( method );
		  }
          pos += memberre.matchedLength();
        }
	  }
	} else if (attr_accessorre.search(line) != -1 && lastClass != 0) {
	  QString attr( attr_accessorre.cap(1) );
	  QString symbolList( attr_accessorre.cap(2) );
      int pos = 0;

      while ( pos >= 0 ) {
        pos = symbolre.search( symbolList, pos );
		if (pos == -1) {
		  if (line_contre.search(line) != -1) {
            rawline = stream.readLine();
			if (!stream.atEnd()) {
              line = rawline.stripWhiteSpace().local8Bit();
              ++lineNo;
			  symbolList = line;
			  pos = 0;
			}
		  }
		} else {
 		    QStringList scope( lastClass->name() );
			if (	!lastClass->hasFunction(symbolre.cap(1))
					&& (attr == "attr_accessor" || attr == "attr_reader") )
			{
              FunctionDefinitionDom method = codeModel()->create<FunctionDefinitionModel>();
              method->setName(symbolre.cap(1));
              kdDebug() << "Add method: " << method->name() << endl;
              method->setFileName( fileName );
              method->setStartPosition( lineNo, 0 );
			  method->setScope(scope);
              lastClass->addFunction( model_cast<FunctionDom>(method) );
              lastClass->addFunctionDefinition( method );
			}

            if (	!lastClass->hasFunction(symbolre.cap(1) + "=")
					&& (attr == "attr_accessor" || attr == "attr_writer") )
			{
              FunctionDefinitionDom method = codeModel()->create<FunctionDefinitionModel>();
              method->setName(symbolre.cap(1) + "=");
              kdDebug() << "Add method: " << method->name() << endl;
              method->setFileName( fileName );
              method->setStartPosition( lineNo, 0 );
			  method->setScope(scope);
              lastClass->addFunction( model_cast<FunctionDom>(method) );
              lastClass->addFunctionDefinition( method );
			}

            pos  += symbolre.matchedLength();
        }
	  }
   } else if (variablere.search(line) != -1 && lastClass != 0) {
     VariableDom attr;
     if ( lastClass->hasVariable( variablere.cap(1) ) ) {
	   attr = lastClass->variableByName( variablere.cap(1) );
	 } else {
       attr = codeModel()->create<VariableModel>();
       attr->setName( variablere.cap(1) );
       attr->setFileName( fileName );
       attr->setStartPosition( lineNo, 0 );
 	   attr->setAccess( CodeModelItem::Private );
	   if (QRegExp("^@@").search(attr->name()) != -1) {
	     attr->setStatic( true );
	   }
       lastClass->addVariable( attr );
	 }

	 // Give priority to any variable initialized in the constructor
	 // Otherwise, take the first one found in the source file
	 if (lastMethod != 0 && lastMethod->name() == "initialize") {
       attr->setFileName( fileName );
       attr->setStartPosition( lineNo, 0 );
	 }

	 if (QRegExp("^(/|%r)").search(variablere.cap(2)) != -1) {
       attr->setType( "Regexp" );
	 } else if (QRegExp("^[\"'%<]").search(variablere.cap(2)) != -1) {
       attr->setType( "String" );
	 } else if (QRegExp("^\\[").search(variablere.cap(2)) != -1) {
       attr->setType( "Array" );
	 } else if (QRegExp("^\\{").search(variablere.cap(2)) != -1) {
       attr->setType( "Hash" );
	 } else if (QRegExp("^:").search(variablere.cap(2)) != -1) {
       attr->setType( "Symbol" );
	 } else if (QRegExp("\\.\\.").search(variablere.cap(2)) != -1) {
       attr->setType( "Range" );
	 } else if (variablere.cap(2) == "true" || variablere.cap(2) == "false") {
       attr->setType( "Boolean" );
	 } else if (  QRegExp("^[-+]?[0-9_]+").exactMatch(variablere.cap(2))
	              || QRegExp("^[-+]?(0x|0|0b|\\?)").search(variablere.cap(2)) != -1 )
	 {
       attr->setType( "Integer" );
	 } else if (QRegExp("[0-9._]+(e[-+0-9]+)?").exactMatch(variablere.cap(2))) {
       attr->setType( "Float" );
	 } else if (variablere.cap(2) != "nil" && variablere.cap(3) != "") {
       attr->setType( variablere.cap(3) );
	 }
   } else if (begin_commentre.search(line) != -1) {
     while (!stream.atEnd() && end_commentre.search(line) == -1) {
       rawline = stream.readLine();
       line = rawline.stripWhiteSpace().local8Bit();
       ++lineNo;
	 }
   }

    ++lineNo;
  }

  f.close();

  codeModel()->addFile( m_file );
}


void RubySupportPart::slotRun ()
{
    // if we can't save all parts, then the user canceled
    if ( partController()->saveAllFiles() == false )
        return;
    QFileInfo program(mainProgram());
    if (mainProgram().endsWith("script/server")) {
         QString cmd;
         QFileInfo server(project()->projectDirectory() + "/script/server");

        // Starting WEBrick for a Rails app. Translate a SIGTERM signal sent by KDevelop
        // to a SIGINT expected by WEBrick (ie control&c) to terminate it.
        cmd += "script/server& \n trap \"kill -s SIGINT $!\" TERM \n wait \n exit 0";
        if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
            appFrontend->startAppCommand(project()->projectDirectory(), cmd, false);
    } else {
        QString cmd = QString("%1 -K%2 -C\"%3\" -I\"%4\" \"%5\" %6")
                          .arg(interpreter())
                          .arg(characterCoding())
                          .arg(runDirectory())
                          .arg(program.dirPath())
                          .arg(program.fileName())
                          .arg(programArgs());
        startApplication(cmd);
    }
}

QString RubySupportPart::interpreter() {
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/interpreter");
    if (prog.isEmpty()) prog = "ruby";
    return prog;
}

QString RubySupportPart::shell() {
    QString shell = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/shell");
    if (shell.isEmpty()) shell = "irb";
    return shell;
}

QString RubySupportPart::mainProgram() {
	QString prog;
	int runMainProgram = DomUtil::readIntEntry(*projectDom(), "/kdevrubysupport/run/runmainprogram");

	if (runMainProgram == 0) {
    	prog = project()->projectDirectory() + "/" + DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/mainprogram");
	} else {
		KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
		if (ro_part != 0) {
			prog = ro_part->url().path();
		}
	}

    return prog;
}

QString RubySupportPart::runDirectory() {
    QString cwd = DomUtil::readEntry(*projectDom(), "/kdevscriptproject/run/globalcwd");
    if (cwd.isEmpty())
    {
      QString mainProg = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/mainprogram");
      KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
      if (mainProg.isEmpty() && ro_part)
        cwd = ro_part->url().directory();
      else
        cwd = project()->buildDirectory();
    }
    return cwd;
}

QString RubySupportPart::programArgs() {
    QString args = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/programargs");
    return args;
}

QString RubySupportPart::characterCoding() {
    int coding = DomUtil::readIntEntry(*projectDom(), "/kdevrubysupport/run/charactercoding");
	QString code("A");

	switch (coding) {
	case 0:
		code = "A";
		break;
	case 1:
		code = "E";
		break;
	case 2:
		code = "S";
		break;
	case 3:
		code = "U";
		break;
	}
    return code;
}


void RubySupportPart::startApplication(const QString &program) {
	bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevrubysupport/run/terminal");
    if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
        appFrontend->startAppCommand(QString::QString(), program, inTerminal);
}


KMimeType::List RubySupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "text/x-ruby" );
    if( mime )
	list << mime;
    return list;
}

KDevDesignerIntegration *RubySupportPart::designer(KInterfaceDesigner::DesignerType type)
{
    KDevDesignerIntegration *des = 0;
    switch (type)
    {
        case KInterfaceDesigner::QtDesigner:
            des = m_designers[type];
            if (des == 0)
            {
                RubyImplementationWidget *impl = new RubyImplementationWidget(this);
                des = new QtDesignerRubyIntegration(this, impl);
                des->loadSettings(*project()->projectDom(),
                    "kdevrubysupport/designerintegration");
                m_designers[type] = des;
            }
            break;
        case KInterfaceDesigner::Glade:
		    break;
    }
    return des;
}

void RubySupportPart::projectClosed( )
{
    for (QMap<KInterfaceDesigner::DesignerType, KDevDesignerIntegration*>::const_iterator it =  m_designers.begin();
        it != m_designers.end(); ++it)
    {
        kdDebug() << "calling save settings fro designer integration" << endl;
        it.data()->saveSettings(*project()->projectDom(), "kdevrubysupport/designerintegration");
    }
}

void RubySupportPart::contextMenu( QPopupMenu * popup, const Context * context )
{
    if (context->hasType(Context::FileContext)){
        const FileContext *fc = static_cast<const FileContext*>(context);
        //this is a .ui file and only selection contains only one such file
        KURL url = fc->urls().first();
        if (url.fileName().endsWith(".ui"))
        {
            m_contextFileName = url.fileName();
            int id = popup->insertItem(i18n("Create or Select Implementation..."), this, SLOT(slotCreateSubclass()));
            popup->setWhatsThis(id, i18n("<b>Create or select implementation</b><p>Creates or selects a subclass of selected form for use with integrated KDevDesigner."));
        }
    }
}

void RubySupportPart::slotCreateSubclass()
{
    QFileInfo fi(m_contextFileName);
    if (fi.extension(false) != "ui")
        return;
    QtDesignerRubyIntegration *des = dynamic_cast<QtDesignerRubyIntegration*>(designer(KInterfaceDesigner::QtDesigner));
    if (des)
        des->selectImplementation(m_contextFileName);
}

void RubySupportPart::slotBrowse()
{
    kapp->invokeBrowser("http://localhost:3000/");
}

void RubySupportPart::slotSwitchToController()
{
    KParts::Part *activePart = partController()->activePart();
    if (!activePart)
        return;
    KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(activePart);
    if (!ropart)
        return;
    QFileInfo file(ropart->url().path());
    if (!file.exists())
        return;
    QString ext = file.extension();
    QString name = file.baseName();
    QString switchTo = "";
    if ((ext == "rb") && !name.endsWith("_controller"))
    {
        if (name.endsWith("_test"))
        {
            switchTo = name.remove(QRegExp("_test$"));  //the file is the test
            switchTo = name.remove(QRegExp("_controller$"));  //remove functional test name parts
        }
        else
            switchTo = name;
    }
    else if (ext == "rjs" || ext == "rxml" || ext == "rhtml" || ext == "js.rjs" || ext == "xml.builder" || ext == "html.erb")
    {
        //this is a view, we need to find the directory of this view and try to find
        //the controller basing on the directory information
        switchTo = file.dir().dirName();
    }
    QString controllersDir = project()->projectDirectory() + "/app/controllers/";
    if (!switchTo.isEmpty())
    {
        if (switchTo.endsWith("s"))
            switchTo = switchTo.mid(0, switchTo.length()-1);
        QString singular = controllersDir + switchTo + "_controller.rb";
        QString plural = controllersDir + switchTo + "s_controller.rb";
        KURL url = KURL::fromPathOrURL(QFile::exists(singular) ? singular : plural);
        partController()->editDocument(url);
    }
}

void RubySupportPart::slotSwitchToTest()
{
    KParts::Part *activePart = partController()->activePart();
    if (!activePart)
        return;
    KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(activePart);
    if (!ropart)
        return;
    QFileInfo file(ropart->url().path());
    if (!file.exists())
        return;
    QString ext = file.extension();
    QString name = file.baseName();
    QString switchTo = "";

    if (ext == "rjs" || ext == "rxml" || ext == "rhtml" || ext == "js.rjs" || ext == "xml.builder" || ext == "html.erb")
    {
        //this is a view already, let's show the list of all views for this model
        switchTo = file.dir().dirName();
    }
    else if (ext == "rb")
        switchTo = name.remove(QRegExp("_controller$")).remove(QRegExp("_controller_test$")).remove(QRegExp("_test$"));

    if (switchTo.isEmpty())
        return;

    if (switchTo.endsWith("s"))
        switchTo = switchTo.mid(0, switchTo.length() - 1);

    KURL::List urls;
    QString testDir = project()->projectDirectory() + "/test/";
    QString functionalTestS = testDir + "functional/" + switchTo + "_controller_test.rb";
    QString functionalTestP = testDir + "functional/" + switchTo + "s_controller_test.rb";
    QString integrationTestS = testDir + "integration/" + switchTo + "_test.rb";
    QString integrationTestP = testDir + "integration/" + switchTo + "s_test.rb";
    QString unitTestS = testDir + "unit/" + switchTo + "_test.rb";
    QString unitTestP = testDir + "unit/" + switchTo + "s_test.rb";
    if (QFile::exists(functionalTestP)) urls << KURL::fromPathOrURL(functionalTestP);
    if (QFile::exists(integrationTestP)) urls << KURL::fromPathOrURL(integrationTestP);
    if (QFile::exists(unitTestP)) urls << KURL::fromPathOrURL(unitTestP);
    if (QFile::exists(functionalTestS)) urls << KURL::fromPathOrURL(functionalTestS);
    if (QFile::exists(integrationTestS)) urls << KURL::fromPathOrURL(integrationTestS);
    if (QFile::exists(unitTestS)) urls << KURL::fromPathOrURL(unitTestS);

    KDevQuickOpen *qo = extension<KDevQuickOpen>("KDevelop/QuickOpen");
    if (qo && !urls.isEmpty())
        qo->quickOpenFile(urls);
}

void RubySupportPart::slotSwitchToModel()
{
    KParts::Part *activePart = partController()->activePart();
    if (!activePart)
        return;
    KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(activePart);
    if (!ropart)
        return;
    QFileInfo file(ropart->url().path());
    if (!file.exists())
        return;
    QString ext = file.extension();
    QString name = file.baseName();
    QString switchTo = "";

    if (ext == "rjs" || ext == "rxml" || ext == "rhtml" || ext == "js.rjs" || ext == "xml.builder" || ext == "html.erb")
    {
        //this is a view already, let's show the list of all views for this model
        switchTo = file.dir().dirName();
    }
    else if (ext == "rb" && (name.endsWith("_controller") || name.endsWith("_test")))
    {
        switchTo = name.remove(QRegExp("_controller$")).remove(QRegExp("_controller_test$")).remove(QRegExp("_test$"));
    }

    if (switchTo.isEmpty())
        return;

    if (switchTo.endsWith("s"))
        switchTo = switchTo.mid(0, switchTo.length() - 1);

    QString modelsDir = project()->projectDirectory() + "/app/models/";
    QString singular = modelsDir + switchTo + "_controller.rb";
    QString plural = modelsDir + switchTo + "s_controller.rb";
    KURL url = KURL::fromPathOrURL(QFile::exists(singular) ? singular : plural);

    partController()->editDocument(KURL::fromPathOrURL(modelsDir + switchTo + ".rb"));
}

void RubySupportPart::slotSwitchToView()
{
    KParts::Part *activePart = partController()->activePart();
    if (!activePart)
        return;
    KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(activePart);
    if (!ropart)
        return;
    QFileInfo file(ropart->url().path());
    if (!file.exists())
        return;
    QString ext = file.extension();
    QString name = file.baseName();
    QString switchTo = "";

    if (ext == "rjs" || ext == "rxml" || ext == "rhtml" || ext == "js.rjs" || ext == "xml.builder" || ext == "html.erb")
    {
        //this is a view already, let's show the list of all views for this model
        switchTo = file.dir().dirName();
    }
    else if (ext == "rb")
        switchTo = name.remove(QRegExp("_controller$")).remove(QRegExp("_controller_test$")).remove(QRegExp("_test$"));

    if (switchTo.isEmpty())
        return;

    if (switchTo.endsWith("s"))
        switchTo = switchTo.mid(0, switchTo.length() - 1);

    KURL::List urls;
    QDir viewsDir;
    QDir viewsDirS = QDir(project()->projectDirectory() + "/app/views/" + switchTo);
    QDir viewsDirP = QDir(project()->projectDirectory() + "/app/views/" + switchTo + "s");
    if (viewsDirS.exists())
        viewsDir = viewsDirS;
    else if (viewsDirP.exists())
        viewsDir = viewsDirP;
    else
        return;

    QStringList views = viewsDir.entryList();

    for (QStringList::const_iterator it = views.begin(); it != views.end(); ++it)
    {
        QString viewName = *it;
        if ( !(viewName.endsWith("~") || viewName == "." || viewName == "..") )
            urls << KURL::fromPathOrURL(viewsDir.absPath() + "/" + viewName);
    }
    KDevQuickOpen *qo = extension<KDevQuickOpen>("KDevelop/QuickOpen");
    if (qo)
        qo->quickOpenFile(urls);
}

void RubySupportPart::slotRunTestUnderCursor()
{
    // if we can't save all parts, then the user canceled
    if ( partController()->saveAllFiles() == false )
        return;

    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
    QString prog;
    if (ro_part != 0) {
        prog = ro_part->url().path();
    } else
        return;

    KTextEditor::ViewCursorInterface* activeViewCursor = dynamic_cast<KTextEditor::ViewCursorInterface*>( ro_part->widget() );
    if (!activeViewCursor) return;

    unsigned int line, column;
    activeViewCursor->cursorPositionReal(&line, &column);
    CodeModelUtils::CodeModelHelper hlp(codeModel(), codeModel()->fileByName(prog));
    FunctionDom fun = hlp.functionAt(line, column);
    if (fun == 0) return;

    QFileInfo program(prog);
    QString cmd = QString("%1 -K%2 -C\"%3\" -I\"%4\" \"%5\" %6")
                          .arg(interpreter())
                          .arg(characterCoding())
                          .arg(runDirectory())
                          .arg(program.dirPath())
                          .arg(program.fileName())
                          .arg(" -n " + fun->name());
    startApplication(cmd);
}

#include "rubysupport_part.moc"

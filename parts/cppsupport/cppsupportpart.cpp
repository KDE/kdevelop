/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
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
#include "ast.h"
#include "ast_utils.h"

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
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdeversion.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>

#if defined(KDE_MAKE_VERSION) && KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
#  include <ktexteditor/texthintinterface.h>
#else
#  include <kde30x_texthintinterface.h>
#endif

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "classstore.h"
#include "kdevpartcontroller.h"

#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classparser.h"
#include "addclassattributedlg.h"
#include "cppnewclassdlg.h"
#include "cppaddmethoddlg.h"
#include "cppcodecompletion.h"
#include "cppsupportwidget.h"
#include "ccconfigwidget.h"
#include "config.h"
#include "domutil.h"

#if defined(__GLIBC__)
#include <malloc.h>
void showMemUsage()
{
    struct mallinfo mi = mallinfo();
    kdDebug() << "Mem usage: " << mi.uordblks << endl;
}
#else
void showMemUsage()
{}
#endif

typedef KGenericFactory<CppSupportPart> CppSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcppsupport, CppSupportFactory( "kdevcppsupport" ) );

CppSupportPart::CppSupportPart(QObject *parent, const char *name, const QStringList &args)
    : KDevLanguageSupport(parent, name ? name : "CppSupportPart"), m_activeSelection( 0 ), m_activeEditor( 0 )
{
    setInstance(CppSupportFactory::instance());

    setXMLFile("kdevcppsupport.rc");
 
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)));

    m_problemReporter = new ProblemReporter( this );
    mainWindow( )->embedOutputView( m_problemReporter, i18n("Problems"), i18n("problem reporter"));
    
    m_astView = new KListView();
    m_astView->setSorting( -1 );
    m_astView->addColumn( "" );
    m_astView->header()->hide();
    mainWindow()->embedSelectViewRight( m_astView, i18n("AST Debug"), i18n("Show the AST for the current translation unit") );
    connect( m_astView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotNodeSelected(QListViewItem*)) );

    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             m_problemReporter, SLOT(configWidget(KDialogBase*)) );

    m_backgroundParser = new BackgroundParser( this );
    m_backgroundParser->start();

    KAction *action;

    action = new KAction(i18n("Switch Header/Implementation"), Key_F12,
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

    action = new KAction(i18n("Type of Expression"), CTRL+Key_T,
                         this, SLOT(slotTypeOfExpression()),
                         actionCollection(), "edit_type_of_expression");
    action->setStatusText( i18n("Type of current expression") );
    action->setWhatsThis( i18n("Type of current expression") );
    action->setEnabled(false);

    action = new KAction(i18n("New Class..."), "classnew", 0,
                         this, SLOT(slotNewClass()),
                         actionCollection(), "project_newclass");
    action->setStatusText( i18n("Generate a new class") );
    action->setWhatsThis( i18n("Generate a new class") );

    m_pParser      = 0;
	m_pCCParser    = 0;
    m_pCompletion  = 0;

    withcpp = false;
    if ( args.count() == 1 && args[ 0 ] == "Cpp" )
        withcpp = true;

    // daniel
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    m_bEnableCC = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/codecompletion/enablecc" );

    m_pCHWidget = 0;
    QDomElement element = projectDom( )->documentElement( )
                          .namedItem( "cppsupportpart" ).toElement( )
                          .namedItem( "codecompletion" ).toElement( )
                          .namedItem( "codehinting" ).toElement( );

    if( !element.isNull( ) ){
        if( element.attribute( "enablech" ).toInt( ) ){
            if( element.attribute( "selectview" ).toInt( ) )
                slotEnableCodeHinting( true, false );
            else
                slotEnableCodeHinting( true, true );
        }
    }
}


CppSupportPart::~CppSupportPart()
{
    m_backgroundParser->close();
    m_backgroundParser->reparse();
    m_backgroundParser->wait();
    
    mainWindow( )->removeView( m_pCHWidget );
    mainWindow( )->removeView( m_problemReporter );
    mainWindow()->removeView( m_astView );
    
    delete m_backgroundParser;
    delete m_pParser;
    delete m_pCompletion;
    delete m_astView;

    delete m_pCCParser;
    delete m_pCHWidget;
    delete m_problemReporter;
}

void CppSupportPart::customEvent( QCustomEvent* ev )
{
    if( ev->type() == Event_FoundProblems && m_problemReporter ){
	m_backgroundParser->lock();
	FoundProblemsEvent* event = (FoundProblemsEvent*) ev;
	QString fileName = event->fileName();
	
	m_problemReporter->removeAllErrors( fileName );
	
	QValueList<Problem> problems = event->problems();
	QValueList<Problem>::ConstIterator it = problems.begin();
	while( it != problems.end() ){
	    const Problem& p = *it++;
	    m_problemReporter->reportError( p.text(), fileName, p.line(), p.column() );
	}
	
	m_astView->clear();
	
	if( fileName == m_activeFileName ){
	    AST* ast = m_backgroundParser->translationUnit( fileName );
	    if( ast ){
		m_astView->clear();
		QListViewItem* root = new KListViewItem( m_astView, QFileInfo(fileName).fileName() );
		root->setExpandable( true );
		root->setOpen( true );
		buildView( ast, m_activeEditor, root );
	    }
	}
	
	m_backgroundParser->unlock();
    } else if( ev->type() == Event_FileParsed ){
	FileParsedEvent* event = (FileParsedEvent*) ev;
	QString fileName = event->fileName();
	emit fileParsed( fileName );
	// mainWindow()->statusBar()->message( i18n("%1 Parsed").arg(event->fileName()), 1000 );
    }
}

// daniel
void CppSupportPart::projectConfigWidget( KDialogBase* dlg )
{
    QVBox* vbox = dlg->addVBoxPage( i18n( "C++ specific" ) );
    CCConfigWidget* w = new CCConfigWidget( this, vbox );
    connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );

    connect( w, SIGNAL( enablePersistantClassStore( bool ) ),
             this, SLOT( slotEnablePersistantClassStore( bool ) ) );

    connect( w, SIGNAL( enablePreParsing( bool ) ),
             this, SLOT( slotEnablePreParsing( bool ) ) );

    connect( w, SIGNAL( changedPreParsingPath( ) ),
             this, SLOT( slotChangedPreParsingPath( ) ) );

    connect( w, SIGNAL( enableCodeHinting( bool, bool ) ),
             this, SLOT( slotEnableCodeHinting( bool, bool ) ) );

    connect( w, SIGNAL( enableCodeCompletion( bool ) ),
             this, SLOT( slotEnableCodeCompletion( bool ) ) );

}


void
CppSupportPart::slotEnablePersistantClassStore( bool setEnable )
{
    kdDebug( 9007 ) << "slotEnablePersistantClassStore setEnable = " << setEnable << endl;

    QString pcsFile = project( )->projectDirectory( ) + "/" + project( )->projectName( );

    if( setEnable ){
	classStore( )->wipeout( );
	createProjectPCS( pcsFile + pcsFileExt( ) );
    }
    else {
	QFile filePCS( pcsFile + pcsFileExt( ) );
	if( filePCS.exists( ) ){
	    if( filePCS.remove( ) )
		kdDebug( 9007 ) << "success removing file " << pcsFile << pcsFileExt( ) << endl;
	    else
		kdDebug( 9007 ) << "failed removing file " << pcsFile << pcsFileExt( ) << endl;
	}

	QFile filePP( pcsFile + ppFileExt( ) );
	if( filePP.exists( ) ){
	    if( filePP.remove( ) )
		kdDebug( 9007 ) << "success removing file " << pcsFile << ppFileExt( ) << endl;
	    else
		kdDebug( 9007 ) << "failed removing file " << pcsFile << ppFileExt( ) << endl;
	}
    }
}


void
CppSupportPart::slotEnablePreParsing( bool setEnable )
{
    kdDebug( 9007 ) << "slotEnablePreParsing setEnable = " << setEnable << endl;

    QString pcsFile = project( )->projectDirectory( ) + "/" + project( )->projectName( ) + ppFileExt( );

    if( setEnable == true ){
        ccClassStore( )->wipeout( );
	createPreParsePCS( pcsFile );
    }
    else {
	QFile file( pcsFile );
	if( file.exists( ) ){
	    if( file.remove( ) )
		kdDebug( 9007 ) << "success removing file " << pcsFile << endl;
	    else
		kdDebug( 9007 ) << "failed removing file " << pcsFile << endl;
	}
    }
}


void
CppSupportPart::slotChangedPreParsingPath( )
{
    kdDebug( 9007 ) << "slotChangedPreParsingPath" << endl;

    // we just use this slot to avoid doubling of code
    slotEnablePreParsing( true );
}


void
CppSupportPart::slotEnableCodeCompletion( bool setEnable )
{
    kdDebug( 9007 ) << "slotEnableCodeCompletion" << endl;

    if( m_pCompletion )
        m_pCompletion->setEnableCodeCompletion( setEnable );
}


void
CppSupportPart::slotEnableCodeHinting( bool setEnable, bool setOutputView )
{
    kdDebug( 9007 ) << "slotEnableCodeHinting" << endl;

    if( setEnable == false ){
        if( m_pCHWidget ){
            mainWindow( )->removeView( m_pCHWidget );
            delete m_pCHWidget;
            m_pCHWidget = 0;
        }
        return;
    }
    // enable it
    else {
        if( m_pCHWidget )
            mainWindow( )->removeView( m_pCHWidget );
        else
            m_pCHWidget = new CppSupportWidget( this );

        if( setOutputView )
            mainWindow( )->embedOutputView( m_pCHWidget, i18n( "Code Hinting" ), i18n( "code hinting") );
        else
            mainWindow( )->embedSelectView( m_pCHWidget, i18n( "Code Hinting" ), i18n( "code hinting") );
    }
}


void CppSupportPart::activePartChanged(KParts::Part *part)
{
    kdDebug(9032) << "CppSupportPart::activePartChanged()" << endl;
    
    bool enabled = false;

    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(part);
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
    m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
    
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
    actionCollection()->action("edit_type_of_expression")->setEnabled(enabled);
 
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

    // standard project classstore - displayed in classview - widget
    m_pParser     = new CClassParser( classStore( ) );

    // code completion classstore - just for preparsing - stuff
    m_pCCParser   = new CClassParser( ccClassStore( ) );

    // code completion working class
    m_pCompletion = new CppCodeCompletion( this, classStore( ), ccClassStore( ) );

    QTimer::singleShot( 0, this, SLOT( initialParse( ) ) );
}


void
CppSupportPart::projectClosed( )
{
    kdDebug( 9007 ) << "projectClosed( )" << endl;
    bool enablePCS = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepcs" );

    if( enablePCS == true ){
	QString pcsFile = project( )->projectDirectory( ) + "/" +
	                  project( )->projectName( ) +
	                  pcsFileExt( );

	// saving project classstore - code completion cs is automatically saved on creation time
        if( !classStore( )->storeAll( pcsFile ) )
            kdDebug( 9007 ) << "EE: can't write file '" << pcsFile << "'" << endl;
    }

    delete m_pParser;
    delete m_pCCParser;
    delete m_pCompletion;

    m_pParser     = 0;
    m_pCCParser   = 0;
    m_pCompletion = 0;
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

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString str = econtext->currentLine();
    if (str.isEmpty())
        return;

    QRegExp re("[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*");
    if (!re.exactMatch(str))
        return;

    QString popupstr = re.cap(1);
    m_contextFileName = findHeader(project()->allFiles(), popupstr);
    if (m_contextFileName.isEmpty())
        return;

    popup->insertSeparator();
    popup->insertItem( i18n("Goto include file: %1").arg(popupstr),
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

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
	    kdDebug(9007) << "addedFilesToProject(): " << project()->projectDirectory() + "/" + ( *it ) << endl;

		// changed - daniel
		QString path = project()->projectDirectory() + "/" + ( *it );
		maybeParse( path, classStore( ), m_pParser );

		partController()->editDocument ( KURL ( path ) );
	}

    emit updatedSourceInfo();
}


void CppSupportPart::removedFilesFromProject(const QStringList &fileList)
{

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		kdDebug(9007) << "removedFilesFromProject(): " << project()->projectDirectory() + "/" + ( *it ) << endl;

		QString path = project()->projectDirectory() + "/" + ( *it );
		classStore()->removeWithReferences(path);
	}

	emit updatedSourceInfo();
}


void CppSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9007) << "savedFile(): " << fileName.mid ( project()->projectDirectory().length() + 1 ) << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        // changed - daniel
        maybeParse( fileName, classStore( ), m_pParser );
        emit updatedSourceInfo();
    }
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


QStringList CppSupportPart::fileFilters()
{
    if (withcpp)
        return QStringList::split(",", "*.c,*.C,*.cc,*.cpp,*.c++,*.cxx,*.m,*.mm,*.M,*.h,*.H,*.hh,*.hxx,*.hpp,*.inl,*.tlh,*.diff");
    else
        return QStringList::split(",", "*.c,*.h");
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
        return QStringList::split(",", "c,C,cc,cpp,c++,cxx,m,mm,M,h,H,hh,hxx,hpp,inl,tlh,diff");
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
    ParsedClass *pc = classStore()->getClassByName(className);

    if (!pc) {
	QMessageBox::critical(0,i18n("Error"),i18n("Please select a class!"));
	return;
    }

    CppAddMethodDialog dlg( m_pParser->getClassStore(), m_pCCParser->getClassStore(),
			    className, 0, "methodDlg"); //TODO: Leak ?
    if (!dlg.exec())
        return;

    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;

    if (pm->isSignal()) {
        for (pc->signalIterator.toFirst(); pc->signalIterator.current(); ++pc->signalIterator) {
            ParsedMethod *meth = pc->signalIterator.current();
            if (meth->access() == pm->access() &&
                atLine < (int)meth->declarationEndsOnLine())
                atLine = (int)meth->declarationEndsOnLine();
        }
    } else if (pm->isSlot()) {
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            ParsedMethod *meth = pc->slotIterator.current();
            if (meth->access() == pm->access() &&
                atLine < (int)meth->declarationEndsOnLine())
                atLine = (int)meth->declarationEndsOnLine();
        }
    } else {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            ParsedMethod *meth = pc->methodIterator.current();
            if (meth->access() == pm->access() &&
                atLine < (int)meth->declarationEndsOnLine())
                atLine = (int)meth->declarationEndsOnLine();
        }
    }

    QString headerCode = asHeaderCode(pm);

    if (atLine == -1) {
        if (pm->isSignal())
            headerCode.prepend(QString("signals:\n"));
        else if (pm->access() == PIE_PUBLIC)
            headerCode.prepend(QString("public%1:\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PROTECTED)
            headerCode.prepend(QString("protected%1:\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PRIVATE)
            headerCode.prepend(QString("private%1:\n").arg(pm->isSlot()? " slots" :  ""));
        else
            kdDebug(9007) << "selectedAddMethod: Unknown access "
                          << (int)pm->access() << endl;

        atLine = pc->declarationEndsOnLine();
    } else
        atLine++;

    partController()->editDocument(pc->declaredInFile(), atLine);
    kdDebug(9007) << "Adding to .h: " << atLine << " " << headerCode << endl;

    KTextEditor::EditInterface *editiface;

    editiface = dynamic_cast<KTextEditor::EditInterface*>(partController()->activePart());
    if (editiface)
        editiface->insertText(atLine, 0, headerCode);
    else
        kdDebug(9007) << "no edit" << endl;

    QString cppCode = asCppCode(pm);

    partController()->editDocument(pc->definedInFile(), atLine);
    kdDebug(9007) << "Adding to .cpp: " << atLine << " " << cppCode << endl;

    editiface = dynamic_cast<KTextEditor::EditInterface*>(partController()->activePart());
    if (editiface)
        editiface->insertText(editiface->numLines()-1, 0, cppCode);
    else
        kdDebug(9007) << "no edit" << endl;

    delete pm;
}


void CppSupportPart::addAttribute(const QString &className)
{
    ParsedClass *pc = classStore()->getClassByName(className);

    if (!pc) {
	QMessageBox::critical(0,i18n("Error"),i18n("Please select a class!"));
	return;
    }

    AddClassAttributeDialog dlg(m_pParser->getClassStore(), m_pCCParser->getClassStore(), 0, "attrDlg");
    if( !dlg.exec() )
      return;

    ParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;

    for (pc->attributeIterator.toFirst(); pc->attributeIterator.current(); ++pc->attributeIterator) {
        ParsedAttribute *attr = pc->attributeIterator.current();
        if (attr->access() == pa->access() &&
            atLine < (int)attr->declarationEndsOnLine())
            atLine = (int)attr->declarationEndsOnLine();
    }

    QString headerCode = asHeaderCode(pa);

    if (atLine == -1) {
        if (pa->access() == PIE_PUBLIC)
            headerCode.prepend("public: // Public attributes\n");
        else if (pa->access() == PIE_PROTECTED)
            headerCode.prepend("protected: // Protected attributes\n");
        else if (pa->access() == PIE_PRIVATE)
            headerCode.prepend("private: // Private attributes\n");
        else
            kdDebug(9007) << "selectedAddAttribute: Unknown access "
                          << (int)pa->access() << endl;

        atLine = pc->declarationEndsOnLine();
    } else
        atLine++;

    partController()->editDocument(pc->declaredInFile(), atLine);
    kdDebug(9007) << "Adding at line " << atLine << " " << headerCode << endl;

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(partController()->activePart());
    if (editiface)
        editiface->insertText(atLine, 0, headerCode);
    else
        kdDebug(9007) << "no edit" << endl;

    delete pa;
}


QString CppSupportPart::asHeaderCode(ParsedMethod *pm)
{
    QString str;

    KConfig *config = kapp->config ();
    config->setGroup ("AStyle");
    QString fill = config->readEntry ("Fill", "Tabs");
    QString fillSpaces = config->readEntry ("FillSpaces", "2");
    QString style = config->readEntry ("Style", "UserDefined");

    // set the default indent : space
    QString indent = " ";
   
    if (style == "UserDefined")
    { 
      bool ok = true; 
      // set indention according to the configuration 
      indent = ((fill == "Tabs") ? QString ("\t") : indent.fill (' ', fillSpaces.toInt (&ok)));
      // if the fillSpaces property is not a number -> reset indention to 2 spaces
      indent = (ok ? indent : QString ("  "));
    }
    else
    {
      indent = "\t";
    } 
 
    if( !pm->comment().isEmpty() ) {
        str += indent + "/**\n" + indent;
        str += pm->comment().replace( QRegExp("\n"), "\n" + indent );
        str += "\n" + indent + "*/\n";
    }
 
    str += indent;

    if (pm->isVirtual())
        str += "virtual ";

    if (pm->isStatic())
        str += "static ";

    str += pm->type();
    str += " ";
    str += pm->name();

    /*
    if( pm->arguments.count() > 0 ) {
	str += "( ";
        for ( ParsedArgument* arg = pm->arguments.first(); arg != NULL; arg = pm->arguments.next() ) {
            if ( arg != pm->arguments.getFirst() )
                str += ", ";

            str += arg->toString();
        }
        str += " )";
    } else {
        str += "()";
    }
    */

    if (pm->isConst())
        str += " const";

    if (pm->isPure())
        str += " = 0";

    str += ";\n";

    return str;
}


QString CppSupportPart::asCppCode(ParsedMethod *pm)
{
    if (pm->isPure() || pm->isSignal())
        return QString();

    QString str = "\n";

    if( !pm->comment().isEmpty() )
        str += "/**\n" + pm->comment() + "\n*/\n";

    str += pm->type();
    str += " ";
    if( !pm->declaredInScope().isEmpty() ) {
        str += pm->declaredInScope();
        str += "::";
    }
    str += pm->name();

    /*
    if( pm->arguments.count() > 0 ) {
	str += "( ";
        for ( ParsedArgument* arg = pm->arguments.first(); arg != NULL; arg = pm->arguments.next() ) {
            if ( arg != pm->arguments.getFirst() )
                str += ", ";

            str += arg->toString();
        }
        str += " )";
    } else {
        str += "()";
    }
    */

    if (pm->isConst())
        str += " const";

    str += "\n{\n}\n";

    return str;
}


QString CppSupportPart::asHeaderCode(ParsedAttribute *pa)
{
    QString str;

    if( !pa->comment().isEmpty() ) {
        str += "\t/**\n\t";
        str += pa->comment().replace( QRegExp("\n"), "\n\t" );
        str += "\n\t*/\n";
    }

    str += "\t";

    if (pa->isConst())
        str += "const ";

    if (pa->isStatic())
        str += "static ";

    str += pa->asString();
    str += ";\n";

    return str;
}

void CppSupportPart::slotCompleteText()
{
    m_pCompletion->completeText();
}

void CppSupportPart::slotTypeOfExpression()
{
    m_pCompletion->typeOf();
}

/**
 * parsing stuff for project persistant classstore and code completion
 */
void
CppSupportPart::initialParse( )
{
    // For debugging
    showMemUsage( );

    if( !project( ) ){
        // messagebox ?
        kdDebug( 9007 ) << "No project" << endl;
        return;
    }

    bool enablePCS = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepcs" );
    bool enablePP  = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepp" );
    bool bCreateProjectPCS  = false;
    bool bCreatePreParsePCS = false;

    QString pcsFile = project( )->projectDirectory( ) + "/" + project( )->projectName( );

    // no pcs - just parsing the project
    if( enablePCS == false ){
	parseProject( );
	emit updatedSourceInfo( );
	return;
    }
    else
	bCreateProjectPCS  = !restorePreParsedClassStore( classStore( )  , pcsFile + pcsFileExt( ) );

    if( enablePP == true  )
	bCreatePreParsePCS = !restorePreParsedClassStore( ccClassStore( ), pcsFile + ppFileExt( ) );

    if( bCreateProjectPCS == true  )
	createProjectPCS( pcsFile + pcsFileExt( ) );

    if( bCreatePreParsePCS == true )
        createPreParsePCS( pcsFile + ppFileExt( ) );

    showMemUsage( );
}


bool
CppSupportPart::restorePreParsedClassStore( ClassStore* cs, const QString fileToLoad )
{
    QLabel* label = new QLabel( "", mainWindow( )->statusBar( ) );
    label->setMinimumWidth( 600 );
    mainWindow( )->statusBar( )->addWidget( label );
    label->show( );
    label->setText( i18n( "Wait please - loading classstore file: %1" )
                    .arg( fileToLoad ) );

    kapp->processEvents( );
    kapp->setOverrideCursor( waitCursor );

    bool success = cs->restoreAll( fileToLoad );
    if( success ) {
        kdDebug( 9007 ) << "succeeded loading file '" << fileToLoad << "'" << endl;
	kdDebug( 9007 ) << "updating sourceinfo" << endl;
        emit updatedSourceInfo( );
    }
    else
        kdDebug( 9007 ) << "failed loading file '" << fileToLoad << "'" << endl;

    mainWindow( )->statusBar( )->removeWidget( label );
    delete label;

    kapp->restoreOverrideCursor( );
    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    return success;
}


bool
CppSupportPart::parseProject( )
{
    QLabel* label = new QLabel( "", mainWindow( )->statusBar( ) );
    label->setMinimumWidth( 600 );
    mainWindow( )->statusBar( )->addWidget( label );
    label->show( );

    kapp->processEvents( );
    kapp->setOverrideCursor( waitCursor );

    QStringList files = reorder( project( )->allFiles( ) );

    QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
    bar->setMinimumWidth( 120 );
    bar->setCenterIndicator( true );
    mainWindow( )->statusBar( )->addWidget( bar );
    bar->show( );

    int n = 0;
	QString filePath;
    for( QStringList::Iterator it = files.begin( ); it != files.end( ); ++it ) {
        bar->setProgress( n++ );
        kapp->processEvents( );
		filePath = project()->projectDirectory() + "/" + ( *it );
	label->setText( i18n( "Currently parsing: '%1'" )
	                .arg( filePath ) );
        maybeParse( filePath, classStore( ), m_pParser );
    }

    kdDebug( 9007 ) << "updating sourceinfo" << endl;
    emit updatedSourceInfo( );

    mainWindow( )->statusBar( )->removeWidget( bar );
    delete bar;
    mainWindow( )->statusBar( )->removeWidget( label );
    delete label;

    kapp->restoreOverrideCursor( );
    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    return true;
}


bool
CppSupportPart::createProjectPCS( const QString fileToSave )
{
    if( !parseProject( ) )
	return false;

    mainWindow( )->statusBar( )->message( i18n( "Saving Project File: %1" )
                			.arg( fileToSave ) );
    if( !classStore( )->storeAll( fileToSave ) ){
	// KMessageBox for user ?
        kdDebug( 9007 ) << "failed saving file '" << fileToSave << "'" << endl;
    }

    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    return true;
}


void
CppSupportPart::maybeParse( const QString fileName, ClassStore *store, CClassParser *parser )
{
    if( !fileExtensions( ).contains( QFileInfo( fileName ).extension( ) ) )
        return;

    store->removeWithReferences( fileName );
    parser->parse( fileName );
}


bool
CppSupportPart::createPreParsePCS( const QString fileToSave )
{
    QLabel* label = new QLabel( "", mainWindow( )->statusBar( ) );
    label->setMinimumWidth( 600 );
    mainWindow( )->statusBar( )->addWidget( label );
    label->show( );

    kapp->processEvents( );
    kapp->setOverrideCursor( waitCursor );

    QDomElement ppDirs = projectDom( )->documentElement( )
                         .namedItem( "cppsupportpart" ).toElement( )
                         .namedItem( "classstore" ).toElement( )
                         .namedItem( "preparsing" ).toElement( )
                         .firstChild( ).toElement( );

    label->setText( i18n( "Preparsing" ) );

    QProgressBar* bar = new QProgressBar( 0, mainWindow( )->statusBar( ) );
    bar->setMinimumWidth( 120 );
    bar->setCenterIndicator( true );
    mainWindow( )->statusBar( )->addWidget( bar );
    bar->show( );

    QDir    dirObject;
    // should be the project dir ?
    QString startDir = dirObject.absPath( );

    // going through the dirs stored in the projectDOM
    while( !ppDirs.isNull( ) ){
	kapp->processEvents( );
        if( ppDirs.tagName( ) == "directory" )
            parseDirectory( ppDirs.attribute( "dir" ), ppDirs.attribute( "parsesubdir" ) == "Yes",
                            bar, label );
        else
            kdDebug( 9007 ) << "CppSupportPart::createPreParsePCS unknown tag: '"
                            << ppDirs.tagName( ) << "'" << endl;

        ppDirs = ppDirs.nextSibling( ).toElement( );
    }

    // going back to where we started
    dirObject.cd( startDir );

    label->setText( i18n( "Saving File for CodeCompletion: %1" )
                    .arg( fileToSave ) );

    if( !ccClassStore( )->storeAll( fileToSave ) ) {
	// KMessageBox for user ?
        kdDebug( 9007 ) << "failed saving file " << fileToSave << endl;
    }

    mainWindow( )->statusBar( )->removeWidget( bar );
    delete bar;
    mainWindow( )->statusBar( )->removeWidget( label );
    delete label;

    kapp->restoreOverrideCursor( );
    mainWindow( )->statusBar( )->message( i18n( "Done" ), 2000 );

    // could be overridden later if file operations fail
    return true;
}


// better idea needed for not always calling with QProgressBar & QLabel
void
CppSupportPart::parseDirectory( const QString &startDir, bool withSubDir,
                                      QProgressBar *bar, QLabel *label   )
{
    QFileInfo* fi = 0;
    QDir       dirObject;

    dirObject.cd( startDir );

    if( withSubDir == true ){
        dirObject.setFilter( QDir::Dirs );
        const QFileInfoList* list = dirObject.entryInfoList( );
        QFileInfoListIterator it( *list );

        // if we find a directory we call recursively parseDirectory( )
        while( ( fi = it.current( ) ) ){
            // skipping "." - named files and directories
            if( fi->fileName( ).at( 0 ) != '.' )
                parseDirectory( fi->dirPath( true ) + "/" + fi->fileName( ), withSubDir, bar, label );
            ++it;
        }
    }

    dirObject.cd( startDir );
    dirObject.setFilter( QDir::Files );
    const QFileInfoList* list = dirObject.entryInfoList( );
    QFileInfoListIterator it( *list );

    bar->setTotalSteps( it.count( ) );
    int n = 0;

    while( ( fi = it.current( ) ) ){
	kapp->processEvents( );
        bar->setProgress( n++ );
        label->setText( i18n( "Currently parsing: '%1'" )
	                .arg( fi->filePath( ) ) );
        maybeParse( fi->filePath( ), ccClassStore ( ), m_pCCParser );
        ++it;
    }

    label->setText( "" );
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
    if( !m_activeEditor )
	return;
    
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
	    kdDebug(9007) << "------------> ast = " << (ast->text() ? tr("<empty>") : ast->text()) << endl;
	    if( node->text() )
	        textHint = node->text();
	    else
	        textHint = m_activeEditor->textLine( startLine ).simplifyWhiteSpace();
	}
    }
    m_backgroundParser->unlock();
}

void CppSupportPart::slotNodeSelected( QListViewItem* item )
{
    if( !item || !m_activeSelection )
	return;    
    
    m_activeSelection->setSelection( item->text(1).toInt(), item->text(2).toInt(),
				     item->text(3).toInt(), item->text(4).toInt() );
}

#include "cppsupportpart.moc"

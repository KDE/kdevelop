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

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
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

#if __GLIBC__
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
    : KDevLanguageSupport(parent, name)
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
    action->setWhatsThis( i18n("Complete current expression.") );
    action->setEnabled(false);

    action = new KAction(i18n("Type of Expression"), CTRL+Key_T,
                         this, SLOT(slotTypeOfExpression()),
                         actionCollection(), "edit_type_of_expression");
    action->setStatusText( i18n("Type of current expression") );
    action->setWhatsThis( i18n("Type of current expression.") );
    action->setEnabled(false);

    action = new KAction(i18n("New Class..."), "classnew", 0,
                         this, SLOT(slotNewClass()),
                         actionCollection(), "project_newclass");
    action->setStatusText( i18n("Generate a new class") );
    action->setWhatsThis( i18n("Generate a new class") );

    m_pParser      = 0;
    m_pCompletion  = 0;

    withcpp = false;
    if ( args.count() == 1 && args[ 0 ] == "Cpp" )
        withcpp = true;

    // daniel
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );

    // maybe this should be done within a slot so if the user changes the values they are applied immediately
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
    topLevel()->removeView(m_pCHWidget);

    delete m_pParser;
    delete m_pCompletion;

    delete m_pCCParser;
    delete m_pCHWidget;
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
    kdDebug( 9007 ) << "slotEnablePersistantClassStore called with setEnable = " << setEnable << endl;
}


void
CppSupportPart::slotEnablePreParsing( bool setEnable )
{
    kdDebug( 9007 ) << "slotEnablePreParsing called with setEnable = " << setEnable << endl;
}


void
CppSupportPart::slotChangedPreParsingPath( )
{
    kdDebug( 9007 ) << "slotChangedPreParsingPath called" << endl;
}


void
CppSupportPart::slotEnableCodeCompletion( bool setEnable )
{
    if( m_pCompletion )
        m_pCompletion->setEnableCodeCompletion( setEnable );
}


void
CppSupportPart::slotEnableCodeHinting( bool setEnable, bool setOutputView )
{
    if( setEnable == false ){
        if( m_pCHWidget ){
            topLevel( )->removeView( m_pCHWidget );
            delete m_pCHWidget;
            m_pCHWidget = 0;
        }
        return;
    }
    // enable it
    else {
        if( m_pCHWidget )
            topLevel()->removeView( m_pCHWidget );
        else
            m_pCHWidget = new CppSupportWidget( this );

        if( setOutputView )
            topLevel()->embedOutputView( m_pCHWidget, i18n("Code Hinting"));
        else
            topLevel()->embedSelectView( m_pCHWidget, i18n( "Code Hinting" ) );	    
    }
}


void CppSupportPart::activePartChanged(KParts::Part *part)
{
    bool enabled = false;

    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(part);

    if (doc) {
        QFileInfo fi(doc->url().path());
        QString ext = fi.extension();
        if (fileExtensions().contains(ext))
            enabled = true;
    }

    actionCollection()->action("edit_switchheader")->setEnabled(enabled);
    actionCollection()->action("edit_complete_text")->setEnabled(enabled);
    actionCollection()->action("edit_type_of_expression")->setEnabled(enabled);
}


void CppSupportPart::projectOpened()
{
    kdDebug( 9007 ) << "projectOpened()" << endl;

    connect( project( ), SIGNAL( addedFileToProject( const QString & ) ),
             this, SLOT( addedFileToProject( const QString & ) ) );
    connect( project( ), SIGNAL( removedFileFromProject( const QString &) ),
             this, SLOT( removedFileFromProject( const QString & ) ) );

    // We want to parse only after all components have been
    // properly initialized

    // normal classstore - displayed in classview-widget
    m_pParser     = new CClassParser( classStore( ) );
    // cc classstore - just for preparsing - stuff
    m_pCCParser   = new CClassParser( ccClassStore( ) );
    m_pCompletion = new CppCodeCompletion( this, classStore( ), ccClassStore( ) );

    // ???
    //    connect( m_pCompletion, SIGNAL( setCodeHintingText( const QString& ) ),
    //	     this, SLOT( slotCodeHintingText( const QString& ) ) );

    QTimer::singleShot( 0, this, SLOT( initialParse( ) ) );
}


void
CppSupportPart::projectClosed( )
{
    // daniel
    bool enablePCS = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepcs" );

    if( enablePCS == true ){
        QString projectFile = project( )->projectDirectory( ) + "/" + project( )->projectName( );
        QString pcsFile = projectFile + pcsFileExt( );
        if ( !classStore()->storeAll(pcsFile) )
            kdDebug(9007) << "EE: can't write file '" << pcsFile << "'" << endl;
    }

    // todo: save -pp file from new slot

    delete m_pParser;
    delete m_pCompletion;
    delete m_pCCParser;

    m_pCompletion = 0;
    m_pCCParser   = 0;
    m_pParser     = 0;
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


void CppSupportPart::maybeParse(const QString fileName, ClassStore *store, CClassParser *parser)
{
    if (!fileExtensions().contains(QFileInfo(fileName).extension()))
        return;

    store->removeWithReferences(fileName);
    parser->parse(fileName);
}


// Makes sure that header files come first
static QStringList reorder(const QStringList &list)
{
    QStringList headers, others;

    QStringList headerExtensions = QStringList::split(",", "h,H,hh,hxx,hpp");

    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        if (headerExtensions.contains(QFileInfo(*it).extension()))
            headers << (*it);
        else
            others << (*it);

    return headers + others;
}


void CppSupportPart::initialParse()
{
    // For debugging
    showMemUsage();

    if( !project( ) ){
        // messagebox ?
        kdDebug( 9007 ) << "No project" << endl;
        return;
    }

    bool enablePCS = DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepcs" );
    bool createProjectPCS = true;
    bool createPreParseCS = true;
    QString pcsFile = project( )->projectDirectory( ) + "/" + project( )->projectName( );

    QLabel* label = new QLabel( "", topLevel( )->statusBar( ) );
    label->setMinimumWidth( 600 );
    topLevel( )->statusBar( )->addWidget( label );
    label->show( );    

    if( enablePCS == true ){

        label->setText( i18n("Wait please - loading classstore-file: %1")
                        .arg(pcsFile + pcsFileExt()) );
        kapp->processEvents();
        kapp->setOverrideCursor(waitCursor);

        if ( classStore()->restoreAll(pcsFile + pcsFileExt()) ) {
            kdDebug(9007) << "loaded pcs-file '"
                          << pcsFile << pcsFileExt( ) << "'" << endl;
            createProjectPCS = false;
            emit updatedSourceInfo( );
        } else {
            kdDebug(9007) << "pcs-file: '" << pcsFile << pcsFileExt( ) << "' couldn't be opened" << endl;
        }

        label->setText( i18n("Wait please - loading preparsed file: %1")
                        .arg(pcsFile + ppFileExt()) );
        kapp->processEvents();

        if ( ccClassStore()->restoreAll(pcsFile + ppFileExt()) ) {
            kdDebug(9007) << "loaded persistant preparsed classstore: '"
                          << pcsFile << ppFileExt( ) << "'" << endl;
            createPreParseCS = false;
        } else {
            kdDebug(9007) << "persistant preparse file: '" << pcsFile << ppFileExt( )
                          << "' couldn't be opened" << endl;
        }

        label->setText( i18n("Done") );
        kapp->restoreOverrideCursor();

    }

    if( createProjectPCS ){
        // pcsFiles don't exist or couldn't be opened - normal procedure
        kdDebug(9007) << "no persistant classstore - starting to parse" << endl;
        kapp->setOverrideCursor(waitCursor);

        QStringList files = reorder( project()->allFiles() );

        QProgressBar* bar = new QProgressBar( files.count( ), topLevel( )->statusBar( ) );
        bar->setMinimumWidth( 120 );
        bar->setCenterIndicator( true );
        topLevel( )->statusBar( )->addWidget( bar );
        bar->show( );

        int n = 0;
        for( QStringList::Iterator it = files.begin( ); it != files.end( ); ++it ){
            bar->setProgress( n++ );
            kapp->processEvents( );
            maybeParse( *it, classStore( ), m_pParser );
        }

        topLevel( )->statusBar( )->removeWidget( bar );
        delete bar;

        emit updatedSourceInfo( );

        kapp->restoreOverrideCursor( );
    }

    // For debugging
    showMemUsage();

    if( createPreParseCS ){
        if( DomUtil::readBoolEntry( *projectDom( ), "/cppsupportpart/classstore/enablepp" ) == false ) {
            delete label;
            return;
        }

        // ok, from here we're pre-parsing directories
        kapp->setOverrideCursor( waitCursor );

        QDomElement ppDirs = projectDom( )->documentElement( )
                             .namedItem( "cppsupportpart" ).toElement( )
                             .namedItem( "classstore" ).toElement( )
                             .namedItem( "preparsing" ).toElement( )
                             .firstChild( ).toElement( );	

        label->setText( i18n( "Preparsing" ) );

        QProgressBar* bar = new QProgressBar( 0, topLevel( )->statusBar( ) );
        bar->setMinimumWidth( 120 );
        bar->setCenterIndicator( true );
        topLevel( )->statusBar( )->addWidget( bar );
        bar->show( );

        QDir    dirObject;
        QString startDir = dirObject.absPath( ); // should be the project dir ?

        // going through the dirs stored in the projectDOM
        while( !ppDirs.isNull( ) ){
            if( ppDirs.tagName( ) == "directory" )
                parseDirectory( ppDirs.attribute( "dir" ), ppDirs.attribute( "parsesubdir" ) == "Yes",
                                bar, label );
            else
                cerr << "ClassStoreOptionsWidget::ClassStoreOptionsWidget unknown tag: '"
                     << ppDirs.tagName( ) << "'" << endl;

            ppDirs = ppDirs.nextSibling( ).toElement( );
        }
        dirObject.cd( startDir );

        label->setText( i18n( "Saving ccClassStore: " ) + pcsFile + ppFileExt( ) );
        if ( !ccClassStore()->storeAll(pcsFile + ppFileExt()) )
            kdDebug(9007) << "EE: saving ccclassstore not successful!" << endl;

        topLevel( )->statusBar( )->removeWidget( bar );
        delete bar;

        kapp->restoreOverrideCursor( );
    }

    topLevel( )->statusBar( )->removeWidget( label );
    delete label;

    topLevel( )->statusBar( )->message( i18n( "Done" ), 2000 );
}


// better idea needed for not always calling with QProgressBar & QLabel
void CppSupportPart::parseDirectory(const QString &startDir, bool withSubDir,
                                    QProgressBar *bar, QLabel *label )
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
            // skipping ".", ".." and so all "dot-directories"
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
        bar->setProgress( n++ );
        label->setText( i18n( "Currently parsing: '" ) + fi->filePath( ) );
        maybeParse( fi->filePath( ), ccClassStore ( ), m_pCCParser );
        ++it;
    }
}


void CppSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9007) << "addedFileToProject()" << endl;
    // changed - daniel
    QString path = project()->projectDirectory() + "/" + fileName;
    maybeParse( path, classStore( ), m_pParser );
    emit updatedSourceInfo();
}


void CppSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9007) << "removedFileFromProject()" << endl;
    QString path = project()->projectDirectory() + "/" + fileName;
    classStore()->removeWithReferences(path);
    emit updatedSourceInfo();
}


void CppSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9007) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName)) {
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
    if (ext == "h" || ext == "hxx" || ext == "hpp" ) {
        candidates << (base + "c");
        candidates << (base + "cc");
        candidates << (base + "cpp");
        candidates << (base + "cxx");
        candidates << (base + "C");
        candidates << (base + "m");
        candidates << (base + "mm");
        candidates << (base + "M");
    } else if (QStringList::split(',', "c,cc,cpp,cxx,C,m,mm,M").contains(ext)) {
        candidates << (base + "h");
        candidates << (base + "hxx");
        candidates << (base + "hpp");
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
        return QStringList::split(",", "*.c,*.C,*.cc,*.cpp,*.cxx,*.m,*.mm,*.M,*.h,*.H,*.hh,*.hxx,*.hpp");
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


QStringList CppSupportPart::fileExtensions()
{
    if (withcpp)
        return QStringList::split(",", "c,C,cc,cpp,cxx,m,mm,M,h,H,hh,hxx,hpp");
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
    CppAddMethodDialog dlg( m_pParser->getClassStore(), className, 0, "methodDlg"); //TODO: Leak ?
    if (!dlg.exec())
        return;

    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);

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
            headerCode.prepend(QString("public:%1\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PROTECTED)
            headerCode.prepend(QString("protected:\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PRIVATE)
            headerCode.prepend(QString("private:\n").arg(pm->isSlot()? " slots" :  ""));
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
        editiface->insertLine(atLine, headerCode);
    else
        kdDebug(9007) << "no edit" << endl;

    QString cppCode = asCppCode(pm);

    partController()->editDocument(pc->definedInFile(), atLine);
    kdDebug(9007) << "Adding to .cpp: " << atLine << " " << cppCode << endl;

    editiface = dynamic_cast<KTextEditor::EditInterface*>(partController()->activePart());
    if (editiface)
        editiface->insertLine(atLine, cppCode);
    else
        kdDebug(9007) << "no edit" << endl;

    delete pm;
}


void CppSupportPart::addAttribute(const QString &className)
{
    AddClassAttributeDialog dlg(0, "attrDlg");
    if( !dlg.exec() )
      return;

    ParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);

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
        editiface->insertLine(atLine, headerCode);
    else
        kdDebug(9007) << "no edit" << endl;

    delete pa;
}


QString CppSupportPart::asHeaderCode(ParsedMethod *pm)
{
    QString str = "  ";
    str += pm->comment();
    str += "\n  ";

    if (pm->isVirtual())
        str += "virtual ";

    if (pm->isStatic())
        str += "static ";

    str += pm->type();
    str += " ";
    str += pm->name();

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

    QString str = pm->comment();
    str += "\n";

    str += pm->type();
    str += " ";
    str += formatClassName(pm->path());

    if (pm->isConst())
        str += " const";

    str += "{\n}\n";

    return str;
}


QString CppSupportPart::asHeaderCode(ParsedAttribute *pa)
{
    QString str = "  ";
    str += pa->comment();
    str += "\n  ";

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


#include "cppsupportpart.moc"

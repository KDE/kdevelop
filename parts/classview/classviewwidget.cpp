/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 * Partially based on KDE Studio ClassListView http://www.thekompany.com/projects/kdestudio/
 */

#include "classviewpart.h"
#include "classviewwidget.h"

#include <kiconloader.h>
#include <kinstance.h>
#include <kurl.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kconfig.h>

#include <kdevcore.h>
#include <kdevlanguagesupport.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>
#include <codemodel.h>
#include <codemodel_utils.h>

#include <klocale.h>
#include <kdebug.h>

#include <qheader.h>
#include <qdir.h>

// namespace ?!?
struct FindOp
{
   FindOp( const FunctionDom& dom ): m_dom( dom ) {}

   bool operator() ( const FunctionDefinitionDom& def ) const
   {
       if( m_dom->name() != def->name() )
           return false;

       if( m_dom->isConstant() != m_dom->isConstant() )
           return false;

       QString scope1 = QString("::") + m_dom->scope().join("::");
       QString scope2 = QString("::") + def->scope().join("::");
       if( !scope1.endsWith(scope2) )
           return false;

       const ArgumentList args = m_dom->argumentList();
       const ArgumentList args2 = def->argumentList();
       if( args.size() != args2.size() )
           return false;

       for( uint i=0; i<args.size(); ++i ){
           if( args[i]->type() != args[i]->type() )
	       return false;
       }

       return true;
   }

private:
   const FunctionDom& m_dom;
};

ClassViewWidget::ClassViewWidget( ClassViewPart * part )
    : KListView( 0, "ClassViewWidget" ), m_part( part ), m_projectDirectoryLength( 0 )
{
    addColumn( "" );
    header()->hide();
    setSorting( 0 );
    setRootIsDecorated( true );

    m_projectItem = 0;

    connect( this, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(slotExecuted(QListViewItem*)) );
    connect( this, SIGNAL(executed(QListViewItem*)), this, SLOT(slotExecuted(QListViewItem*)) );
    connect( m_part->core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( m_part->core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

    QStringList lst;
    lst << i18n( "KDevelop 3.x mode" ) << i18n( "KDevelop 2.x mode" ) << i18n( "Java like mode" );
    m_actionViewMode = new KSelectAction( i18n("View Mode"), KShortcut(), m_part->actionCollection(), "classview_mode" );
    m_actionViewMode->setItems( lst );

    m_actionNewClass = new KAction( i18n("New Class..."), KShortcut(), this, SLOT(slotNewClass()),
				    m_part->actionCollection(), "classview_new_class" );
    m_actionAddMethod = new KAction( i18n("Add Method..."), KShortcut(), this, SLOT(slotAddMethod()),
				    m_part->actionCollection(), "classview_add_method" );
    m_actionAddAttribute = new KAction( i18n("Add Attribute..."), KShortcut(), this, SLOT(slotAddAttribute()),
				    m_part->actionCollection(), "classview_add_attribute" );

    m_actionOpenDeclaration = new KAction( i18n("Open declaration"), KShortcut(), this, SLOT(slotOpenDeclaration()),
				    m_part->actionCollection(), "classview_open_declaration" );
    m_actionOpenImplementation = new KAction( i18n("Open implementation"), KShortcut(), this, SLOT(slotOpenImplementation()),
				    m_part->actionCollection(), "classview_open_implementation" );

    KConfig* config = m_part->instance()->config();
    config->setGroup( "General" );
    setViewMode( config->readNumEntry( "ViewMode", KDevelop3ViewMode ) );
}

ClassViewWidget::~ ClassViewWidget( )
{
    KConfig* config = m_part->instance()->config();
    config->setGroup( "General" );
    config->writeEntry( "ViewMode", viewMode() );
    config->sync();
}

void ClassViewWidget::slotExecuted( QListViewItem* item )
{
    if( ClassViewItem* cbitem = dynamic_cast<ClassViewItem*>( item ) ){
	if( cbitem->hasImplementation() )
	    cbitem->openImplementation();
	else
	    cbitem->openDeclaration();
    }
}

void ClassViewWidget::clear( )
{
    KListView::clear();
    removedText.clear();
    m_projectItem = 0;
}

void ClassViewWidget::refresh()
{
    if( !m_part->project() )
	return;

    clear();
    m_projectItem = new FolderBrowserItem( this, m_part->project()->projectName() );
    m_projectItem->setOpen( true );
    blockSignals( true );

    FileList fileList = m_part->codeModel()->fileList();
    FileList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
	insertFile( (*it)->name() );
	++it;
    }

    blockSignals( false );
}

void ClassViewWidget::slotProjectOpened( )
{
    m_projectItem = new FolderBrowserItem( this, m_part->project()->projectName() );
    m_projectItem->setOpen( true );

    m_projectDirectory = QDir(m_part->project()->projectDirectory()).canonicalPath();
    if( m_projectDirectory.isEmpty() )
	m_projectDirectory = m_part->project()->projectDirectory();
    
    m_projectDirectoryLength = m_projectDirectory.length() + 1;

    connect( m_part->languageSupport(), SIGNAL(updatedSourceInfo()),
	     this, SLOT(refresh()) );
    connect( m_part->languageSupport(), SIGNAL(aboutToRemoveSourceInfo(const QString&)),
	     this, SLOT(removeFile(const QString&)) );
    connect( m_part->languageSupport(), SIGNAL(addedSourceInfo(const QString&)),
	     this, SLOT(insertFile(const QString&)) );
}

void ClassViewWidget::slotProjectClosed( )
{
}

void ClassViewWidget::insertFile( const QString& fileName )
{
    QString fn = QDir( fileName ).canonicalPath();
    if( fn.isEmpty() )
	fn = fileName;
    //kdDebug() << "======================== insertFile(" << fn << ")" << endl;

    FileDom dom = m_part->codeModel()->fileByName( fn );
    if( !dom )
	return;

    if( fn.startsWith(m_projectDirectory) )
	fn = fn.mid( m_projectDirectoryLength );

    QStringList path;

    switch( viewMode() )
    {
    case KDevelop3ViewMode:
	{
	    path = QStringList::split( "/", fn );
	    path.pop_back();
	}
	break;

    case KDevelop2ViewMode:
	{
	}
	break;

    case JavaLikeViewMode:
	{
	    QStringList l = QStringList::split( "/", fn );
	    l.pop_back();

	    QString package = l.join(".");
	    if( !package.isEmpty() )
		path.push_back( package );
	}
	break;
    }

    m_projectItem->processFile( dom, path );
}

void ClassViewWidget::removeFile( const QString& fileName )
{
    QString fn = QDir( fileName ).canonicalPath();
    if( fn.isEmpty() )
	fn = fileName;
    //kdDebug() << "======================== removeFile(" << fn << ")" << endl;

    FileDom dom = m_part->codeModel()->fileByName( fn);
    if( !dom )
	return;

    if( fn.startsWith(m_projectDirectory) )
	fn = fn.mid( m_projectDirectoryLength );

    QStringList path;

    switch( viewMode() )
    {
    case KDevelop3ViewMode:
	{
	    path = QStringList::split( "/", fn );
	    path.pop_back();
	}
	break;

    case KDevelop2ViewMode:
	{
	}
	break;

    case JavaLikeViewMode:
	{
	    QStringList l = QStringList::split( "/", fn );
	    l.pop_back();

	    QString package = l.join(".");
	    if( !package.isEmpty() )
		path.push_back( package );
	}
	break;
    }

    m_projectItem->processFile( dom, path, true );
}

void ClassViewWidget::contentsContextMenuEvent( QContextMenuEvent * ev )
{
    KPopupMenu menu( this );

    ClassViewItem* item = dynamic_cast<ClassViewItem*>( selectedItem() );

    m_actionOpenDeclaration->setEnabled( item && item->hasDeclaration() );
    m_actionOpenImplementation->setEnabled( item && item->hasImplementation() );

    m_actionOpenDeclaration->plug( &menu );
    m_actionOpenImplementation->plug( &menu );
    menu.insertSeparator();

    if( item && item->isClass() ){
        m_actionAddMethod->plug( &menu );
	m_actionAddAttribute->plug( &menu );
    }

    if( item && item->model() ){
	CodeModelItemContext context( item->model() );
	m_part->core()->fillContextMenu( &menu, &context );
    }
    menu.insertSeparator();

    int oldViewMode = viewMode();
    m_actionViewMode->plug( &menu );

    menu.exec( ev->globalPos() );

    if( viewMode() != oldViewMode )
	refresh();

    ev->consume();
}

void ClassViewWidget::setViewMode( int mode )
{
    m_actionViewMode->setCurrentItem( mode );
}

int ClassViewWidget::viewMode( ) const
{
     return m_actionViewMode->currentItem();
}

void FolderBrowserItem::processFile( FileDom file, QStringList& path, bool remove )
{
    if( path.isEmpty() ){
	NamespaceList namespaceList = file->namespaceList();
	ClassList classList = file->classList();
	FunctionList functionList = file->functionList();
	VariableList variableList = file->variableList();

	for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	    processNamespace( *it, remove );
	for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	    processClass( *it, remove );
	for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	    processFunction( *it, remove );
	for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	    processVariable( *it, remove );

	return;
    }

    QString current = path.front();
    path.pop_front();

    FolderBrowserItem* item = m_folders.contains( current ) ? m_folders[ current ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new FolderBrowserItem( this, current );
	if( listView()->removedText.contains(current) )
	    item->setOpen( true );
	m_folders.insert( current, item );
    }

    item->processFile( file, path, remove );

    if( remove && item->childCount() == 0 ){
	m_folders.remove( current );
	if( item->isOpen() ){
	    listView()->removedText << current;
	}
	delete( item );
	item = 0;
    }
}

void FolderBrowserItem::processNamespace( NamespaceDom ns, bool remove )
{
    NamespaceDomBrowserItem* item = m_namespaces.contains( ns->name() ) ? m_namespaces[ ns->name() ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new NamespaceDomBrowserItem( this, ns );
	if( listView()->removedText.contains(ns->name()) )
	    item->setOpen( true );
	m_namespaces.insert( ns->name(), item );
    }

    NamespaceList namespaceList = ns->namespaceList();
    ClassList classList = ns->classList();
    FunctionList functionList = ns->functionList();
    VariableList variableList = ns->variableList();

    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	item->processNamespace( *it, remove );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	item->processFunction( *it, remove );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	item->processVariable( *it, remove );

    if( remove && item->childCount() == 0 ){
	m_namespaces.remove( ns->name() );
	if( item->isOpen() ){
	    listView()->removedText << ns->name();
	}
	delete( item );
	item = 0;
    }
}

void FolderBrowserItem::processClass( ClassDom klass, bool remove )
{
    ClassDomBrowserItem* item = m_classes.contains( klass ) ? m_classes[ klass ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new ClassDomBrowserItem( this, klass );
	if( listView()->removedText.contains(klass->name()) )
	    item->setOpen( true );
	m_classes.insert( klass, item );
    }

    ClassList classList = klass->classList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	item->processFunction( *it, remove );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	item->processVariable( *it, remove );

    if( remove && item->childCount() == 0 ){
	m_classes.remove( klass );
	if( item->isOpen() ){
	    listView()->removedText << klass->name();
	}
	delete( item );
	item = 0;
    }
}

void FolderBrowserItem::processFunction( FunctionDom fun, bool remove )
{
    FunctionDomBrowserItem* item = m_functions.contains( fun ) ? m_functions[ fun ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new FunctionDomBrowserItem( this, fun );
	m_functions.insert( fun, item );
    }

    if( remove ){
	m_functions.remove( fun );
	delete( item );
	item = 0;
    }
}

void FolderBrowserItem::processVariable( VariableDom var, bool remove )
{
    VariableDomBrowserItem* item = m_variables.contains( var ) ? m_variables[ var ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new VariableDomBrowserItem( this, var );
	m_variables.insert( var, item );
    }

    if( remove ){
	m_variables.remove( var );
	delete( item );
	item = 0;
    }
}

// ------------------------------------------------------------------------
void NamespaceDomBrowserItem::processNamespace( NamespaceDom ns, bool remove )
{
    NamespaceDomBrowserItem* item = m_namespaces.contains( ns->name() ) ? m_namespaces[ ns->name() ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new NamespaceDomBrowserItem( this, ns );
	if( listView()->removedText.contains(ns->name()) )
	    item->setOpen( true );
	m_namespaces.insert( ns->name(), item );
    }

    NamespaceList namespaceList = ns->namespaceList();
    ClassList classList = ns->classList();
    FunctionList functionList = ns->functionList();
    VariableList variableList = ns->variableList();

    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	item->processNamespace( *it, remove );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	item->processFunction( *it, remove );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	item->processVariable( *it, remove );

    if( remove && item->childCount() == 0 ){
	m_namespaces.remove( ns->name() );
	if( item->isOpen() ){
	    listView()->removedText << ns->name();
	}
	delete( item );
	item = 0;
    }
}

void NamespaceDomBrowserItem::processClass( ClassDom klass, bool remove )
{
    ClassDomBrowserItem* item = m_classes.contains( klass ) ? m_classes[ klass ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new ClassDomBrowserItem( this, klass );
	if( listView()->removedText.contains(klass->name()) )
	    item->setOpen( true );
	m_classes.insert( klass, item );
    }

    ClassList classList = klass->classList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	item->processFunction( *it, remove );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	item->processVariable( *it, remove );

    if( remove && item->childCount() == 0 ){
	m_classes.remove( klass );
	if( item->isOpen() ){
	    listView()->removedText << klass->name();
	}
	delete( item );
	item = 0;
    }
}

void NamespaceDomBrowserItem::processFunction( FunctionDom fun, bool remove )
{
    FunctionDomBrowserItem* item = m_functions.contains( fun ) ? m_functions[ fun ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new FunctionDomBrowserItem( this, fun );
	m_functions.insert( fun, item );
    }

    if( remove ){
	m_functions.remove( fun );
	delete( item );
	item = 0;
    }
}

void NamespaceDomBrowserItem::processVariable( VariableDom var, bool remove )
{
    VariableDomBrowserItem* item = m_variables.contains( var ) ? m_variables[ var ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new VariableDomBrowserItem( this, var );
	m_variables.insert( var, item );
    }

    if( remove ){
	m_variables.remove( var );
	delete( item );
	item = 0;
    }
}

// ------------------------------------------------------------------------
void ClassDomBrowserItem::processClass( ClassDom klass, bool remove )
{
    ClassDomBrowserItem* item = m_classes.contains( klass ) ? m_classes[ klass ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new ClassDomBrowserItem( this, klass );
	if( listView()->removedText.contains(klass->name()) )
	    item->setOpen( true );
	m_classes.insert( klass, item );
    }

    ClassList classList = klass->classList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	item->processFunction( *it, remove );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	item->processVariable( *it, remove );

    if( remove && item->childCount() == 0 ){
	m_classes.remove( klass );
	if( item->isOpen() ){
	    listView()->removedText << klass->name();
	}
	delete( item );
	item = 0;
    }
}

void ClassDomBrowserItem::processFunction( FunctionDom fun, bool remove )
{
    FunctionDomBrowserItem* item = m_functions.contains( fun ) ? m_functions[ fun ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new FunctionDomBrowserItem( this, fun );
	m_functions.insert( fun, item );
    }

    if( remove ){
	m_functions.remove( fun );
	delete( item );
	item = 0;
    }
}

void ClassDomBrowserItem::processVariable( VariableDom var, bool remove )
{
    VariableDomBrowserItem* item = m_variables.contains( var ) ? m_variables[ var ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new VariableDomBrowserItem( this, var );
	m_variables.insert( var, item );
    }

    if( remove ){
	m_variables.remove( var );
	delete( item );
	item = 0;
    }
}

void FolderBrowserItem::setup( )
{
    ClassViewItem::setup();
    setPixmap( 0, SmallIcon("folder") );
    setExpandable( true );
}

void NamespaceDomBrowserItem::setup( )
{
    ClassViewItem::setup();
    setPixmap( 0, UserIcon("CVnamespace", KIcon::DefaultState, listView()->m_part->instance()) );
    setExpandable( true );

    QString txt = listView()->m_part->languageSupport()->formatModelItem(m_dom.data(), true);
    setText( 0, txt );
}

void ClassDomBrowserItem::setup( )
{
    ClassViewItem::setup();
    setPixmap( 0, UserIcon("CVclass", KIcon::DefaultState, listView()->m_part->instance()) );
    setExpandable( true );

    QString txt = listView()->m_part->languageSupport()->formatModelItem(m_dom.data(), true);
    setText( 0, txt );
}

void FunctionDomBrowserItem::setup( )
{
    ClassViewItem::setup();

    QString iconName;
    if( m_dom->access() == CodeModelItem::Private )
        iconName = "CVprivate_meth";
    else if( m_dom->access() == CodeModelItem::Protected )
        iconName = "CVprotected_meth";
    else
        iconName = "CVpublic_meth";

    setPixmap( 0, UserIcon(iconName, KIcon::DefaultState, listView()->m_part->instance()) );

    QString txt = listView()->m_part->languageSupport()->formatModelItem(m_dom.data(), true);
    setText( 0, txt );
}

void FunctionDomBrowserItem::openDeclaration()
{
    int startLine, startColumn;
    m_dom->getStartPosition( &startLine, &startColumn );
    listView()->m_part->partController()->editDocument( KURL(m_dom->fileName()), startLine );
}

void FunctionDomBrowserItem::openImplementation()
{
    FunctionDefinitionList lst;
    FileList fileList = listView()->m_part->codeModel()->fileList();
    CodeModelUtils::findFunctionDefinitions( FindOp(m_dom), fileList, lst );

    if( lst.isEmpty() )
        return;

    FunctionDefinitionDom fun = lst.front();
    QString path = QFileInfo( m_dom->fileName() ).dirPath( true );

    for( FunctionDefinitionList::Iterator it=lst.begin(); it!=lst.end(); ++it ){
	if( path == QFileInfo((*it)->fileName()).dirPath(true) )
	    fun = *it;
    }

    int startLine, startColumn;
    fun->getStartPosition( &startLine, &startColumn );
    listView()->m_part->partController()->editDocument( KURL(fun->fileName()), startLine );
}

void VariableDomBrowserItem::setup( )
{
    ClassViewItem::setup();
    QString iconName;
    if( m_dom->access() == CodeModelItem::Private )
        iconName = "CVprivate_var";
    else if( m_dom->access() == CodeModelItem::Protected )
        iconName = "CVprotected_var";
    else
        iconName = "CVpublic_var";

    setPixmap( 0, UserIcon(iconName, KIcon::DefaultState, listView()->m_part->instance()) );

    QString txt = listView()->m_part->languageSupport()->formatModelItem(m_dom.data(), true);
    setText( 0, txt );
}

void VariableDomBrowserItem::openDeclaration()
{
    int startLine, startColumn;
    m_dom->getStartPosition( &startLine, &startColumn );

    listView()->m_part->partController()->editDocument( KURL(m_dom->fileName()), startLine );
}

void VariableDomBrowserItem::openImplementation()
{
}

QString FolderBrowserItem::key( int , bool ) const
{
    return "0 " + text( 0 );
}

QString NamespaceDomBrowserItem::key( int , bool ) const
{
    return "1 " + text( 0 );
}

QString ClassDomBrowserItem::key( int , bool ) const
{
    return "2 " + text( 0 );
}

QString FunctionDomBrowserItem::key( int , bool ) const
{
    return "3 " + text( 0 );
}

QString VariableDomBrowserItem::key( int , bool ) const
{
    return "4 " + text( 0 );
}

void ClassViewWidget::slotNewClass( )
{
    if( m_part->languageSupport()->features() & KDevLanguageSupport::NewClass )
	m_part->languageSupport()->addClass();
}

void ClassViewWidget::slotAddMethod( )
{
    if( m_part->languageSupport()->features() & KDevLanguageSupport::AddMethod )
        m_part->languageSupport()->addMethod( static_cast<ClassDomBrowserItem*>( selectedItem() )->dom() );
}

void ClassViewWidget::slotAddAttribute( )
{
    if( m_part->languageSupport()->features() & KDevLanguageSupport::AddAttribute )
        m_part->languageSupport()->addAttribute( static_cast<ClassDomBrowserItem*>( selectedItem() )->dom() );
}

void ClassViewWidget::slotOpenDeclaration( )
{
    static_cast<ClassViewItem*>( selectedItem() )->openDeclaration();
}

void ClassViewWidget::slotOpenImplementation( )
{
    static_cast<ClassViewItem*>( selectedItem() )->openImplementation();
}

void ClassDomBrowserItem::openDeclaration( )
{
    int startLine, startColumn;
    m_dom->getStartPosition( &startLine, &startColumn );
    listView()->m_part->partController()->editDocument( KURL(m_dom->fileName()), startLine );
}

bool FunctionDomBrowserItem::hasImplementation() const
{
    FunctionDefinitionList lst;
    FileList fileList = listView()->m_part->codeModel()->fileList();
    CodeModelUtils::findFunctionDefinitions( FindOp(m_dom), fileList, lst );

    return !lst.isEmpty();
}


#include "classviewwidget.moc"


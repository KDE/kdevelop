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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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

#include <urlutil.h>
#include <kdevcore.h>
#include <kdevlanguagesupport.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>
#include <codemodel.h>
#include <codemodel_utils.h>

#include <klocale.h>
#include <kdebug.h>

#include <q3header.h>
#include <qdir.h>
#include <q3stylesheet.h>
//Added by qt3to4:
#include <QContextMenuEvent>

// namespace ?!?

ClassViewWidget::ClassViewWidget( ClassViewPart * part )
    : KListView( 0, "ClassViewWidget" ), QToolTip( viewport() ), m_part( part ), m_projectDirectoryLength( 0 )
{
    addColumn( "" );
    header()->hide();
    setSorting( 0 );
    setRootIsDecorated( true );
	setAllColumnsShowFocus( true );

    m_projectItem = 0;

    connect( this, SIGNAL(returnPressed(Q3ListViewItem*)), this, SLOT(slotExecuted(Q3ListViewItem*)) );
    connect( this, SIGNAL(executed(Q3ListViewItem*)), this, SLOT(slotExecuted(Q3ListViewItem*)) );
    connect( m_part->core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( m_part->core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
    connect( m_part->core(), SIGNAL(languageChanged()), this, SLOT(slotProjectOpened()) );

    QStringList lst;
    lst << i18n( "Group by Directories" ) << i18n( "Plain List" ) << i18n( "Java Like Mode" );
    m_actionViewMode = new KSelectAction( i18n("View Mode"), KShortcut(), m_part->actionCollection(), "classview_mode" );
    m_actionViewMode->setItems( lst );
    m_actionViewMode->setWhatsThis(i18n("<b>View mode</b><p>Class browser items can be grouped by directories, listed in a plain or java like view."));

    m_actionNewClass = new KAction( i18n("New Class..."), KShortcut(), this, SLOT(slotNewClass()),
				    m_part->actionCollection(), "classview_new_class" );
    m_actionNewClass->setWhatsThis(i18n("<b>New class</b><p>Calls the <b>New Class</b> wizard."));
	
	m_actionCreateAccessMethods = new KAction( i18n("Create get/set Methods"), KShortcut(), this, SLOT(slotCreateAccessMethods()), m_part->actionCollection(), "classview_create_access_methods" );
	
	
    m_actionAddMethod = new KAction( i18n("Add Method..."), KShortcut(), this, SLOT(slotAddMethod()),
				    m_part->actionCollection(), "classview_add_method" );
    m_actionAddMethod->setWhatsThis(i18n("<b>Add method</b><p>Calls the <b>New Method</b> wizard."));
    m_actionAddAttribute = new KAction( i18n("Add Attribute..."), KShortcut(), this, SLOT(slotAddAttribute()),
				    m_part->actionCollection(), "classview_add_attribute" );
    m_actionAddAttribute->setWhatsThis(i18n("<b>Add attribute</b><p>Calls the <b>New Attribute</b> wizard."));

    m_actionOpenDeclaration = new KAction( i18n("Open Declaration"), KShortcut(), this, SLOT(slotOpenDeclaration()),
				    m_part->actionCollection(), "classview_open_declaration" );
    m_actionOpenDeclaration->setWhatsThis(i18n("<b>Open declaration</b><p>Opens a file where the selected item is declared and jumps to the declaration line."));
    m_actionOpenImplementation = new KAction( i18n("Open Implementation"), KShortcut(), this, SLOT(slotOpenImplementation()),
				    m_part->actionCollection(), "classview_open_implementation" );
    m_actionOpenImplementation->setWhatsThis(i18n("<b>Open implementation</b><p>Opens a file where the selected item is defined (implemented) and jumps to the definition line."));

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

void ClassViewWidget::slotExecuted( Q3ListViewItem* item )
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
    m_projectItem = new FolderBrowserItem( this, this, m_part->project()->projectName() );
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
    m_projectItem = new FolderBrowserItem( this, this, m_part->project()->projectName() );
    m_projectItem->setOpen( true );

    m_projectDirectory = URLUtil::canonicalPath( m_part->project()->projectDirectory() );
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
    QString fn = URLUtil::canonicalPath( fileName );
    //kdDebug() << "======================== insertFile(" << fn << ")" << endl;

    FileDom dom = m_part->codeModel()->fileByName( fn );
    if( !dom )
	return;

    fn = URLUtil::relativePathToFile(m_part->project()->projectDirectory(), fn);
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
    QString fn = URLUtil::canonicalPath( fileName );
    //kdDebug() << "======================== removeFile(" << fn << ")" << endl;

    FileDom dom = m_part->codeModel()->fileByName( fn );
    if( !dom )
	return;

    fn = URLUtil::relativePathToFile(m_part->project()->projectDirectory(), fn);
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

    bool sep = false;
    if( item && item->isClass() ){
        if( m_part->langHasFeature(KDevLanguageSupport::AddMethod) ) {
            m_actionAddMethod->plug( &menu );
            sep = true;
        }
	
        if( m_part->langHasFeature(KDevLanguageSupport::AddAttribute) ) {
	    m_actionAddAttribute->plug( &menu );
            sep = true;
        }	
    }
	
	if (item && item->isVariable()){
		if( m_part->langHasFeature(KDevLanguageSupport::CreateAccessMethods) ) 
				m_actionCreateAccessMethods->plug( &menu );
	}

    if( item && item->model() ){
	CodeModelItemContext context( item->model() );
	m_part->core()->fillContextMenu( &menu, &context );
//        sep = true;
    }
    if (sep)
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
	TypeAliasList typeAliasList = file->typeAliasList();
	FunctionList functionList = file->functionList();
	VariableList variableList = file->variableList();

	for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	    processNamespace( *it, remove );
	for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	    processClass( *it, remove );
	for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	    processTypeAlias( *it, remove );
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

	item = new FolderBrowserItem( m_widget, this, current );
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
    TypeAliasList typeAliasList = ns->typeAliasList();
    FunctionList functionList = ns->functionList();
    VariableList variableList = ns->variableList();

    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	item->processNamespace( *it, remove );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	item->processTypeAlias( *it, remove );
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
    emit m_widget->removedNamespace(ns->name());
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
    TypeAliasList typeAliasList = klass->typeAliasList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	item->processTypeAlias( *it, remove );
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

void FolderBrowserItem::processTypeAlias( TypeAliasDom typeAlias, bool remove )
{
    TypeAliasDomBrowserItem* item = m_typeAliases.contains( typeAlias ) ? m_typeAliases[ typeAlias ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new TypeAliasDomBrowserItem( this, typeAlias );
	if( listView()->removedText.contains(typeAlias->name()) )
	    item->setOpen( true );
	m_typeAliases.insert( typeAlias, item );
    }

    if( remove && item->childCount() == 0 ){
	m_typeAliases.remove( typeAlias );
	if( item->isOpen() ){
	    listView()->removedText << typeAlias->name();
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
    TypeAliasList typeAliasList = ns->typeAliasList();
    FunctionList functionList = ns->functionList();
    VariableList variableList = ns->variableList();

    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	item->processNamespace( *it, remove );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	item->processTypeAlias( *it, remove );
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
    TypeAliasList typeAliasList = klass->typeAliasList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	item->processTypeAlias( *it, remove );
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

void NamespaceDomBrowserItem::processTypeAlias( TypeAliasDom typeAlias, bool remove )
{
    TypeAliasDomBrowserItem* item = m_typeAliases.contains( typeAlias ) ? m_typeAliases[ typeAlias ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new TypeAliasDomBrowserItem( this, typeAlias );
	if( listView()->removedText.contains(typeAlias->name()) )
	    item->setOpen( true );
	m_typeAliases.insert( typeAlias, item );
    }

    if( remove && item->childCount() == 0 ){
	m_typeAliases.remove( typeAlias );
	if( item->isOpen() ){
	    listView()->removedText << typeAlias->name();
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
    TypeAliasList typeAliasList = klass->typeAliasList();
    FunctionList functionList = klass->functionList();
    VariableList variableList = klass->variableList();

    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	item->processClass( *it, remove );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	item->processTypeAlias( *it, remove );
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

void ClassDomBrowserItem::processTypeAlias( TypeAliasDom typeAlias, bool remove )
{
    TypeAliasDomBrowserItem* item = m_typeAliases.contains( typeAlias ) ? m_typeAliases[ typeAlias ] : 0;
    if( !item ){
	if( remove )
	    return;

	item = new TypeAliasDomBrowserItem( this, typeAlias );
	if( listView()->removedText.contains(typeAlias->name()) )
	    item->setOpen( true );
	m_typeAliases.insert( typeAlias, item );
    }

    if( remove && item->childCount() == 0 ){
	m_typeAliases.remove( typeAlias );
	if( item->isOpen() ){
	    listView()->removedText << typeAlias->name();
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

void TypeAliasDomBrowserItem::setup( )
{
    ClassViewItem::setup();
    setPixmap( 0, UserIcon("CVtypedef", KIcon::DefaultState, listView()->m_part->instance()) );
    setExpandable( false );

    QString txt = listView()->m_part->languageSupport()->formatModelItem(m_dom.data(), true);
    setText( 0, txt );
}

void FunctionDomBrowserItem::setup( )
{
    ClassViewItem::setup();

    QString iconName;
	QString methodType;

	if ( m_dom->isSignal() )
		methodType = "signal";
	else if (m_dom->isSlot() ) 
		methodType = "slot";
	else
		methodType = "meth";

    if( m_dom->access() == CodeModelItem::Private )
        iconName = "CVprivate_" + methodType;
    else if( m_dom->access() == CodeModelItem::Protected )
        iconName = "CVprotected_" + methodType;
    else
        iconName = "CVpublic_" + methodType;

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

    FunctionDefinitionDom fun;
    QFileInfo fileInfo( m_dom->fileName() );
    QString path = fileInfo.dirPath( true );

    for( FunctionDefinitionList::Iterator it=lst.begin(); it!=lst.end(); ++it )
    {
        QFileInfo defFileInfo( (*it)->fileName() );
        QString defPath = defFileInfo.dirPath( true );

        if( path != defPath )
            continue;

	if( defFileInfo.baseName() == fileInfo.baseName() ) {
            fun = *it;
        } else if( !fun ) {
	    fun = *it;
        }
    }

    if( !fun ) {
        fun = lst.front();
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

QString TypeAliasDomBrowserItem::key( int , bool ) const
{
    return "3 " + text( 0 );
}

QString FunctionDomBrowserItem::key( int , bool ) const
{
    return "4 " + text( 0 );
}

QString VariableDomBrowserItem::key( int , bool ) const
{
    return "5 " + text( 0 );
}

void ClassViewWidget::slotNewClass( )
{
    if( m_part->languageSupport()->features() & KDevLanguageSupport::NewClass )
	m_part->languageSupport()->addClass();
}

void ClassViewWidget::slotAddMethod( )
{
    if ( !selectedItem() ) return;

    if( m_part->languageSupport()->features() & KDevLanguageSupport::AddMethod )
        m_part->languageSupport()->addMethod( static_cast<ClassDomBrowserItem*>( selectedItem() )->dom() );
}

void ClassViewWidget::slotAddAttribute( )
{
    if ( !selectedItem() ) return;
    
    if( m_part->languageSupport()->features() & KDevLanguageSupport::AddAttribute )
        m_part->languageSupport()->addAttribute( static_cast<ClassDomBrowserItem*>( selectedItem() )->dom() );
}

void ClassViewWidget::slotOpenDeclaration( )
{
    if ( !selectedItem() ) return;
    
    static_cast<ClassViewItem*>( selectedItem() )->openDeclaration();
}

void ClassViewWidget::slotOpenImplementation( )
{
    if ( !selectedItem() ) return;
    
    static_cast<ClassViewItem*>( selectedItem() )->openImplementation();
}

void ClassDomBrowserItem::openDeclaration( )
{
    int startLine, startColumn;
    m_dom->getStartPosition( &startLine, &startColumn );
    listView()->m_part->partController()->editDocument( KURL(m_dom->fileName()), startLine );
}

void TypeAliasDomBrowserItem::openDeclaration( )
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

void ClassViewWidget::maybeTip( QPoint const & p )
{
	ClassViewItem * item = dynamic_cast<ClassViewItem*>( itemAt( p ) );
	if ( !item ) return;
	
	QString tooltip;
	
	if ( item->isNamespace() )
	{
		NamespaceDomBrowserItem * nitem = dynamic_cast<NamespaceDomBrowserItem*>( item );
		if ( nitem )
		{
			tooltip = nitem->dom()->scope().join("::") + "::" + nitem->dom()->name();
		}
	}
	else if ( item->isClass() )
	{
		ClassDomBrowserItem * citem = dynamic_cast<ClassDomBrowserItem*>( item );
		if ( citem )
		{
			tooltip = citem->dom()->scope().join("::") + "::" 
				+ citem->dom()->name() + " : " 
				+ citem->dom()->baseClassList().join(", ");
		}
	}
	else if ( item->isFunction() )
	{
		FunctionDomBrowserItem * fitem = dynamic_cast<FunctionDomBrowserItem*>( item );
		if ( fitem )
		{
			QString access;
			if ( fitem->dom()->access() == CodeModelItem::Private )
				access = "[private] ";
			else if ( fitem->dom()->access() == CodeModelItem::Protected )
				access = "[protected] ";
			else if ( fitem->dom()->access() == CodeModelItem::Public )
				access = "[public] ";

			QStringList arguments;
			ArgumentList const & list = fitem->dom()->argumentList();
			ArgumentList::ConstIterator it( list.begin() );
			while ( it != list.end() )
			{
				arguments << (*it)->type();
				++it;
			}
			
			QString strstatic = fitem->dom()->isStatic() ? QString( "[static] " ) : QString::null;
			QString strsignal = fitem->dom()->isSignal() ? QString( "[signal] " ) : QString::null;
			QString strslot = fitem->dom()->isSlot() ? QString( "[slot] " ) : QString::null;
			QString strresult = !fitem->dom()->resultType().isEmpty() ? fitem->dom()->resultType() + " " : QString::null;
			
			QString strconstant = fitem->dom()->isConstant() ? QString( " [const]" ) : QString::null;
			QString strabstract = fitem->dom()->isAbstract() ? QString( " [abstract]" ) : QString::null;
			
			tooltip = access + strstatic + strsignal + strslot + strresult
				+ fitem->dom()->scope().join("::") + "::" + fitem->dom()->name() 
				+ "(" + arguments.join(", ") + ")" + strconstant + strabstract;
		}
	}
	else if ( item->isVariable() )
	{
		VariableDomBrowserItem * vitem = dynamic_cast<VariableDomBrowserItem*>( item );
		if ( vitem )
		{
			QString access;
			if ( vitem->dom()->access() == CodeModelItem::Private )
				access = "[private] ";
			else if ( vitem->dom()->access() == CodeModelItem::Protected )
				access = "[protected] ";
			else if ( vitem->dom()->access() == CodeModelItem::Public )
				access = "[public] ";

			QString strstatic = vitem->dom()->isStatic() ? QString( "[static] " ) : QString::null;
			tooltip = access + strstatic + vitem->dom()->type() + " " + vitem->dom()->name();
		}	
	}
	else if ( item->isTypeAlias() )
	{
		if( TypeAliasDomBrowserItem * titem = dynamic_cast<TypeAliasDomBrowserItem*>( item ) )
		{
			tooltip = QString( "[Type] " ) + titem->dom()->type() + " " + titem->dom()->name();
		}
	}
	
	kdDebug(0) << tooltip << endl;
	
	QRect r = itemRect( item );

	if ( item && r.isValid() && !tooltip.isEmpty() )
	{
		tip( r, QString("<qt><pre>") + Q3StyleSheet::escape( tooltip ) + QString("</pre></qt>") );
	}
}

void ClassViewWidget::slotCreateAccessMethods( )
{
	if ( !selectedItem() ) return;

    if( m_part->languageSupport()->features() & KDevLanguageSupport::CreateAccessMethods )
	{
        VariableDomBrowserItem* item = dynamic_cast<VariableDomBrowserItem*>( selectedItem() );
		if (item == 0)
			return;
		
		m_part->languageSupport()->createAccessMethods(static_cast<ClassModel*>(static_cast<ClassDomBrowserItem*>(item->parent())->dom()),static_cast<VariableModel*>(item->dom()));
	}
}


#include "classviewwidget.moc"


/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classbrowser_widget.h"
#include "classbrowser_part.h"
#include "catalog.h"

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevcoderepository.h>
#include <kdevlanguagesupport.h>

typedef KGenericFactory<ClassBrowserPart> ClassBrowserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclassbrowser, ClassBrowserFactory( "kdevclassbrowser" ) );

ClassBrowserPart::ClassBrowserPart( QObject* parent, const char* name, const QStringList& )
    : KDevPlugin("ClassBrowser", "classbrowser", parent, name ? name : "ClassBrowserPart" )
{
    setInstance( ClassBrowserFactory::instance() );
    setXMLFile( "kdevpart_classbrowser.rc" );

    m_widget = new ClassBrowserWidget( this );
    QWhatsThis::add( m_widget, i18n("Class Browser") );

    setupActions();

    mainWindow()->embedSelectView( m_widget, i18n("Class Browser"), i18n("Class Browser") );

    connect( codeRepository(), SIGNAL(catalogRegistered(Catalog*)),
	     this, SLOT(slotCatalogAdded(Catalog*)) );
    connect( codeRepository(), SIGNAL(catalogUnregistered(Catalog*)),
	     this, SLOT(slotCatalogRemoved(Catalog*)) );
    connect( codeRepository(), SIGNAL(catalogChanged(Catalog*)),
	     this, SLOT(slotCatalogChanged(Catalog*)) );

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
}

ClassBrowserPart::~ClassBrowserPart()
{
    delete (ClassBrowserWidget*) m_widget;
}

void ClassBrowserPart::slotProjectOpened()
{
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
        Catalog* catalog = *it;
        ++it;

        m_widget->addCatalog( catalog );
    }

    connect( languageSupport(), SIGNAL(updatedSourceInfo()), this, SLOT(refresh()) );
}

void ClassBrowserPart::slotProjectClosed()
{
    //m_widget->clear();
}

void ClassBrowserPart::setupActions( )
{
    m_actionNamespaces = new KSelectAction( i18n("Namespaces"), 0, actionCollection(), "namespaces_combo" );
    //m_actionNamespaces->setEditable( true );
    m_actionNamespaces->setMenuAccelsEnabled( false );
    connect( m_actionNamespaces, SIGNAL(activated(const QString&)), this, SLOT(selectNamespace(const QString&)) );

    m_actionClasses = new KSelectAction( i18n("Classes"), 0, actionCollection(), "classes_combo" );
    //m_actionClasses->setEditable( true );
    m_actionClasses->setMenuAccelsEnabled( false );
    connect( m_actionClasses, SIGNAL(activated(const QString&)), this, SLOT(selectClass(const QString&)) );

    m_actionMethods = new KSelectAction( i18n("Methods"), 0, actionCollection(), "methods_combo" );
    //m_actionMethods->setEditable( true );
    m_actionMethods->setMenuAccelsEnabled( false );
    connect( m_actionMethods, SIGNAL(activated(const QString&)), this, SLOT(selectMethod(const QString&)) );
}

void ClassBrowserPart::slotCatalogAdded( Catalog * catalog )
{
    Q_UNUSED( catalog );
    refresh();
}

void ClassBrowserPart::slotCatalogRemoved( Catalog * catalog )
{
    Q_UNUSED( catalog );
    refresh();
}

void ClassBrowserPart::slotCatalogChanged( Catalog * catalog )
{
    Q_UNUSED( catalog );

    QString m_selectedNamespaces = m_actionNamespaces->currentText();
    QString m_selectedClasses = m_actionClasses->currentText();
    QString m_selectedMethods = m_actionMethods->currentText();

    refresh();

    //m_actionNamespaces->setCurrentText( m_selectedNamespaces );
    //m_actionClasses->setCurrentText( m_selectedClasses );
    //m_actionMethods->setCurrentText( m_selectedMethods );
    //m_actionFields->setCurrentText( m_selectedFields );
}

void ClassBrowserPart::refresh( )
{
    kdDebug() << "---------------------------------- REFRESH!!!!!!!!" << endl;
    QValueList<Tag> namespaceList;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace );

    m_actionNamespaces->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
	Catalog* catalog = *it;
	++it;

	namespaceList += catalog->query( args );
    }

    namespaceList = ClassBrowserUtils::simplifyNamespaces( namespaceList );
    QStringList items;
    items << QString::fromLatin1("");

    QValueList<Tag>::Iterator dit = namespaceList.begin();
    while( dit != namespaceList.end() ){
	Tag& t = *dit;
	++dit;

	items.append( t.path() );
    }

    items.sort();

    kdDebug() << "#items is " << items.size() << endl;
    m_actionNamespaces->setItems( items );

    refreshClasses();
    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectNamespace( const QString & name )
{
    Q_UNUSED( name );

    refreshClasses();
    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectClass( const QString & name )
{
    Q_UNUSED( name );

    refreshMethods();
    adjust();
}

void ClassBrowserPart::selectMethod( const QString & name )
{
    if( name.isEmpty() )
	return;

    int idx = name.find( '(' );
    if( idx == -1 )
	return;

    QString methodName = name.left( idx );
    kdDebug() << "search implementation of method " << methodName << endl;

    QStringList scope = QStringList::split( "::", m_actionNamespaces->currentText() );
    if( !m_actionClasses->currentText().isEmpty() )
	scope << m_actionClasses->currentText();

    // try with function implementation first
    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Function )
	<< Catalog::QueryArgument( "name", methodName )
	<< Catalog::QueryArgument( "scope", scope );

    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
	Catalog* catalog = *it;
	++it;

	QValueList<Tag> methodList = catalog->query( args );
	kdDebug() << "#items is " << methodList.size() << endl;

	QValueList<Tag>::Iterator dit = methodList.begin();
	while( dit != methodList.end() ){
	    Tag& t = *dit;
	    ++dit;

	    QString str = languageSupport()->formatTag( t );
	    //kdDebug() << "str = " << str << endl;

	    if( str == name ){
		int line, col;
		t.getStartPosition( &line, &col );
		kdDebug() << "found implementation at " << t.fileName() << " " << line << ", " << col << endl;
		partController()->editDocument( KURL(t.fileName()), line );
		adjust();
		return;
	    }
	}
    }


    // no implementation try to find a declaration
    kdDebug() << "search declaration of method " << methodName << endl;

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
	<< Catalog::QueryArgument( "name", methodName )
	<< Catalog::QueryArgument( "scope", scope );

    it = l.begin();
    while( it != l.end() ){
	Catalog* catalog = *it;
	++it;

	QValueList<Tag> methodList = catalog->query( args );
	kdDebug() << "#items is " << methodList.size() << endl;

	QValueList<Tag>::Iterator dit = methodList.begin();
	while( dit != methodList.end() ){
	    Tag& t = *dit;
	    ++dit;

	    QString str = languageSupport()->formatTag( t );
	    //kdDebug() << "str = " << str << endl;

	    if( str == name ){
		int line, col;
		t.getStartPosition( &line, &col );
		kdDebug() << "found declaration at " << t.fileName() << " " << line << ", " << col << endl;
		partController()->editDocument( t.fileName(), line );
		adjust();
		return;
	    }
	}
    }

    adjust();
}

void ClassBrowserPart::refreshClasses( )
{
    QValueList<Tag> classList;

    QStringList scope = QStringList::split( "::", m_actionNamespaces->currentText() );

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
	<< Catalog::QueryArgument( "scope", scope );

    m_actionClasses->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
	Catalog* catalog = *it;
	++it;

	classList += catalog->query( args );
    }

    QStringList items;
    items << QString::fromLatin1("");

    QValueList<Tag>::Iterator dit = classList.begin();
    while( dit != classList.end() ){
	Tag& t = *dit;
	++dit;

        QString name = t.name();
        if( !name.isEmpty() )
	    items.append( name );
    }

    items.sort();

    kdDebug() << "#items is " << items.size() << endl;
    m_actionClasses->setItems( items );
    adjust();
}

void ClassBrowserPart::refreshMethods( )
{
    QValueList<Tag> methodList;

    QStringList scope = QStringList::split( "::", m_actionNamespaces->currentText() );
    if( !m_actionClasses->currentText().isEmpty() )
	scope << m_actionClasses->currentText();

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
	<< Catalog::QueryArgument( "scope", scope );

    if( !m_selectedFileName.isEmpty() )
	args << Catalog::QueryArgument( "fileName", m_selectedFileName );

    m_actionMethods->clear();
    QValueList<Catalog*> l = codeRepository()->registeredCatalogs();
    QValueList<Catalog*>::Iterator it = l.begin();
    while( it != l.end() ){
	Catalog* catalog = *it;
	++it;

	methodList += catalog->query( args );
    }

    QStringList items;
    items << QString::fromLatin1("");

    QValueList<Tag>::Iterator dit = methodList.begin();
    while( dit != methodList.end() ){
	Tag& t = *dit;
	++dit;

	items.append( languageSupport()->formatTag(t) );
    }

    items.sort();
    kdDebug() << "#items is " << items.size() << endl;
    m_actionMethods->setItems( items );
    adjust();
}

void ClassBrowserPart::adjust( )
{
    m_actionNamespaces->setComboWidth( 200 );
    m_actionClasses->setComboWidth( 150 );
    m_actionMethods->setComboWidth( 300 );
}

#include "classbrowser_part.moc"

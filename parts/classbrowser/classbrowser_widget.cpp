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

#include "classbrowser_part.h"
#include "classbrowser_widget.h"

#include <catalog.h>
#include <kdevlanguagesupport.h>
#include <kdevproject.h>
#include <kdevcoderepository.h>
#include <kdevlanguagesupport.h>
#include <kdevpartcontroller.h>

#include <kparts/part.h>

#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <kdevcore.h>
#include <kpushbutton.h>

#include <qlabel.h>
#include <qheader.h>
#include <qfileinfo.h>

class TagListViewItem: public KListViewItem
{
public:
    TagListViewItem( KListView* parent, const Tag& tag, Catalog* c )
	: KListViewItem( parent ), m_tag( tag ), m_catalog( c ) { init(); }
    TagListViewItem( KListViewItem* parent, const Tag& tag, Catalog* c )
	: KListViewItem( parent ), m_tag( tag ), m_catalog( c ) { init(); }

    void init()
    {
	if( m_tag.kind() == Tag::Kind_Namespace || m_tag.kind() == Tag::Kind_Class ){
	    setExpandable( true );
	}

	QString fileName = m_tag.fileName();
	if( fileName.isEmpty() )
	    fileName = "<nofile>";

	setText( 0, languageSupport()->formatTag(m_tag) );
	setText( 1, fileName );
	int line, column;
	m_tag.getStartPosition( &line, &column );
	setText( 2, QString::number(line+1) );
	setText( 3, QString::number(column+1) );
    }

    bool needRefresh()
    {
	return true;
    }

    void refresh()
    {
	if( needRefresh() ){
	    clear();
	    computeChilds();
	}
    }

    void computeChilds()
    {
	QStringList scope;
	scope += m_tag.scope();
	scope << m_tag.name();

	QValueList<Catalog::QueryArgument> args;
	args << Catalog::QueryArgument( "scope", scope );
	args << Catalog::QueryArgument( "kind", Tag::Kind_Class );
        if( m_tag.kind() == Tag::Kind_Class )
            args << Catalog::QueryArgument( "fileName", m_tag.fileName() );
	QValueList<Tag> tags = m_catalog->query( args );

	args.clear();
	args << Catalog::QueryArgument( "scope", scope );
	args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace );
        tags += ClassBrowserUtils::simplifyNamespaces( m_catalog->query(args) );

	args.clear();
	args << Catalog::QueryArgument( "scope", scope );
	args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration );
        if( m_tag.kind() == Tag::Kind_Class )
            args << Catalog::QueryArgument( "fileName", m_tag.fileName() );
	tags += m_catalog->query( args );

	args.clear();
	args << Catalog::QueryArgument( "scope", scope );
	args << Catalog::QueryArgument( "kind", Tag::Kind_Variable );
        if( m_tag.kind() == Tag::Kind_Class )
            args << Catalog::QueryArgument( "fileName", m_tag.fileName() );
	tags += m_catalog->query( args );

	QValueList<Tag>::Iterator it = tags.begin();
	while( it != tags.end() ){
	    const Tag& tag = *it;
	    ++it;

	    if( !tag.name().isEmpty() ){
		TagListViewItem* i = new TagListViewItem( this, tag, m_catalog );
		if( m_removedItems.contains(i->text(0)) )
		    i->setOpen( true );
	    }
	}
    }

    void setOpen( bool opened )
    {
	if( opened == true ){
	    refresh();
	}
	KListViewItem::setOpen( opened );
    }

    void clear()
    {
	m_removedItems.clear();
	while( firstChild() ){
	    if( firstChild()->isOpen() )
		m_removedItems << firstChild()->text( 0 );

	    delete firstChild();
	}
    }

    Catalog* catalog()
    {
	return m_catalog;
    }

    KDevLanguageSupport* languageSupport()
    {
        ClassBrowserWidget* cb = static_cast<ClassBrowserWidget*>( listView() );
        return cb->languageSupport();
    }

private:
    Tag m_tag;
    Catalog* m_catalog;
    QStringList m_removedItems;
};

class CatalogListViewItem: public KListViewItem
{
public:
    CatalogListViewItem( KListView* parent, Catalog* c )
	: KListViewItem( parent ), m_catalog( c ) 
    {
	QFileInfo info( m_catalog->dbName() );
	setText( 0, info.fileName() );
    }

    bool needRefresh()
    {
	return true;
    }

    void refresh()
    {
	if( needRefresh() ){
	    clear();
	    computeChilds();
	}
    }

    void computeChilds()
    {
	QValueList<Catalog::QueryArgument> args;
	args << Catalog::QueryArgument( "scope", QStringList() );
	args << Catalog::QueryArgument( "kind", Tag::Kind_Class );
	QValueList<Tag> tags = m_catalog->query( args );

	args.clear();
	args << Catalog::QueryArgument( "scope", QStringList() );
	args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace );
	tags += ClassBrowserUtils::simplifyNamespaces( m_catalog->query(args) );

	QValueList<Tag>::Iterator it = tags.begin();
	while( it != tags.end() ){
	    const Tag& tag = *it;
	    ++it;

	    if( !tag.name().isEmpty() ){
		TagListViewItem* i = new TagListViewItem( this, tag, m_catalog );
		if( m_removedItems.contains(i->text(0)) )
		    i->setOpen( true );
	    }
	}
    }

    void setOpen( bool opened )
    {
	if( opened == true ){
	    refresh();
	}
	KListViewItem::setOpen( opened );
    }

    void clear()
    {
	m_removedItems.clear();
	while( firstChild() ){
	    if( firstChild()->isOpen() )
		m_removedItems << firstChild()->text( 0 );

	    delete firstChild();
	}
    }

    Catalog* catalog()
    {
	return m_catalog;
    }

private:
    Catalog* m_catalog;
    QStringList m_removedItems;
};

ClassBrowserWidget::ClassBrowserWidget( ClassBrowserPart* part )
    : KListView( 0, "ClassBrowserWidget" ), m_part( part )
{

    //this->header()->hide();
    this->addColumn( i18n("Symbol") );
//    this->addColumn( i18n("File Name") );
//    this->addColumn( i18n("Line") );
//    this->addColumn( i18n("Column") );
    this->header()->hide();
    
    this->setRootIsDecorated( true );

    init();

    connect( m_part->codeRepository(), SIGNAL(catalogRegistered(Catalog*)),
	     this, SLOT(addCatalog(Catalog*)) );
    connect( m_part->codeRepository(), SIGNAL(catalogUnemoved(Catalog*)),
	     this, SLOT(removeCatalog(Catalog*)) );
    connect( m_part->codeRepository(), SIGNAL(catalogChanged(Catalog*)),
	     this, SLOT(slotCatalogChanged(Catalog*)) );
    
    connect( this, SIGNAL(executed(QListViewItem*)),
	     this, SLOT(slotItemExecuted(QListViewItem*)) );
}

ClassBrowserWidget::~ClassBrowserWidget()
{
}

void ClassBrowserWidget::slotStartSearch( )
{
}

void ClassBrowserWidget::init( )
{
}

void ClassBrowserWidget::addCatalog( Catalog * catalog )
{
    KListViewItem* item = new CatalogListViewItem( this, catalog );
    item->setExpandable( true );
    m_items.insert( catalog, item );
}

void ClassBrowserWidget::removeCatalog( Catalog * catalog )
{
    m_items.remove( catalog );
}

void ClassBrowserWidget::slotCatalogChanged( Catalog * catalog )
{
    kdDebug(9000) << "ClassBrowserWidget::slotCatalogChanged()" << endl;
    
    CatalogListViewItem* item = static_cast< CatalogListViewItem* >( m_items[catalog] );
    if( !item || !item->isOpen() )
	return;
    
    item->refresh();
}

KDevLanguageSupport* ClassBrowserWidget::languageSupport()
{
    return m_part->languageSupport();
}

void ClassBrowserWidget::slotItemExecuted( QListViewItem* item )
{
    m_part->partController()->editDocument( KURL(item->text(1)), item->text(2).toInt() );
}

#include "classbrowser_widget.moc"

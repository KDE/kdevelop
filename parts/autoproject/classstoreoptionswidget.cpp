/***************************************************************************
 *   Copyright (C) 2001 by Daniel Engelschalt                              *
 *   daniel.engelschalt@gmx.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include "domutil.h"
#include "autoprojectpart.h"
#include "classstoreoptionswidget.h"

#include <iostream>
#include <qlistview.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "kdevpart.h"
#include "kdevproject.h"

ClassStoreOptionsWidget::ClassStoreOptionsWidget( AutoProjectPart* part, QWidget* parent, const char* name )
    : ClassStoreOptionsWidgetBase( parent, name )
{
    m_pPart = part;

    QDomDocument dom = *m_pPart->projectDom( );
    
    cbEnablePCS->setChecked( DomUtil::readBoolEntry( dom, "/autoprojectpart/classstore/enablepcs" ) );
    bool b = DomUtil::readBoolEntry( dom, "/autoprojectpart/classstore/enablepp" );
    cbEnablePP->setChecked( b );
    if( b == false )
	lvPCSPaths->setEnabled( false );
	
    lePCSFile->setText( m_pPart->project( )->projectDirectory( ) + "/" +
                        m_pPart->project( )->projectName( ) + ".pcs" );

    lvPCSPaths->setSorting( -1 );

    QDomElement element    = dom.documentElement( );
    QDomElement apPart     = element.namedItem( "autoprojectpart" ).toElement( );
    QDomElement classstore = apPart.namedItem( "classstore" ).toElement( );
    QDomElement preparsing = classstore.namedItem( "preparsing" ).toElement( );
    QDomElement dir        = preparsing.firstChild( ).toElement( );
    
    QListViewItem* lastItem = 0;
    while( !dir.isNull( ) ){
	if( dir.tagName( ) == "directory" ){
	    QListViewItem* newItem = new QListViewItem( lvPCSPaths, dir.attribute( "parsesubdir" ),
			 					    dir.attribute( "dir" ) );
	    if( lastItem )
		newItem->moveItem( lastItem );

	    lastItem = newItem;
	}
	else
	    cerr << "ClassStoreOptionsWidget::ClassStoreOptionsWidget unknown tag: '" << dir.tagName( ) << "'" << endl;

	dir = dir.nextSibling( ).toElement( );
    }
}


ClassStoreOptionsWidget::~ClassStoreOptionsWidget( )
{
}

void
ClassStoreOptionsWidget::accept( )
{
    QDomDocument dom     = *m_pPart->projectDom( );
    QDomElement  element = dom.documentElement( );

    QDomElement apPart  = element.namedItem( "autoprojectpart" ).toElement( );
    if( apPart.isNull( ) ){
	apPart = dom.createElement( "autoprojectpart" );
	element.appendChild( apPart );
    }

    QDomElement classstore = apPart.namedItem( "classstore" ).toElement( );
    if( classstore.isNull( ) ){
	classstore = dom.createElement( "classstore" );
	apPart.appendChild( classstore );
    }
    
    DomUtil::writeBoolEntry( dom, "/autoprojectpart/classstore/enablepcs", cbEnablePCS->isChecked( ) );
    DomUtil::writeBoolEntry( dom, "/autoprojectpart/classstore/enablepp", cbEnablePP->isChecked( ) );
    
    QDomElement directories = classstore.namedItem( "preparsing" ).toElement( );
    if( directories.isNull( ) ){
	directories = dom.createElement( "preparsing" );
	classstore.appendChild( directories );
    }
    
    // clear old entries
    while( !directories.firstChild( ).isNull( ) )
	directories.removeChild( directories.firstChild( ) );

    // write content of the listview to dom
    QListViewItemIterator it( lvPCSPaths );    
    for( ; it.current( ); ++it ){
	QDomElement dir = dom.createElement( "directory" );
	dir.setAttribute( "parsesubdir", it.current( )->text( 0 ) );
	dir.setAttribute( "dir", it.current( )->text( 1 ) );
	directories.appendChild( dir );
    }
}

void
ClassStoreOptionsWidget::slotEnablePCS( bool isChecked )
{
    QDomDocument dom = *m_pPart->projectDom( );
    DomUtil::writeBoolEntry( dom, "/autoprojectpart/classstore/enablepcs", isChecked );
}

void
ClassStoreOptionsWidget::slotEnablePreParsing( )
{
    if( !cbEnablePP->isChecked( ) )
	lvPCSPaths->setEnabled( false );
    else
	lvPCSPaths->setEnabled( true );
}

void
ClassStoreOptionsWidget::slotAddPreParsingPath( )
{
    QString newDir = KFileDialog::getExistingDirectory( QString::null, 0, i18n( "Select a directory to preparse" ) );
    //cerr << "slotAddPreParsingPath: selected directory: '" << newDir << "'" << endl;
    
    int answer = KMessageBox::questionYesNo( 0, i18n( "Include subdirectories ?" ) );

    if( answer == KMessageBox::Yes )
	( void ) new QListViewItem( lvPCSPaths, i18n( "Yes" ), newDir );
    else
	( void ) new QListViewItem( lvPCSPaths, i18n( "No"  ), newDir );
}

void
ClassStoreOptionsWidget::slotRemovePreParsingPath( )
{
    QString text = lvPCSPaths->selectedItem( )->text( 1 );

    int answer = KMessageBox::warningYesNo( 0, i18n( "Delete entry ?\n" ) + text );
                                            //QString::null, QString::null, QString::null, false );

    if( answer == KMessageBox::Yes )
	delete lvPCSPaths->selectedItem( );
}

#include "classstoreoptionswidget.moc"

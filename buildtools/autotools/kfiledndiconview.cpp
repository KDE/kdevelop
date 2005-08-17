/***************************************************************************
*                        kfiledndiconview.cpp  -  description
*                           -------------------
*  begin                : Wed Nov 1 2000
*  copyright            : (C) 2000 by Björn Sahlström
*  email                : kbjorn@users.sourceforge.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

//////////////////////////////////////////////////////
// Qt specific includes
#include <q3ptrlist.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QPixmap>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
//////////////////////////////////////////////////////
// KDE specific includes
#include <kfileitem.h>
#include <kiconloader.h>
#include <kdebug.h>
//////////////////////////////////////////////////////
// Application specific includes
#include "kfiledndiconview.h"

#ifndef AUTO_OPEN_TIME
#define AUTO_OPEN_TIME
	static int autoOpenTime = 750;
#endif
//-----------------------------------------------
KFileDnDIconView::KFileDnDIconView( QWidget *parent, const char *name )
	: KFileIconView(parent,name), m_autoOpenTimer( this ),
		m_autoOpenTime( autoOpenTime ), m_useAutoOpenTimer( true ),
		m_dropItem(0), m_dndEnabled( true )
{
	setDnDEnabled( true );
	setAutoUpdate( true );
	useAutoOpenTimer( true );
}
//-----------------------------------------------
KFileDnDIconView::~KFileDnDIconView(){
}
//-----------------------------------------------
void KFileDnDIconView::readConfig( KConfig* config, const QString& group ) {
	KConfigGroupSaver cs( config, group );
	bool dnd = config->readBoolEntry("EnableDND", true );
	setDnDEnabled( dnd );
	KFileIconView::readConfig( config, group );
}
//-----------------------------------------------
void KFileDnDIconView::writeConfig( KConfig* config, const QString& group ) {
	KConfigGroupSaver cs( config, group );
	config->writeEntry("EnableDND", m_dndEnabled );
	KFileIconView::writeConfig( config, group );
}
//-----------------------------------------------
void KFileDnDIconView::slotOpenFolder(){
	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		if( !m_dropItem )
			return;
	}
	KFileItemListIterator it( * KFileView::items() );
	for( ; it.current() ;++it ){
		if( (*it)->name() == m_dropItem->text() ) {
			if( (*it)->isFile() )
				return;
			else if( (*it)->isDir() || (*it)->isLink()) {
				sig->activate( (*it) );
				return;
			}
		}
	}
}
//-----------------------------------------------
void KFileDnDIconView::contentsDragEnterEvent( QDragEnterEvent *e ) {
	if ( ! acceptDrag( e ) ) { // can we decode this ?
		e->accept( false );            // No
		return;
	}
	e->acceptAction();     // Yes
	Q3IconViewItem *i = findItem( contentsToViewport( e->pos() ) );
	if ( i && m_useAutoOpenTimer) {  // are we over an item ?
		m_dropItem = i;  // set new m_dropItem
		m_autoOpenTimer.start( m_autoOpenTime ); // restart timer
	}
}
//-----------------------------------------------
void KFileDnDIconView::contentsDragMoveEvent( QDragMoveEvent *e ) {
	if ( ! acceptDrag( e ) ) { // can we decode this ?
		e->accept( false ); // No
		return;
	}
	e->acceptAction();     // Yes
	Q3IconViewItem *i = findItem( contentsToViewport( e->pos() ) );
	if( ! m_useAutoOpenTimer )
		return;
	if ( i ) {  // are we over an item ?
		if ( i != m_dropItem ) { // if so, is it a new one ?
			m_autoOpenTimer.stop(); // stop timer
			m_dropItem = i;      // set new m_dropItem
			m_autoOpenTimer.start( m_autoOpenTime ); // restart timer
		}
	}
	else
		m_autoOpenTimer.stop(); // stop timer
}
//-----------------------------------------------
void KFileDnDIconView::contentsDragLeaveEvent( QDragLeaveEvent* ) {
	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		m_dropItem = 0L;
	}
}
//-----------------------------------------------
void KFileDnDIconView::contentsDropEvent( QDropEvent* e ) {
	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		m_dropItem = 0L;
	}
	if( ! acceptDrag( e ) ) {
		e->acceptAction( false );
		return;
	}
	e->acceptAction();				
	// the drop was accepted so lets emit this
	KURL::List urls;
	KURLDrag::decode( e, urls );
	emit dropped( e );
}
//-----------------------------------------------
void KFileDnDIconView::startDrag(){
	if ( ! currentItem() ) // is there any selected items ?
		return;              // nope
	dragObject()->dragCopy();   // start the drag
}
//-----------------------------------------------
Q3DragObject* KFileDnDIconView::dragObject() {
	// create a list of the URL:s that we want to drag
	KURL::List urls;
	KFileItemListIterator it( * KFileView::selectedItems() );
	for ( ; it.current(); ++it ){
		urls.append( (*it)->url() );
	}
	QPixmap pixmap;
	if( urls.count() > 1 )
		pixmap = DesktopIcon( "kmultiple", iconSize() );
	if( pixmap.isNull() )
		pixmap = currentFileItem()->pixmap( iconSize() );	
	QPoint hotspot;
	hotspot.setX( pixmap.width() / 2 );
	hotspot.setY( pixmap.height() / 2 );
	Q3DragObject* myDragObject = KURLDrag::newDrag( urls, widget() );
	myDragObject->setPixmap( pixmap, hotspot );
	return myDragObject;
}
//-----------------------------------------------
void KFileDnDIconView::setAutoOpenTime( const int& time ){
	m_autoOpenTime = time;
	useAutoOpenTimer();
}
//-----------------------------------------------
void KFileDnDIconView::useAutoOpenTimer( bool use ){
	m_useAutoOpenTimer = use;
	if ( use )
		connect( &m_autoOpenTimer, SIGNAL( timeout() ),this, SLOT( slotOpenFolder() ) );
	else {
		disconnect( &m_autoOpenTimer, SIGNAL( timeout() ),this, SLOT( slotOpenFolder() ) );
		m_dropItem = 0L;
		m_autoOpenTimer.stop();
	}
}
//-----------------------------------------------
void KFileDnDIconView::setDnDEnabled( bool useDnD ){
	m_dndEnabled = useDnD;
	setAcceptDrops( useDnD );
	viewport()->setAcceptDrops( useDnD );
}
//-----------------------------------------------
bool KFileDnDIconView::acceptDrag(QDropEvent* e ) const {
	return KURLDrag::canDecode( e ) &&
				( e->action() == QDropEvent::Copy
				|| e->action() == QDropEvent::Move
				|| e->action() == QDropEvent::Link );
}
//-----------------------------------------------
#ifndef NO_INCLUDE_MOCFILES
#include "kfiledndiconview.moc"
#endif

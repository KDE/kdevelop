/***************************************************************************
*                        kfilednddetailview.cpp  -  description
*                           -------------------
*  begin                : Wed Nov 1 2000
*  copyright            : (C) 2000 by Bj�n Sahlstr�
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
#include <qptrlist.h>
#include <qapplication.h>
//////////////////////////////////////////////////////
// KDE specific includes
#include <kfileitem.h>
#include <kiconloader.h>
#include <kdebug.h>
//////////////////////////////////////////////////////
// Application specific includes
#include "kfilednddetailview.h"

#ifndef AUTO_OPEN_TIME
#define AUTO_OPEN_TIME
	static int autoOpenTime = 750;
#endif
//-----------------------------------------------
KFileDnDDetailView::KFileDnDDetailView(QWidget *parent, const char *name )
	:	KFileDetailView(parent,name), m_autoOpenTimer( this ),
		m_autoOpenTime( autoOpenTime ), m_useAutoOpenTimer( true ),
		m_dropItem(0), m_dndEnabled( true )
{
	setAutoUpdate( true );
	setDnDEnabled( true );
	useAutoOpenTimer( true );
}
//-----------------------------------------------
KFileDnDDetailView::~KFileDnDDetailView(){
}
//-----------------------------------------------
void KFileDnDDetailView::readConfig( KConfig* config, const QString& group ) {
	KConfigGroupSaver cs( config, group );
	bool dnd = config->readBoolEntry("DragAndDrop", true );
	setDnDEnabled( dnd );
	KFileDetailView::readConfig( config, group );
}
//-----------------------------------------------
void KFileDnDDetailView::writeConfig( KConfig* config, const QString& group ) {
	KConfigGroupSaver cs( config, group );
	config->writeEntry("DragAndDrop", m_dndEnabled );
	KFileDetailView::writeConfig( config, group );
}
//-----------------------------------------------
void KFileDnDDetailView::slotOpenFolder(){
	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		if( !m_dropItem )
			return;
	}
	KFileItemListIterator it( * KFileView::items() );
	for( ; it.current() ;++it ){
		if( (*it)->name() == m_dropItem->text(0) ) {
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
void KFileDnDDetailView::contentsDragEnterEvent( QDragEnterEvent *e ) {

    kdDebug (9020) << "KFileDnDDetailView::contentsDragEnterEvent" << endl;

	if ( ! acceptDrag( e ) ) {
		e->accept( false );
		return;
	}
	e->acceptAction();
	QListViewItem *i = itemAt( contentsToViewport( e->pos() ) );
	if ( i && m_useAutoOpenTimer ) {
		m_dropItem = i;
		m_autoOpenTimer.start( m_autoOpenTime );
	}
}
//-----------------------------------------------
void KFileDnDDetailView::contentsDragMoveEvent( QDragMoveEvent *e ) {

    kdDebug (9020) << "KFileDnDDetailView::contentsDragMoveEvent" << endl;

	if ( ! acceptDrag( e ) ) {
		e->accept( false );
		return;
	}
	e->acceptAction();
	QListViewItem *i = itemAt( contentsToViewport(  e->pos() ) );
	if( ! m_useAutoOpenTimer )
		return;
	if ( i ) {
		if ( i != m_dropItem ) {
			m_autoOpenTimer.stop();
			m_dropItem = i;
			m_autoOpenTimer.start( m_autoOpenTime );
		}
	}
	else
		m_autoOpenTimer.stop();
}
//-----------------------------------------------
void KFileDnDDetailView::contentsDragLeaveEvent( QDragLeaveEvent* ) {

    kdDebug (9020) << "KFileDnDDetailView::contentsDragLeaveEvent" << endl;

	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		m_dropItem = 0L;
	}
}
//-----------------------------------------------
void KFileDnDDetailView::contentsDropEvent( QDropEvent* e ) {

    kdDebug (9020) << "KFileDndDetailView::contentsDropEvent" << endl;

	if( m_useAutoOpenTimer ) {
		m_autoOpenTimer.stop();
		m_dropItem = 0L;
	}
	if( ! acceptDrag( e ) ) {
		e->acceptAction( false );
		return;
	}
	e->acceptAction();				
	// the drop was accepted so lets emit this to the outside world
	KURL::List urls;
	KURLDrag::decode( e, urls );
	emit dropped( e );
	emit dropped( this, e );
	emit dropped( this, urls );
}
//-----------------------------------------------
void KFileDnDDetailView::startDrag(){

    kdDebug (9020) << "KFileDnDDetailView::startDrag()" << endl;

	// create a list of the URL:s that we want to drag
	KURL::List urls;
	KFileItemListIterator it( * KFileView::selectedItems() );
	for ( ; it.current(); ++it ){
		urls.append( (*it)->url() );
	}
	QPixmap pixmap;
	if( urls.count() > 1 ){
		pixmap = DesktopIcon( "kmultiple", 16 );
	}
	if( pixmap.isNull() )
		pixmap = currentFileItem()->pixmap( 16 );	
	QPoint hotspot;
	hotspot.setX( pixmap.width() / 2 );
	hotspot.setY( pixmap.height() / 2 );
	m_dragObject = KURLDrag::newDrag( urls, widget() );
	m_dragObject->setPixmap( pixmap, hotspot );
	m_dragObject->drag();   // start the drag
}
//-----------------------------------------------
QDragObject* KFileDnDDetailView::dragObject() const {
	return m_dragObject;
}
//-----------------------------------------------
bool KFileDnDDetailView::acceptDrag(QDropEvent* e ) const {
	return KURLDrag::canDecode( e ) &&
				( e->action() == QDropEvent::Copy
				|| e->action() == QDropEvent::Move
				|| e->action() == QDropEvent::Link );
}
//-----------------------------------------------
void KFileDnDDetailView::setAutoOpenTime( const int& time ){
	m_autoOpenTime = time;
	useAutoOpenTimer();
}
//-----------------------------------------------
void KFileDnDDetailView::useAutoOpenTimer( bool use ){
	m_useAutoOpenTimer = use;
	if( use )
		connect( &m_autoOpenTimer, SIGNAL( timeout() ),this, SLOT( slotOpenFolder() ) );
	else {
		disconnect( &m_autoOpenTimer, SIGNAL( timeout() ),this, SLOT( slotOpenFolder() ) );
		m_dropItem = 0L;
		m_autoOpenTimer.stop();
	}
}
//-----------------------------------------------
void KFileDnDDetailView::setDnDEnabled( bool useDnD ){
	m_dndEnabled = useDnD;
	setDragEnabled( useDnD );
	setDropVisualizer( useDnD );
	setAcceptDrops( useDnD );
	viewport()->setAcceptDrops( useDnD );
}
//-----------------------------------------------
#ifndef NO_INCLUDE_MOCFILES
#include "kfilednddetailview.moc"
#endif

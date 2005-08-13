/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "connectionitems.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "metadatabase.h"
#include "widgetfactory.h"
#include "project.h"
#include <qpainter.h>
#include <qcombobox.h>
#include <qmetaobject.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qaction.h>
#include <qdatabrowser.h>

static const char* const ignore_slots[] = {
    "destroyed()",
    "setCaption(const QString&)",
    "setIcon(const QPixmap&)",
    "setIconText(const QString&)",
    "setMouseTracking(bool)",
    "clearFocus()",
    "setUpdatesEnabled(bool)",
    "update()",
    "update(int,int,int,int)",
    "update(const QRect&)",
    "repaint()",
    "repaint(bool)",
    "repaint(int,int,int,int,bool)",
    "repaint(const QRect&,bool)",
    "repaint(const QRegion&,bool)",
    "show()",
    "hide()",
    "iconify()",
    "showMinimized()",
    "showMaximized()",
    "showFullScreen()",
    "showNormal()",
    "polish()",
    "constPolish()",
    "raise()",
    "lower()",
    "stackUnder(QWidget*)",
    "move(int,int)",
    "move(const QPoint&)",
    "resize(int,int)",
    "resize(const QSize&)",
    "setGeometry(int,int,int,int)",
    "setGeometry(const QRect&)",
    "focusProxyDestroyed()",
    "showExtension(bool)",
    "setUpLayout()",
    "showDockMenu(const QPoint&)",
    "init()",
    "destroy()",
    "deleteLater()",
    0
};

ConnectionItem::ConnectionItem( QTable *table, FormWindow *fw )
    : QComboTableItem( table, QStringList(), FALSE ), formWindow( fw ), conn( 0 )
{
    setReplaceable( FALSE );
}

void ConnectionItem::senderChanged( QObject * )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::receiverChanged( QObject * )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::signalChanged( const QString & )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::slotChanged( const QString & )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::setSender( SenderItem *i )
{
    connect( i, SIGNAL( currentSenderChanged( QObject * ) ),
	     this, SLOT( senderChanged( QObject * ) ) );
}

void ConnectionItem::setReceiver( ReceiverItem *i )
{
    connect( i, SIGNAL( currentReceiverChanged( QObject * ) ),
	     this, SLOT( receiverChanged( QObject * ) ) );
}

void ConnectionItem::setSignal( SignalItem *i )
{
    connect( i, SIGNAL( currentSignalChanged( const QString & ) ),
	     this, SLOT( signalChanged( const QString & ) ) );
}

void ConnectionItem::setSlot( SlotItem *i )
{
    connect( i, SIGNAL( currentSlotChanged( const QString & ) ),
	     this, SLOT( slotChanged( const QString & ) ) );
}

void ConnectionItem::paint( QPainter *p, const QColorGroup &cg,
			    const QRect &cr, bool selected )
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
		 selected ? cg.brush( QColorGroup::Highlight )
			  : cg.brush( QColorGroup::Base ) );

    int w = cr.width();
    int h = cr.height();

    int x = 0;

    if ( currentText()[0] == '<' )
	p->setPen( QObject::red );
    else if ( selected )
	p->setPen( cg.highlightedText() );
    else
	p->setPen( cg.text() );

    QFont f( p->font() );
    QFont oldf( p->font() );
    if ( conn && conn->isModified() ) {
	f.setBold( TRUE );
	p->setFont( f );
    }

    p->drawText( x + 2, 0, w - x - 4, h, alignment(), currentText() );
    p->setFont( oldf );
}

void ConnectionItem::setConnection( ConnectionContainer *c )
{
    conn = c;
}

// ------------------------------------------------------------------

static void appendChildActions( QAction *action, QStringList &lst )
{
    QObjectListIt it( *action->children() );
    while ( it.current() ) {
	QObject *o = it.current();
	++it;
	if ( !::qt_cast<QAction*>(o) )
	    continue;
	lst << o->name();
	if ( o->children() && ::qt_cast<QActionGroup*>(o) )
	    appendChildActions( (QAction*)o, lst );
    }
}

static QStringList flatActions( const QPtrList<QAction> &l )
{
    QStringList lst;

    QPtrListIterator<QAction> it( l );
    while ( it.current() ) {
	QAction *action = it.current();
	lst << action->name();
	if ( action->children() && ::qt_cast<QActionGroup*>(action) )
	    appendChildActions( action, lst );
	++it;
    }

    return lst;
}

// ------------------------------------------------------------------

SenderItem::SenderItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QPtrDictIterator<QWidget> it( *formWindow->widgets() );
    while ( it.current() ) {
	if ( lst.find( it.current()->name() ) != lst.end() ) {
	    ++it;
	    continue;
	}
	if ( !QString( it.current()->name() ).startsWith( "qt_dead_widget_" ) &&
	     !::qt_cast<QLayoutWidget*>(it.current()) &&
	     !::qt_cast<Spacer*>(it.current()) &&
	     !::qt_cast<SizeHandle*>(it.current()) &&
	     qstrcmp( it.current()->name(), "central widget" ) != 0 ) {
	    lst << it.current()->name();
	}
	++it;
    }

    lst += flatActions( formWindow->actionList() );

    lst.prepend( "<No Sender>" );
    lst.sort();
    setStringList( lst );
}

QWidget *SenderItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SLOT( senderChanged( const QString & ) ) );
    return cb;
}

void SenderItem::setSenderEx( QObject *sender )
{
    setCurrentItem( sender->name() );
    emit currentSenderChanged( sender );
}

void SenderItem::senderChanged( const QString &sender )
{
    QObject *o = formWindow->child( sender, "QObject" );
    if ( !o )
	o = formWindow->findAction( sender );
    if ( !o )
	return;
    emit currentSenderChanged( o );
}



// ------------------------------------------------------------------

ReceiverItem::ReceiverItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QPtrDictIterator<QWidget> it( *formWindow->widgets() );
    while ( it.current() ) {
	if ( lst.find( it.current()->name() ) != lst.end() ) {
	    ++it;
	    continue;
	}
	if ( !QString( it.current()->name() ).startsWith( "qt_dead_widget_" ) &&
	     !::qt_cast<QLayoutWidget*>(it.current()) &&
	     !::qt_cast<Spacer*>(it.current()) &&
	     !::qt_cast<SizeHandle*>(it.current()) &&
	     qstrcmp( it.current()->name(), "central widget" ) != 0 ) {
	    lst << it.current()->name();
	}
	++it;
    }

    lst += flatActions( formWindow->actionList() );

    lst.prepend( "<No Receiver>" );
    lst.sort();
    setStringList( lst );
}

QWidget *ReceiverItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SLOT( receiverChanged( const QString & ) ) );
    return cb;
}

void ReceiverItem::setReceiverEx( QObject *receiver )
{
    setCurrentItem( receiver->name() );
    emit currentReceiverChanged( receiver );
}

void ReceiverItem::receiverChanged( const QString &receiver )
{
    QObject *o = formWindow->child( receiver, "QObject" );
    if ( !o )
	o = formWindow->findAction( receiver );
    if ( !o )
	return;
    emit currentReceiverChanged( o );
}



// ------------------------------------------------------------------

SignalItem::SignalItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;
    lst << "<No Signal>";
    lst.sort();
    setStringList( lst );
}

void SignalItem::senderChanged( QObject *sender )
{
    QStrList sigs = sender->metaObject()->signalNames( TRUE );
    sigs.remove( "destroyed()" );
    sigs.remove( "destroyed(QObject*)" );
    sigs.remove( "accessibilityChanged(int)" );
    sigs.remove( "accessibilityChanged(int,int)" );

    QStringList lst = QStringList::fromStrList( sigs );

    if ( ::qt_cast<CustomWidget*>(sender) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)sender )->customWidget();
	for ( QValueList<QCString>::Iterator it = w->lstSignals.begin();
	      it != w->lstSignals.end(); ++it )
	    lst << MetaDataBase::normalizeFunction( *it );
    }

    if ( sender == formWindow->mainContainer() ) {
	QStringList extra = MetaDataBase::signalList( formWindow );
	if ( !extra.isEmpty() )
	    lst += extra;
    }

    lst.prepend( "<No Signal>" );

    lst.sort();
    setStringList( lst );

    ConnectionItem::senderChanged( sender );
}

QWidget *SignalItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SIGNAL( currentSignalChanged( const QString & ) ) );
    return cb;
}

// ------------------------------------------------------------------

SlotItem::SlotItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;
    lst << "<No Slot>";
    lst.sort();
    setStringList( lst );

    lastReceiver = 0;
    lastSignal = "<No Signal>";
}

void SlotItem::receiverChanged( QObject *receiver )
{
    lastReceiver = receiver;
    updateSlotList();
    ConnectionItem::receiverChanged( receiver );
}

void SlotItem::signalChanged( const QString &signal )
{
    lastSignal = signal;
    updateSlotList();
    ConnectionItem::signalChanged( signal );
}

bool SlotItem::ignoreSlot( const char* slot ) const
{
#ifndef QT_NO_SQL
    if ( qstrcmp( slot, "update()" ) == 0 &&
	 ::qt_cast<QDataBrowser*>(lastReceiver) )
	return FALSE;
#endif

    for ( int i = 0; ignore_slots[i]; i++ ) {
	if ( qstrcmp( slot, ignore_slots[i] ) == 0 )
	    return TRUE;
    }

    if ( !formWindow->isMainContainer( (QWidget*)lastReceiver ) ) {
	if ( qstrcmp( slot, "close()" ) == 0  )
	    return TRUE;
    }

    if ( qstrcmp( slot, "setFocus()" ) == 0  )
	if ( lastReceiver->isWidgetType() &&
	     ( (QWidget*)lastReceiver )->focusPolicy() == QWidget::NoFocus )
	    return TRUE;

    return FALSE;
}

void SlotItem::updateSlotList()
{
    QStringList lst;
    if ( !lastReceiver || lastSignal == "<No Signal>" ) {
	lst << "<No Slot>";
	lst.sort();
	setStringList( lst );
	return;
    }

    QString signal = MetaDataBase::normalizeFunction( lastSignal );
    int n = lastReceiver->metaObject()->numSlots( TRUE );
    QStringList slts;

    for( int i = 0; i < n; ++i ) {
	// accept only public slots. For the form window, also accept protected slots
	const QMetaData* md = lastReceiver->metaObject()->slot( i, TRUE );
	if ( ( (lastReceiver->metaObject()->slot( i, TRUE )->access == QMetaData::Public) ||
	       (formWindow->isMainContainer( (QWidget*)lastReceiver ) &&
		lastReceiver->metaObject()->slot(i, TRUE)->access ==
		QMetaData::Protected) ) &&
	     !ignoreSlot( md->name ) &&
	     checkConnectArgs( signal.latin1(), lastReceiver, md->name ) )
	    if ( lst.find( md->name ) == lst.end() )
		lst << MetaDataBase::normalizeFunction( md->name );
    }

    LanguageInterface *iface =
	MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !iface || iface->supports( LanguageInterface::ConnectionsToCustomSlots ) ) {
	if ( formWindow->isMainContainer( (QWidget*)lastReceiver ) ) {
	    QValueList<MetaDataBase::Function> moreSlots = MetaDataBase::slotList( formWindow );
	    if ( !moreSlots.isEmpty() ) {
		for ( QValueList<MetaDataBase::Function>::Iterator it = moreSlots.begin();
		      it != moreSlots.end(); ++it ) {
		    QCString s = (*it).function;
		    if ( !s.data() )
			continue;
		    s = MetaDataBase::normalizeFunction( s );
		    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
			if ( lst.find( (*it).function ) == lst.end() )
			    lst << s;
		    }
		}
	    }
	}
    }

    if ( ::qt_cast<CustomWidget*>(lastReceiver) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)lastReceiver )->customWidget();
	for ( QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.begin();
	      it != w->lstSlots.end(); ++it ) {
	    QCString s = (*it).function;
	    if ( !s.data() )
		continue;
	    s = MetaDataBase::normalizeFunction( s );
	    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
		if ( lst.find( (*it).function ) == lst.end() )
		    lst << s;
	    }
	}
    }

    lst.prepend( "<No Slot>" );
    lst.sort();
    setStringList( lst );
}

QWidget *SlotItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SIGNAL( currentSlotChanged( const QString & ) ) );
    return cb;
}

void SlotItem::customSlotsChanged()
{
    QString currSlot = currentText();
    updateSlotList();
    setCurrentItem( "<No Slot>" );
    setCurrentItem( currSlot );
    emit currentSlotChanged( currentText() );
}

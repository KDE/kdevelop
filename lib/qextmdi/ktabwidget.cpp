/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kiconloader.h>

#include "ktabwidget.h"
#include "ktabbar.h"

KTabWidget::KTabWidget( QWidget *parent, const char *name, WFlags f )
    : QTabWidget( parent, name, f )
{
    setTabBar( new KTabBar(this, "tabbar") );
    setAcceptDrops( true );

    connect(tabBar(), SIGNAL(contextMenu( int, const QPoint & )), SLOT(contextMenu( int, const QPoint & )));
    connect(tabBar(), SIGNAL(mouseDoubleClick( int )), SLOT(mouseDoubleClick( int )));
    connect(tabBar(), SIGNAL(mouseMiddleClick( int )), SLOT(mouseMiddleClick( int )));
    connect(tabBar(), SIGNAL(initiateDrag( int )), SLOT(initiateDrag( int )));
    connect(tabBar(), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )));
    connect(tabBar(), SIGNAL(receivedDropEvent( int, QDropEvent * )), SLOT(receivedDropEvent( int, QDropEvent * )));
    connect(tabBar(), SIGNAL(moveTab( int, int )), SLOT(moveTab( int, int )));
    connect(tabBar(), SIGNAL(closeRequest( int )), SLOT(closeRequest( int )));
}

KTabWidget::~KTabWidget()
{
    //for the futur.
    //delete d;
}

void KTabWidget::setTabColor( QWidget *w, const QColor& color )
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        static_cast<KTabBar*>(tabBar())->setTabColor( t->identifier(), color );
    }
}

QColor KTabWidget::tabColor( QWidget *w ) const
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        return static_cast<KTabBar*>(tabBar())->tabColor( t->identifier() );
    } else {
        return QColor();
    }
}

void KTabWidget::setTabReorderingEnabled( bool on)
{
    static_cast<KTabBar*>(tabBar())->setTabReorderingEnabled( on );
}

bool KTabWidget::isTabReorderingEnabled() const
{
    return static_cast<KTabBar*>(tabBar())->isTabReorderingEnabled();
}

void KTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        e->accept( accept );
        return;
    }
    e->accept( false );
    QTabWidget::dragMoveEvent( e );
}

void KTabWidget::dropEvent( QDropEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        emit ( receivedDropEvent( e ) );
        return;
    }
    QTabWidget::dropEvent( e );
}

void KTabWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( contextMenu( mapToGlobal( e->pos() ) ) );
            return;
        }
    } else if ( e->button() == MidButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( mouseMiddleClick() );
            return;
        }
    }
    QTabWidget::mousePressEvent( e );
}

void KTabWidget::receivedDropEvent( int index, QDropEvent *e )
{
    emit( receivedDropEvent( page( index ), e ) );
}

void KTabWidget::initiateDrag( int index )
{
    emit( initiateDrag( page( index ) ) );
}

void KTabWidget::contextMenu( int index, const QPoint &p )
{
    emit( contextMenu( page( index ), p ) );
}

void KTabWidget::mouseDoubleClick( int index )
{
    emit( mouseDoubleClick( page( index ) ) );
}

void KTabWidget::mouseMiddleClick( int index )
{
    emit( mouseMiddleClick( page( index ) ) );
}

void KTabWidget::moveTab( int from, int to )
{
    QString tablabel = label( from );
    QWidget *w = page( from );
    QColor color = tabColor( w );
    QIconSet tabiconset = tabIconSet( w );
    QString tabtooltip = tabToolTip( w );
    bool current = ( w == currentPage() );
    bool enabled = isTabEnabled( w );
    blockSignals(true);
    removePage( w );

    insertTab( w, tablabel, to );
    w = page( to );
    changeTab( w, tabiconset, tablabel );
    setTabToolTip( w, tabtooltip );
    setTabColor( w, color );
    if ( current )
        showPage( w );
    setTabEnabled( w, enabled );
    blockSignals(false);

    emit ( movedTab( from, to ) );
}

bool KTabWidget::isEmptyTabbarSpace( const QPoint &p ) const
{
    QPoint point( p );
    QSize size( tabBar()->sizeHint() );
    if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
#if QT_VERSION >= 0x030200
	// QTabWidget::cornerWidget isn't const even it doesn't write any data ;(
	KTabWidget *that = const_cast<KTabWidget*>(this);
        if ( that->cornerWidget( TopLeft ) )
            point.setX( point.x()-size.height() );
#endif
	if ( tabPosition()==Bottom )
            point.setY( point.y()-( height()-size.height() ) );
        QTab *tab = tabBar()->selectTab( point);
        if( tab== 0L )
            return true;
    }
    return false;
}

void KTabWidget::setHoverCloseButton( bool button )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButton( button );
}

bool KTabWidget::hoverCloseButton() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButton();
}

void KTabWidget::setHoverCloseButtonDelayed( bool delayed )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButtonDelayed( delayed );
}

bool KTabWidget::hoverCloseButtonDelayed() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButtonDelayed();
}

void KTabWidget::closeRequest( int index )
{
    emit( closeRequest( page( index ) ) );
}

#include "ktabwidget.moc"

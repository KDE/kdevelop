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

#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

#include "ktabbar.h"
#include "ktabwidget.h"

KTabBar::KTabBar( QWidget *parent, const char *name )
    : QTabBar( parent, name ), mReorderStartTab( -1 ), mReorderPreviousTab( -1 ),
      mHoverCloseButtonTab( 0 ), mDragSwitchTab( 0 ), mHoverCloseButton( 0 ),
      mHoverCloseButtonEnabled( false ), mHoverCloseButtonDelayed( true ),
      mTabReorderingEnabled( false )
{
    setAcceptDrops( true );
    setMouseTracking( true );

    mEnableCloseButtonTimer = new QTimer( this );
    connect( mEnableCloseButtonTimer, SIGNAL( timeout() ), SLOT( enableCloseButton() ) );

    mActivateDragSwitchTabTimer = new QTimer( this );
    connect( mActivateDragSwitchTabTimer, SIGNAL( timeout() ), SLOT( activateDragSwitchTab() ) );

#if QT_VERSION >= 0x030200
    connect(this, SIGNAL(layoutChanged()), SLOT(onLayoutChange()));
#endif
}

KTabBar::~KTabBar()
{
    //For the future
    //delete d;
}

void KTabBar::setTabEnabled( int id, bool enabled )
{
    QTab * t = tab( id );
    if ( t ) {
        if ( t->isEnabled() != enabled ) {
            t->setEnabled( enabled );
            QRect r( t->rect() );
            if ( !enabled && id == currentTab() && count()>1 ) {
                int index = indexOf( id );
                index += ( index+1 == count() ) ? -1 : 1;
                t = tabAt( index );

                if ( t->isEnabled() ) {
                    r = r.unite( t->rect() );
                    QPtrList<QTab> *tablist = tabList();
                    tablist->append( tablist->take( tablist->findRef( t ) ) );
                    emit selected( t->identifier() );
                }
            }
            repaint( r );
        }
    }
}

void KTabBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        emit( mouseDoubleClick( indexOf( tab->identifier() ) ) );
        return;
    }
    QTabBar::mouseDoubleClickEvent( e );
}

void KTabBar::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == LeftButton ) {
        mEnableCloseButtonTimer->stop();
        mDragStart = e->pos();
    }
    else if( e->button() == RightButton ) {
        QTab *tab = selectTab( e->pos() );
        if( tab!= 0L ) {
            emit( contextMenu( indexOf( tab->identifier() ), mapToGlobal( e->pos() ) ) );
            return;
        }
    }
    QTabBar::mousePressEvent( e );
}

void KTabBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->state() == LeftButton ) {
        QTab *tab = selectTab( e->pos() );
        if ( mDragSwitchTab && tab != mDragSwitchTab ) {
          mActivateDragSwitchTabTimer->stop();
          mDragSwitchTab = 0;
        }

        int delay = KGlobalSettings::dndEventDelay();
        QPoint newPos = e->pos();
        if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
            newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
         {
            if( tab!= 0L ) {
                emit( initiateDrag( indexOf( tab->identifier() ) ) );
                return;
           }
       }
    }
    else if ( e->state() == MidButton ) {
        if (mReorderStartTab==-1) {
            int delay = KGlobalSettings::dndEventDelay();
            QPoint newPos = e->pos();
            if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
                newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
            {
                QTab *tab = selectTab( e->pos() );
                if( tab!= 0L && mTabReorderingEnabled ) {
                    mReorderStartTab = indexOf( tab->identifier() );
                    grabMouse( sizeAllCursor );
                    return;
                }
            }
        }
        else {
            QTab *tab = selectTab( e->pos() );
            if( tab!= 0L ) {
                int reorderStopTab = indexOf( tab->identifier() );
                if ( mReorderStartTab!=reorderStopTab && mReorderPreviousTab!=reorderStopTab ) {
                    emit( moveTab( mReorderStartTab, reorderStopTab ) );
                    mReorderPreviousTab=mReorderStartTab;
                    mReorderStartTab=reorderStopTab;
                    return;
                }
            }
        }
    }

    if ( mHoverCloseButtonEnabled && mReorderStartTab==-1) {
        QTab *t = selectTab( e->pos() );

        //BEGIN Workaround
        //Qt3.2.0 (and 3.2.1) emit wrong local coordinates
        //for MouseMove events when the pointer leaves a widget. Discard those
        //to avoid enabling the wrong hover button
#ifdef __GNUC__
#warning "Workaround for Qt 3.2.0, 3.2.1 bug"
#endif
        if ( e->globalPos() != mapToGlobal( e->pos() ) )
            return;
        //END Workaround

        if( t && t->iconSet() && t->isEnabled() ) {
            QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal );
            QRect rect( 0, 0, pixmap.width() + 4, pixmap.height() +4);

            int xoff = 0, yoff = 0;
            // The additional offsets were found by try and error, TODO: find the rational behind them
            if ( t == tab( currentTab() ) ) {
#if QT_VERSION >= 0x030200
                xoff = style().pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, this ) + 3;
                yoff = style().pixelMetric( QStyle::PM_TabBarTabShiftVertical, this ) - 4;
#else
                xoff = 3;
                yoff = -4;
#endif
            }
            else {
                xoff = 7;
                yoff = 0;
            }
            rect.moveLeft( t->rect().left() + 2 + xoff );
            rect.moveTop( t->rect().center().y()-pixmap.height()/2 + yoff );
            if ( rect.contains( e->pos() ) ) {
                if ( mHoverCloseButton ) {
                    if ( mHoverCloseButtonTab == t )
                        return;
                    mEnableCloseButtonTimer->stop();
                    delete mHoverCloseButton;
                }

                mHoverCloseButton = new QPushButton( this );
                mHoverCloseButton->setIconSet( KGlobal::iconLoader()->loadIcon("fileclose", KIcon::Toolbar, KIcon::SizeSmall, KIcon::ActiveState) );
                mHoverCloseButton->setGeometry( rect );
                QToolTip::add(mHoverCloseButton,i18n("Close this tab"));
                mHoverCloseButton->setFlat(true);
                mHoverCloseButton->show();
                if ( mHoverCloseButtonDelayed ) {
                  mHoverCloseButton->setEnabled(false);
                  mEnableCloseButtonTimer->start( QApplication::doubleClickInterval(), true );
                }
                mHoverCloseButtonTab = t;
                connect( mHoverCloseButton, SIGNAL( clicked() ), SLOT( closeButtonClicked() ) );
                return;
            }
        }
        if ( mHoverCloseButton ) {
            mEnableCloseButtonTimer->stop();
            delete mHoverCloseButton;
            mHoverCloseButton = 0;
        }
    }

    QTabBar::mouseMoveEvent( e );
}

void KTabBar::enableCloseButton()
{
    mHoverCloseButton->setEnabled(true);
}

void KTabBar::activateDragSwitchTab()
{
    setCurrentTab( mDragSwitchTab );
    mDragSwitchTab = 0;
}

void KTabBar::mouseReleaseEvent( QMouseEvent *e )
{
    if( e->button() == MidButton ) {
        QTab *tab = selectTab( e->pos() );
        if ( mReorderStartTab==-1 ) {
            if( tab!= 0L ) {
                emit( mouseMiddleClick( indexOf( tab->identifier() ) ) );
                return;
            }
        }
        else {
            releaseMouse();
            setCursor( arrowCursor );
            mReorderStartTab=-1;
            mReorderPreviousTab=-1;
        }
    }
    QTabBar::mouseReleaseEvent( e );
}

void KTabBar::dragMoveEvent( QDragMoveEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        if ( accept && tab != QTabBar::tab( currentTab() ) ) {
          mDragSwitchTab = tab;
          mActivateDragSwitchTabTimer->start( QApplication::doubleClickInterval()*2, true );
        }
        e->accept( accept );
        return;
    }
    e->accept( false );
    QTabBar::dragMoveEvent( e );
}

void KTabBar::dropEvent( QDropEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        mActivateDragSwitchTabTimer->stop();
        mDragSwitchTab = 0;
        emit( receivedDropEvent( indexOf( tab->identifier() ) , e ) );
        return;
    }
    QTabBar::dropEvent( e );
}

void KTabBar::setTabColor( int id, const QColor& color )
{
    QTab *t = tab( id );
    if ( t ) {
        mTabColors.insert( id, color );
        repaint( t->rect(), false );
    }
}

const QColor &KTabBar::tabColor( int id  ) const
{
    if ( mTabColors.contains( id ) )
        return mTabColors[id];

    return colorGroup().foreground();
}

void KTabBar::removeTab( QTab *t )
{
    mTabColors.remove( t->identifier() );
    QTabBar::removeTab( t );
}

void KTabBar::paintLabel( QPainter *p, const QRect& br,
                          QTab *t, bool has_focus ) const
{
    QRect r = br;
    bool selected = currentTab() == t->identifier();
    if ( t->iconSet() ) {
        // the tab has an iconset, draw it in the right mode
        QIconSet::Mode mode = ( t->isEnabled() && isEnabled() )
                                 ? QIconSet::Normal : QIconSet::Disabled;
        if ( mode == QIconSet::Normal && has_focus )
            mode = QIconSet::Active;
        QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        r.setLeft( r.left() + pixw + 4 );
        r.setRight( r.right() + 2 );

        int inactiveXShift = 2;//style().pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, this );
        int inactiveYShift = 2;//style().pixelMetric( QStyle::PM_TabBarTabShiftVertical, this );

        int right = t->text().isEmpty() ? br.right() - pixw : br.left() + 2;

        p->drawPixmap( right + ((selected == true) ? 0 : inactiveXShift),
                       br.center().y() - pixh / 2 + ((selected == true) ? 0 : inactiveYShift),
                       pixmap );
    }

    QStyle::SFlags flags = QStyle::Style_Default;

    if ( isEnabled() && t->isEnabled() )
        flags |= QStyle::Style_Enabled;
    if ( has_focus )
        flags |= QStyle::Style_HasFocus;

    QColorGroup cg( colorGroup() );
    if ( mTabColors.contains( t->identifier() ) )
        cg.setColor( QColorGroup::Foreground, mTabColors[t->identifier()] );

    style().drawControl( QStyle::CE_TabBarLabel, p, this, r,
                             t->isEnabled() ? cg : palette().disabled(),
                             flags, QStyleOption(t) );
}

bool KTabBar::isTabReorderingEnabled() const
{
    return mTabReorderingEnabled;
}

void KTabBar::setTabReorderingEnabled( bool on )
{
    mTabReorderingEnabled = on;
}

void KTabBar::closeButtonClicked()
{
    emit closeRequest( indexOf( mHoverCloseButtonTab->identifier() ) );
#if QT_VERSION < 0x030200
    onLayoutChange();
#endif
}

void KTabBar::setHoverCloseButton( bool button )
{
    mHoverCloseButtonEnabled = button;
    if ( !button )
        onLayoutChange();
}

bool KTabBar::hoverCloseButton() const
{
    return mHoverCloseButtonEnabled;
}

void KTabBar::setHoverCloseButtonDelayed( bool delayed )
{
    mHoverCloseButtonDelayed = delayed;
}

bool KTabBar::hoverCloseButtonDelayed() const
{
    return mHoverCloseButtonDelayed;
}

void KTabBar::onLayoutChange()
{
    mEnableCloseButtonTimer->stop();
    delete mHoverCloseButton;
    mHoverCloseButton = 0;
    mHoverCloseButtonTab = 0;
    mActivateDragSwitchTabTimer->stop();
    mDragSwitchTab = 0;
}

#include "ktabbar.moc"

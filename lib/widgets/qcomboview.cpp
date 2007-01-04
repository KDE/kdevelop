/**********************************************************************
**
**
** Implementation of QComboView widget class
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
** Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
**********************************************************************/

#include "qcomboview.h"
#include <kdeversion.h>
#ifndef QT_NO_COMBOBOX
#include "qpopupmenu.h"
#include "qlistview.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qlineedit.h"
#include "qbitmap.h"
#include "private/qeffects_p.h"
#include "qstringlist.h"
#include "qcombobox.h"
#include "qstyle.h"
#include "qheader.h"
#include <limits.h>

class QComboViewData
{
public:
    QComboViewData( QComboView *cb ): current(0), lView( 0 ), combo( cb )
    {
        duplicatesEnabled = TRUE;
        cb->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    }

    inline QListView * listView() { return lView; }
    void updateLinedGeometry();

    void setListView( QListView *l ) { lView = l ;
        l->setMouseTracking( TRUE );}

    QListViewItem *current;
    int maxCount;
    int sizeLimit;
    QComboView::Policy p;
    bool autoresize;
    bool poppedUp;
    bool mouseWasInsidePopup;
    bool arrowPressed;
    bool arrowDown;
    bool discardNextMousePress;
    bool shortClick;
    bool useCompletion;
    bool completeNow;
    int completeAt;
    bool duplicatesEnabled;
    int fullHeight, currHeight;

    QLineEdit * ed;  // /bin/ed rules!
    QTimer *completionTimer;

    QSize sizeHint;

private:
    bool	usinglView;
    QListView   *lView;
    QComboView *combo;

};

void QComboViewData::updateLinedGeometry()
{
    if ( !ed || !combo )
        return;
    QRect r = QStyle::visualRect( combo->style().querySubControlMetrics(QStyle::CC_ComboBox, combo,
                                        QStyle::SC_ComboBoxEditField), combo );

//    qWarning("updateLinedGeometry(): currentItem is %d", combo->currentItem() == 0 ? 0 : 1);
    const QPixmap *pix = combo->currentItem() ? combo->currentItem()->pixmap(0) : 0;
    if ( pix && pix->width() < r.width() )
        r.setLeft( r.left() + pix->width() + 4 );
    if ( r != ed->geometry() )
        ed->setGeometry( r );
}

static inline bool checkInsertIndex( const char *method, const char * name,
				     int count, int *index)
{
    bool range_err = (*index > count);
#if defined(QT_CHECK_RANGE)
    if ( range_err )
	qWarning( "QComboView::%s: (%s) Index %d out of range",
		 method, name ? name : "<no name>", *index );
#else
    Q_UNUSED( method )
    Q_UNUSED( name )
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}


static inline bool checkIndex( const char *method, const char * name,
			       int count, int index )
{
    bool range_err = (index >= count);
#if defined(QT_CHECK_RANGE)
    if ( range_err )
	qWarning( "QComboView::%s: (%s) Index %i out of range",
		 method, name ? name : "<no name>", index );
#else
    Q_UNUSED( method )
    Q_UNUSED( name )
#endif
    return !range_err;
}


/*!
    Constructs a combobox with a maximum size and either Motif 2.0 or
    Windows look and feel.

    The input field can be edited if \a rw is TRUE, otherwise the user
    may only choose one of the items in the combobox.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/


QComboView::QComboView( bool rw, QWidget *parent, const char *name )
    : QWidget( parent, name, WResizeNoErase )
{
    d = new QComboViewData( this );
    setUpListView();

    d->current = 0;
    d->maxCount = INT_MAX;
    setSizeLimit(10);
    d->p = AtBottom;
    d->autoresize = FALSE;
    d->poppedUp = FALSE;
    d->arrowDown = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick = FALSE;
    d->useCompletion = FALSE;
    d->completeAt = 0;
    d->completeNow = FALSE;
    d->completionTimer = new QTimer( this );

    setFocusPolicy( StrongFocus );

    d->ed = 0;
    if ( rw )
	setUpLineEdit();
    setBackgroundMode( PaletteButton, PaletteBase );
}



/*!
    Destroys the combobox.
*/

QComboView::~QComboView()
{
    delete d;
}

void QComboView::setDuplicatesEnabled( bool enable )
{
   d->duplicatesEnabled = enable;
}

bool QComboView::duplicatesEnabled() const
{
    return d->duplicatesEnabled;
}

int QComboView::childCount() const
{
    return d->listView()->childCount();
}


/*!
    Removes all comboview items.
*/

void QComboView::clear()
{
    d->listView()->resize( 0, 0 );
    d->listView()->clear();

    d->current = 0;
    if ( d->ed ) {
        d->ed->setText( QString::fromLatin1("") );
        d->updateLinedGeometry();
    }
    currentChanged();
}

QListViewItem *QComboView::currentItem() const
{
    return d->current;
}

void QComboView::setCurrentItem( QListViewItem *item )
{
    if ( item == d->current && !d->ed ) {
        return;
    }

    if (!item)
    {
        d->current = 0;
        if ( d->ed ) {
//            d->ed->setText( "" );
            d->updateLinedGeometry();
        }
        return;
    }

    d->current = item;
    d->completeAt = 0;
    if ( d->ed ) {
        d->ed->setText( item->text(0) );
//        qWarning("setCurrentItem( %s )", item->text(0).latin1());
        d->updateLinedGeometry();
    }
    if ( d->listView() ) {
        d->listView()->setCurrentItem( item );
    } else {
        internalHighlight( item );
    // internalActivate( item ); ### this leads to weird behavior, as in 3.0.1
    }

    currentChanged();

    d->listView()->ensureItemVisible(item);
}

bool QComboView::autoResize() const
{
    return d->autoresize;
}

void QComboView::setAutoResize( bool enable )
{
    if ( (bool)d->autoresize != enable ) {
        d->autoresize = enable;
    if ( enable )
        adjustSize();
    }
}


/*!
    reimp

    This implementation caches the size hint to avoid resizing when
    the contents change dynamically. To invalidate the cached value
    call setFont().
*/
QSize QComboView::sizeHint() const
{
    if ( isVisible() && d->sizeHint.isValid() )
        return d->sizeHint;

    constPolish();
//    int i, w;
    QFontMetrics fm = fontMetrics();

    int maxW = childCount() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = QMAX( fm.lineSpacing(), 14 ) + 2;

/*    for( i = 0; i < count(); i++ ) {
        w = d->listView()->item( i )->width( d->listView() );
        if ( w > maxW )
            maxW = w;
    }
*/
    d->sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
        QSize(maxW, maxH)).expandedTo(QApplication::globalStrut()));

    return d->sizeHint;
}


/*!
  \internal
  Receives activated signals from an internal popup list and emits
  the activated() signal.
*/

void QComboView::internalActivate( QListViewItem * item )
{
    if (!item)
    {
        d->current = 0;
        if ( d->ed ) {
//            d->ed->setText( "" );
            d->updateLinedGeometry();
        }
        return;
    }
    popDownListView();
    d->poppedUp = FALSE;

    d->current = item;

    QString t( item->text(0) );
    if ( d->ed ) {
        d->ed->setText( t );
//        qWarning("internalActivate( %s )", item->text(0).latin1());
        d->updateLinedGeometry();
    }
    emit activated( item );
    emit activated( t );

//    item->setOpen(true);
}

/*!
  \internal
  Receives highlighted signals from an internal popup list and emits
  the highlighted() signal.
*/

void QComboView::internalHighlight( QListViewItem * item )
{
    if (!item)
    {
        d->current = 0;
        if ( d->ed ) {
//            d->ed->setText( "" );
            d->updateLinedGeometry();
        }
        return;
    }
    emit highlighted( item );
    QString t = item->text(0);
    if ( !t.isNull() )
        emit highlighted( t );
}

/*!
  \internal
  Receives timeouts after a click. Used to decide if a Motif style
  popup should stay up or not after a click.
*/
void QComboView::internalClickTimeout()
{
    d->shortClick = FALSE;
}

/*!
    Sets the palette for both the combobox button and the combobox
    popup list to \a palette.
*/

void QComboView::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    if( d ) {
        if(d->listView())
            d->listView()->setPalette( palette );
    }
}

/*!
    Sets the font for both the combobox button and the combobox popup
    list to \a font.
*/

void QComboView::setFont( const QFont &font )
{
    d->sizeHint = QSize();      // invalidate size hint
    QWidget::setFont( font );
    d->listView()->setFont( font );
    if (d->ed)
        d->ed->setFont( font );
    if ( d->autoresize )
        adjustSize();
}


/*!reimp
*/

void QComboView::resizeEvent( QResizeEvent * e )
{
    if ( d->ed )
        d->updateLinedGeometry();
    d->listView()->resize( width(), d->listView()->height() );
    QWidget::resizeEvent( e );
}

/*!reimp
*/

void QComboView::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    const QColorGroup & g = colorGroup();
    p.setPen(g.text());

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;

    if ( width() < 5 || height() < 5 ) {
        qDrawShadePanel( &p, rect(), g, FALSE, 2,
                &g.brush( QColorGroup::Button ) );
        return;
    }

//    bool reverse = QApplication::reverseLayout();
    style().drawComplexControl( QStyle::CC_ComboBox, &p, this, rect(), g,
                    flags, QStyle::SC_All,
                    (d->arrowDown ?
                    QStyle::SC_ComboBoxArrow :
                    QStyle::SC_None ));

    QRect re = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                        QStyle::SC_ComboBoxEditField );
    re = QStyle::visualRect(re, this);
    p.setClipRect( re );

    if ( !d->ed ) {
        QListViewItem * item = d->current;
        if ( item ) {
            // we calculate the QListBoxTexts height (ignoring strut)
            int itemh = d->listView()->fontMetrics().lineSpacing() + 2;
            p.translate( re.x(), re.y() + (re.height() - itemh)/2  );
            item->paintCell( &p, d->listView()->colorGroup(), 0, width(), AlignLeft | AlignVCenter );
        }
    } else if ( d->listView() && d->listView()->currentItem( ) && d->current ) {
        QListViewItem * item = d->current ;
        const QPixmap *pix = item->pixmap(0);
        if ( pix ) {
            p.fillRect( re.x(), re.y(), pix->width() + 4, re.height(),
                    colorGroup().brush( QColorGroup::Base ) );
            p.drawPixmap( re.x() + 2, re.y() +
                    ( re.height() - pix->height() ) / 2, *pix );
        }
    }
    p.setClipping( FALSE );
}


/*!reimp
*/

void QComboView::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
        return;
    if ( d->discardNextMousePress ) {
        d->discardNextMousePress = FALSE;
        return;
    }
    QRect arrowRect = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                                QStyle::SC_ComboBoxArrow);
    arrowRect = QStyle::visualRect(arrowRect, this);

    // Correction for motif style, where arrow is smaller
    // and thus has a rect that doesn't fit the button.
    arrowRect.setHeight( QMAX(  height() - (2 * arrowRect.y()), arrowRect.height() ) );

    if ( childCount() && ( !editable() || arrowRect.contains( e->pos() ) ) ) {
        d->arrowPressed = FALSE;
        listView()->blockSignals( TRUE );
        qApp->sendEvent( listView(), e ); // trigger the listbox's autoscroll
        listView()->blockSignals( FALSE );
        popup();
        if ( arrowRect.contains( e->pos() ) ) {
            d->arrowPressed = TRUE;
            d->arrowDown    = TRUE;
            repaint( FALSE );
        }
        QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
        d->shortClick = TRUE;
    }
}

/*!reimp
*/

void QComboView::mouseMoveEvent( QMouseEvent * )
{
}

/*!reimp
*/

void QComboView::mouseReleaseEvent( QMouseEvent * )
{
}

/*!reimp
*/

void QComboView::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}


/*!reimp
*/

void QComboView::keyPressEvent( QKeyEvent *e )
{
    QListViewItem *c = currentItem();
    if ( ( e->key() == Key_F4 && e->state() == 0 ) ||
        ( e->key() == Key_Down && (e->state() & AltButton) ) ||
        ( !d->ed && e->key() == Key_Space ) ) {
        if ( childCount() ) {
            popup();
        }
        return;
    } else if ( e->key() == Key_Up ) {
/*        if ((!c) && (listView()->firstChild()))
            setCurrentItem(listView()->firstChild());*/
        if (c && c->itemAbove() )
            setCurrentItem( c->itemAbove() );
        else
            return;
    } else if ( e->key() == Key_Down ) {
        if ((!c) && (listView()->firstChild()))
        {
            setCurrentItem(listView()->firstChild());
            return;
        }
        if ( c && c->itemBelow() )
            setCurrentItem( c->itemBelow() );
        else
            return;
    } else if ( e->key() == Key_Home && ( !d->ed || !d->ed->hasFocus() ) ) {
        if (listView()->firstChild())
            setCurrentItem( listView()->firstChild() );
        else
            return;
    } else if ( e->key() == Key_End && ( !d->ed || !d->ed->hasFocus() ) ) {
        if (listView()->lastItem())
            setCurrentItem( listView()->lastItem() );
        else
            return;
    } else if ( !d->ed && e->ascii() >= 32 && !e->text().isEmpty() ) {
        if ( !d->completionTimer->isActive() ) {
            d->completeAt = 0;
            c = completionIndex( e->text(), c->itemBelow() );
            if ( c ) {
                setCurrentItem( c );
                d->completeAt = e->text().length();
            }
            else
                return;
        } else {
            d->completionTimer->stop();
            QString ct = currentText().left( d->completeAt ) + e->text();
            c = completionIndex( ct, c );
            if ( c == 0 && d->completeAt > 0 ) {
                c = completionIndex( e->text(), listView()->firstChild() );
                ct = e->text();
            }
            d->completeAt = 0;
            if ( c ) {
                setCurrentItem( c );
                d->completeAt = ct.length();
            }
            else
                return;
        }
        d->completionTimer->start( 400, TRUE );
    } else {
        e->ignore();
        return;
    }

    c = currentItem();
    if ( childCount() && c && !c->text(0).isNull() )
        emit activated( c->text(0) );
    emit activated( c );
}

QString QComboView::currentText() const
{
    if ( d->ed )
        return d->ed->text();
    else if ( d->current )
        return currentItem()->text(0);
    else
        return QString::null;
}

/*!reimp
*/

void QComboView::focusInEvent( QFocusEvent * e )
{
    QWidget::focusInEvent( e );
    d->completeNow = FALSE;
    d->completeAt = 0;

    emit focusGranted();
}

/*!reimp
*/

void QComboView::focusOutEvent( QFocusEvent * e )
{
    QWidget::focusOutEvent( e );
    d->completeNow = FALSE;
    d->completeAt = 0;

    emit focusLost();
}

/*!reimp
*/

void QComboView::wheelEvent( QWheelEvent *e )
{
    if ( d->poppedUp ) {
        QApplication::sendEvent( d->listView(), e );
    } else {
        if ( e->delta() > 0 ) {
            QListViewItem *c = currentItem();
            if ( c && c->itemAbove() ) {
                setCurrentItem( c->itemAbove() );
                emit activated( currentItem() );
                emit activated( currentText() );
            }
        } else {
            QListViewItem *c = currentItem();
            if ( c && c->itemBelow() ) {
                setCurrentItem( c->itemBelow() );
                emit activated( currentItem() );
                emit activated( currentText() );
            }
        }
        e->accept();
    }
}

int childCount(QListViewItem *it)
{
    int count = 1;
    QListViewItem * myChild = it->firstChild();
    while( myChild ) {
        count += childCount(myChild);
        myChild = myChild->nextSibling();
    }
    return count;
}

int childCount(QListView *lv)
{
    int count = 0;
    QListViewItem * myChild = lv->firstChild();
    while( myChild ) {
        count += childCount(myChild);
//        count += 1;
        myChild = myChild->nextSibling();
    }
    return count;
}

/*!
  \internal
   Calculates the listbox height needed to contain all items, or as
   many as the list box is supposed to contain.
*/
static int listHeight( QListView *l, int /*sl*/ )
{
/*    if ( l->childCount() > 0 )
        return QMIN( l->childCount(), (uint)sl) * l->firstChild()->height();
    else*/

    int prefH = 0;
    int ch = childCount(l);
    ch = QMIN(ch, 10);
    if (l->firstChild())
    {
        prefH = ch * l->firstChild()->height();
    }
    else
        prefH = l->sizeHint().height();

    if (l->header()->isVisible())
        prefH += l->header()->sizeHint().height();

//    return prefH < l->sizeHint().height() ? prefH : l->sizeHint().height();

    return prefH+2;
}

/*!
    Pops up the combobox popup list.

    If the list is empty, no items appear.
*/

void QComboView::popup()
{
    if ( !childCount() )
        return;

    // Send all listbox events to eventFilter():
    QListView* lb = d->listView();
    lb->triggerUpdate( );
    lb->installEventFilter( this );
    lb->viewport()->installEventFilter( this );
    d->mouseWasInsidePopup = FALSE;
//    int w = lb->variableWidth() ? lb->sizeHint().width() : width();
    int w = width();
    int h = listHeight( lb, d->sizeLimit );
    QRect screen = QApplication::desktop()->availableGeometry( const_cast<QComboView*>(this) );

    int sx = screen.x();        // screen pos
    int sy = screen.y();
    int sw = screen.width();    // screen width
    int sh = screen.height();   // screen height
    QPoint pos = mapToGlobal( QPoint(0,height()) );
    // ## Similar code is in QPopupMenu
    int x = pos.x();
    int y = pos.y();

    // the complete widget must be visible
    if ( x + w > sx + sw )
        x = sx+sw - w;
    if ( x < sx )
        x = sx;
    if (y + h > sy+sh && y - h - height() >= 0 )
        y = y - h - height();
    QRect rect =
    style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                    QStyle::SC_ComboBoxListBoxPopup,
                    QStyleOption( x, y, w, h ) );
    if ( rect.isNull() )
        rect.setRect( x, y, w, h );
    lb->setGeometry( rect );

    lb->raise();
    bool block = lb->signalsBlocked();
    lb->blockSignals( TRUE );
    QListViewItem *currentLBItem = d->current ;
    lb->setCurrentItem( currentLBItem );
    // set the current item to also be the selected item if it isn't already
    if ( currentLBItem && currentLBItem->isSelectable() && !currentLBItem->isSelected() )
        lb->setSelected( currentLBItem, TRUE );
    lb->blockSignals( block );
    lb->setVScrollBarMode(QScrollView::Auto);

//#ifndef QT_NO_EFFECTS
/*    if ( QApplication::isEffectEnabled( UI_AnimateCombo ) ) {
        if ( lb->y() < mapToGlobal(QPoint(0,0)).y() )
        qScrollEffect( lb, QEffects::UpScroll );
        else
        qScrollEffect( lb );
    } else*/
//#endif
        lb->show();
    d->poppedUp = TRUE;
}


/*!
  reimp
*/
void QComboView::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    {
        QPainter p( &bm, this );
        style().drawComplexControlMask(QStyle::CC_ComboBox, &p, this, rect());
    }

    setMask( bm );
}

/*!
  \internal
  Pops down (removes) the combobox popup list box.
*/
void QComboView::popDownListView()
{
    d->listView()->removeEventFilter( this );
    d->listView()->viewport()->removeEventFilter( this );
    d->listView()->hide();
    d->listView()->setCurrentItem( d->current );
    if ( d->arrowDown ) {
        d->arrowDown = FALSE;
        repaint( FALSE );
    }
    d->poppedUp = FALSE;
}


/*!
  \internal
  Re-indexes the identifiers in the popup list.
*/

void QComboView::reIndex()
{
}

/*!
  \internal
  Repaints the combobox.
*/

void QComboView::currentChanged()
{
    if ( d->autoresize )
        adjustSize();
    update();
}

/*! reimp

  \internal

  The event filter steals events from the popup or listbox when they
  are popped up. It makes the popup stay up after a short click in
  motif style. In windows style it toggles the arrow button of the
  combobox field, and activates an item and takes down the listbox
  when the mouse button is released.
*/

bool QComboView::eventFilter( QObject *object, QEvent *event )
{
    if ( !event )
        return TRUE;
    else if ( object == d->ed ) {
        if ( event->type() == QEvent::KeyPress ) {
            bool isAccepted = ( (QKeyEvent*)event )->isAccepted();
            keyPressEvent( (QKeyEvent *)event );
            if ( ((QKeyEvent *)event)->isAccepted() ) {
                d->completeNow = FALSE;
                return TRUE;
            } else if ( ((QKeyEvent *)event)->key() != Key_End ) {
                d->completeNow = TRUE;
                d->completeAt = d->ed->cursorPosition();
            }
            if ( isAccepted )
                ( (QKeyEvent*)event )->accept();
            else
                ( (QKeyEvent*)event )->ignore();
        } else if ( event->type() == QEvent::KeyRelease ) {
            d->completeNow = FALSE;
            keyReleaseEvent( (QKeyEvent *)event );
            return ((QKeyEvent *)event)->isAccepted();
        } else if ( event->type() == QEvent::FocusIn ) {
            focusInEvent( (QFocusEvent *)event );
        } else if ( event->type() == QEvent::FocusOut ) {
            focusOutEvent( (QFocusEvent *)event );
        } else if ( d->useCompletion && d->completeNow ) {
            if ( !d->ed->text().isNull() &&
            d->ed->cursorPosition() > d->completeAt &&
            d->ed->cursorPosition() == (int)d->ed->text().length() ) {
                d->completeNow = FALSE;
                QString ct( d->ed->text() );
                QListViewItem *i = completionIndex( ct, currentItem() );
                if ( i ) {
                    QString it = i->text(0);
                    d->ed->validateAndSet( it, ct.length(),
                            ct.length(), it.length() );
                }
            }
        }
    } else if ( ( object == d->listView() ||
                        object == d->listView()->viewport() )) {
        QMouseEvent *e = (QMouseEvent*)event;
        switch( event->type() ) {
        case QEvent::MouseMove:
            if ( !d->mouseWasInsidePopup  ) {
//                qWarning("!d->mouseWasInsidePopup");
                QPoint pos = e->pos();
                if ( d->listView()->rect().contains( pos ) )
                    d->mouseWasInsidePopup = TRUE;
                // Check if arrow button should toggle
                if ( d->arrowPressed ) {
                    QPoint comboPos;
                    comboPos = mapFromGlobal( d->listView()->mapToGlobal(pos) );
                    QRect arrowRect =
                    style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                                    QStyle::SC_ComboBoxArrow);
                    arrowRect = QStyle::visualRect(arrowRect, this);
                    if ( arrowRect.contains( comboPos ) ) {
                        if ( !d->arrowDown  ) {
                            d->arrowDown = TRUE;
                            repaint( FALSE );
                        }
                        } else {
                        if ( d->arrowDown  ) {
                            d->arrowDown = FALSE;
                            repaint( FALSE );
                        }
                    }
                }
            } else if ((e->state() & ( RightButton | LeftButton | MidButton ) ) == 0 &&
                style().styleHint(QStyle::SH_ComboBox_ListMouseTracking, this)) {
//                qWarning("event filter:: emu");
                QWidget *mouseW = QApplication::widgetAt( e->globalPos(), TRUE );
//                if ( mouseW == d->listView()->viewport() ) { //###
                if ( mouseW == d->listView()->viewport() ) {
                    QListViewItem *sel = d->listView()->itemAt(e->pos());
                    if (sel)
                    {
                        d->listView()->setCurrentItem(sel);
                        d->listView()->setSelected(sel, true);
                    }
                    return TRUE;
                }
            }

            break;
        case QEvent::MouseButtonRelease:
            if ( d->listView()->rect().contains( e->pos() ) ) {
                QMouseEvent tmp( QEvent::MouseButtonDblClick,
                        e->pos(), e->button(), e->state() ) ;
                // will hide popup
                QApplication::sendEvent( object, &tmp );
                return TRUE;
            } else {
                if ( d->mouseWasInsidePopup ) {
                    popDownListView();
                } else {
                    d->arrowPressed = FALSE;
                    if ( d->arrowDown  ) {
                        d->arrowDown = FALSE;
                        repaint( FALSE );
                    }
                }
            }
            break;
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
            if ( !d->listView()->rect().contains( e->pos() ) ) {
                QPoint globalPos = d->listView()->mapToGlobal(e->pos());
                if ( QApplication::widgetAt( globalPos, TRUE ) == this ) {
                    d->discardNextMousePress = TRUE;
                    // avoid popping up again
                }
                popDownListView();
                return TRUE;
            }
            break;
        case QEvent::KeyPress:
            switch( ((QKeyEvent *)event)->key() ) {
                case Key_Up:
                case Key_Down:
                    if ( !(((QKeyEvent *)event)->state() & AltButton) )
                        break;
                case Key_F4:
                case Key_Escape:
                    if ( d->poppedUp ) {
                        popDownListView();
                        return TRUE;
                    }
                break;
                case Key_Enter:
                case Key_Return:
                    // work around QDialog's enter handling
                    return FALSE;
                default:
                break;
            }
            break;
        case QEvent::Hide:
            popDownListView();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter( object, event );
}


/*!
    Returns the index of the first item \e after \a startingAt of
    which \a prefix is a case-insensitive prefix. Returns -1 if no
    items start with \a prefix.
*/

QListViewItem *QComboView::completionIndex( const QString & prefix,
                    QListViewItem *startingAt ) const
{
    QListViewItem *start = startingAt;
/*    if ( start < 0 || start >= count() )
        start = 0;
    if ( start >= count() )
        return -1;*/
    if (!start)
        start = listView()->firstChild();
    if (!start)
        return 0;
/*    if (!start->itemBelow())
        return 0;*/
    QString match = prefix.lower();
    if ( match.length() < 1 )
        return start;

    QString current;
    QListViewItem *i = start;
    do {
        current = i->text(0).lower();
        if ( current.startsWith( match ) )
            return i;
        i = i->itemBelow();
        if ( i )
            i = listView()->firstChild();
    } while ( i != start );
    return 0;
}


int QComboView::sizeLimit() const
{
    return d ? d->sizeLimit : INT_MAX;
}

void QComboView::setSizeLimit( int lines )
{
    d->sizeLimit = lines;
}


/*int QComboView::maxCount() const
{
    return d ? d->maxCount : INT_MAX;
}

void QComboView::setMaxCount( int count )
{
    int l = this->count();
    while( --l > count )
        removeItem( l );
    d->maxCount = count;
}
*/
QComboView::Policy QComboView::insertionPolicy() const
{
    return d->p;
}

void QComboView::setInsertionPolicy( Policy policy )
{
    d->p = policy;
}



/*!
  Internal slot to keep the line editor up to date.
*/

void QComboView::returnPressed()
{
    QString s( d->ed->text() );

    if ( s.isEmpty() )
        return;

    QListViewItem *c = 0;
    bool doInsert = TRUE;
    if ( !d->duplicatesEnabled ) {
        c = listView()->findItem(s, 0);
        if ( c )
            doInsert = FALSE;
    }

    if ( doInsert ) {
        if ( insertionPolicy() != NoInsertion ) {
/*            int cnt = count();
            while ( cnt >= d->maxCount ) {
                removeItem( --cnt );
            }*/
        }

        switch ( insertionPolicy() ) {
            case AtCurrent:
                if ( s != currentItem()->text(0) )
                    currentItem()->setText(0, s);
                emit activated( currentItem() );
                emit activated( s );
                return;
            case NoInsertion:
                emit activated( s );
                return;
            case AtTop:
                c = 0;
                return;
//                break;
            case AtBottom:
                c = new QListViewItem(listView(), listView()->lastItem(), s);
                break;
            case BeforeCurrent:
                if (currentItem() && currentItem()->itemAbove())
                    c = new QListViewItem(listView(), currentItem()->itemAbove(), s);
                else
                {
                    c = 0;
                    return;
                }
                break;
            case AfterCurrent:
                if (currentItem() && currentItem()->itemBelow())
                    c = new QListViewItem(listView(), currentItem()->itemBelow(), s);
                else
                {
                    c = 0;
                    return;
                }
                break;
        }
    }

    if (c)
    {
        setCurrentItem( c );
        emit activated( c );
        emit activated( s );
    }
}


/*! reimp
*/

void QComboView::setEnabled( bool enable )
{
    QWidget::setEnabled( enable );
}



/*!
    Applies the validator \a v to the combobox so that only text which
    is valid according to \a v is accepted.

    This function does nothing if the combobox is not editable.

    \sa validator() clearValidator() QValidator
*/

void QComboView::setValidator( const QValidator * v )
{
    if ( d && d->ed )
        d->ed->setValidator( v );
}


/*!
    Returns the validator which constrains editing for this combobox
    if there is one; otherwise returns 0.

    \sa setValidator() clearValidator() QValidator
*/

const QValidator * QComboView::validator() const
{
    return d && d->ed ? d->ed->validator() : 0;
}


/*!
    This slot is equivalent to setValidator( 0 ).
*/

void QComboView::clearValidator()
{
    if ( d && d->ed )
        d->ed->setValidator( 0 );
}


/*!
    Sets the combobox to use \a newListBox instead of the current list
    box or popup. As a side effect, it clears the combobox of its
    current contents.

    \warning QComboView assumes that newListBox->text(n) returns
    non-null for 0 \<= n \< newListbox->count(). This assumption is
    necessary because of the line edit in QComboView.
*/

void QComboView::setListView( QListView * newListView )
{
    clear();

    delete d->listView();

    newListView->reparent( this, WType_Popup, QPoint(0,0), FALSE );
    d->setListView( newListView );
    d->listView()->setFont( font() );
    d->listView()->setPalette( palette() );
/*    d->listView()->setVScrollBarMode(QScrollView::AlwaysOff);
    d->listView()->setHScrollBarMode(QScrollView::AlwaysOff);*/
    d->listView()->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listView()->setLineWidth( 1 );
/*    d->listView()->setRootIsDecorated( true );
    d->listView()->setAllColumnsShowFocus(true);*/
    d->listView()->resize( 100, 10 );

    if (d->listView()->firstChild())
        d->current = d->listView()->firstChild();

//    d->listView()->header()->hide();


/*    d->listView()->setFont( font() );
    d->listView()->setPalette( palette() );
    d->listView()->setVScrollBarMode( QScrollView::AlwaysOff );
    d->listView()->setHScrollBarMode( QScrollView::AlwaysOff );
    d->listView()->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listView()->setLineWidth( 1 );
    d->listView()->setRootIsDecorated( true );
    d->listView()->setAllColumnsShowFocus(true);
    d->listView()->addColumn("");
    d->listView()->resize( 100, 10 );
*/

    connect( d->listView(), SIGNAL(returnPressed(QListViewItem*)),
            SLOT(internalActivate(QListViewItem*)));
    connect( d->listView(), SIGNAL(doubleClicked(QListViewItem*)),
            SLOT(internalActivate(QListViewItem*)));
    connect( d->listView(), SIGNAL(doubleClicked(QListViewItem*)),
            SLOT(checkState(QListViewItem*)));
    connect( d->listView(), SIGNAL(currentChanged(QListViewItem*)),
            SLOT(internalHighlight(QListViewItem*)));
    connect( d->listView(), SIGNAL(selectionChanged(QListViewItem*)),
            SLOT(internalHighlight(QListViewItem*)));
}


/*!
    Returns the current list box, or 0 if there is no list box.
    (QComboView can use QPopupMenu instead of QListBox.) Provided to
    match setlistView().

    \sa setlistView()
*/

QListView * QComboView::listView() const
{
    return d ? d->listView() : 0;
}

/*!
    Returns the line edit, or 0 if there is no line edit.

    Only editable listboxes have a line editor.
*/
QLineEdit* QComboView::lineEdit() const
{
    return d->ed;
}



/*!
    Clears the line edit without changing the combobox's contents.
    Does nothing if the combobox isn't editable.

    This is particularly useful when using a combobox as a line edit
    with history. For example you can connect the combobox's
    activated() signal to clearEdit() in order to present the user
    with a new, empty line as soon as Enter is pressed.

    \sa setEditText()
*/

void QComboView::clearEdit()
{
    if ( d && d->ed )
        d->ed->clear();
}


/*!
    Sets the text in the line edit to \a newText without changing the
    combobox's contents. Does nothing if the combobox isn't editable.

    This is useful e.g. for providing a good starting point for the
    user's editing and entering the change in the combobox only when
    the user presses Enter.

    \sa clearEdit() insertItem()
*/

void QComboView::setEditText( const QString &newText )
{
    if ( d && d->ed ) {
        d->updateLinedGeometry();
        d->ed->setText( newText );
    }
}

void QComboView::setAutoCompletion( bool enable )
{
    d->useCompletion = enable;
    d->completeNow = FALSE;
}


bool QComboView::autoCompletion() const
{
    return d->useCompletion;
}

/*!reimp
 */
void QComboView::styleChange( QStyle& s )
{
    d->sizeHint = QSize();		// invalidate size hint...
    if ( d->ed )
        d->updateLinedGeometry();
    QWidget::styleChange( s );
}

bool QComboView::editable() const
{
    return d->ed != 0;
}

void QComboView::setEditable( bool y )
{
    if ( y == editable() )
        return;
    if ( y ) {
        setUpListView();
        setUpLineEdit();
        d->ed->show();
        if ( currentItem() )
            setEditText( currentText() );
        } else {
            delete d->ed;
        d->ed = 0;
    }

    setFocusPolicy( StrongFocus );
    updateGeometry();
    update();
}


void QComboView::setUpListView()
{
    d->setListView( new QListView( this, "in-combo", WType_Popup ) );

    d->listView()->setFont( font() );
    d->listView()->setPalette( palette() );
/*    d->listView()->setVScrollBarMode( QScrollView::AlwaysOff );
    d->listView()->setHScrollBarMode( QScrollView::AlwaysOff );*/
    d->listView()->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listView()->setLineWidth( 1 );
    d->listView()->setRootIsDecorated( false );
    d->listView()->setAllColumnsShowFocus(true);
    d->listView()->addColumn("");
    d->listView()->resize( 100, 10 );
    d->listView()->setResizeMode(QListView::LastColumn);

    if (d->listView()->firstChild())
        d->current = d->listView()->firstChild();

    d->listView()->header()->hide();

    connect( d->listView(), SIGNAL(returnPressed(QListViewItem*)),
            SLOT(internalActivate(QListViewItem*)));
    connect( d->listView(), SIGNAL(doubleClicked(QListViewItem*)),
            SLOT(internalActivate(QListViewItem*)));
    connect( d->listView(), SIGNAL(doubleClicked(QListViewItem*)),
            SLOT(checkState(QListViewItem*)));
    connect( d->listView(), SIGNAL(currentChanged(QListViewItem*)),
            SLOT(internalHighlight(QListViewItem*)));
    connect( d->listView(), SIGNAL(selectionChanged(QListViewItem*)),
            SLOT(internalHighlight(QListViewItem*)));
}


void QComboView::setUpLineEdit()
{
    if ( !d->ed )
        setLineEdit( new QLineEdit( this, "combo edit" ) );
}

/*!
    Sets the line edit to use \a edit instead of the current line edit.
*/

void QComboView::setLineEdit( QLineEdit *edit )
{
    if ( !edit ) {
#if defined(QT_CHECK_NULL)
        Q_ASSERT( edit != 0 );
#endif
        return;
    }

    edit->setText( currentText() );
    if ( d->ed ) {
        int start = 0, end = 0;
        d->ed->getSelection( &start, &end );
        edit->setSelection( start, end );
        edit->setCursorPosition( d->ed->cursorPosition() );
        edit->setEdited( d->ed->edited() );
        delete d->ed;
    }

    d->ed = edit;

    if ( edit->parent() != this ) {
        edit->reparent( this, QPoint(0,0), FALSE );
        edit->setFont( font() );
    }

    connect (edit, SIGNAL( textChanged( const QString& ) ),
            this, SIGNAL( textChanged( const QString& ) ) );
    connect( edit, SIGNAL(returnPressed()), SLOT(returnPressed()) );

    edit->setFrame( FALSE );
    d->updateLinedGeometry();
    edit->installEventFilter( this );
    setFocusProxy( edit );
    setFocusPolicy( StrongFocus );

    setUpListView();

    if ( isVisible() )
        edit->show();

    updateGeometry();
    update();
}

void QComboView::setCurrentText( const QString& txt )
{
    QListViewItem *i;
    i = listView()->findItem(txt, 0);
    if ( i )
        setCurrentItem( i );
    else if ( d->ed )
        d->ed->setText( txt );
    else if (currentItem())
        currentItem()->setText(0, txt);
}

void QComboView::checkState( QListViewItem * item)
{
    item->setOpen(!item->isOpen());
}

void QComboView::setCurrentActiveItem( QListViewItem * item )
{
    if ( item == d->current && !d->ed ) {
        return;
    }

    d->current = item;
    d->completeAt = 0;
    if ( d->ed ) {
        d->ed->setText( item->text(0) );
//        qWarning("setCurrentActiveItem( %s )", item->text(0).latin1());
        d->updateLinedGeometry();
    }
    if ( d->listView() ) {
        d->listView()->setCurrentItem( item );
        emit activated( item );
        emit activated( item->text(0) );
    } else {
        internalHighlight( item );
        internalActivate( item );
    }

    currentChanged();

    d->listView()->ensureItemVisible(item);
}

#include "qcomboview.moc"

#endif // QT_NO_COMBOBOX


/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#include "actiondnd.h"
#include "command.h"
#include "defs.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "metadatabase.h"
#include "widgetfactory.h"
#include "hierarchyview.h"

#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdragobject.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenudata.h>
#include <qmessagebox.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qtimer.h>

#include <klocale.h>

QAction *ActionDrag::the_action = 0;

ActionDrag::ActionDrag(QAction *action, QWidget *source)
: QStoredDrag("application/x-designer-actions", source)
{
    Q_ASSERT(the_action == 0);
    the_action = action;
}

ActionDrag::ActionDrag(QActionGroup *group, QWidget *source)
: QStoredDrag("application/x-designer-actiongroup", source)
{
    Q_ASSERT(the_action == 0);
    the_action = group;
}

ActionDrag::ActionDrag(const QString &type, QAction *action, QWidget *source)
: QStoredDrag(type, source)
{
    Q_ASSERT(the_action == 0);
    the_action = action;
}

bool ActionDrag::canDecode(QDropEvent *e)
{
    return e->provides( "application/x-designer-actions" ) ||
	   e->provides( "application/x-designer-actiongroup" ) ||
	   e->provides( "application/x-designer-separator" );
}

ActionDrag::~ActionDrag()
{
    the_action = 0;
}

void QDesignerAction::init()
{
    MetaDataBase::addEntry( this );
    int id = WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( this ) );
    WidgetFactory::saveDefaultProperties( this, id );
    WidgetFactory::saveChangedProperties( this, id );
}

void QDesignerActionGroup::init()
{
    MetaDataBase::addEntry( this );
    int id = WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( this ) );
    WidgetFactory::saveDefaultProperties( this, id );
    WidgetFactory::saveChangedProperties( this, id );
}

bool QDesignerAction::addTo( QWidget *w )
{
    if ( !widgetToInsert )
	return QAction::addTo( w );

    if ( ::qt_cast<QPopupMenu*>(w) )
	return FALSE;

    widgetToInsert->reparent( w, QPoint( 0, 0 ), FALSE );
    widgetToInsert->show();
    addedTo( widgetToInsert, w );
    return TRUE;
}

bool QDesignerAction::removeFrom( QWidget *w )
{
    if ( !widgetToInsert )
	return QAction::removeFrom( w );

    remove();
    return TRUE;
}

void QDesignerAction::remove()
{
    if ( !widgetToInsert )
	return;
    MainWindow::self->formWindow()->selectWidget( widgetToInsert, FALSE );
    widgetToInsert->reparent( 0, QPoint( 0, 0 ), FALSE );
}

QDesignerToolBarSeparator::QDesignerToolBarSeparator(Orientation o , QToolBar *parent,
                                     const char* name )
    : QWidget( parent, name )
{
    connect( parent, SIGNAL(orientationChanged(Orientation)),
             this, SLOT(setOrientation(Orientation)) );
    setOrientation( o );
    setBackgroundMode( parent->backgroundMode() );
    setBackgroundOrigin( ParentOrigin );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
}

void QDesignerToolBarSeparator::setOrientation( Orientation o )
{
    orient = o;
}

void QDesignerToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize QDesignerToolBarSeparator::sizeHint() const
{
    int extent = style().pixelMetric( QStyle::PM_DockWindowSeparatorExtent,
				      this );
    if ( orient == Horizontal )
	return QSize( extent, 0 );
    else
	return QSize( 0, extent );
}

void QDesignerToolBarSeparator::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    QStyle::SFlags flags = QStyle::Style_Default;

    if ( orientation() == Horizontal )
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive( QStyle::PE_DockWindowSeparator, &p, rect(),
			   colorGroup(), flags );
}



QSeparatorAction::QSeparatorAction( QObject *parent )
    : QAction( parent, "qt_designer_separator" ), wid( 0 )
{
}

bool QSeparatorAction::addTo( QWidget *w )
{
    if ( ::qt_cast<QToolBar*>(w) ) {
	QToolBar *tb = (QToolBar*)w;
	wid = new QDesignerToolBarSeparator( tb->orientation(), tb );
	return TRUE;
    } else if ( ::qt_cast<QPopupMenu*>(w) ) {
	idx = ( (QPopupMenu*)w )->count();
	( (QPopupMenu*)w )->insertSeparator( idx );
	return TRUE;
    }
    return FALSE;
}

bool QSeparatorAction::removeFrom( QWidget *w )
{
    if ( ::qt_cast<QToolBar*>(w) ) {
	delete wid;
	return TRUE;
    } else if ( ::qt_cast<QPopupMenu*>(w) ) {
	( (QPopupMenu*)w )->removeItemAt( idx );
	return TRUE;
    }
    return FALSE;
}

QWidget *QSeparatorAction::widget() const
{
    return wid;
}



QDesignerToolBar::QDesignerToolBar( QMainWindow *mw )
    : QToolBar( mw ), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    MetaDataBase::addEntry( this );
    lastIndicatorPos = QPoint( -1, -1 );
    indicator = new QDesignerIndicatorWidget( this );
    indicator->hide();
    installEventFilter( this );
    widgetInserting = FALSE;
    findFormWindow();
    mw->setDockEnabled( DockTornOff, FALSE );
}

QDesignerToolBar::QDesignerToolBar( QMainWindow *mw, Dock dock )
    : QToolBar( QString::null, mw, dock), lastIndicatorPos( -1, -1 )
{
    insertAnchor = 0;
    afterAnchor = TRUE;
    setAcceptDrops( TRUE );
    indicator = new QDesignerIndicatorWidget( this );
    indicator->hide();
    MetaDataBase::addEntry( this );
    installEventFilter( this );
    widgetInserting = FALSE;
    findFormWindow();
    mw->setDockEnabled( DockTornOff, FALSE );
}

void QDesignerToolBar::findFormWindow()
{
    QWidget *w = this;
    while ( w ) {
	formWindow = ::qt_cast<FormWindow*>(w);
	if ( formWindow )
	    break;
	w = w->parentWidget();
    }
}

void QDesignerToolBar::addAction( QAction *a )
{
    actionList.append( a );
    connect( a, SIGNAL( destroyed() ), this, SLOT( actionRemoved() ) );
    if ( ::qt_cast<QActionGroup*>(a) ) {
	( (QDesignerActionGroup*)a )->widget()->installEventFilter( this );
	actionMap.insert( ( (QDesignerActionGroup*)a )->widget(), a );
    } else if ( ::qt_cast<QSeparatorAction*>(a) ) {
	( (QSeparatorAction*)a )->widget()->installEventFilter( this );
	actionMap.insert( ( (QSeparatorAction*)a )->widget(), a );
    } else {
	( (QDesignerAction*)a )->widget()->installEventFilter( this );
	actionMap.insert( ( (QDesignerAction*)a )->widget(), a );
    }
}

static void fixObject( QObject *&o )
{
    while ( o && o->parent() && !::qt_cast<QDesignerToolBar*>(o->parent()) )
	o = o->parent();
}

bool QDesignerToolBar::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e || o->inherits( "QDockWindowHandle" ) || o->inherits( "QDockWindowTitleBar" ) )
	return QToolBar::eventFilter( o, e );

    if ( o == this && e->type() == QEvent::MouseButtonPress &&
	 ( ( QMouseEvent*)e )->button() == LeftButton ) {
	mousePressEvent( (QMouseEvent*)e );
	return TRUE;
    }

    if ( o == this )
	return QToolBar::eventFilter( o, e );

    if ( e->type() == QEvent::MouseButtonPress ) {
	QMouseEvent *ke = (QMouseEvent*)e;
	fixObject( o );
	if ( !o )
	    return FALSE;
	buttonMousePressEvent( ke, o );
	return TRUE;
    } else if(e->type() == QEvent::ContextMenu ) {
	QContextMenuEvent *ce = (QContextMenuEvent*)e;
	fixObject( o );
	if( !o )
	    return FALSE;
	buttonContextMenuEvent( ce, o );
	return TRUE;
    } else if ( e->type() == QEvent::MouseMove ) {
	QMouseEvent *ke = (QMouseEvent*)e;
	fixObject( o );
	if ( !o )
	    return FALSE;
	buttonMouseMoveEvent( ke, o );
	return TRUE;
    } else if ( e->type() == QEvent::MouseButtonRelease ) {
	QMouseEvent *ke = (QMouseEvent*)e;
	fixObject( o );
	if ( !o )
	    return FALSE;
	buttonMouseReleaseEvent( ke, o );
	return TRUE;
    } else if ( e->type() == QEvent::DragEnter ) {
	QDragEnterEvent *de = (QDragEnterEvent*)e;
	if (ActionDrag::canDecode(de))
	    de->accept();
    } else if ( e->type() == QEvent::DragMove ) {
	QDragMoveEvent *de = (QDragMoveEvent*)e;
	if (ActionDrag::canDecode(de))
	    de->accept();
    }

    return QToolBar::eventFilter( o, e );
}

void QDesignerToolBar::paintEvent( QPaintEvent *e )
{
    QToolBar::paintEvent( e );
    if ( e->rect() != rect() )
	return;
    lastIndicatorPos = QPoint( -1, -1 );
}

void QDesignerToolBar::contextMenuEvent( QContextMenuEvent *e )
{
    e->accept();
    QPopupMenu menu( 0 );
    menu.insertItem( i18n( "Delete Toolbar" ), 1 );
    int res = menu.exec( e->globalPos() );
    if ( res != -1 ) {
	RemoveToolBarCommand *cmd = new RemoveToolBarCommand( i18n( "Delete Toolbar '%1'" ).arg( name() ),
							      formWindow, 0, this );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void QDesignerToolBar::mousePressEvent( QMouseEvent *e )
{
    widgetInserting = FALSE;
    if ( e->button() == LeftButton &&
	 MainWindow::self->currentTool() != POINTER_TOOL &&
	 MainWindow::self->currentTool() != ORDER_TOOL &&
	 MainWindow::self->currentTool() != CONNECT_TOOL &&
	 MainWindow::self->currentTool() != BUDDY_TOOL )
	widgetInserting = TRUE;
}

void QDesignerToolBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( widgetInserting )
	doInsertWidget( mapFromGlobal( e->globalPos() ) );
    widgetInserting = FALSE;
}

void QDesignerToolBar::buttonMouseReleaseEvent( QMouseEvent *e, QObject *w )
{
    if ( widgetInserting )
	doInsertWidget( mapFromGlobal( e->globalPos() ) );
    else if ( w->isWidgetType() && formWindow->widgets()->find( w ) ) {
	formWindow->clearSelection( FALSE );
	formWindow->selectWidget( w );
    }
    widgetInserting = FALSE;
}

void QDesignerToolBar::buttonContextMenuEvent( QContextMenuEvent *e, QObject *o )
{
    e->accept();
    QPopupMenu menu( 0 );
    const int ID_DELETE = 1;
    const int ID_SEP = 2;
    const int ID_DELTOOLBAR = 3;
    QMap<QWidget*, QAction*>::Iterator it = actionMap.find( (QWidget*)o );
    if ( it != actionMap.end() && ::qt_cast<QSeparatorAction*>(*it) )
	menu.insertItem( i18n( "Delete Separator" ), ID_DELETE );
    else
	menu.insertItem( i18n( "Delete Item" ), ID_DELETE );
    menu.insertItem( i18n( "Insert Separator" ), ID_SEP );
    menu.insertSeparator();
    menu.insertItem( i18n( "Delete Toolbar" ), ID_DELTOOLBAR );
    int res = menu.exec( e->globalPos() );
    if ( res == ID_DELETE ) {
	QMap<QWidget*, QAction*>::Iterator it = actionMap.find( (QWidget*)o );
	if ( it == actionMap.end() )
	    return;
	QAction *a = *it;
	int index = actionList.find( a );
	RemoveActionFromToolBarCommand *cmd = new RemoveActionFromToolBarCommand(
	    i18n( "Delete Action '%1' from Toolbar '%2'" ).
	    arg( a->name() ).arg( caption() ),
	    formWindow, a, this, index );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else if ( res == ID_SEP ) {
	calcIndicatorPos( mapFromGlobal( e->globalPos() ) );
	QAction *a = new QSeparatorAction( 0 );
	int index = actionList.findRef( *actionMap.find( insertAnchor ) );
	if ( index != -1 && afterAnchor )
	    ++index;
	if ( !insertAnchor )
	    index = 0;

	AddActionToToolBarCommand *cmd = new AddActionToToolBarCommand(
	    i18n( "Add Separator to Toolbar '%1'" ).
	    arg( a->name() ),
	    formWindow, a, this, index );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else if ( res == ID_DELTOOLBAR ) {
	RemoveToolBarCommand *cmd = new RemoveToolBarCommand( i18n( "Delete Toolbar '%1'" ).arg( name() ),
							      formWindow, 0, this );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void QDesignerToolBar::buttonMousePressEvent( QMouseEvent *e, QObject * )
{
    widgetInserting = FALSE;

    if ( e->button() == MidButton )
	return;

    if ( e->button() == LeftButton &&
	 MainWindow::self->currentTool() != POINTER_TOOL &&
	 MainWindow::self->currentTool() != ORDER_TOOL &&
	 MainWindow::self->currentTool() != CONNECT_TOOL &&
	 MainWindow::self->currentTool() != BUDDY_TOOL ) {
	    widgetInserting = TRUE;
	    return;
    }

    dragStartPos = e->pos();
}

void QDesignerToolBar::removeWidget( QWidget *w )
{
    QMap<QWidget*, QAction*>::Iterator it = actionMap.find( w );
    if ( it == actionMap.end() )
	return;
    QAction *a = *it;
    int index = actionList.find( a );
    RemoveActionFromToolBarCommand *cmd =
	new RemoveActionFromToolBarCommand( i18n( "Delete Action '%1' from Toolbar '%2'" ).
					    arg( a->name() ).arg( caption() ),
					    formWindow, a, this, index );
    formWindow->commandHistory()->addCommand( cmd );
    cmd->execute();
    QApplication::sendPostedEvents();
    adjustSize();
}

void QDesignerToolBar::buttonMouseMoveEvent( QMouseEvent *e, QObject *o )
{
    if ( widgetInserting || ( e->state() & LeftButton ) == 0 )
	return;
    if ( QABS( QPoint( dragStartPos - e->pos() ).manhattanLength() ) < QApplication::startDragDistance() )
	return;
    QMap<QWidget*, QAction*>::Iterator it = actionMap.find( (QWidget*)o );
    if ( it == actionMap.end() )
	return;
    QAction *a = *it;
    if ( !a )
	return;
    int index = actionList.find( a );
    RemoveActionFromToolBarCommand *cmd =
	new RemoveActionFromToolBarCommand( i18n( "Delete Action '%1' from Toolbar '%2'" ).
					    arg( a->name() ).arg( caption() ),
					    formWindow, a, this, index );
    formWindow->commandHistory()->addCommand( cmd );
    cmd->execute();
    QApplication::sendPostedEvents();
    adjustSize();

    QString type = ::qt_cast<QActionGroup*>(a) ? QString( "application/x-designer-actiongroup" ) :
	::qt_cast<QSeparatorAction*>(a) ? QString( "application/x-designer-separator" ) : QString( "application/x-designer-actions" );
    QStoredDrag *drag = new ActionDrag( type, a, this );
    drag->setPixmap( a->iconSet().pixmap() );
    if ( ::qt_cast<QDesignerAction*>(a) ) {
	if ( formWindow->widgets()->find( ( (QDesignerAction*)a )->widget() ) )
	    formWindow->selectWidget( ( (QDesignerAction*)a )->widget(), FALSE );
    }
    if ( !drag->drag() ) {
	AddActionToToolBarCommand *cmd = new AddActionToToolBarCommand( i18n( "Add Action '%1' to Toolbar '%2'" ).
									arg( a->name() ).arg( caption() ),
									formWindow, a, this, index );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
    lastIndicatorPos = QPoint( -1, -1 );
    indicator->hide();
}

#ifndef QT_NO_DRAGANDDROP

void QDesignerToolBar::dragEnterEvent( QDragEnterEvent *e )
{
    widgetInserting = FALSE;
    lastIndicatorPos = QPoint( -1, -1 );
    if (ActionDrag::canDecode(e))
	e->accept();
}

void QDesignerToolBar::dragMoveEvent( QDragMoveEvent *e )
{
    if (ActionDrag::canDecode(e)) {
	e->accept();
	drawIndicator( calcIndicatorPos( e->pos() ) );
    }
}

void QDesignerToolBar::dragLeaveEvent( QDragLeaveEvent * )
{
    indicator->hide();
    insertAnchor = 0;
    afterAnchor = TRUE;
}

void QDesignerToolBar::dropEvent( QDropEvent *e )
{
    if (!ActionDrag::canDecode(e))
	return;

    e->accept();

    indicator->hide();
    QAction *a = 0;
    int index = actionList.findRef( *actionMap.find( insertAnchor ) );
    if ( index != -1 && afterAnchor )
	++index;
    if ( !insertAnchor )
	index = 0;
    if ( e->provides( "application/x-designer-actions" ) ||
	 e->provides( "application/x-designer-separator" ) ) {
	if ( e->provides( "application/x-designer-actions" ) )
	    a = ::qt_cast<QDesignerAction*>(ActionDrag::action());
	else
	    a = ::qt_cast<QSeparatorAction*>(ActionDrag::action());
    } else {
	a = ::qt_cast<QDesignerActionGroup*>(ActionDrag::action());
    }

    if ( actionList.findRef( a ) != -1 ) {
	QMessageBox::warning( MainWindow::self, i18n( "Insert/Move Action" ),
			      i18n( "Action '%1' has already been added to this toolbar.\n"
				  "An Action may only occur once in a given toolbar." ).
			      arg( a->name() ) );
	return;
    }

    AddActionToToolBarCommand *cmd = new AddActionToToolBarCommand( i18n( "Add Action '%1' to Toolbar '%2'" ).
								    arg( a->name() ).arg( caption() ),
								    formWindow, a, this, index );
    formWindow->commandHistory()->addCommand( cmd );
    cmd->execute();

    lastIndicatorPos = QPoint( -1, -1 );
}

#endif

void QDesignerToolBar::reInsert()
{
    QAction *a = 0;
    actionMap.clear();
    clear();
    for ( a = actionList.first(); a; a = actionList.next() ) {
	a->addTo( this );
	if ( ::qt_cast<QActionGroup*>(a) ) {
	    actionMap.insert( ( (QDesignerActionGroup*)a )->widget(), a );
	    if ( ( (QDesignerActionGroup*)a )->widget() )
		( (QDesignerActionGroup*)a )->widget()->installEventFilter( this );
	} else if ( ::qt_cast<QDesignerAction*>(a) ) {
	    actionMap.insert( ( (QDesignerAction*)a )->widget(), a );
	    ( (QDesignerAction*)a )->widget()->installEventFilter( this );
	} else if ( ::qt_cast<QSeparatorAction*>(a) ) {
	    actionMap.insert( ( (QSeparatorAction*)a )->widget(), a );
	    ( (QSeparatorAction*)a )->widget()->installEventFilter( this );
	}
    }
    QApplication::sendPostedEvents();
    adjustSize();
}

void QDesignerToolBar::actionRemoved()
{
    actionList.removeRef( (QAction*)sender() );
}

QPoint QDesignerToolBar::calcIndicatorPos( const QPoint &pos )
{
    if ( orientation() == Horizontal ) {
	QPoint pnt( width() - 2, 0 );
	insertAnchor = 0;
	afterAnchor = TRUE;
	if ( !children() )
	    return pnt;
	pnt = QPoint( 13, 0 );
	QObjectListIt it( *children() );
	QObject * obj;
	while( (obj=it.current()) != 0 ) {
	    ++it;
	    if ( obj->isWidgetType() &&
		 qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 ) {
		QWidget *w = (QWidget*)obj;
		if ( w->x() < pos.x() ) {
		    pnt.setX( w->x() + w->width() + 1 );
		    insertAnchor = w;
		    afterAnchor = TRUE;
		}
	    }
	}
	return pnt;
    } else {
	QPoint pnt( 0, height() - 2 );
	insertAnchor = 0;
	afterAnchor = TRUE;
	if ( !children() )
	    return pnt;
	pnt = QPoint( 0, 13 );
	QObjectListIt it( *children() );
	QObject * obj;
	while( (obj=it.current()) != 0 ) {
	    ++it;
	    if ( obj->isWidgetType() &&
		 qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 ) {
		QWidget *w = (QWidget*)obj;
		if ( w->y() < pos.y() ) {
		    pnt.setY( w->y() + w->height() + 1 );
		    insertAnchor = w;
		    afterAnchor = TRUE;
		}
	    }
	}
	return pnt;
    }
}

void QDesignerToolBar::drawIndicator( const QPoint &pos )
{
    if ( lastIndicatorPos == pos )
	return;
    bool wasVsisible = indicator->isVisible();
    if ( orientation() == Horizontal ) {
	indicator->resize( 3, height() );
	if ( pos != QPoint( -1, -1 ) )
	     indicator->move( pos.x() - 1, 0 );
	indicator->show();
	indicator->raise();
	lastIndicatorPos = pos;
    } else {
	indicator->resize( width(), 3 );
	if ( pos != QPoint( -1, -1 ) )
	     indicator->move( 0, pos.y() - 1 );
	indicator->show();
	indicator->raise();
	lastIndicatorPos = pos;
    }
    if ( !wasVsisible )
	QApplication::sendPostedEvents();
}

void QDesignerToolBar::doInsertWidget( const QPoint &p )
{
    if ( formWindow != MainWindow::self->formWindow() )
	return;
    calcIndicatorPos( p );
    QWidget *w = WidgetFactory::create( MainWindow::self->currentTool(), this, 0, TRUE );
    installEventFilters( w );
    MainWindow::self->formWindow()->insertWidget( w, TRUE );
    QDesignerAction *a = new QDesignerAction( w, parent() );
    int index = actionList.findRef( *actionMap.find( insertAnchor ) );
    if ( index != -1 && afterAnchor )
	++index;
    if ( !insertAnchor )
	index = 0;
    AddActionToToolBarCommand *cmd = new AddActionToToolBarCommand( i18n( "Add Widget '%1' to Toolbar '%2'" ).
								    arg( w->name() ).arg( caption() ),
								    formWindow, a, this, index );
    formWindow->commandHistory()->addCommand( cmd );
    cmd->execute();
    MainWindow::self->resetTool();
}

void QDesignerToolBar::clear()
{
    for ( QAction *a = actionList.first(); a; a = actionList.next() ) {
	if ( ::qt_cast<QDesignerAction*>(a) )
	    ( (QDesignerAction*)a )->remove();
    }
    QToolBar::clear();
}

void QDesignerToolBar::installEventFilters( QWidget *w )
{
    if ( !w )
	return;
    QObjectList *l = w->queryList( "QWidget" );
    for ( QObject *o = l->first(); o; o = l->next() )
	o->installEventFilter( this );
    delete l;
}

//#include "actiondnd.moc"

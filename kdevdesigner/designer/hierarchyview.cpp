/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "hierarchyview.h"
#include "formwindow.h"
#include "globaldefs.h"
#include "mainwindow.h"
#include "command.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "project.h"
#include "sourceeditor.h"
#include "propertyeditor.h"
#include "editfunctionsimpl.h"
#include "listeditor.h"
#include "actiondnd.h"
#include "actioneditorimpl.h"
#include "variabledialogimpl.h"
#include "popupmenueditor.h"
#include "menubareditor.h"

#include <kiconloader.h>
#include "kdevdesigner_part.h"
#include <klocale.h>

#include <qpalette.h>
#include <qobject.h>
#include <q3header.h>
#include <q3popupmenu.h>
#include <qtabwidget.h>
#include <q3wizard.h>
#include <q3widgetstack.h>
#include <qtabbar.h>
#include <qfeatures.h>
#include <qapplication.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <Q3CString>
#include <Q3ValueList>
#include <Q3PtrList>
#include <QCloseEvent>
#include <QPixmap>
#include "../interfaces/languageinterface.h"
#include <qworkspace.h>
#include <q3accel.h>
#include <qmessagebox.h>

#include <stdlib.h>

Q3ListViewItem *newItem = 0;

const QPixmap DesignerFormPix = SmallIcon( "designer_form.png" , KDevDesignerPartFactory::instance());
const QPixmap DesignerLayoutPix = SmallIcon( "designer_layout.png" , KDevDesignerPartFactory::instance());
const QPixmap DesignerFolderPix = SmallIcon( "designer_folder.png" , KDevDesignerPartFactory::instance());
const QPixmap DesignerEditSlotsPix = SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance());

static QPluginManager<ClassBrowserInterface> *classBrowserInterfaceManager = 0;

HierarchyItem::HierarchyItem( Type type, Q3ListViewItem *parent, Q3ListViewItem *after,
			      const QString &txt1, const QString &txt2, const QString &txt3 )
    : Q3ListViewItem( parent, after, txt1, txt2, txt3 ), typ( type )
{
}

HierarchyItem::HierarchyItem( Type type, Q3ListView *parent, Q3ListViewItem *after,
			      const QString &txt1, const QString &txt2, const QString &txt3 )
    : Q3ListViewItem( parent, after, txt1, txt2, txt3 ), typ( type )
{
}

void HierarchyItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup g( cg );
    g.setColor( QColorGroup::Base, backgroundColor() );
    g.setColor( QColorGroup::Foreground, Qt::black );
    g.setColor( QColorGroup::Text, Qt::black );
    QString txt = text( 0 );
    if ( rtti() == Function &&
	 MainWindow::self->currProject()->isCpp() &&
	 ( txt == "init()" || txt == "destroy()" ) ) {
	listView()->setUpdatesEnabled( FALSE );
	if ( txt == "init()" )
	    setText( 0, txt + " " + "(Constructor)" );
	else
	    setText( 0, txt + " " + "(Destructor)" );
	Q3ListViewItem::paintCell( p, g, column, width, align );
	setText( 0, txt );
	listView()->setUpdatesEnabled( TRUE );
    } else {
	Q3ListViewItem::paintCell( p, g, column, width, align );
    }
    p->save();
    p->setPen( QPen( cg.dark(), 1 ) );
    if ( column == 0 )
	p->drawLine( 0, 0, 0, height() - 1 );
    if ( listView()->firstChild() != this ) {
	if ( nextSibling() != itemBelow() && itemBelow()->depth() < depth() ) {
	    int d = depth() - itemBelow()->depth();
	    p->drawLine( -listView()->treeStepSize() * d, height() - 1, 0, height() - 1 );
	}
    }
    p->drawLine( 0, height() - 1, width, height() - 1 );
    p->drawLine( width - 1, 0, width - 1, height() );
    p->restore();
}

QColor HierarchyItem::backgroundColor()
{
    updateBackColor();
    return backColor;
}

void HierarchyItem::updateBackColor()
{
    if ( listView()->firstChild() == this ) {
	backColor = *backColor1;
	return;
    }

    Q3ListViewItemIterator it( this );
    --it;
    if ( it.current() ) {
	if ( ( ( HierarchyItem*)it.current() )->backColor == *backColor1 )
	    backColor = *backColor2;
	else
	    backColor = *backColor1;
    } else {
	backColor = *backColor1;
    }
}

void HierarchyItem::setObject( QObject *o )
{
    obj = o;
}

QObject *HierarchyItem::object() const
{
    return obj;
}

void HierarchyItem::okRename( int col )
{
    if ( newItem == this )
	newItem = 0;
    Q3ListViewItem::okRename( col );
}

void HierarchyItem::cancelRename( int col )
{
    if ( newItem == this ) {
	newItem = 0;
	Q3ListViewItem::cancelRename( col );
	delete this;
	return;
    }
    Q3ListViewItem::cancelRename( col );
}




HierarchyList::HierarchyList( QWidget *parent, FormWindow *fw, bool doConnects )
    : Q3ListView( parent ), formWindow( fw )
{
    init_colors();

    setDefaultRenameAction( Accept );
    header()->setMovingEnabled( FALSE );
    header()->setStretchEnabled( TRUE );
    normalMenu = 0;
    tabWidgetMenu = 0;
    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Class" ) );
    QPalette p( palette() );
    p.setColor( QColorGroup::Base, QColor( *backColor2 ) );
    (void)*selectedBack; // hack
    setPalette( p );
    disconnect( header(), SIGNAL( sectionClicked( int ) ),
		this, SLOT( changeSortColumn( int ) ) );
    setSorting( -1 );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    if ( doConnects ) {
	connect( this, SIGNAL( clicked( Q3ListViewItem * ) ),
		 this, SLOT( objectClicked( Q3ListViewItem * ) ) );
	connect( this, SIGNAL( doubleClicked( Q3ListViewItem * ) ),
		 this, SLOT( objectDoubleClicked( Q3ListViewItem * ) ) );
	connect( this, SIGNAL( returnPressed( Q3ListViewItem * ) ),
		 this, SLOT( objectClicked( Q3ListViewItem * ) ) );
	connect( this, SIGNAL( contextMenuRequested( Q3ListViewItem *, const QPoint&, int ) ),
		 this, SLOT( showRMBMenu( Q3ListViewItem *, const QPoint & ) ) );
    }
    deselect = TRUE;
    setColumnWidthMode( 1, Manual );
}

void HierarchyList::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Shift || e->key() == Key_Control )
	deselect = FALSE;
    else
	deselect = TRUE;
    Q3ListView::keyPressEvent( e );
}

void HierarchyList::keyReleaseEvent( QKeyEvent *e )
{
    deselect = TRUE;
    Q3ListView::keyReleaseEvent( e );
}

void HierarchyList::viewportMousePressEvent( QMouseEvent *e )
{
    if ( e->state() & ShiftButton || e->state() & ControlButton )
	deselect = FALSE;
    else
	deselect = TRUE;
    Q3ListView::viewportMousePressEvent( e );
}

void HierarchyList::viewportMouseReleaseEvent( QMouseEvent *e )
{
    Q3ListView::viewportMouseReleaseEvent( e );
}

QObject *HierarchyList::handleObjectClick( Q3ListViewItem *i )
{
    if ( !i )
	return 0;

    QObject *o = findObject( i );
    if ( !o )
	return 0;
    if ( formWindow == o ) {
	if ( deselect )
	    formWindow->clearSelection( FALSE );
	formWindow->emitShowProperties( formWindow );
	return 0;
    }
    if ( o->isWidgetType() ) {
	QWidget *w = (QWidget*)o;
	if ( !formWindow->widgets()->find( w ) ) {
	    if ( ::qt_cast<Q3WidgetStack*>(w->parent()) ) {
		if (::qt_cast<QTabWidget*>(w->parent()->parent()) ) {
		    ((QTabWidget*)w->parent()->parent())->showPage( w );
		    o = (QWidget*)w->parent()->parent();
		    formWindow->emitUpdateProperties( formWindow->currentWidget() );
		} else if ( ::qt_cast<Q3Wizard*>(w->parent()->parent()) ) {
		    ((QDesignerWizard*)w->parent()->parent())->
			setCurrentPage( ( (QDesignerWizard*)w->parent()->parent() )->pageNum( w ) );
		    o = (QWidget*)w->parent()->parent();
		    formWindow->emitUpdateProperties( formWindow->currentWidget() );
		} else {
		    ( (Q3WidgetStack*)w->parent() )->raiseWidget( w );
		    if ( (Q3WidgetStack*)w->parent()->isA( "QDesignerWidgetStack" ) )
			( (QDesignerWidgetStack*)w->parent() )->updateButtons();
		}
	    } else if ( ::qt_cast<QMenuBar*>(w) || ::qt_cast<Q3DockWindow*>(w) ) {
		formWindow->setActiveObject( w );
	    } else if ( ::qt_cast<Q3PopupMenu*>(w) ) {
		return 0; // ### we could try to find our menu bar and change the currentMenu to our index
	    } else {
		return 0;
	    }
	}
    } else if ( ::qt_cast<QAction*>(o) ) {
	MainWindow::self->actioneditor()->setCurrentAction( (QAction*)o );
	deselect = TRUE;
    }

    if ( deselect )
	formWindow->clearSelection( FALSE );

    return o;
}


void HierarchyList::objectDoubleClicked( Q3ListViewItem *i )
{
    QObject *o = handleObjectClick( i );
    if ( !o )
	return;
    if ( o->isWidgetType() && ( (QWidget*)o )->isVisibleTo( formWindow ) ) {
	QWidget *w = (QWidget*)o;
	if ( !w->parentWidget() ||
	     WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout )
	    w->raise();
	formWindow->selectWidget( w, TRUE );
    }
}

void HierarchyList::objectClicked( Q3ListViewItem *i )
{
    QObject *o = handleObjectClick( i );
    if ( !o )
	return;
    if ( o->isWidgetType() && ( (QWidget*)o )->isVisibleTo( formWindow ) ) {
	QWidget *w = (QWidget*)o;
	formWindow->selectWidget( w, TRUE );
    }
}

QObject *HierarchyList::findObject( Q3ListViewItem *i )
{
    return ( (HierarchyItem*)i )->object();
}

Q3ListViewItem *HierarchyList::findItem( QObject *o )
{
    Q3ListViewItemIterator it( this );
    while ( it.current() ) {
	if ( ( (HierarchyItem*)it.current() )->object() == o )
	    return it.current();
	++it;
    }
    return 0;
}

QObject *HierarchyList::current() const
{
    if ( currentItem() )
	return ( (HierarchyItem*)currentItem() )->object();
    return 0;
}

void HierarchyList::changeNameOf( QObject *o, const QString &name )
{
    Q3ListViewItem *item = findItem( o );
    if ( !item )
	return;
    item->setText( 0, name );
}


void HierarchyList::changeDatabaseOf( QObject *o, const QString &info )
{
#ifndef QT_NO_SQL
    if ( !formWindow->isDatabaseAware() )
	return;
    Q3ListViewItem *item = findItem( o );
    if ( !item )
	return;
    item->setText( 2, info );
#endif
}

static Q3PtrList<Q3WidgetStack> *widgetStacks = 0;

void HierarchyList::setup()
{
    if ( !formWindow || formWindow->isFake() )
	return;
    clear();
    QWidget *w = formWindow->mainContainer();
#ifndef QT_NO_SQL
    if ( formWindow->isDatabaseAware() ) {
	if ( columns() == 2 ) {
	    addColumn( i18n( "Database" ) );
	    header()->resizeSection( 0, 1 );
	    header()->resizeSection( 1, 1 );
	    header()->resizeSection( 2, 1 );
	    header()->adjustHeaderSize();
	}
    } else {
	if ( columns() == 3 ) {
	    removeColumn( 2 );
	}
    }
#endif
    if ( !widgetStacks )
	widgetStacks = new Q3PtrList<Q3WidgetStack>;
    if ( w )
	insertObject( w, 0 );
    widgetStacks->clear();
}

void HierarchyList::setOpen( Q3ListViewItem *i, bool b )
{
    Q3ListView::setOpen( i, b );
}

void HierarchyList::insertObject( QObject *o, Q3ListViewItem *parent )
{
    if ( QString( o->name() ).startsWith( "qt_dead_widget_" ) )
	return;
    bool fakeMainWindow = FALSE;
    if ( ::qt_cast<Q3MainWindow*>(o) ) {
	QObject *cw = ( (Q3MainWindow*)o )->centralWidget();
	if ( cw ) {
	    o = cw;
	    fakeMainWindow = TRUE;
	}
    }
    Q3ListViewItem *item = 0;
    QString className = WidgetFactory::classNameOf( o );
    if ( ::qt_cast<QLayoutWidget*>(o) ) {
	switch ( WidgetFactory::layoutType( (QWidget*)o ) ) {
	case WidgetFactory::HBox:
	    className = "HBox";
	    break;
	case WidgetFactory::VBox:
	    className = "VBox";
	    break;
	case WidgetFactory::Grid:
	    className = "Grid";
	    break;
	default:
	    break;
	}
    }

    QString dbInfo;
#ifndef QT_NO_SQL
    dbInfo = MetaDataBase::fakeProperty( o, "database" ).toStringList().join(".");
#endif

    QString name = o->name();
    if ( ::qt_cast<Q3WidgetStack*>(o->parent()) ) {
	if ( ::qt_cast<QTabWidget*>(o->parent()->parent()) )
	    name = ( (QTabWidget*)o->parent()->parent() )->tabLabel( (QWidget*)o );
	else if ( ::qt_cast<Q3Wizard*>(o->parent()->parent()) )
	    name = ( (Q3Wizard*)o->parent()->parent() )->title( (QWidget*)o );
    }

    QToolBox *tb;
    if ( o->parent() && o->parent()->parent() &&
	 (tb = ::qt_cast<QToolBox*>(o->parent()->parent()->parent())) )
	name = tb->itemLabel( tb->indexOf((QWidget*)o) );

    if ( fakeMainWindow ) {
	name = o->parent()->name();
	className = "QMainWindow";
    }

    if ( !parent )
	item = new HierarchyItem( HierarchyItem::Widget, this, 0, name, className, dbInfo );
    else
	item = new HierarchyItem( HierarchyItem::Widget, parent, 0, name, className, dbInfo );
    item->setOpen( TRUE );
    if ( !parent )
	item->setPixmap( 0, DesignerFormPix );
    else if ( ::qt_cast<QLayoutWidget*>(o) )
	item->setPixmap( 0, DesignerLayoutPix );
    else
	item->setPixmap( 0, WidgetDatabase::iconSet(
		    WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( o ) ) ).
			 pixmap( QIcon::Small, QIcon::Normal ) );
    if ( ::qt_cast<QAction*>(o) )
	item->setPixmap( 0, ( (QAction*)o )->iconSet().pixmap() );

    ( (HierarchyItem*)item )->setObject( o );
    const QObjectList *l = o->children();
    if ( ::qt_cast<QDesignerToolBar*>(o) )
	l = 0;
    if ( l ) {
	QObjectListIt it( *l );
	it.toLast();
	for ( ; it.current(); --it ) {
	    if ( !it.current()->isWidgetType() ||
		 ( (QWidget*)it.current() )->isHidden() )
		continue;
	    if ( !formWindow->widgets()->find( (QWidget*)it.current() ) ) {
		if ( ::qt_cast<Q3WidgetStack*>(it.current()->parent()) ||
		     ::qt_cast<Q3WidgetStack*>(it.current()) ) {
		    QObject *obj = it.current();
		    QDesignerTabWidget *tw = ::qt_cast<QDesignerTabWidget*>(it.current()->parent());
		    QDesignerWizard *dw = ::qt_cast<QDesignerWizard*>(it.current()->parent());
		    Q3WidgetStack *stack = 0;
		    if ( dw || tw || ::qt_cast<Q3WidgetStack*>(obj) )
			stack = (Q3WidgetStack*)obj;
		    else
			stack = (Q3WidgetStack*)obj->parent();
		    if ( widgetStacks->findRef( stack ) != -1 )
			continue;
		    widgetStacks->append( stack );
		    QObjectList *l2 = stack->queryList( "QWidget", 0, TRUE, FALSE );
		    for ( obj = l2->last(); obj; obj = l2->prev() ) {
			if ( qstrcmp( obj->className(),
				      "QWidgetStackPrivate::Invisible" ) == 0 ||
			     ( tw && !tw->tabBar()->tab( stack->id( (QWidget*)obj ) ) ) ||
			     ( dw && dw->isPageRemoved( (QWidget*)obj ) ) )
			    continue;
			if ( qstrcmp( obj->name(), "designer_wizardstack_button" ) == 0 )
			    continue;
			if ( stack->id( (QWidget*)obj ) == -1 )
			    continue;
			insertObject( obj, item );
		    }
		    delete l2;
		} else if ( ::qt_cast<QToolBox*>(it.current()->parent()) ) {
		    if ( !::qt_cast<Q3ScrollView*>(it.current()) )
			continue;
		    QToolBox *tb = (QToolBox*)it.current()->parent();
		    for ( int i = tb->count() - 1; i >= 0; --i )
			insertObject( tb->item( i ), item );
		}
		continue;
	    }
	    insertObject( it.current(), item );
	}
    }

    if ( fakeMainWindow ) {
	QObjectList *l = o->parent()->queryList( "QDesignerToolBar" );
	QObject *obj;
	for ( obj = l->first(); obj; obj = l->next() )
	    insertObject( obj, item );
	delete l;
	l = o->parent()->queryList( "MenuBarEditor" );
	for ( obj = l->first(); obj; obj = l->next() )
	    insertObject( obj, item );
	delete l;
    } else if ( ::qt_cast<QDesignerToolBar*>(o) || ::qt_cast<PopupMenuEditor*>(o) ) {
	Q3PtrList<QAction> actions;
	if ( ::qt_cast<QDesignerToolBar*>(o) )
	    actions = ( (QDesignerToolBar*)o )->insertedActions();
	else
	    ( (PopupMenuEditor*)o )->insertedActions( actions );

	Q3PtrListIterator<QAction> it( actions );
	it.toLast();
	while ( it.current() ) {
	    QAction *a = it.current();
	    if ( ::qt_cast<QDesignerAction*>(a) ) {
		QDesignerAction *da = (QDesignerAction*)a;
		if ( da->supportsMenu() )
		    insertObject( da, item );
		else
		    insertObject( da->widget(), item );
	    } else if ( ::qt_cast<QDesignerActionGroup*>(a) ) {
		insertObject( a, item );
	    }
	    --it;
	}
    } else if ( ::qt_cast<QDesignerActionGroup*>(o) && o->children() ) {
	QObjectList *l = (QObjectList*)o->children();
	for ( QObject *obj = l->last(); obj; obj = l->prev() ) {
	    if ( ::qt_cast<QDesignerAction*>(obj) ) {
		QDesignerAction *da = (QDesignerAction*)obj;
		if ( da->supportsMenu() )
		    insertObject( da, item );
		else
		    insertObject( da->widget(), item );
	    } else if ( ::qt_cast<QDesignerActionGroup*>(obj) ) {
		insertObject( obj, item );
	    }
	}
    } else if ( ::qt_cast<MenuBarEditor*>(o) ) {
	MenuBarEditor *mb = (MenuBarEditor*)o;
	for ( int i = mb->count() -1; i >= 0; --i ) {
	    MenuBarEditorItem *md = mb->item( i );
	    if ( !md || !md->menu() )
		continue;
	    insertObject( md->menu(), item );
	}
    }
}

void HierarchyList::setCurrent( QObject *o )
{
    Q3ListViewItemIterator it( this );
    while ( it.current() ) {
	if ( ( (HierarchyItem*)it.current() )->object() == o ) {
	    blockSignals( TRUE );
	    setCurrentItem( it.current() );
	    ensureItemVisible( it.current() );
	    blockSignals( FALSE );
	    return;
	}
	++it;
    }
}

void HierarchyList::showRMBMenu( Q3ListViewItem *i, const QPoint & p )
{
    if ( !i )
	return;

    QObject *o = findObject( i );
    if ( !o )
	return;

    if ( !o->isWidgetType() ||
	 ( o != formWindow && !formWindow->widgets()->find( (QWidget*)o ) ) )
	return;

    QWidget *w = (QWidget*)o;
    if ( w->isVisibleTo( formWindow ) ) {
	if ( !::qt_cast<QTabWidget*>(w) && !::qt_cast<Q3Wizard*>(w) ) {
	    if ( !normalMenu )
		normalMenu = formWindow->mainWindow()->setupNormalHierarchyMenu( this );
	    normalMenu->popup( p );
	} else {
	    if ( !tabWidgetMenu )
		tabWidgetMenu =
		    formWindow->mainWindow()->setupTabWidgetHierarchyMenu(
				  this, SLOT( addTabPage() ),
				  SLOT( removeTabPage() ) );
	    tabWidgetMenu->popup( p );
	}
    }
}

void HierarchyList::addTabPage()
{
    QObject *o = current();
    if ( !o || !o->isWidgetType() )
	return;
    QWidget *w = (QWidget*)o;
    if ( ::qt_cast<QTabWidget*>(w) ) {
	QTabWidget *tw = (QTabWidget*)w;
	AddTabPageCommand *cmd = new AddTabPageCommand( i18n( "Add Page to %1" ).
							arg( tw->name() ), formWindow,
							tw, "Tab" );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else if ( ::qt_cast<Q3Wizard*>(w) ) {
	Q3Wizard *wiz = (Q3Wizard*)formWindow->mainContainer();
	AddWizardPageCommand *cmd = new AddWizardPageCommand( i18n( "Add Page to %1" ).
							      arg( wiz->name() ), formWindow,
							      wiz, "Page" );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void HierarchyList::removeTabPage()
{
    QObject *o = current();
    if ( !o || !o->isWidgetType() )
	return;
    QWidget *w = (QWidget*)o;
    if ( ::qt_cast<QTabWidget*>(w) ) {
	QTabWidget *tw = (QTabWidget*)w;
	if ( tw->currentPage() ) {
	    QDesignerTabWidget *dtw = (QDesignerTabWidget*)tw;
	    DeleteTabPageCommand *cmd =
		new DeleteTabPageCommand( i18n( "Delete Page %1 of %2" ).
					  arg( dtw->pageTitle() ).arg( tw->name() ),
					  formWindow, tw, tw->currentPage() );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    } else if ( ::qt_cast<Q3Wizard*>(w) ) {
	Q3Wizard *wiz = (Q3Wizard*)formWindow->mainContainer();
	if ( wiz->currentPage() ) {
	    QDesignerWizard *dw = (QDesignerWizard*)wiz;
	    DeleteWizardPageCommand *cmd =
		new DeleteWizardPageCommand( i18n( "Delete Page %1 of %2" ).
					     arg( dw->pageTitle() ).arg( wiz->name() ),
					     formWindow, wiz,
					     wiz->indexOf( wiz->currentPage() ), TRUE );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    }
}

// ------------------------------------------------------------

FormDefinitionView::FormDefinitionView( QWidget *parent, FormWindow *fw )
    : HierarchyList( parent, fw, TRUE )
{
    header()->hide();
    removeColumn( 1 );
    connect( this, SIGNAL( itemRenamed( Q3ListViewItem *, int, const QString & ) ),
	     this, SLOT( renamed( Q3ListViewItem * ) ) );
    popupOpen = FALSE;
}

void FormDefinitionView::setup()
{
    if ( popupOpen || !formWindow )
	return;
    if ( !formWindow->project()->isCpp() )
	return;
    Q3ListViewItem *i = firstChild();
    while ( i ) {
	if ( i->rtti() == HierarchyItem::DefinitionParent ) {
	    Q3ListViewItem *a = i;
	    i = i->nextSibling();
	    delete a;
	    continue;
	}
	i = i->nextSibling();
    }

    LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( lIface ) {
	QStringList defs = lIface->definitions();
	for ( QStringList::Iterator dit = defs.begin(); dit != defs.end(); ++dit ) {
	    HierarchyItem *itemDef = new HierarchyItem( HierarchyItem::DefinitionParent, this, 0,
							i18n( *dit ), QString::null, QString::null );
	    itemDef->setPixmap( 0, DesignerFolderPix );
	    itemDef->setOpen( TRUE );
	    QStringList entries =
		lIface->definitionEntries( *dit, formWindow->mainWindow()->designerInterface() );
	    HierarchyItem *item = 0;
	    for ( QStringList::Iterator eit = entries.begin(); eit != entries.end(); ++eit ) {
		item = new HierarchyItem( HierarchyItem::Definition,
					  itemDef, item, *eit, QString::null, QString::null );
		item->setRenameEnabled( 0, TRUE );
	    }
	}
	lIface->release();
    }
    setupVariables();
    refresh();
}

void FormDefinitionView::setupVariables()
{
    bool pubOpen, protOpen, privOpen;
    pubOpen = protOpen = privOpen = TRUE;
    Q3ListViewItem *i = firstChild();
    while ( i ) {
	if ( i->rtti() == HierarchyItem::VarParent ) {
	    Q3ListViewItem *a = i;
	    i = i->firstChild();
	    while ( i ) {
		if ( i->rtti() == HierarchyItem::VarPublic )
		    pubOpen = i->isOpen();
		else if ( i->rtti() == HierarchyItem::VarProtected )
		    protOpen = i->isOpen();
		else if ( i->rtti() == HierarchyItem::VarPrivate )
		    privOpen = i->isOpen();
		i = i->nextSibling();
	    }
	    delete a;
	    break;
	}
	i = i->nextSibling();
    }

    HierarchyItem *itemVar = new HierarchyItem( HierarchyItem::VarParent, this, 0, i18n( "Class Variables" ),
						QString::null, QString::null );
    itemVar->setPixmap( 0, DesignerFolderPix );
    itemVar->setOpen( TRUE );

    itemVarPriv = new HierarchyItem( HierarchyItem::VarPrivate, itemVar, 0, i18n( "private" ),
				     QString::null, QString::null );
    itemVarProt = new HierarchyItem( HierarchyItem::VarProtected, itemVar, 0, i18n( "protected" ),
				     QString::null, QString::null );
    itemVarPubl = new HierarchyItem( HierarchyItem::VarPublic, itemVar, 0, i18n( "public" ),
				     QString::null, QString::null );

    Q3ValueList<MetaDataBase::Variable> varList = MetaDataBase::variables( formWindow );
    Q3ValueList<MetaDataBase::Variable>::Iterator it = --( varList.end() );
    if ( !varList.isEmpty() && itemVar ) {
	for (;;) {
	    Q3ListViewItem *item = 0;
	    if ( (*it).varAccess == "public" )
		item = new HierarchyItem( HierarchyItem::Variable, itemVarPubl, 0, (*it).varName,
					  QString::null, QString::null );
	    else if ( (*it).varAccess == "private" )
		item = new HierarchyItem( HierarchyItem::Variable, itemVarPriv, 0, (*it).varName,
					  QString::null, QString::null );
	    else // default is protected
		item = new HierarchyItem( HierarchyItem::Variable, itemVarProt, 0, (*it).varName,
					  QString::null, QString::null );
	    item->setPixmap( 0, DesignerEditSlotsPix );
	    if ( it == varList.begin() )
		break;
	    --it;
	}
    }
    itemVar->setOpen( TRUE );
    itemVarPriv->setOpen( privOpen );
    itemVarProt->setOpen( protOpen );
    itemVarPubl->setOpen( pubOpen );
}

void FormDefinitionView::refresh()
{
    if ( popupOpen || !formWindow || !formWindow->project()->isCpp() )
	return;

    bool fuPub, fuProt, fuPriv, slPub, slProt, slPriv;
    fuPub = fuProt = fuPriv = slPub = slProt = slPriv = TRUE;
    Q3ListViewItem *i = firstChild();
    while ( i ) {
        if ( i->rtti() == HierarchyItem::SlotParent ||
	     i->rtti() == HierarchyItem::FunctParent ) {
	    Q3ListViewItem *a = i;
	    i = i->firstChild();
	    while ( i ) {
		switch( i->rtti() ) {
		case HierarchyItem::FunctPublic:
		    fuPub = i->isOpen();
		case HierarchyItem::FunctProtected:
		    fuProt = i->isOpen();
		    break;
		case HierarchyItem::FunctPrivate:
		    fuPriv = i->isOpen();
		    break;
		case HierarchyItem::SlotPublic:
		    slPub = i->isOpen();
		    if ( slPub )
		    break;
		case HierarchyItem::SlotProtected:
		    slProt = i->isOpen();
		    break;
		case HierarchyItem::SlotPrivate:
		    slPriv = i->isOpen();
		}
		i = i->nextSibling();
	    }
	    i = a->nextSibling();
	    delete a;
	    continue;
	}
	i = i->nextSibling();
    }


    itemFunct = new HierarchyItem( HierarchyItem::FunctParent,
				   this, 0, i18n( "Functions" ), QString::null, QString::null );
    itemFunct->moveItem( i );
    itemFunct->setPixmap( 0, DesignerFolderPix );
    itemFunctPriv = new HierarchyItem( HierarchyItem::FunctPrivate, itemFunct, 0,
				       i18n( "private" ), QString::null, QString::null );
    itemFunctProt = new HierarchyItem( HierarchyItem::FunctProtected, itemFunct, 0,
				       i18n( "protected" ), QString::null, QString::null );
    itemFunctPubl = new HierarchyItem( HierarchyItem::FunctPublic, itemFunct, 0,
				       i18n( "public" ), QString::null, QString::null );

    itemSlots = new HierarchyItem( HierarchyItem::SlotParent,
				   this, 0, i18n( "Slots" ), QString::null, QString::null );
    itemSlots->setPixmap( 0, DesignerFolderPix );
    itemPrivate = new HierarchyItem( HierarchyItem::SlotPrivate, itemSlots, 0, i18n( "private" ),
				     QString::null, QString::null );
    itemProtected = new HierarchyItem( HierarchyItem::SlotProtected, itemSlots, 0, i18n( "protected" ),
				       QString::null, QString::null );
    itemPublic = new HierarchyItem( HierarchyItem::SlotPublic, itemSlots, 0, i18n( "public" ),
				    QString::null, QString::null );

    Q3ValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow );
    Q3ValueList<MetaDataBase::Function>::Iterator it = --( functionList.end() );
    if ( !functionList.isEmpty() && itemFunct ) {
	for (;;) {
	    Q3ListViewItem *item = 0;
	    if ( (*it).type == "slot" ) {
		if ( (*it).access == "protected" )
		    item = new HierarchyItem( HierarchyItem::Slot, itemProtected, 0, (*it).function,
					      QString::null, QString::null );
		else if ( (*it).access == "private" )
		    item = new HierarchyItem( HierarchyItem::Slot, itemPrivate, 0, (*it).function,
					      QString::null, QString::null );
		else // default is public
		    item = new HierarchyItem( HierarchyItem::Slot, itemPublic, 0, (*it).function,
					      QString::null, QString::null );
	    } else {
		if ( (*it).access == "protected" )
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctProt, 0, (*it).function,
					      QString::null, QString::null );
		else if ( (*it).access == "private" )
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctPriv, 0, (*it).function,
					      QString::null, QString::null );
		else // default is public
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctPubl, 0, (*it).function,
					      QString::null, QString::null );
	    }
	    item->setPixmap( 0, DesignerEditSlotsPix );
	    if ( it == functionList.begin() )
		break;
	    --it;
	}
    }

    itemFunct->setOpen( TRUE );
    itemFunctPubl->setOpen( fuPub );
    itemFunctProt->setOpen( fuProt );
    itemFunctPriv->setOpen( fuPriv );

    itemSlots->setOpen( TRUE );
    itemPublic->setOpen( slPub );
    itemProtected->setOpen( slProt );
    itemPrivate->setOpen( slPriv );
}


void FormDefinitionView::setCurrent( QWidget * )
{
}

void FormDefinitionView::objectClicked( Q3ListViewItem *i )
{
    if ( !i )
	return;
    if ( (i->rtti() == HierarchyItem::Slot) || (i->rtti() == HierarchyItem::Function) )
    {
        formWindow->clearSelection(false);
        formWindow->mainWindow()->part()->emitEditFunction(formWindow->fileName(), i->text( 0 ));
    }
//	formWindow->mainWindow()->editFunction( i->text( 0 ) );
}

static HierarchyItem::Type getChildType( int type )
{
    switch ( (HierarchyItem::Type)type ) {
    case HierarchyItem::Widget:
	qWarning( "getChildType: Inserting childs dynamically to Widget or SlotParent is not allowed!" );
	break;
    case HierarchyItem::SlotParent:
    case HierarchyItem::SlotPublic:
    case HierarchyItem::SlotProtected:
    case HierarchyItem::SlotPrivate:
    case HierarchyItem::Slot:
	return HierarchyItem::Slot;
    case HierarchyItem::DefinitionParent:
    case HierarchyItem::Definition:
	return HierarchyItem::Definition;
    case HierarchyItem::Event:
    case HierarchyItem::EventFunction:
	return HierarchyItem::Event;
    case HierarchyItem::FunctParent:
    case HierarchyItem::FunctPublic:
    case HierarchyItem::FunctProtected:
    case HierarchyItem::FunctPrivate:
    case HierarchyItem::Function:
	return HierarchyItem::Function;
    case HierarchyItem::VarParent:
    case HierarchyItem::VarPublic:
    case HierarchyItem::VarProtected:
    case HierarchyItem::VarPrivate:
    case HierarchyItem::Variable:
	return HierarchyItem::Variable;
    }
    return (HierarchyItem::Type)type;
}

void HierarchyList::insertEntry( Q3ListViewItem *i, const QPixmap &pix, const QString &s )
{
    Q3ListViewItem *after = i->firstChild();
    while ( after && after->nextSibling() )
	after = after->nextSibling();
    HierarchyItem *item = new HierarchyItem( getChildType( i->rtti() ), i, after, s,
					     QString::null, QString::null );
    if ( !pix.isNull() )
	item->setPixmap( 0, pix );
    item->setRenameEnabled( 0, TRUE );
    setCurrentItem( item );
    ensureItemVisible( item );
    qApp->processEvents();
    newItem = item;
    item->startRename( 0 );
}

void FormDefinitionView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    Q3ListViewItem *i = itemAt( contentsToViewport( e->pos() ) );
    if ( !i )
	return;

    if ( i->rtti() == HierarchyItem::SlotParent || i->rtti() == HierarchyItem::FunctParent ||
	 i->rtti() == HierarchyItem::VarParent )
	return;

    HierarchyItem::Type t = getChildType( i->rtti() );
    if ( (int)t == i->rtti() )
	i = i->parent();

    if ( formWindow->project()->isCpp() )
	switch( i->rtti() ) {
	case HierarchyItem::FunctPublic:
	    execFunctionDialog( "public", "function", TRUE );
	    break;
	case HierarchyItem::FunctProtected:
	    execFunctionDialog( "protected", "function", TRUE );
	    break;
	case HierarchyItem::FunctPrivate:
	    execFunctionDialog( "private", "function", TRUE );
	    break;
	case HierarchyItem::SlotPublic:
	    execFunctionDialog( "public", "slot", TRUE );
	    break;
	case HierarchyItem::SlotProtected:
	    execFunctionDialog( "protected", "slot", TRUE );
	    break;
	case HierarchyItem::SlotPrivate:
	    execFunctionDialog( "private", "slot", TRUE );
	    break;
	case HierarchyItem::VarPublic:
	case HierarchyItem::VarProtected:
	case HierarchyItem::VarPrivate: {
	    VariableDialog varDia( formWindow, this );
	    Q3ListViewItem *i = selectedItem();
	    if ( i )
		varDia.setCurrentItem( i->text( 0 ) );
	    varDia.exec();
	    break;
	}
	default:
	    insertEntry( i );
    } else
	insertEntry( i );
}

void FormDefinitionView::execFunctionDialog( const QString &access, const QString &type, bool addFunc )
{
    FormFile *formFile = formWindow->formFile();
    if ( !formFile || !formFile->isUihFileUpToDate() )
	    return;

    // refresh the functions list in the metadatabase
    SourceEditor *editor = formFile->editor();
    if ( editor )
	editor->refresh( TRUE );

    EditFunctions dlg( this, formWindow );
    if ( addFunc )
	dlg.functionAdd( access, type );
    dlg.exec();
}

void FormDefinitionView::showRMBMenu( Q3ListViewItem *i, const QPoint &pos )
{
    if ( !i )
	return;

    const int EDIT = 1;
    const int NEW = 2;
    const int DEL = 3;
    const int PROPS = 4;
    const int GOIMPL = 5;

    Q3PopupMenu menu;
    bool insertDelete = FALSE;

    if ( i->rtti() == HierarchyItem::FunctParent || i->rtti() == HierarchyItem::SlotParent ||
	 i->rtti() == HierarchyItem::VarParent ) {
	menu.insertItem( SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()), i18n( "Edit..." ), EDIT );
    } else
	menu.insertItem( SmallIcon( "designer_filenew.png" , KDevDesignerPartFactory::instance()), i18n( "New..." ), NEW );
    if ( i->rtti() == HierarchyItem::DefinitionParent || i->rtti() == HierarchyItem::Variable ||
	 i->rtti() == HierarchyItem::Definition ) {
	menu.insertItem( SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()), i18n( "Edit..." ), EDIT );
    }
    if ( i->rtti() == HierarchyItem::Function || i->rtti() == HierarchyItem::Slot ) {
	if ( formWindow->project()->isCpp() )
	    menu.insertItem( SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()), i18n( "Properties" ), PROPS );
	if ( MetaDataBase::hasEditor( formWindow->project()->language() ) )
	    menu.insertItem( i18n( "Goto Implementation" ), GOIMPL );
	insertDelete = TRUE;
    }
    if ( insertDelete || i->rtti() == HierarchyItem::Variable ||
	 i->rtti() == HierarchyItem::Function || i->rtti() == HierarchyItem::Slot ||
	 i->rtti() == HierarchyItem::Definition ) {
	menu.insertSeparator();
	menu.insertItem( SmallIcon( "designer_editcut.png" , KDevDesignerPartFactory::instance()), i18n( "Delete" ), DEL );
    }
    popupOpen = TRUE;
    int res = menu.exec( pos );
    popupOpen = FALSE;
    if ( res == -1 )
	return;

    if ( res == EDIT ) {
	switch( i->rtti() ) {
	case HierarchyItem::FunctParent:
	    execFunctionDialog( "public", "function", FALSE );
	    break;
	case HierarchyItem::SlotParent:
	    execFunctionDialog( "public", "slot", FALSE );
	    break;
	case HierarchyItem::VarParent:
	case HierarchyItem::VarPublic:
	case HierarchyItem::VarProtected:
	case HierarchyItem::VarPrivate:
	case HierarchyItem::Variable: {
	    VariableDialog varDia( formWindow, this );
	    Q3ListViewItem *i = selectedItem();
	    if ( i )
		varDia.setCurrentItem( i->text( 0 ) );
	    if ( varDia.exec() == QDialog::Accepted )
		formWindow->commandHistory()->setModified( TRUE );
	    break;
	}
	case HierarchyItem::Definition:
	case HierarchyItem::DefinitionParent:
	    LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
	    if ( !lIface )
		return;
	    if ( i->rtti() == HierarchyItem::Definition )
		i = i->parent();
	    ListEditor dia( this, 0, TRUE );
	    dia.setCaption( i18n( "Edit %1" ).arg( i->text( 0 ) ) );
	    QStringList entries = lIface->definitionEntries( i->text( 0 ), MainWindow::self->designerInterface() );
	    dia.setList( entries );
	    dia.exec();
	    Command *cmd = new EditDefinitionsCommand( i18n( "Edit %1" ).arg( i->text( 0 )), formWindow,
						       lIface, i->text( 0 ), dia.items() );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    } else if ( res == NEW ) {
	HierarchyItem::Type t = getChildType( i->rtti() );
	if ( (int)t == i->rtti() )
	    i = i->parent();
	switch( i->rtti() ) {
	case HierarchyItem::SlotPublic:
	    execFunctionDialog( "public", "slot", TRUE );
	    break;
	case HierarchyItem::SlotProtected:
	    execFunctionDialog( "protected", "slot", TRUE );
	    break;
	case HierarchyItem::SlotPrivate:
	    execFunctionDialog( "private" , "slot", TRUE );
	    break;
	case HierarchyItem::FunctPublic:
	    execFunctionDialog( "public", "function", TRUE );
	    break;
	case HierarchyItem::FunctProtected:
	    execFunctionDialog( "protected", "function", TRUE );
	    break;
	case HierarchyItem::FunctPrivate:
	    execFunctionDialog( "private" , "function", TRUE );
	    break;
    	default:
	    insertEntry( i );
	}
    } else if ( res == DEL ) {
	if ( i->rtti() == HierarchyItem::Slot || i->rtti() == HierarchyItem::Function ) {

	    Q3CString funct( MetaDataBase::normalizeFunction( i->text( 0 ) ).latin1() );
	    Command *cmd = new RemoveFunctionCommand( i18n( "Remove Function" ), formWindow, funct,
						     QString::null, QString::null, QString::null,
						     QString::null, formWindow->project()->language() );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
	} else if ( i->rtti() == HierarchyItem::Variable ) {
	    Command *cmd = new RemoveVariableCommand( i18n( "Remove Variable" ), formWindow,
						      i->text( 0 ) );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else {
	    Q3ListViewItem *p = i->parent();
	    delete i;
	    save( p, 0 );
	}
    } else if ( res == PROPS ) {
	if ( i->rtti() == HierarchyItem::Slot ||
	     i->rtti() == HierarchyItem::Function ) {
	    EditFunctions dlg( this, formWindow );
	    dlg.setCurrentFunction( MetaDataBase::normalizeFunction( i->text( 0 ) ) );
	    dlg.exec();
	}
    } else if ( res == GOIMPL ) {
	if ( i->rtti() == HierarchyItem::Slot ||
	     i->rtti() == HierarchyItem::Function ) {
	     formWindow->clearSelection(false);
	     formWindow->mainWindow()->part()->emitEditFunction(formWindow->fileName(), i->text( 0 ));
//	    formWindow->mainWindow()->editFunction( i->text( 0 ) );
	}
    }
}

void FormDefinitionView::renamed( Q3ListViewItem *i )
{
    if ( newItem == i )
	newItem = 0;
    if ( !i->parent() )
	return;
    save( i->parent(), i );
}


void FormDefinitionView::save( Q3ListViewItem *p, Q3ListViewItem *i )
{
    if ( i && i->text( 0 ).isEmpty() ) {
	delete i;
	return;
    }

    if ( i && i->rtti() == HierarchyItem::Variable ) {
	i->setRenameEnabled( 0, FALSE );
	QString varName = i->text( 0 );
	varName = varName.simplifyWhiteSpace();
	if ( varName[(int)varName.length() - 1] != ';' )
	    varName += ";";
	if ( MetaDataBase::hasVariable( formWindow, varName ) ) {
	    QMessageBox::information( this, i18n( "Edit Variables" ),
				      i18n( "This variable has already been declared." ) );
	} else {
	    if ( p->rtti() == HierarchyItem::VarPublic )
		addVariable( varName, "public" );
	    else if ( p->rtti() == HierarchyItem::VarProtected )
		addVariable( varName, "protected" );
	    else if ( p->rtti() == HierarchyItem::VarPrivate )
		addVariable( varName, "private" );
	}
    } else {
	LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
	if ( !lIface )
	    return;
	QStringList lst;
	i = p->firstChild();
	while ( i ) {
	    lst << i->text( 0 );
	    i = i->nextSibling();
	}
	Command *cmd = new EditDefinitionsCommand( i18n( "Edit %1" ).arg( p->text( 0 ) ), formWindow,
						   lIface, p->text( 0 ), lst );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void FormDefinitionView::addVariable( const QString &varName, const QString &access )
{
    Command *cmd = new AddVariableCommand( i18n( "Add Variable" ), formWindow,
					      varName, access );
    formWindow->commandHistory()->addCommand( cmd );
    cmd->execute();
}

// ------------------------------------------------------------

HierarchyView::HierarchyView( QWidget *parent )
    : QTabWidget( parent, 0, Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_Title |
		  Qt::WStyle_Tool |Qt::WStyle_MinMax | Qt::WStyle_SysMenu )
{
    formwindow = 0;
    editor = 0;
    listview = new HierarchyList( this, formWindow() );
    fView = new FormDefinitionView( this, formWindow() );
    if ( !MainWindow::self->singleProjectMode() ) {
	addTab( listview, i18n( "Objects" ) );
	setTabToolTip( listview, i18n( "List of all widgets and objects of the current form in hierarchical order" ) );
	addTab( fView, i18n( "Members" ) );
	setTabToolTip( fView, i18n( "List of all members of the current form" ) );
    } else {
	listview->hide();
	fView->hide();
    }

    if ( !classBrowserInterfaceManager ) {
	classBrowserInterfaceManager =
	    new QPluginManager<ClassBrowserInterface>( IID_ClassBrowser, QApplication::libraryPaths(),
						       MainWindow::self->pluginDirectory() );
    }

    classBrowsers = new QMap<QString, ClassBrowser>();
    QStringList langs = MetaDataBase::languages();
    for ( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
	QInterfacePtr<ClassBrowserInterface> ciface = 0;
	classBrowserInterfaceManager->queryInterface( *it, &ciface );
	if ( ciface ) {
	    ClassBrowser cb( ciface->createClassBrowser( this ), ciface );
	    addTab( cb.lv, i18n( "Class Declarations" ) );
	    setTabToolTip( cb.lv, i18n( "List of all classes and its declarations of the current source file" ) );
	    ciface->onClick( this, SLOT( jumpTo( const QString &, const QString &, int ) ) );
	    classBrowsers->insert( *it, cb );
	    setTabEnabled( cb.lv, FALSE );
	}
    }
}

HierarchyView::~HierarchyView()
{
}

void HierarchyView::clear()
{
    listview->clear();
    fView->clear();
    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	(*it).iface->clear();
    }
}

void HierarchyView::setFormWindow( FormWindow *fw, QObject *o )
{
    bool fake = fw && qstrcmp( fw->name(), "qt_fakewindow" ) == 0;
    if ( fw == 0 || o == 0 ) {
	listview->clear();
	fView->clear();
	listview->setFormWindow( fw );
	fView->setFormWindow( fw );
	formwindow = 0;
	editor = 0;
    }

    setTabEnabled( listview, TRUE );
    setTabEnabled( fView, fw && fw->project()->isCpp() );

    if ( fw == formwindow ) {
	if ( fw ) {
	    if ( !fake )
		listview->setCurrent( (QWidget*)o );
	    else
		listview->clear();
	    if ( MainWindow::self->qWorkspace()->activeWindow() == fw )
		showPage( listview );
	    else if ( fw->project()->isCpp() )
		showPage( fView );
	    else
		showClasses( fw->formFile()->editor() );
	}
    }

    formwindow = fw;
    if ( !fake ) {
	listview->setFormWindow( fw );
    } else {
	listview->setFormWindow( 0 );
	listview->clear();
    }

    fView->setFormWindow( fw );
    if ( !fake ) {
	listview->setup();
	listview->setCurrent( (QWidget*)o );
    }
    fView->setup();

    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	(*it).iface->clear();
	setTabEnabled( (*it).lv, fw && !fw->project()->isCpp() );
    }

    if ( MainWindow::self->qWorkspace()->activeWindow() == fw )
	showPage( listview );
    else if ( fw && fw->project()->isCpp() )
	showPage( fView );
    else if ( fw )
	showClasses( fw->formFile()->editor() );

    editor = 0;
}

void HierarchyView::showClasses( SourceEditor *se )
{
    if ( !se->object() )
	return;

    lastSourceEditor = se;
    QTimer::singleShot( 100, this, SLOT( showClassesTimeout() ) );
}

void HierarchyView::showClassesTimeout()
{
    if ( !lastSourceEditor )
	return;
    SourceEditor *se = (SourceEditor*)lastSourceEditor;
    if ( !se->object() )
	return;
    if ( se->formWindow() && se->formWindow()->project()->isCpp() ) {
	setFormWindow( se->formWindow(), se->formWindow()->currentWidget() );
	MainWindow::self->propertyeditor()->setWidget( se->formWindow()->currentWidget(),
						       se->formWindow() );
	return;
    }

    setTabEnabled( listview, !!se->formWindow() && !se->formWindow()->isFake() );
    setTabEnabled( fView, FALSE );

    formwindow = 0;
    listview->setFormWindow( 0 );
    fView->setFormWindow( 0 );
    listview->clear();
    fView->clear();
    if ( !se->formWindow() )
	MainWindow::self->propertyeditor()->setWidget( 0, 0 );
    editor = se;

    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	if ( it.key() == se->project()->language() ) {
	    (*it).iface->update( se->text() );
	    setTabEnabled( (*it).lv, TRUE );
	    showPage( (*it).lv );
	} else {
	    setTabEnabled( (*it).lv, FALSE );
	    (*it).iface->clear();
	}
    }
}

void HierarchyView::updateClassBrowsers()
{
    if ( !editor )
	return;
    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	if ( it.key() == editor->project()->language() )
	    (*it).iface->update( editor->text() );
	else
	    (*it).iface->clear();
    }
}

FormWindow *HierarchyView::formWindow() const
{
    return formwindow;
}

void HierarchyView::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void HierarchyView::widgetInserted( QWidget * )
{
    listview->setup();
}

void HierarchyView::widgetRemoved( QWidget * )
{
    listview->setup();
}

void HierarchyView::widgetsInserted( const QWidgetList & )
{
    listview->setup();
}

void HierarchyView::widgetsRemoved( const QWidgetList & )
{
    listview->setup();
}

void HierarchyView::namePropertyChanged( QWidget *w, const QVariant & )
{
    QWidget *w2 = w;
    if ( ::qt_cast<Q3MainWindow*>(w) )
	w2 = ( (Q3MainWindow*)w )->centralWidget();
    listview->changeNameOf( w2, w->name() );
}


void HierarchyView::databasePropertyChanged( QWidget *w, const QStringList& info )
{
#ifndef QT_NO_SQL
    QString i = info.join( "." );
    listview->changeDatabaseOf( w, i );
#endif
}


void HierarchyView::tabsChanged( QTabWidget * )
{
    listview->setup();
}

void HierarchyView::pagesChanged( Q3Wizard * )
{
    listview->setup();
}

void HierarchyView::rebuild()
{
    listview->setup();
}

void HierarchyView::closed( FormWindow *fw )
{
    if ( fw == formwindow ) {
	listview->clear();
	fView->clear();
    }
}

void HierarchyView::updateFormDefinitionView()
{
    fView->setup();
}

void HierarchyView::jumpTo( const QString &func, const QString &clss, int type )
{
    if ( !editor )
	return;
    if ( type == ClassBrowserInterface::Class )
	editor->setClass( func );
    else
	editor->setFunction( func, clss );
}

HierarchyView::ClassBrowser::ClassBrowser( Q3ListView *l, ClassBrowserInterface *i )
    : lv( l ), iface( i )
{
}

HierarchyView::ClassBrowser::~ClassBrowser()
{
}

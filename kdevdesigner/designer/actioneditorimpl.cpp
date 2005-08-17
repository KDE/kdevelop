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

#include "actioneditorimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "actionlistview.h"
#include "connectiondialog.h"
#include "mainwindow.h"
#include "hierarchyview.h"
#include "formfile.h"

#include <qaction.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <q3listview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <q3popupmenu.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3ActionGroup>
#include <QCloseEvent>

#include <klocale.h>

ActionEditor::ActionEditor( QWidget* parent,  const char* name, Qt::WFlags fl )
    : ActionEditorBase( parent, name, fl ), currentAction( 0 ), formWindow( 0 ),
    explicitlyClosed(false)
{
    listActions->addColumn( i18n( "Actions" ) );
    setEnabled( FALSE );
    buttonConnect->setEnabled( FALSE );

    Q3PopupMenu *popup = new Q3PopupMenu( this );
    popup->insertItem( i18n( "New &Action" ), this, SLOT( newAction() ) );
    popup->insertItem( i18n( "New Action &Group" ), this, SLOT( newActionGroup() ) );
    popup->insertItem( i18n( "New &Dropdown Action Group" ), this, SLOT( newDropDownActionGroup() ) );
    buttonNewAction->setPopup( popup );
    buttonNewAction->setPopupDelay( 0 );

    connect( listActions, SIGNAL( insertAction() ), this, SLOT( newAction() ) );
    connect( listActions, SIGNAL( insertActionGroup() ), this, SLOT( newActionGroup() ) );
    connect( listActions, SIGNAL( insertDropDownActionGroup() ), this, SLOT( newDropDownActionGroup() ) );
    connect( listActions, SIGNAL( deleteAction() ), this, SLOT( deleteAction() ) );
    connect( listActions, SIGNAL( connectAction() ), this, SLOT( connectionsClicked() ) );
}

void ActionEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void ActionEditor::currentActionChanged( Q3ListViewItem *i )
{
    buttonConnect->setEnabled( i != 0 );
    if ( !i )
	return;
    currentAction = ( (ActionItem*)i )->action();
    if ( !currentAction )
	currentAction = ( (ActionItem*)i )->actionGroup();
    if ( formWindow && currentAction )
	formWindow->setActiveObject( currentAction );
    MainWindow::self->objectHierarchy()->hierarchyList()->setCurrent( currentAction );
}

void ActionEditor::setCurrentAction( QAction *a )
{
    Q3ListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a || ( (ActionItem*)it.current() )->actionGroup() == a ) {
	    listActions->setCurrentItem( it.current() );
	    listActions->ensureItemVisible( it.current() );
	    break;
	}
	++it;
    }
}

QAction *ActionEditor::newActionEx()
{
    ActionItem *i = new ActionItem( listActions, (bool)FALSE );
    QAction *a = i->action();
    QObject::connect( a, SIGNAL( destroyed( QObject * ) ),
		      this, SLOT( removeConnections( QObject* ) ) );
    MetaDataBase::addEntry( i->action() );
    QString n = "Action";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->action()->setName( n );
    i->action()->setText( i->action()->name() );
    MetaDataBase::setPropertyChanged( i->action(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->action(), "name", TRUE );
    formWindow->actionList().append( i->action() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
    return i->action();
}

void ActionEditor::deleteAction()
{
    if ( !currentAction )
	return;

    Q3ListViewItemIterator it( listActions );
    ActionItem *ai = 0;
    while ( it.current() ) {
	ai = (ActionItem*)it.current();
	if ( ai->action() == currentAction || ai->actionGroup() == currentAction ) {
	    emit removing( currentAction );
	    formWindow->actionList().removeRef( currentAction );
	    delete currentAction;
	    currentAction = 0;
	    delete it.current();
	    break;
	}
	++it;
    }
    if ( formWindow ) {
	formWindow->setActiveObject( formWindow->mainContainer() );
	if ( formWindow->formFile() )
	    formWindow->formFile()->setModified( TRUE );
    }
}

void ActionEditor::newAction()
{
    ActionItem *actionParent = (ActionItem*)listActions->selectedItem();
    if ( actionParent ) {
	if ( !::qt_cast<Q3ActionGroup*>(actionParent->actionGroup()) )
	    actionParent = (ActionItem*)actionParent->parent();
    }

    ActionItem *i = 0;
    if ( actionParent )
	i = new ActionItem( actionParent );
    else
	i = new ActionItem( listActions, (bool)FALSE );
    QAction *a = i->action();
    QObject::connect( a, SIGNAL( destroyed( QObject * ) ),
		      this, SLOT( removeConnections( QObject* ) ) );
    MetaDataBase::addEntry( i->action() );
    QString n = "Action";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->action()->setName( n );
    i->action()->setText( i->action()->name() );
    if ( actionParent && actionParent->actionGroup() &&
	 actionParent->actionGroup()->usesDropDown() ) {
	i->action()->setToggleAction( TRUE );
	MetaDataBase::setPropertyChanged( i->action(), "toggleAction", TRUE );
    }
    MetaDataBase::setPropertyChanged( i->action(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->action(), "name", TRUE );
    listActions->setCurrentItem( i );
    if ( !actionParent )
	formWindow->actionList().append( i->action() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
}

void ActionEditor::newActionGroup()
{
    ActionItem *actionParent = (ActionItem*)listActions->selectedItem();
    if ( actionParent ) {
	if ( !::qt_cast<Q3ActionGroup*>(actionParent->actionGroup()) )
	    actionParent = (ActionItem*)actionParent->parent();
    }

    ActionItem *i = 0;
    if ( actionParent )
	i = new ActionItem( actionParent, TRUE );
    else
	i = new ActionItem( listActions, TRUE );
    QAction *ag = i->actionGroup();
    QObject::connect( ag, SIGNAL( destroyed( QObject * ) ),
		      this, SLOT( removeConnections( QObject* ) ) );
    MetaDataBase::addEntry( i->actionGroup() );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "usesDropDown", TRUE );
    QString n = "ActionGroup";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->actionGroup()->setName( n );
    i->actionGroup()->setText( i->actionGroup()->name() );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "name", TRUE );
    listActions->setCurrentItem( i );
    i->setOpen( TRUE );
    if ( !actionParent )
	formWindow->actionList().append( i->actionGroup() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
}

void ActionEditor::newDropDownActionGroup()
{
    newActionGroup();
    ( (ActionItem*)listActions->currentItem() )->actionGroup()->setUsesDropDown( TRUE );
}

void ActionEditor::setFormWindow( FormWindow *fw )
{
    listActions->clear();
    formWindow = fw;
    if ( !formWindow ||
	 !::qt_cast<Q3MainWindow*>(formWindow->mainContainer()) ) {
	setEnabled( FALSE );
    } else {
	setEnabled( TRUE );
	for ( QAction *a = formWindow->actionList().first(); a; a = formWindow->actionList().next() ) {
	    ActionItem *i = 0;
	    if ( ::qt_cast<QAction*>(a->parent()) )
		continue;
	    i = new ActionItem( listActions, a );
	    i->setText( 0, a->name() );
	    i->setPixmap( 0, a->iconSet().pixmap() );
	    // make sure we don't duplicate the connection
 	    QObject::disconnect( a, SIGNAL( destroyed( QObject * ) ),
 				 this, SLOT( removeConnections( QObject * ) ) );
	    QObject::connect( a, SIGNAL( destroyed( QObject * ) ),
			      this, SLOT( removeConnections( QObject* ) ) );
	    if ( ::qt_cast<Q3ActionGroup*>(a) ) {
		insertChildActions( i );
	    }
	}
	if ( listActions->firstChild() ) {
	    listActions->setCurrentItem( listActions->firstChild() );
	    listActions->setSelected( listActions->firstChild(), TRUE );
	}
    }
}

void ActionEditor::insertChildActions( ActionItem *i )
{
    if ( !i->actionGroup() || !i->actionGroup()->children() )
	return;
    QObjectListIt it( *i->actionGroup()->children() );
    while ( it.current() ) {
	QObject *o = it.current();
	++it;
	if ( !::qt_cast<QAction*>(o) )
	    continue;
	QAction *a = (QAction*)o;
	ActionItem *i2 = new ActionItem( (Q3ListViewItem*)i, a );
	i->setOpen( TRUE );
	i2->setText( 0, a->name() );
	i2->setPixmap( 0, a->iconSet().pixmap() );
	// make sure we don't duplicate the connection
 	QObject::disconnect( o, SIGNAL( destroyed( QObject * ) ),
 			     this, SLOT( removeConnections( QObject * ) ) );
 	QObject::connect( o, SIGNAL( destroyed( QObject * ) ),
 			  this, SLOT( removeConnections( QObject * ) ) );
	if ( ::qt_cast<Q3ActionGroup*>(a) )
	    insertChildActions( i2 );
    }
}

void ActionEditor::updateActionName( QAction *a )
{
    Q3ListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a )
	    ( (ActionItem*)it.current() )->setText( 0, a->name() );
	else if ( ( (ActionItem*)it.current() )->actionGroup() == a )
	    ( (ActionItem*)it.current() )->setText( 0, a->name() );
	++it;
    }
}

void ActionEditor::updateActionIcon( QAction *a )
{
    Q3ListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a )
	    ( (ActionItem*)it.current() )->setPixmap( 0, a->iconSet().pixmap() );
	else if ( ( (ActionItem*)it.current() )->actionGroup() == a )
	    ( (ActionItem*)it.current() )->setPixmap( 0, a->iconSet().pixmap() );
	++it;
    }
}

void ActionEditor::connectionsClicked()
{
    ConnectionDialog dlg( formWindow->mainWindow() );
    dlg.setDefault( currentAction, formWindow );
    dlg.addConnection();
    dlg.exec();
}

void ActionEditor::removeConnections( QObject *o )
{
    Q3ValueList<MetaDataBase::Connection> conns =
	MetaDataBase::connections( formWindow, o );
    for ( Q3ValueList<MetaDataBase::Connection>::Iterator it2 = conns.begin();
	  it2 != conns.end(); ++it2 )
	MetaDataBase::removeConnection( formWindow, (*it2).sender, (*it2).signal,
					(*it2).receiver, (*it2).slot );
}

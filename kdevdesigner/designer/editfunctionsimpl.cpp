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

#include "editfunctionsimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "asciivalidator.h"
#include "mainwindow.h"
#include "hierarchyview.h"
#include "project.h"

#include <kiconloader.h>
#include <klineedit.h>
#include "kdevdesigner_part.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qstrlist.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qcheckbox.h>

#include <klocale.h>

EditFunctions::EditFunctions( QWidget *parent, FormWindow *fw, bool justSlots )
    : EditFunctionsBase( parent, 0, TRUE ), formWindow( fw )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    id = 0;
    functList.clear();

    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( fw );
    for ( QValueList<MetaDataBase::Function>::Iterator it = functionList.begin(); it != functionList.end(); ++it ) {
	QListViewItem *i = new QListViewItem( functionListView );

	i->setPixmap( 0, SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()) );
	i->setText( 0, (*it).function );
	i->setText( 1, (*it).returnType );
	i->setText( 2, (*it).specifier );
	i->setText( 3, (*it).access  );
	i->setText( 4, (*it).type );

	FunctItem fui;
	fui.id = id;
	fui.oldName = (*it).function;
	fui.newName = fui.oldName;
	fui.oldRetTyp = (*it).returnType;
	fui.retTyp = fui.oldRetTyp;
	fui.oldSpec = (*it).specifier;
	fui.spec = fui.oldSpec;
	fui.oldAccess = (*it).access;
	fui.access = fui.oldAccess;
	fui.oldType = (*it).type;
	fui.type = fui.oldType;
	functList.append( fui );

	functionIds.insert( i, id );
	id++;

	if ( (*it).type == "slot" ) {
	    if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( (*it).function ).latin1() ) )
		i->setText( 5, i18n( "Yes" ) );
	    else
		i->setText( 5, i18n( "No" ) );
	} else {
	    i->setText( 5, "---" );
	}
    }

    boxProperties->setEnabled( FALSE );
    functionName->setValidator( new AsciiValidator( TRUE, functionName ) );

    if ( functionListView->firstChild() )
	functionListView->setCurrentItem( functionListView->firstChild() );

    showOnlySlots->setChecked( justSlots );
    lastType = "function";

    // Enable rename for all QListViewItems
    QListViewItemIterator lvit = functionListView->firstChild();
    for ( ; *lvit; lvit++ )
	(*lvit)->setRenameEnabled( 0, TRUE );

    // Connect listview signal to signal-relay
    QObject::connect( functionListView,
		      SIGNAL( itemRenamed( QListViewItem*, int, const QString & ) ),
		      this,
		      SLOT( emitItemRenamed(QListViewItem*, int, const QString&) ) );

    // Connect signal-relay to QLineEdit "functionName"
    QObjectList *l = parent->queryList( "QLineEdit", "functionName" );
    QObject *obj;
    QObjectListIt itemsLineEditIt( *l );
    while ( (obj = itemsLineEditIt.current()) != 0 ) {
        ++itemsLineEditIt;
	QObject::connect( this,
			  SIGNAL( itemRenamed( const QString & ) ),
			  obj,
			  SLOT( setText( const QString & ) ) );
    }
    delete l;
}

void EditFunctions::okClicked()
{
    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow );
    QString n = i18n( "Add/Remove functions of '%1'" ).arg( formWindow->name() );
    QPtrList<Command> commands;
    QValueList<MetaDataBase::Function>::Iterator fit;
    if ( !functionList.isEmpty() ) {
	for ( fit = functionList.begin(); fit != functionList.end(); ++fit ) {
	    bool functionFound = FALSE;
	    QValueList<FunctItem>::Iterator it = functList.begin();
	    for ( ; it != functList.end(); ++it ) {
		if ( MetaDataBase::normalizeFunction( (*it).oldName ) ==
		     MetaDataBase::normalizeFunction( (*fit).function ) ) {
		    functionFound = TRUE;
		    break;
		}
	    }
	    if ( !functionFound )
		commands.append( new RemoveFunctionCommand( i18n( "Remove Function" ),
							    formWindow, (*fit).function, (*fit).specifier,
							    (*fit).access,
							    (*fit).type,
							    formWindow->project()->language(),
							    (*fit).returnType ) );
	}
    }

    bool invalidFunctions = FALSE;
    QValueList<FunctItem> invalidItems;

    if ( !functList.isEmpty() ) {
	QStrList lst;
	QValueList<FunctItem>::Iterator it = functList.begin();
	for ( ; it != functList.end(); ++it ) {
	    MetaDataBase::Function function;
	    function.function = (*it).newName;
	    function.returnType = (*it).retTyp;
	    function.specifier = (*it).spec;
	    function.access = (*it).access;
	    function.type = (*it).type;
	    function.language = formWindow->project()->language();
	    if ( function.returnType.isEmpty() )
		function.returnType = "void";
	    QString s = function.function;
	    s = s.simplifyWhiteSpace();
	    bool startNum = s[ 0 ] >= '0' && s[ 0 ] <= '9';
	    bool noParens = s.contains( '(' ) != 1 || s.contains( ')' ) != 1;
	    bool illegalSpace = s.find( ' ' ) != -1 && s.find( ' ' ) < s.find( '(' );

	    if ( startNum || noParens || illegalSpace || lst.find( function.function ) != -1 ) {
		invalidFunctions = TRUE;
		invalidItems.append( (*it) );
		continue;
	    }
	    bool functionFound = FALSE;
	    for ( fit = functionList.begin(); fit != functionList.end(); ++fit ) {
		if ( MetaDataBase::normalizeFunction( (*fit).function ) ==
		     MetaDataBase::normalizeFunction( (*it).oldName ) ) {
		    functionFound = TRUE;
		    break;
		}
	    }
	    if ( !functionFound )
		commands.append( new AddFunctionCommand( i18n( "Add Function" ),
							formWindow, function.function, function.specifier,
							function.access,
							function.type, formWindow->project()->language(),
							function.returnType ) );
	    if ( MetaDataBase::normalizeFunction( (*it).newName ) != MetaDataBase::normalizeFunction( (*it).oldName ) ||
		 (*it).spec != (*it).oldSpec || (*it).access != (*it).oldAccess || (*it).type != (*it).oldType ||
		 (*it).retTyp != (*it).oldRetTyp ) {
		QString normalizedOldName = MetaDataBase::normalizeFunction( (*it).oldName );
		if ((*it).oldName.endsWith("const")) // make sure we get the 'const' when we remove the old name
		    normalizedOldName += " const";
		commands.append( new ChangeFunctionAttribCommand( i18n( "Change Function Attributes" ),
								  formWindow, function, normalizedOldName,
								  (*it).oldSpec, (*it).oldAccess, (*it).oldType,
								  formWindow->project()->language(), (*it).oldRetTyp ) );
	    }
	    lst.append( function.function );
	}
    }

    if ( invalidFunctions ) {
	if ( QMessageBox::information( this, i18n( "Edit Functions" ),
				       i18n( "Some syntactically incorrect functions have been defined.\n"
				       "Remove these functions?" ), i18n( "&Yes" ), i18n( "&No" ) ) == 0 ) {
	    QValueList<FunctItem>::Iterator it = functList.begin();
	    while ( it != functList.end() ) {
		bool found = FALSE;
		QValueList<FunctItem>::Iterator vit = invalidItems.begin();
		for ( ; vit != invalidItems.end(); ++vit ) {
		    if ( (*vit).newName == (*it).newName ) {
			invalidItems.remove( vit );
			found = TRUE;
			break;
		    }
		}
		if ( found ) {
		    int delId = (*it).id;
		    it = functList.remove( it );
		    QMap<QListViewItem*, int>::Iterator fit = functionIds.begin();
		    while ( fit != functionIds.end() ) {
			if ( *fit == delId ) {
			    QListViewItem *litem = fit.key();
			    functionIds.remove( fit );
			    delete litem;
			    if ( functionListView->currentItem() )
				functionListView->setSelected( functionListView->currentItem(), TRUE );
			    currentItemChanged( functionListView->currentItem() );
			    break;
			}
			++fit;
		    }
		}
		else
		    ++it;
	    }
	    if ( functionListView->firstChild() ) {
		functionListView->setCurrentItem( functionListView->firstChild() );
		functionListView->setSelected( functionListView->firstChild(), TRUE );
	    }
	}
	formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
	return;
    }

    if ( !commands.isEmpty() ) {
	MacroCommand *cmd = new MacroCommand( n, formWindow, commands );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }

    formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    accept();
}

void EditFunctions::functionAdd( const QString &access, const QString &type )
{
    QListViewItem *i = new QListViewItem( functionListView );
    i->setPixmap( 0, SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()) );
    i->setRenameEnabled( 0, TRUE );
    i->setText( 1, "void" );
    i->setText( 2, "virtual" );

    if ( access.isEmpty() )
	i->setText( 3, "public" );
    else
	i->setText( 3, access );

    if( type.isEmpty() ) {
	if ( showOnlySlots->isChecked() )
	    i->setText( 4, "slot" );
	else {
	    i->setText( 4, lastType );
	}
    } else {
	i->setText( 4, type );
    }

    if ( i->text( 4 ) == "slot" ) {
	i->setText( 0, "newSlot()" );
	if ( MetaDataBase::isSlotUsed( formWindow, "newSlot()" ) )
	    i->setText( 5, i18n( "Yes" ) );
	else
	    i->setText( 5, i18n( "No" ) );
    } else {
	i->setText( 0, "newFunction()" );
	i->setText( 5, "---" );
    }

    functionListView->setCurrentItem( i );
    functionListView->setSelected( i, TRUE );
    functionListView->ensureItemVisible( i );
    functionName->setFocus();
    functionName->selectAll();

    FunctItem fui;
    fui.id = id;
    fui.oldName = i->text( 0 );
    fui.newName = fui.oldName;
    fui.oldRetTyp = i->text( 1 );
    fui.retTyp = fui.oldRetTyp;
    fui.oldSpec = i->text ( 2 );
    fui.spec = fui.oldSpec;
    fui.oldAccess = i->text( 3 );
    fui.access = fui.oldAccess;
    fui.oldType = i->text( 4 );
    fui.type = fui.oldType;
    lastType = fui.oldType;
    functList.append( fui );
    functionIds.insert( i, id );
    id++;
}

void EditFunctions::functionRemove()
{
    if ( !functionListView->currentItem() )
	return;

    functionListView->blockSignals( TRUE );
    removedFunctions << MetaDataBase::normalizeFunction( functionListView->currentItem()->text( 0 ) );
    int delId = functionIds[ functionListView->currentItem() ];
    QValueList<FunctItem>::Iterator it = functList.begin();
    while ( it != functList.end() ) {
	if ( (*it).id == delId ) {
	    functList.remove( it );
	    break;
	}
	++it;
    }
    functionIds.remove( functionListView->currentItem() );
    delete functionListView->currentItem();
    if ( functionListView->currentItem() )
	functionListView->setSelected( functionListView->currentItem(), TRUE );
    functionListView->blockSignals( FALSE );
    currentItemChanged( functionListView->currentItem() );
}

void EditFunctions::currentItemChanged( QListViewItem *i )
{
    functionName->blockSignals( TRUE );
    functionName->setText( "" );
    functionAccess->setCurrentItem( 0 );
    functionName->blockSignals( FALSE );

    if ( !i ) {
	boxProperties->setEnabled( FALSE );
	return;
    }

    functionName->blockSignals( TRUE );
    functionName->setText( i->text( 0 ) );
    editType->setText( i->text( 1 ) );
    QString specifier = i->text( 2 );
    QString access = i->text( 3 );
    QString type = i->text( 4 );
    if ( specifier == "pure virtual" )
	functionSpecifier->setCurrentItem( 2 );
    else if ( specifier == "non virtual" )
	functionSpecifier->setCurrentItem( 0 );
    else if ( specifier == "virtual" )
	functionSpecifier->setCurrentItem( 1 );
    else
	functionSpecifier->setCurrentItem( 3 );
    if ( access == "private" )
	functionAccess->setCurrentItem( 2 );
    else if ( access == "protected" )
	functionAccess->setCurrentItem( 1 );
    else
	functionAccess->setCurrentItem( 0 );
    if ( type == "slot" )
	functionType->setCurrentItem( 0 );
    else
	functionType->setCurrentItem( 1 );

    functionName->blockSignals( FALSE );
    boxProperties->setEnabled( TRUE );
}

void EditFunctions::currentTextChanged( const QString &txt )
{
    if ( !functionListView->currentItem() )
	return;

    changeItem( functionListView->currentItem(), Name, txt );
    functionListView->currentItem()->setText( 0, txt );

    if ( functionListView->currentItem()->text( 4 ) == "slot" ) {
	if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( txt.latin1() ).latin1() ) )
	    functionListView->currentItem()->setText( 5, i18n( "Yes" ) );
	else
	    functionListView->currentItem()->setText( 5, i18n( "No" ) );
    } else {
	functionListView->currentItem()->setText( 5, "---" );
    }
}

void EditFunctions::currentSpecifierChanged( const QString& s )
{
    if ( !functionListView->currentItem() )
	return;

    changeItem( functionListView->currentItem(), Specifier, s );
    functionListView->currentItem()->setText( 2, s );
}

void EditFunctions::currentAccessChanged( const QString& a )
{
    if ( !functionListView->currentItem() )
	return;
    changeItem( functionListView->currentItem(), Access, a );
    functionListView->currentItem()->setText( 3, a );
}


void EditFunctions::currentReturnTypeChanged( const QString &type )
{
    if ( !functionListView->currentItem() )
	return;
    changeItem( functionListView->currentItem(), ReturnType, type );
    functionListView->currentItem()->setText( 1, type );
}

void EditFunctions::currentTypeChanged( const QString &type )
{
    if ( !functionListView->currentItem() )
	return;
    changeItem( functionListView->currentItem(), Type,  type );
    lastType = type;
    functionListView->currentItem()->setText( 4, type );
    if ( type == "slot" ) {
	if ( MetaDataBase::isSlotUsed( formWindow,
		MetaDataBase::normalizeFunction( functionListView->currentItem()->text( 0 ).latin1() ).latin1() ) )
	    functionListView->currentItem()->setText( 5, i18n( "Yes" ) );
	else
	    functionListView->currentItem()->setText( 5, i18n( "No" ) );
    } else {
	functionListView->currentItem()->setText( 5, "---" );
    }
}

void EditFunctions::changeItem( QListViewItem *item, Attribute a, const QString &nV )
{
    int itemId;
    QMap<QListViewItem*, int>::Iterator fit = functionIds.find( item );
    if ( fit != functionIds.end() )
	itemId = *fit;
    else
	return;

    QValueList<FunctItem>::Iterator it = functList.begin();
    for ( ; it != functList.end(); ++it ) {
	if ( (*it).id == itemId ) {
	    switch( a ) {
		case Name:
		    (*it).newName = nV;
		    break;
		case Specifier:
		    (*it).spec = nV;
		    break;
		case Access:
		    (*it).access = nV;
		    break;
		case ReturnType:
		    (*it).retTyp = nV;
		    break;
		case Type:
		    (*it).type = nV;
		    break;
	    }
	}
    }
}

void EditFunctions::setCurrentFunction( const QString &function )
{
    QListViewItemIterator it( functionListView );
    while ( it.current() ) {
	if ( MetaDataBase::normalizeFunction( it.current()->text( 0 ) ) == function ) {
	    functionListView->setCurrentItem( it.current() );
	    functionListView->setSelected( it.current(), TRUE );
	    currentItemChanged( it.current() );
	    return;
	}
	++it;
    }
}

void EditFunctions::displaySlots( bool justSlots )
{
    functionIds.clear();
    functionListView->clear();
    for ( QValueList<FunctItem>::Iterator it = functList.begin(); it != functList.end(); ++it ) {
	if ( (*it).type == "function" && justSlots )
	    continue;
	QListViewItem *i = new QListViewItem( functionListView );
	functionIds.insert( i, (*it).id );
	i->setPixmap( 0, SmallIcon( "designer_editslots.png" , KDevDesignerPartFactory::instance()) );
	i->setText( 0, (*it).newName );
	i->setText( 1, (*it).retTyp );
	i->setText( 2, (*it).spec );
	i->setText( 3, (*it).access  );
	i->setText( 4, (*it).type );

	if ( (*it).type == "slot" ) {
	    if ( MetaDataBase::isSlotUsed( formWindow, MetaDataBase::normalizeFunction( (*it).newName ).latin1() ) )
		i->setText( 5, i18n( "Yes" ) );
	    else
		i->setText( 5, i18n( "No" ) );
	} else {
	    i->setText( 5, "---" );
	}
    }

    if ( functionListView->firstChild() )
	functionListView->setSelected( functionListView->firstChild(), TRUE );
}

void EditFunctions::emitItemRenamed( QListViewItem *, int, const QString & text )
{
    emit itemRenamed( text ); // Relay signal ( to QLineEdit )
}

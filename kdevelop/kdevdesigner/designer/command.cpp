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
#include "command.h"
#include "formwindow.h"
#include "widgetfactory.h"
#include "propertyeditor.h"
#include "metadatabase.h"
#include <widgetdatabase.h>
#include "mainwindow.h"
#include "hierarchyview.h"
#include "workspace.h"
#include "actioneditorimpl.h"
#include "actiondnd.h"
#include "formfile.h"
#include "../interfaces/languageinterface.h"
#include "menubareditor.h"
#include "popupmenueditor.h"

#include <qfeatures.h>
#include <qwidget.h>
#include <qmetaobject.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qiconview.h>
#include <qtextedit.h>
#include <qptrstack.h>
#include <qheader.h>
#include <qsplitter.h>
#ifndef QT_NO_TABLE
#include <qtable.h>
#endif
#include <qaction.h>

#include "kdevdesigner_part.h"

#include <klocale.h>

CommandHistory::CommandHistory( int s )
    : current( -1 ), steps( s ), savedAt( -1 )
{
    history.setAutoDelete( TRUE );
    modified = FALSE;
    compressedCommand = 0;
}

void CommandHistory::addCommand( Command *cmd, bool tryCompress )
{
    if ( tryCompress ) {
	if ( !compressedCommand ||
	     compressedCommand->type() != cmd->type() ||
	     !compressedCommand->canMerge( cmd ) ) {
	    checkCompressedCommand();
	    compressedCommand = 0;
	}

	if ( compressedCommand ) {
	    compressedCommand->merge( cmd );
	    modified = TRUE;
	    modificationChanged( modified );
	    return;
	}
	compressedCommand = cmd;
    } else {
	checkCompressedCommand();
    }

    if ( current < (int)history.count() - 1 ) {
	if ( current < savedAt )
	    savedAt = -2;

	QPtrList<Command> commands;
	commands.setAutoDelete( FALSE );

	for( int i = 0; i <= current; ++i ) {
	    commands.insert( i, history.at( 0 ) );
	    history.take( 0 );
	}

	commands.append( cmd );
	history.clear();
	history = commands;
	history.setAutoDelete( TRUE );
    } else {
	history.append( cmd );
    }

    if ( (int)history.count() > steps ) {
	savedAt--;
	history.removeFirst();
    } else {
	++current;
    }

    emitUndoRedo();
    modified = TRUE;
    modificationChanged( modified );
}

void CommandHistory::undo()
{
    checkCompressedCommand();
    compressedCommand = 0;
    if ( current > -1 ) {
	history.at( current )->unexecute();
	--current;
    }
    emitUndoRedo();
    modified = savedAt != current;
    modificationChanged( modified );
}

void CommandHistory::redo()
{
    checkCompressedCommand();
    compressedCommand = 0;
    if ( current > -1 ) {
	if ( current < (int)history.count() - 1 ) {
	    ++current;
	    history.at( current )->execute();
	}
    } else {
	if ( history.count() > 0 ) {
	    ++current;
	    history.at( current )->execute();
	}
    }
    emitUndoRedo();
    modified = savedAt != current;
    modificationChanged( modified );
}

void CommandHistory::emitUndoRedo()
{
    Command *undoCmd = 0;
    Command *redoCmd = 0;

    if ( current >= 0 && current < (int)history.count() )
	undoCmd = history.at( current );
    if ( current + 1 >= 0 && current + 1 < (int)history.count() )
	redoCmd = history.at( current + 1 );

    bool ua = (undoCmd != 0);
    QString uc;
    if ( ua )
	uc = undoCmd->name();
    bool ra = (redoCmd != 0);
    QString rc;
    if ( ra )
	rc = redoCmd->name();
    emit undoRedoChanged( ua, ra, uc, rc );
}

void CommandHistory::setModified( bool m )
{
    modified = m;
    if ( !modified )
	savedAt = current;
    modificationChanged( modified );
}

bool CommandHistory::isModified() const
{
    return modified;
}

void CommandHistory::checkCompressedCommand()
{
    if ( compressedCommand && compressedCommand->type() == Command::SetProperty ) {
	Command *c = compressedCommand;
	compressedCommand = 0;
	if ( !( (SetPropertyCommand*)c )->checkProperty() ) {
	    history.remove( current );
	    --current;
	    emitUndoRedo();
	}
    }
}

// ------------------------------------------------------------

Command::Command( const QString &n, FormWindow *fw )
    : cmdName( n ), formWin( fw )
{
}

Command::~Command()
{
}

QString Command::name() const
{
    return cmdName;
}

FormWindow *Command::formWindow() const
{
    return formWin;
}

void Command::merge( Command * )
{
}

bool Command::canMerge( Command * )
{
    return FALSE;
}

// ------------------------------------------------------------

ResizeCommand::ResizeCommand( const QString &n, FormWindow *fw,
			      QWidget *w, const QRect &oldr, const QRect &nr )
    : Command( n, fw ), widget( w ), oldRect( oldr ), newRect( nr )
{
}

void ResizeCommand::execute()
{
    widget->setGeometry( newRect );
    formWindow()->updateSelection( widget );
    formWindow()->emitUpdateProperties( widget );
    if ( WidgetFactory::layoutType( widget ) != WidgetFactory::NoLayout )
	formWindow()->updateChildSelections( widget );
}

void ResizeCommand::unexecute()
{
    widget->setGeometry( oldRect );
    formWindow()->updateSelection( widget );
    formWindow()->emitUpdateProperties( widget );
    if ( WidgetFactory::layoutType( widget ) != WidgetFactory::NoLayout )
	formWindow()->updateChildSelections( widget );
}

// ------------------------------------------------------------

InsertCommand::InsertCommand( const QString &n, FormWindow *fw,
			      QWidget *w, const QRect &g )
    : Command( n, fw ), widget( w ), geometry( g )
{
}

void InsertCommand::execute()
{
    if ( geometry.size() == QSize( 0, 0 ) ) {
	widget->move( geometry.topLeft() );
	widget->adjustSize();
    } else {
	QSize s = geometry.size().expandedTo( widget->minimumSize() );
	s = s.expandedTo( widget->minimumSizeHint() );
	QRect r( geometry.topLeft(), s );
	widget->setGeometry( r );
    }
    widget->show();
    formWindow()->widgets()->insert( widget, widget );
    formWindow()->clearSelection( FALSE );
    formWindow()->selectWidget( widget );
    formWindow()->mainWindow()->objectHierarchy()->widgetInserted( widget );
}

void InsertCommand::unexecute()
{
    widget->hide();
    formWindow()->selectWidget( widget, FALSE );
    formWindow()->widgets()->remove( widget );
    formWindow()->mainWindow()->objectHierarchy()->widgetRemoved( widget );
 }

// ------------------------------------------------------------

MoveCommand::MoveCommand( const QString &n, FormWindow *fw,
			  const QWidgetList &w,
			  const QValueList<QPoint> op,
			  const QValueList<QPoint> np,
			  QWidget *opr, QWidget *npr )
    : Command( n, fw ), widgets( w ), oldPos( op ), newPos( np ),
      oldParent( opr ), newParent( npr )
{
    widgets.setAutoDelete( FALSE );
}

void MoveCommand::merge( Command *c )
{
    MoveCommand *cmd = (MoveCommand*)c;
    newPos = cmd->newPos;
}

bool MoveCommand::canMerge( Command *c )
{
    MoveCommand *cmd = (MoveCommand*)c;
    return widgets == cmd->widgets;
}


void MoveCommand::execute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( !w->parentWidget() || WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout ) {
	    if ( newParent && oldParent && newParent != oldParent ) {
		QPoint pos = newParent->mapFromGlobal( w->mapToGlobal( QPoint( 0,0 ) ) );
		w->reparent( newParent, pos, TRUE );
		formWindow()->raiseSelection( w );
		formWindow()->raiseChildSelections( w );
		formWindow()->widgetChanged( w );
		formWindow()->mainWindow()->objectHierarchy()->widgetRemoved( w );
		formWindow()->mainWindow()->objectHierarchy()->widgetInserted( w );
	    }
	    w->move( newPos[ widgets.at() ] );
	}
	formWindow()->updateSelection( w );
	formWindow()->updateChildSelections( w );
	formWindow()->emitUpdateProperties( w );
    }
}

void MoveCommand::unexecute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( !w->parentWidget() || WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout ) {
	    if ( newParent && oldParent && newParent != oldParent ) {
		QPoint pos = oldParent->mapFromGlobal( w->mapToGlobal( QPoint( 0,0 ) ) );
		w->reparent( oldParent, pos, TRUE );
		formWindow()->raiseSelection( w );
		formWindow()->raiseChildSelections( w );
		formWindow()->widgetChanged( w );
		formWindow()->mainWindow()->objectHierarchy()->widgetRemoved( w );
		formWindow()->mainWindow()->objectHierarchy()->widgetInserted( w );
	    }
	    w->move( oldPos[ widgets.at() ] );
	}
	formWindow()->updateSelection( w );
	formWindow()->updateChildSelections( w );
	formWindow()->emitUpdateProperties( w );
    }
}

// ------------------------------------------------------------

DeleteCommand::DeleteCommand( const QString &n, FormWindow *fw,
			      const QWidgetList &wl )
    : Command( n, fw ), widgets( wl )
{
    widgets.setAutoDelete( FALSE );
    QWidgetList copyOfWidgets = widgets;
    copyOfWidgets.setAutoDelete(FALSE);

    // Include the children of the selected items when deleting
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	QObjectList *children = w->queryList( "QWidget" );
	for ( QWidget *c = (QWidget *)children->first(); c; c = (QWidget *)children->next() ) {
	    if ( copyOfWidgets.find( c ) == -1 && formWindow()->widgets()->find( c ) ) {
		widgets.insert(widgets.at() + 1, c);
		widgets.prev();
		copyOfWidgets.append(c);
	    }
	}
	delete children;
    }
}

void DeleteCommand::execute()
{
    formWindow()->setPropertyShowingBlocked( TRUE );
    connections.clear();
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->hide();
	QString s = w->name();
	s.prepend( "qt_dead_widget_" );
	w->setName( s );
	formWindow()->selectWidget( w, FALSE );
	formWindow()->widgets()->remove( w );
	QValueList<MetaDataBase::Connection> conns = MetaDataBase::connections( formWindow(), w );
	connections.insert( w, conns );
	QValueList<MetaDataBase::Connection>::Iterator it = conns.begin();
	for ( ; it != conns.end(); ++it ) {
	    MetaDataBase::removeConnection( formWindow(), (*it).sender,
					    (*it).signal, (*it).receiver, (*it).slot );
	}
    }
    formWindow()->setPropertyShowingBlocked( FALSE );
    formWindow()->emitShowProperties();
    formWindow()->mainWindow()->objectHierarchy()->widgetsRemoved( widgets );

}

void DeleteCommand::unexecute()
{
    formWindow()->setPropertyShowingBlocked( TRUE );
    formWindow()->clearSelection( FALSE );
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->show();
	QString s = w->name();
	s.remove( 0, QString( "qt_dead_widget_" ).length() );
	w->setName( s );
	formWindow()->widgets()->insert( w, w );
	formWindow()->selectWidget( w );
	QValueList<MetaDataBase::Connection> conns = *connections.find( w );
	QValueList<MetaDataBase::Connection>::Iterator it = conns.begin();
	for ( ; it != conns.end(); ++it ) {
	    MetaDataBase::addConnection( formWindow(), (*it).sender,
					 (*it).signal, (*it).receiver, (*it).slot );
	}
    }
    formWindow()->setPropertyShowingBlocked( FALSE );
    formWindow()->emitShowProperties();
    formWindow()->mainWindow()->objectHierarchy()->widgetsInserted( widgets );
}

// ------------------------------------------------------------

SetPropertyCommand::SetPropertyCommand( const QString &n, FormWindow *fw,
					QObject *w, PropertyEditor *e,
					const QString &pn, const QVariant &ov,
					const QVariant &nv, const QString &ncut,
					const QString &ocut, bool reset )
    : Command( n, fw ), widget( w ), editor( e ), propName( pn ),
      oldValue( ov ), newValue( nv ), oldCurrentItemText( ocut ), newCurrentItemText( ncut ),
      wasChanged( TRUE ), isResetCommand( reset )
{
    wasChanged = MetaDataBase::isPropertyChanged( w, propName );
    if ( oldCurrentItemText.isNull() )
	oldCurrentItemText = "";
    if ( newCurrentItemText.isNull() )
	newCurrentItemText = "";
}


void SetPropertyCommand::execute()
{
    if ( !wasChanged )
	MetaDataBase::setPropertyChanged( widget, propName, TRUE );
    if ( isResetCommand ) {
	MetaDataBase::setPropertyChanged( widget, propName, FALSE );
	if ( WidgetFactory::resetProperty( widget, propName ) ) {
	    if ( !formWindow()->isWidgetSelected( widget ) && formWindow() != (QObject *)widget )
		formWindow()->selectWidget( (QObject *)widget );
	    if ( editor->widget() != widget )
		editor->setWidget( widget, formWindow() );
	    editor->propertyList()->setCurrentProperty( propName );
	    PropertyItem *i = (PropertyItem*)editor->propertyList()->currentItem();
	    if ( !i )
		return;
	    i->setValue( widget->property( propName ) );
	    i->setChanged( FALSE );
	    editor->refetchData();
	    editor->emitWidgetChanged();
	    return;
	}
    }
    setProperty( newValue, newCurrentItemText );
}

void SetPropertyCommand::unexecute()
{
    if ( !wasChanged )
	MetaDataBase::setPropertyChanged( widget, propName, FALSE );
    if ( isResetCommand )
	MetaDataBase::setPropertyChanged( widget, propName, TRUE );
    setProperty( oldValue, oldCurrentItemText );
}

bool SetPropertyCommand::canMerge( Command *c )
{
    SetPropertyCommand *cmd = (SetPropertyCommand*)c;
    if ( !widget )
	return FALSE;
    const QMetaProperty *p =
	widget->metaObject()->property( widget->metaObject()->findProperty( propName, TRUE ), TRUE );
    if ( !p ) {
	if ( propName == "toolTip" || propName == "whatsThis" )
	    return TRUE;
	if ( ::qt_cast<CustomWidget*>((QObject *)widget) ) {
	    MetaDataBase::CustomWidget *cw = ((CustomWidget*)(QObject*)widget)->customWidget();
	    if ( !cw )
		return FALSE;
	    for ( QValueList<MetaDataBase::Property>::Iterator it = cw->lstProperties.begin(); it != cw->lstProperties.end(); ++it ) {
		if ( QString( (*it ).property ) == propName ) {
		    if ( (*it).type == "String" || (*it).type == "CString" || (*it).type == "Int" || (*it).type == "UInt" )
			return TRUE;
		}
	    }
	}
	return FALSE;
    }
    QVariant::Type t = QVariant::nameToType( p->type() );
    return ( cmd->propName == propName &&
	     t == QVariant::String || t == QVariant::CString || t == QVariant::Int || t == QVariant::UInt );
}

void SetPropertyCommand::merge( Command *c )
{
    SetPropertyCommand *cmd = (SetPropertyCommand*)c;
    newValue = cmd->newValue;
    newCurrentItemText = cmd->newCurrentItemText;
}

bool SetPropertyCommand::checkProperty()
{
    if ( propName == "name" /*|| propName == "itemName"*/ ) { // ### fix that
	QString s = newValue.toString();
	if ( !formWindow()->unify( widget, s, FALSE ) ) {
	    QMessageBox::information( formWindow()->mainWindow(),
				      i18n( "Set 'name' Property" ),
				      i18n( "The name of a widget must be unique.\n"
						      "'%1' is already used in form '%2',\n"
						      "so the name has been reverted to '%3'." ).
				      arg( newValue.toString() ).
				      arg( formWindow()->name() ).
				      arg( oldValue.toString() ));
	    setProperty( oldValue, oldCurrentItemText, FALSE );
	    return FALSE;
	}
	if ( s.isEmpty() ) {
	    QMessageBox::information( formWindow()->mainWindow(),
				      i18n( "Set 'name' Property" ),
				      i18n( "The name of a widget must not be null.\n"
						      "The name has been reverted to '%1'." ).
				      arg( oldValue.toString() ));
	    setProperty( oldValue, oldCurrentItemText, FALSE );
	    return FALSE;
	}

	if ( ::qt_cast<FormWindow*>(widget->parent()) )
	    formWindow()->mainWindow()->formNameChanged( (FormWindow*)((QWidget*)(QObject*)widget)->parentWidget() );
    }
    return TRUE;
}

void SetPropertyCommand::setProperty( const QVariant &v, const QString &currentItemText, bool select )
{
    if ( !widget )
        return;

    if ( !formWindow()->isWidgetSelected( widget ) && !formWindow()->isMainContainer( widget ) && select )
	formWindow()->selectWidget( widget );
    if ( editor->widget() != widget && select )
	editor->setWidget( widget, formWindow() );
    if ( select )
	editor->propertyList()->setCurrentProperty( propName );
    const QMetaProperty *p =
	widget->metaObject()->property( widget->metaObject()->findProperty( propName, TRUE ), TRUE );
    if ( !p ) {
	if ( propName == "hAlign" ) {
	    p = widget->metaObject()->property( widget->metaObject()->findProperty( "alignment", TRUE ), TRUE );
	    int align = widget->property( "alignment" ).toInt();
	    align &= ~( AlignHorizontal_Mask );
	    align |= p->keyToValue( currentItemText );
	    widget->setProperty( "alignment", QVariant( align ) );
	} else if ( propName == "vAlign" ) {
	    p = widget->metaObject()->property( widget->metaObject()->findProperty( "alignment", TRUE ), TRUE );
	    int align = widget->property( "alignment" ).toInt();
	    align &= ~( AlignVertical_Mask );
	    align |= p->keyToValue( currentItemText );
	    widget->setProperty( "alignment", QVariant( align ) );
	} else if ( propName == "wordwrap" ) {
	    int align = widget->property( "alignment" ).toInt();
	    align &= ~WordBreak;
	    if ( v.toBool() )
		align |= WordBreak;
	    widget->setProperty( "alignment", QVariant( align ) );
	} else if ( propName == "layoutSpacing" ) {
	    QVariant val = v;
	    if ( val.toString() == "default" )
		val = -1;
	    MetaDataBase::setSpacing( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ), val.toInt() );
	} else if ( propName == "layoutMargin" ) {
	    QVariant val = v;
	    if ( val.toString() == "default" )
		val = -1;
	    MetaDataBase::setMargin( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ), val.toInt() );
	} else if ( propName == "resizeMode" ) {
	    MetaDataBase::setResizeMode( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ), currentItemText );
	} else if ( propName == "toolTip" || propName == "whatsThis" || propName == "database" || propName == "frameworkCode" ) {
	    MetaDataBase::setFakeProperty( editor->widget(), propName, v );
	} else if ( ::qt_cast<CustomWidget*>(editor->widget()) ) {
	    MetaDataBase::CustomWidget *cw = ((CustomWidget *)(QObject *)widget)->customWidget();
	    if ( cw ) {
		MetaDataBase::setFakeProperty( editor->widget(), propName, v );
	    }
	}
	editor->refetchData();
	editor->emitWidgetChanged();
	( ( PropertyItem* )editor->propertyList()->currentItem() )->setChanged( MetaDataBase::isPropertyChanged( widget, propName ) );
#ifndef QT_NO_SQL
	if ( propName == "database" ) {
	    formWindow()->mainWindow()->objectHierarchy()->databasePropertyChanged( (QWidget*)((QObject *)widget), MetaDataBase::fakeProperty( widget, "database" ).toStringList() );
	}
#endif
	return;
    }

    if ( p->isSetType() ) {
	QStrList strlst;
	QStringList lst = QStringList::split( "|", currentItemText );
	QValueListConstIterator<QString> it = lst.begin();
	for ( ; it != lst.end(); ++it )
	    strlst.append( (*it).latin1() );
	widget->setProperty( propName, p->keysToValue( strlst ) );
    } else if ( p->isEnumType() ) {
	widget->setProperty( propName, p->keyToValue( currentItemText ) );
    } else if ( qstrcmp( p->name(), "buddy" ) == 0 ) {
	widget->setProperty( propName, currentItemText );
    } else {
	QVariant ov;
	if ( propName == "name" || propName == "itemName" )
	    ov = widget->property( propName );
	int oldSerNum = -1;
	if ( v.type() == QVariant::Pixmap )
	    oldSerNum = v.toPixmap().serialNumber();
	widget->setProperty( propName, v );
	if ( oldSerNum != -1 && oldSerNum != widget->property( propName ).toPixmap().serialNumber() )
	    MetaDataBase::setPixmapKey( formWindow(),
					widget->property( propName ).toPixmap().serialNumber(),
					MetaDataBase::pixmapKey( formWindow(), oldSerNum ) );
	if ( propName == "cursor" ) {
	    MetaDataBase::setCursor( (QWidget*)((QObject *)widget), v.toCursor() );
	}
	if ( propName == "name" && widget->isWidgetType() ) {
	    formWindow()->mainWindow()->objectHierarchy()->namePropertyChanged( ((QWidget*)(QObject *)widget), ov );
	    if ( formWindow()->isMainContainer( widget ) )
		formWindow()->setName( v.toCString() );
	}
	if ( propName == "name" && ::qt_cast<QAction*>((QObject *)widget) && ::qt_cast<QMainWindow*>(formWindow()->mainContainer()) ) {
	    formWindow()->mainWindow()->actioneditor()->updateActionName( (QAction*)(QObject *)widget );
	}
	if ( propName == "iconSet" && ::qt_cast<QAction*>((QObject *)widget) && ::qt_cast<QMainWindow*>(formWindow()->mainContainer()) ) {
	    formWindow()->mainWindow()->actioneditor()->updateActionIcon( (QAction*)(QObject *)widget );
	}
	if ( propName == "caption" ) {
	    if ( formWindow()->isMainContainer( widget ) )
		formWindow()->setCaption( v.toString() );
	}
	if ( propName == "icon" ) {
	    if ( formWindow()->isMainContainer( widget ) )
		formWindow()->setIcon( v.toPixmap() );
	}
    }
    editor->refetchData();
    if ( editor->propertyList()->currentItem() && select ) {
	( ( PropertyItem* )editor->propertyList()->currentItem() )->showEditor();
	( ( PropertyItem* )editor->propertyList()->currentItem() )->setChanged( MetaDataBase::isPropertyChanged( widget, propName ) );
    }
    editor->emitWidgetChanged();
    formWindow()->killAccels( widget );
}

// ------------------------------------------------------------

LayoutHorizontalCommand::LayoutHorizontalCommand( const QString &n, FormWindow *fw,
						  QWidget *parent, QWidget *layoutBase,
						  const QWidgetList &wl )
    : Command( n, fw ), layout( wl, parent, fw, layoutBase )
{
}

void LayoutHorizontalCommand::execute()
{
    formWindow()->clearSelection( FALSE );
    layout.doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void LayoutHorizontalCommand::unexecute()
{
    formWindow()->clearSelection( FALSE );
    layout.undoLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

LayoutHorizontalSplitCommand::LayoutHorizontalSplitCommand( const QString &n, FormWindow *fw,
							    QWidget *parent, QWidget *layoutBase,
							    const QWidgetList &wl )
    : Command( n, fw ), layout( wl, parent, fw, layoutBase, TRUE, TRUE )
{
}

void LayoutHorizontalSplitCommand::execute()
{
    formWindow()->clearSelection( FALSE );
    layout.doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void LayoutHorizontalSplitCommand::unexecute()
{
    formWindow()->clearSelection( FALSE );
    layout.undoLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

LayoutVerticalCommand::LayoutVerticalCommand( const QString &n, FormWindow *fw,
					      QWidget *parent, QWidget *layoutBase,
					      const QWidgetList &wl )
    : Command( n, fw ), layout( wl, parent, fw, layoutBase )
{
}

void LayoutVerticalCommand::execute()
{
    formWindow()->clearSelection( FALSE );
    layout.doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void LayoutVerticalCommand::unexecute()
{
    formWindow()->clearSelection( FALSE );
    layout.undoLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

LayoutVerticalSplitCommand::LayoutVerticalSplitCommand( const QString &n, FormWindow *fw,
							QWidget *parent, QWidget *layoutBase,
							const QWidgetList &wl )
    : Command( n, fw ), layout( wl, parent, fw, layoutBase, TRUE, TRUE )
{
}

void LayoutVerticalSplitCommand::execute()
{
    formWindow()->clearSelection( FALSE );
    layout.doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void LayoutVerticalSplitCommand::unexecute()
{
    formWindow()->clearSelection( FALSE );
    layout.undoLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

LayoutGridCommand::LayoutGridCommand( const QString &n, FormWindow *fw,
				      QWidget *parent, QWidget *layoutBase,
				      const QWidgetList &wl, int xres, int yres )
    : Command( n, fw ), layout( wl, parent, fw, layoutBase, QSize( QMAX(5,xres), QMAX(5,yres) ) )
{
}

void LayoutGridCommand::execute()
{
    formWindow()->clearSelection( FALSE );
    layout.doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void LayoutGridCommand::unexecute()
{
    formWindow()->clearSelection( FALSE );
    layout.undoLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

BreakLayoutCommand::BreakLayoutCommand( const QString &n, FormWindow *fw,
					QWidget *layoutBase, const QWidgetList &wl )
    : Command( n, fw ), lb( layoutBase ), widgets( wl )
{
    WidgetFactory::LayoutType lay = WidgetFactory::layoutType( layoutBase );
    spacing = MetaDataBase::spacing( layoutBase );
    margin = MetaDataBase::margin( layoutBase );
    layout = 0;
    if ( lay == WidgetFactory::HBox )
	layout = new HorizontalLayout( wl, layoutBase, fw, layoutBase, FALSE, ::qt_cast<QSplitter*>(layoutBase) != 0 );
    else if ( lay == WidgetFactory::VBox )
	layout = new VerticalLayout( wl, layoutBase, fw, layoutBase, FALSE, ::qt_cast<QSplitter*>(layoutBase) != 0 );
    else if ( lay == WidgetFactory::Grid )
	layout = new GridLayout( wl, layoutBase, fw, layoutBase, QSize( QMAX( 5, fw->grid().x()), QMAX( 5, fw->grid().y()) ), FALSE );
}

void BreakLayoutCommand::execute()
{
    if ( !layout )
	return;
    formWindow()->clearSelection( FALSE );
    layout->breakLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
    for ( QWidget *w = widgets.first(); w; w = widgets.next() )
	w->resize( QMAX( 16, w->width() ), QMAX( 16, w->height() ) );
}

void BreakLayoutCommand::unexecute()
{
    if ( !layout )
	return;
    formWindow()->clearSelection( FALSE );
    layout->doLayout();
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
    MetaDataBase::setSpacing( WidgetFactory::containerOfWidget( lb ), spacing );
    MetaDataBase::setMargin( WidgetFactory::containerOfWidget( lb ), margin );
}

// ------------------------------------------------------------

MacroCommand::MacroCommand( const QString &n, FormWindow *fw,
			    const QPtrList<Command> &cmds )
    : Command( n, fw ), commands( cmds )
{
}

void MacroCommand::execute()
{
    for ( Command *c = commands.first(); c; c = commands.next() )
	c->execute();
}

void MacroCommand::unexecute()
{
    for ( Command *c = commands.last(); c; c = commands.prev() )
	c->unexecute();
}

// ------------------------------------------------------------

AddTabPageCommand::AddTabPageCommand( const QString &n, FormWindow *fw,
				      QTabWidget *tw, const QString &label )
    : Command( n, fw ), tabWidget( tw ), tabLabel( label )
{
    tabPage = new QDesignerWidget( formWindow(), tabWidget, "TabPage" );
    tabPage->hide();
    index = -1;
    MetaDataBase::addEntry( tabPage );
}

void AddTabPageCommand::execute()
{
    if ( index == -1 )
	index = ( (QDesignerTabWidget*)tabWidget )->count();
    tabWidget->insertTab( tabPage, tabLabel, index );
    tabWidget->showPage( tabPage );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

void AddTabPageCommand::unexecute()
{
    tabWidget->removePage( tabPage );
    tabPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

// ------------------------------------------------------------

MoveTabPageCommand::MoveTabPageCommand( const QString &n, FormWindow *fw,
				      QTabWidget *tw, QWidget* page, const QString& label, int nIndex, int oIndex )
    : Command( n, fw ), tabWidget( tw ), tabPage( page ), tabLabel( label )
{
    newIndex = nIndex;
    oldIndex = oIndex;
}

void MoveTabPageCommand::execute()
{
    ((QDesignerTabWidget*)tabWidget )->removePage( tabPage );
    ((QDesignerTabWidget*)tabWidget )->insertTab( tabPage, tabLabel, newIndex );
    ((QDesignerTabWidget*)tabWidget )->showPage( tabPage );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

void MoveTabPageCommand::unexecute()
{
    ((QDesignerTabWidget*)tabWidget )->removePage( tabPage );
    ((QDesignerTabWidget*)tabWidget )->insertTab( tabPage, tabLabel, oldIndex );
    ((QDesignerTabWidget*)tabWidget )->showPage( tabPage );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

// ------------------------------------------------------------

DeleteTabPageCommand::DeleteTabPageCommand( const QString &n, FormWindow *fw,
					    QTabWidget *tw, QWidget *page )
    : Command( n, fw ), tabWidget( tw ), tabPage( page )
{
    tabLabel = ( (QDesignerTabWidget*)tabWidget )->pageTitle();
    index = ( (QDesignerTabWidget*)tabWidget )->currentPage();
}

void DeleteTabPageCommand::execute()
{
    tabWidget->removePage( tabPage );
    tabPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

void DeleteTabPageCommand::unexecute()
{
    tabWidget->insertTab( tabPage, tabLabel, index );
    tabWidget->showPage( tabPage );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( tabWidget );
}

// ------------------------------------------------------------

AddWidgetStackPageCommand::AddWidgetStackPageCommand( const QString &n, FormWindow *fw,
						      QDesignerWidgetStack *ws )
    : Command( n, fw ), widgetStack( ws )
{
    stackPage = new QDesignerWidget( formWindow(), widgetStack, "WStackPage" );
    stackPage->hide();
    index = -1;
    MetaDataBase::addEntry( stackPage );
}

void AddWidgetStackPageCommand::execute()
{
    index = widgetStack->insertPage( stackPage, index );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( 0 );
}

void AddWidgetStackPageCommand::unexecute()
{
    index = widgetStack->removePage( stackPage );
    stackPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->tabsChanged( 0 );
}

DeleteWidgetStackPageCommand::DeleteWidgetStackPageCommand( const QString &n, FormWindow *fw,
							    QDesignerWidgetStack *ws, QWidget *page )
    : Command( n, fw), widgetStack( ws ), stackPage( page )
{
    index = -1;
}

void DeleteWidgetStackPageCommand::execute()
{
    index = widgetStack->removePage( stackPage );
    stackPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->widgetRemoved( stackPage );
}

void DeleteWidgetStackPageCommand::unexecute()
{
    index = widgetStack->insertPage( stackPage, index );
    widgetStack->raiseWidget( stackPage );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->widgetInserted( stackPage );
}

// ------------------------------------------------------------

AddWizardPageCommand::AddWizardPageCommand( const QString &n, FormWindow *fw,
					    QWizard *w, const QString &label, int i, bool s )
    : Command( n, fw ), wizard( w ), pageLabel( label )
{
    page = new QDesignerWidget( formWindow(), wizard, "WizardPage" );
    page->hide();
    index = i;
    show = s;
    MetaDataBase::addEntry( page );
}

void AddWizardPageCommand::execute()
{
    if ( index == -1 )
	index = wizard->pageCount();
    wizard->insertPage( page, pageLabel, index );
    if ( show )
	( (QDesignerWizard*)wizard )->setCurrentPage( ( (QDesignerWizard*)wizard )->pageNum( page ) );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

void AddWizardPageCommand::unexecute()
{
    wizard->removePage( page );
    page->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

// ------------------------------------------------------------

DeleteWizardPageCommand::DeleteWizardPageCommand( const QString &n, FormWindow *fw,
						  QWizard *w, int i, bool s )
    : Command( n, fw ), wizard( w ), index( i )
{
    show = s;
}

void DeleteWizardPageCommand::execute()
{
    page = wizard->page( index );
    pageLabel = wizard->title( page );
    wizard->removePage( page );
    page->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

void DeleteWizardPageCommand::unexecute()
{
    wizard->insertPage( page, pageLabel, index );
    if ( show )
	( (QDesignerWizard*)wizard )->setCurrentPage( ( (QDesignerWizard*)wizard )->pageNum( page ) );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

// ------------------------------------------------------------

RenameWizardPageCommand::RenameWizardPageCommand( const QString &n, FormWindow *fw,
						  QWizard *w, int i, const QString& name )
    : Command( n, fw ), wizard( w ), index( i ), label( name )
{

}

void RenameWizardPageCommand::execute()
{
    page = wizard->page( index );
    QString oldLabel = wizard->title( page );

    wizard->setTitle( page, label );
    label = oldLabel;
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
}

void RenameWizardPageCommand::unexecute()
{
    execute();
}

// ------------------------------------------------------------

SwapWizardPagesCommand::SwapWizardPagesCommand( const QString &n, FormWindow *fw, QWizard *w, int i1, int i2 )
    : Command( n, fw ), wizard( w ), index1( i1 ), index2( i2 )
{
}

void SwapWizardPagesCommand::execute()
{
    QWidget *page1 = wizard->page( index1 );
    QWidget *page2 = wizard->page( index2 );
    QString page1Label = wizard->title( page1 );
    QString page2Label = wizard->title( page2 );
    wizard->removePage( page1 );
    wizard->removePage( page2 );
    wizard->insertPage( page1, page1Label, index2 );
    wizard->insertPage( page2, page2Label, index1 );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

void SwapWizardPagesCommand::unexecute()
{
    execute();
}

// ------------------------------------------------------------

MoveWizardPageCommand::MoveWizardPageCommand( const QString &n, FormWindow *fw, QWizard *w, int i1, int i2 )
    : Command( n, fw ), wizard( w ), index1( i1 ), index2( i2 )
{
}

void MoveWizardPageCommand::execute()
{
    QWidget *page = wizard->page( index1 );
    QString pageLabel = wizard->title( page );
    wizard->removePage( page );
    wizard->insertPage( page, pageLabel, index2 );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

void MoveWizardPageCommand::unexecute()
{
    // ###FIX: index1 may be the wrong place
    QWidget *page = wizard->page( index2 );
    QString pageLabel = wizard->title( page );
    wizard->removePage( page );
    wizard->insertPage( page, pageLabel, index1 );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->pagesChanged( wizard );
}

// ------------------------------------------------------------

AddConnectionCommand::AddConnectionCommand( const QString &name, FormWindow *fw,
					    MetaDataBase::Connection c )
    : Command( name, fw ), connection( c )
{
}

void AddConnectionCommand::execute()
{
    MetaDataBase::addConnection( formWindow(), connection.sender,
				 connection.signal, connection.receiver, connection.slot );
    if ( connection.receiver == formWindow()->mainContainer() )
	formWindow()->mainWindow()->propertyeditor()->eventList()->setup();
}

void AddConnectionCommand::unexecute()
{
    MetaDataBase::removeConnection( formWindow(), connection.sender,
				    connection.signal, connection.receiver, connection.slot );
    if ( connection.receiver == formWindow()->mainContainer() )
	formWindow()->mainWindow()->propertyeditor()->eventList()->setup();
}

// ------------------------------------------------------------

RemoveConnectionCommand::RemoveConnectionCommand( const QString &name, FormWindow *fw,
						  MetaDataBase::Connection c )
    : Command( name, fw ), connection( c )
{
}

void RemoveConnectionCommand::execute()
{
    MetaDataBase::removeConnection( formWindow(), connection.sender,
				    connection.signal, connection.receiver, connection.slot );
    if ( connection.receiver == formWindow()->mainContainer() )
	formWindow()->mainWindow()->propertyeditor()->eventList()->setup();
}

void RemoveConnectionCommand::unexecute()
{
    MetaDataBase::addConnection( formWindow(), connection.sender,
				 connection.signal, connection.receiver, connection.slot );
    if ( connection.receiver == formWindow()->mainContainer() )
	formWindow()->mainWindow()->propertyeditor()->eventList()->setup();
}

// ------------------------------------------------------------

AddFunctionCommand::AddFunctionCommand( const QString &name, FormWindow *fw, const QCString &f,
					const QString& spec, const QString &a, const QString &t,
					const QString &l, const QString &rt )
    : Command( name, fw ), function( f ), specifier( spec ), access( a ), functionType( t ), language( l ),
      returnType( rt )
{
}

void AddFunctionCommand::execute()
{
    MetaDataBase::addFunction( formWindow(), function, specifier, access, functionType, language, returnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (add - execute)
    KInterfaceDesigner::Function f;
    f.returnType = returnType;
    f.function = function;
    f.specifier = specifier;
    f.access = access;
    f.type = (functionType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->clearSelection(false);
    formWindow()->mainWindow()->part()->emitAddedFunction(formWindow()->fileName(), f);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void AddFunctionCommand::unexecute()
{
    MetaDataBase::removeFunction( formWindow(), function, specifier, access, functionType,  language, returnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (add - unexecute)
    KInterfaceDesigner::Function f;
    f.returnType = returnType;
    f.function = function;
    f.specifier = specifier;
    f.access = access;
    f.type = (functionType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->mainWindow()->part()->emitRemovedFunction(formWindow()->fileName(), f);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

ChangeFunctionAttribCommand::ChangeFunctionAttribCommand( const QString &name, FormWindow *fw, MetaDataBase::Function f,
							  const QString &on, const QString &os,
							  const QString &oa, const QString &ot, const QString &ol,
							  const QString &ort )
    : Command( name, fw ), oldName( on ), oldSpec( os ), oldAccess( oa ),
      oldType( ot ), oldLang( ol ), oldReturnType( ort )
{
	    newName = f.function;
	    newSpec = f.specifier;
	    newAccess = f.access;
	    newType = f.type;
	    newLang = f.language;
	    newReturnType = f.returnType;
}

void ChangeFunctionAttribCommand::execute()
{
    MetaDataBase::changeFunctionAttributes( formWindow(), oldName, newName, newSpec, newAccess,
					    newType, newLang, newReturnType );
    formWindow()->formFile()->functionNameChanged( oldName, newName );
    formWindow()->formFile()->functionRetTypeChanged( newName, oldReturnType, newReturnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (edit - execute)
    KInterfaceDesigner::Function f;
    f.returnType = newReturnType;
    f.function = newName;
    f.specifier = newSpec;
    f.access = newAccess;
    f.type = (newType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    KInterfaceDesigner::Function of;
    f.returnType = oldReturnType;
    f.function = oldName;
    f.specifier = oldSpec;
    f.access = oldAccess;
    f.type = (oldType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->mainWindow()->part()->emitEditedFunction(formWindow()->fileName(), of, f);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void ChangeFunctionAttribCommand::unexecute()
{
    MetaDataBase::changeFunctionAttributes( formWindow(), newName, oldName, oldSpec, oldAccess,
					    oldType, oldLang, oldReturnType );
    formWindow()->formFile()->functionNameChanged( newName, oldName );
    formWindow()->formFile()->functionRetTypeChanged( oldName, newReturnType, oldReturnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (edit - execute)
    KInterfaceDesigner::Function f;
    f.returnType = newReturnType;
    f.function = newName;
    f.specifier = newSpec;
    f.access = newAccess;
    f.type = (newType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    KInterfaceDesigner::Function of;
    f.returnType = oldReturnType;
    f.function = oldName;
    f.specifier = oldSpec;
    f.access = oldAccess;
    f.type = (oldType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->mainWindow()->part()->emitEditedFunction(formWindow()->fileName(), f, of);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

RemoveFunctionCommand::RemoveFunctionCommand( const QString &name, FormWindow *fw, const QCString &f,
						const QString& spec, const QString &a, const QString &t,
						const QString &l, const QString &rt )
    : Command( name, fw ), function( f ), specifier( spec ), access( a ), functionType( t ), language( l ),
      returnType( rt )
{
    if ( spec.isNull() ) {
	QValueList<MetaDataBase::Function> lst = MetaDataBase::functionList( fw );
	for ( QValueList<MetaDataBase::Function>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    if ( MetaDataBase::normalizeFunction( (*it).function ) ==
		 MetaDataBase::normalizeFunction( function ) ) {
		specifier = (*it).specifier;
		access = (*it).access;
		functionType = (*it).type;
		returnType = (*it).returnType;
		language = (*it).language;
		break;
	    }
	}
    }
}

void RemoveFunctionCommand::execute()
{
    MetaDataBase::removeFunction( formWindow(), function, specifier, access, functionType, language, returnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (remove - execute)
    KInterfaceDesigner::Function f;
    f.returnType = returnType;
    f.function = function;
    f.specifier = specifier;
    f.access = access;
    f.type = (functionType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->mainWindow()->part()->emitRemovedFunction(formWindow()->fileName(), f);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void RemoveFunctionCommand::unexecute()
{
    if ( MetaDataBase::hasFunction( formWindow(), function ) )
	return;
    MetaDataBase::addFunction( formWindow(), function, specifier, access, functionType, language, returnType );
    formWindow()->mainWindow()->functionsChanged();

    //integration (remove - unexecute)
    KInterfaceDesigner::Function f;
    f.returnType = returnType;
    f.function = function;
    f.specifier = specifier;
    f.access = access;
    f.type = (functionType == "slot") ? KInterfaceDesigner::ftQtSlot : KInterfaceDesigner::ftFunction ;
    formWindow()->clearSelection(false);
    formWindow()->mainWindow()->part()->emitAddedFunction(formWindow()->fileName(), f);

    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

AddVariableCommand::AddVariableCommand( const QString &name, FormWindow *fw, const QString &vn, const QString &a )
    : Command( name, fw ), varName( vn ), access( a )
{
}

void AddVariableCommand::execute()
{
    MetaDataBase::addVariable( formWindow(), varName, access );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void AddVariableCommand::unexecute()
{
    MetaDataBase::removeVariable( formWindow(), varName );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

SetVariablesCommand::SetVariablesCommand( const QString &name, FormWindow *fw,
    QValueList<MetaDataBase::Variable> lst )
    : Command( name, fw ), newList( lst )
{
    oldList = MetaDataBase::variables( formWindow() );
}

void SetVariablesCommand::execute()
{
    MetaDataBase::setVariables( formWindow(), newList );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void SetVariablesCommand::unexecute()
{
    MetaDataBase::setVariables( formWindow(), oldList );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

RemoveVariableCommand::RemoveVariableCommand( const QString &name, FormWindow *fw, const QString &vn )
    : Command( name, fw ), varName( vn )
{
    QValueList<MetaDataBase::Variable> lst = MetaDataBase::variables( fw );
    for ( QValueList<MetaDataBase::Variable>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( (*it).varName == varName ) {
	    access = (*it).varAccess;
	    break;
	}
    }
}

void RemoveVariableCommand::execute()
{
    MetaDataBase::removeVariable( formWindow(), varName );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void RemoveVariableCommand::unexecute()
{
    MetaDataBase::addVariable( formWindow(), varName, access );
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

EditDefinitionsCommand::EditDefinitionsCommand( const QString &name, FormWindow *fw, LanguageInterface *lf,
						const QString &n, const QStringList &nl )
    : Command( name, fw ), lIface( lf ), defName( n ), newList( nl )
{
    oldList = lIface->definitionEntries( defName, formWindow()->mainWindow()->designerInterface() );
}

void EditDefinitionsCommand::execute()
{
    lIface->setDefinitionEntries( defName, newList, formWindow()->mainWindow()->designerInterface() );
    lIface->release();
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

void EditDefinitionsCommand::unexecute()
{
    lIface->setDefinitionEntries( defName, oldList, formWindow()->mainWindow()->designerInterface() );
    lIface->release();
    formWindow()->mainWindow()->objectHierarchy()->updateFormDefinitionView();
    if ( formWindow()->formFile() )
	formWindow()->formFile()->setModified( TRUE );
}

// ------------------------------------------------------------

LowerCommand::LowerCommand( const QString &name, FormWindow *fw, const QWidgetList &w )
    : Command( name, fw ), widgets( w )
{
}

void LowerCommand::execute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->lower();
	formWindow()->raiseSelection( w );
    }

}

void LowerCommand::unexecute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->raise();
	formWindow()->raiseSelection( w );
    }
}

// ------------------------------------------------------------

RaiseCommand::RaiseCommand( const QString &name, FormWindow *fw, const QWidgetList &w )
    : Command( name, fw ), widgets( w )
{
}

void RaiseCommand::execute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->raise();
	formWindow()->raiseSelection( w );
    }

}

void RaiseCommand::unexecute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->lower();
	formWindow()->raiseSelection( w );
    }
}

// ------------------------------------------------------------

PasteCommand::PasteCommand( const QString &n, FormWindow *fw,
			      const QWidgetList &w )
    : Command( n, fw ), widgets( w )
{
}

void PasteCommand::execute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->show();
	formWindow()->selectWidget( w );
	formWindow()->widgets()->insert( w, w );
	formWindow()->mainWindow()->objectHierarchy()->widgetInserted( w );
    }
}

void PasteCommand::unexecute()
{
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	w->hide();
	formWindow()->selectWidget( w, FALSE );
	formWindow()->widgets()->remove( w );
	formWindow()->mainWindow()->objectHierarchy()->widgetRemoved( w );
    }
 }

// ------------------------------------------------------------

TabOrderCommand::TabOrderCommand( const QString &n, FormWindow *fw,
				  const QWidgetList &ol, const QWidgetList &nl )
    : Command( n, fw ), oldOrder( ol ), newOrder( nl )
{
}

void TabOrderCommand::merge( Command *c )
{
    TabOrderCommand *cmd = (TabOrderCommand*)c;
    newOrder = cmd->newOrder;
}

bool TabOrderCommand::canMerge( Command * )
{
    return TRUE;
}

void TabOrderCommand::execute()
{
    MetaDataBase::setTabOrder( formWindow(), newOrder );
}

void TabOrderCommand::unexecute()
{
    MetaDataBase::setTabOrder( formWindow(), oldOrder );
}

// ------------------------------------------------------------

PopulateListBoxCommand::PopulateListBoxCommand( const QString &n, FormWindow *fw,
						QListBox *lb, const QValueList<Item> &items )
    : Command( n, fw ), newItems( items ), listbox( lb )
{
    QListBoxItem *i = 0;
    for ( i = listbox->firstItem(); i; i = i->next() ) {
	Item item;
	if ( i->pixmap() )
	    item.pix = *i->pixmap();
	item.text = i->text();
	oldItems.append( item );
    }
}

void PopulateListBoxCommand::execute()
{
    listbox->clear();
    for ( QValueList<Item>::Iterator it = newItems.begin(); it != newItems.end(); ++it ) {
	Item i = *it;
	if ( !i.pix.isNull() )
	    (void)new QListBoxPixmap( listbox, i.pix, i.text );
	else
	    (void)new QListBoxText( listbox, i.text );
    }
    formWindow()->mainWindow()->propertyeditor()->refetchData();
}

void PopulateListBoxCommand::unexecute()
{
    listbox->clear();
    for ( QValueList<Item>::Iterator it = oldItems.begin(); it != oldItems.end(); ++it ) {
	Item i = *it;
	if ( !i.pix.isNull() )
	    (void)new QListBoxPixmap( listbox, i.pix, i.text );
	else
	    (void)new QListBoxText( listbox, i.text );
    }
    formWindow()->mainWindow()->propertyeditor()->refetchData();
}

// ------------------------------------------------------------

PopulateIconViewCommand::PopulateIconViewCommand( const QString &n, FormWindow *fw,
						  QIconView *iv, const QValueList<Item> &items )
    : Command( n, fw ), newItems( items ), iconview( iv )
{
#ifndef QT_NO_ICONVIEW
    QIconViewItem *i = 0;
    for ( i = iconview->firstItem(); i; i = i->nextItem() ) {
	Item item;
	if ( i->pixmap() )
	    item.pix = *i->pixmap();
	item.text = i->text();
	oldItems.append( item );
    }
#endif
}

void PopulateIconViewCommand::execute()
{
#ifndef QT_NO_ICONVIEW
    iconview->clear();
    for ( QValueList<Item>::Iterator it = newItems.begin(); it != newItems.end(); ++it ) {
	Item i = *it;
	(void)new QIconViewItem( iconview, i.text, i.pix );
    }
#endif
}

void PopulateIconViewCommand::unexecute()
{
#ifndef QT_NO_ICONVIEW
    iconview->clear();
    for ( QValueList<Item>::Iterator it = oldItems.begin(); it != oldItems.end(); ++it ) {
	Item i = *it;
	(void)new QIconViewItem( iconview, i.text, i.pix );
    }
#endif
}

// ------------------------------------------------------------

PopulateListViewCommand::PopulateListViewCommand( const QString &n, FormWindow *fw,
						  QListView *lv, QListView *from )
    : Command( n, fw ), listview( lv )
{
    newItems = new QListView();
    newItems->hide();
    transferItems( from, newItems );
    oldItems = new QListView();
    oldItems->hide();
    transferItems( listview, oldItems );
}

void PopulateListViewCommand::execute()
{
    listview->clear();
    transferItems( newItems, listview );
}

void PopulateListViewCommand::unexecute()
{
    listview->clear();
    transferItems( oldItems, listview );
}

void PopulateListViewCommand::transferItems( QListView *from, QListView *to )
{
    QHeader *header = to->header();
    while ( header->count() )
	to->removeColumn( 0 );
    QHeader *h2 = from->header();
    for ( int i = 0; i < h2->count(); ++i ) {
	to->addColumn( h2->label( i ) );
	if ( h2->iconSet( i ) && !h2->iconSet( i )->pixmap().isNull() )
	    header->setLabel( i, *h2->iconSet( i ), h2->label( i ) );
	header->setResizeEnabled( h2->isResizeEnabled( i ), i );
	header->setClickEnabled( h2->isClickEnabled( i ), i );
    }

    QListViewItemIterator it( from );
    QPtrStack<QListViewItem> fromParents, toParents;
    fromParents.push( 0 );
    toParents.push( 0 );
    QPtrStack<QListViewItem> toLasts;
    QListViewItem *fromLast = 0;
    toLasts.push( 0 );
    int cols = from->columns();
    to->setSorting( -1 );
    from->setSorting( -1 );
    for ( ; it.current(); ++it ) {
	QListViewItem *i = it.current();
	if ( i->parent() == fromParents.top() ) {
	    QListViewItem *pi = toParents.top();
	    QListViewItem *ni = 0;
	    if ( pi )
		ni = new QListViewItem( pi, toLasts.top() );
	    else
		ni = new QListViewItem( to, toLasts.top() );
	    for ( int c = 0; c < cols; ++c ) {
		ni->setText( c, i->text( c ) );
		if ( i->pixmap( c ) )
		    ni->setPixmap( c, *i->pixmap( c ) );
	    }
	    toLasts.pop();
	    toLasts.push( ni );
	    if ( pi )
		pi->setOpen( TRUE );
	} else {
	    if ( i->parent() == fromLast ) {
		fromParents.push( fromLast );
		toParents.push( toLasts.top() );
		toLasts.push( 0 );
		QListViewItem *pi = toParents.top();
		QListViewItem *ni = 0;
		if ( pi )
		    ni = new QListViewItem( pi );
		else
		    ni = new QListViewItem( to );
		for ( int c = 0; c < cols; ++c ) {
		    ni->setText( c, i->text( c ) );
		    if ( i->pixmap( c ) )
			ni->setPixmap( c, *i->pixmap( c ) );
		}
		toLasts.pop();
		toLasts.push( ni );
		if ( pi )
		    pi->setOpen( TRUE );
	    } else {
		while ( fromParents.top() != i->parent() ) {
		    fromParents.pop();
		    toParents.pop();
		    toLasts.pop();
		}

		QListViewItem *pi = toParents.top();
		QListViewItem *ni = 0;
		if ( pi )
		    ni = new QListViewItem( pi, toLasts.top() );
		else
		    ni = new QListViewItem( to, toLasts.top() );
		for ( int c = 0; c < cols; ++c ) {
		    ni->setText( c, i->text( c ) );
		    if ( i->pixmap( c ) )
			ni->setPixmap( c, *i->pixmap( c ) );
		}
		if ( pi )
		    pi->setOpen( TRUE );
		toLasts.pop();
		toLasts.push( ni );
	    }
	}
	fromLast = i;
    }
}



// ------------------------------------------------------------

PopulateMultiLineEditCommand::PopulateMultiLineEditCommand( const QString &n, FormWindow *fw,
						    QTextEdit *mle, const QString &txt )
    : Command( n, fw ), newText( txt ), mlined( mle )
{
    oldText = mlined->text();
    wasChanged = MetaDataBase::isPropertyChanged( mlined, "text" );
}

void PopulateMultiLineEditCommand::execute()
{
    mlined->setText( newText );
    MetaDataBase::setPropertyChanged( mlined, "text", TRUE );
    formWindow()->emitUpdateProperties( mlined );
}

void PopulateMultiLineEditCommand::unexecute()
{
    mlined->setText( oldText );
    MetaDataBase::setPropertyChanged( mlined, "text", wasChanged );
    formWindow()->emitUpdateProperties( mlined );
}

// ------------------------------------------------------------

PopulateTableCommand::PopulateTableCommand( const QString &n, FormWindow *fw, QTable *t,
					    const QValueList<Row> &rows,
					    const QValueList<Column> &columns )
    : Command( n, fw ), newRows( rows ), newColumns( columns ), table( t )
{
#ifndef QT_NO_TABLE
    int i = 0;
    QMap<QString, QString> columnFields = MetaDataBase::columnFields( table );
    for ( i = 0; i < table->horizontalHeader()->count(); ++i ) {
	PopulateTableCommand::Column col;
	col.text = table->horizontalHeader()->label( i );
	if ( table->horizontalHeader()->iconSet( i ) )
	    col.pix = table->horizontalHeader()->iconSet( i )->pixmap();
	col.field = *columnFields.find( col.text );
	oldColumns.append( col );
    }
    for ( i = 0; i < table->verticalHeader()->count(); ++i ) {
	PopulateTableCommand::Row row;
	row.text = table->verticalHeader()->label( i );
	if ( table->verticalHeader()->iconSet( i ) )
	    row.pix = table->verticalHeader()->iconSet( i )->pixmap();
	oldRows.append( row );
    }
#endif
}

void PopulateTableCommand::execute()
{
#ifndef QT_NO_TABLE
    QMap<QString, QString> columnFields;
    table->setNumCols( newColumns.count() );
    int i = 0;
    for ( QValueList<Column>::Iterator cit = newColumns.begin(); cit != newColumns.end(); ++cit, ++i ) {
	table->horizontalHeader()->setLabel( i, (*cit).pix, (*cit).text );
	if ( !(*cit).field.isEmpty() )
	    columnFields.insert( (*cit).text, (*cit).field );
    }
    MetaDataBase::setColumnFields( table, columnFields );
    table->setNumRows( newRows.count() );
    i = 0;
    for ( QValueList<Row>::Iterator rit = newRows.begin(); rit != newRows.end(); ++rit, ++i )
	table->verticalHeader()->setLabel( i, (*rit).pix, (*rit).text );
#endif
}

void PopulateTableCommand::unexecute()
{
#ifndef QT_NO_TABLE
    QMap<QString, QString> columnFields;
    table->setNumCols( oldColumns.count() );
    int i = 0;
    for ( QValueList<Column>::Iterator cit = oldColumns.begin(); cit != oldColumns.end(); ++cit, ++i ) {
	table->horizontalHeader()->setLabel( i, (*cit).pix, (*cit).text );
	if ( !(*cit).field.isEmpty() )
	    columnFields.insert( (*cit).text, (*cit).field );
    }
    MetaDataBase::setColumnFields( table, columnFields );
    table->setNumRows( oldRows.count() );
    i = 0;
    for ( QValueList<Row>::Iterator rit = oldRows.begin(); rit != oldRows.end(); ++rit, ++i )
	table->verticalHeader()->setLabel( i, (*rit).pix, (*rit).text );
#endif
}

// ------------------------------------------------------------

AddActionToToolBarCommand::AddActionToToolBarCommand( const QString &n, FormWindow *fw,
						      QAction *a, QDesignerToolBar *tb, int idx )
    : Command( n, fw ), action( a ), toolBar( tb ), index( idx )
{
}

void AddActionToToolBarCommand::execute()
{
    action->addTo( toolBar );

    if ( ::qt_cast<QDesignerAction*>(action) ) {
	QString s = ( (QDesignerAction*)action )->widget()->name();
	if ( s.startsWith( "qt_dead_widget_" ) ) {
	    s.remove( 0, QString( "qt_dead_widget_" ).length() );
	    ( (QDesignerAction*)action )->widget()->setName( s );
	}
	toolBar->insertAction( ( (QDesignerAction*)action )->widget(), action );
	( (QDesignerAction*)action )->widget()->installEventFilter( toolBar );
    } else if ( ::qt_cast<QDesignerActionGroup*>(action) ) {
	if ( ( (QDesignerActionGroup*)action )->usesDropDown() ) {
	    toolBar->insertAction( ( (QDesignerActionGroup*)action )->widget(), action );
	    ( (QDesignerActionGroup*)action )->widget()->installEventFilter( toolBar );
	}
    } else if ( ::qt_cast<QSeparatorAction*>(action) ) {
	toolBar->insertAction( ( (QSeparatorAction*)action )->widget(), action );
	( (QSeparatorAction*)action )->widget()->installEventFilter( toolBar );
    }
    if ( !::qt_cast<QActionGroup*>(action) || ( (QActionGroup*)action )->usesDropDown()) {
	if ( index == -1 )
	    toolBar->appendAction( action );
	else
	    toolBar->insertAction( index, action );
	toolBar->reInsert();
	QObject::connect( action, SIGNAL( destroyed() ), toolBar, SLOT( actionRemoved() ) );
    } else {
	if ( action->children() ) {
	    QObjectListIt it( *action->children() );
	    int i = 0;
	    while ( it.current() ) {
		QObject *o = it.current();
		++it;
		if ( !::qt_cast<QAction*>(o) )
		    continue;
		// ### fix it for nested actiongroups
		if ( ::qt_cast<QDesignerAction*>(o) ) {
		    QDesignerAction *ac = (QDesignerAction*)o;
		    toolBar->insertAction( ac->widget(), ac );
		    ac->widget()->installEventFilter( toolBar );
		    if ( index == -1 )
			toolBar->appendAction( ac );
		    else
			toolBar->insertAction( index + (i++), ac );
		}
		QObject::connect( o, SIGNAL( destroyed() ), toolBar, SLOT( actionRemoved() ) );
	    }
	}
	toolBar->reInsert();
	QObject::connect( action, SIGNAL( destroyed() ), toolBar, SLOT( actionRemoved() ) );
    }
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void AddActionToToolBarCommand::unexecute()
{
    if ( ::qt_cast<QDesignerAction*>(action) ) {
	QString s = ( (QDesignerAction*)action )->widget()->name();
	s.prepend( "qt_dead_widget_" );
	( (QDesignerAction*)action )->widget()->setName( s );
    }

    toolBar->removeAction( action );
    action->removeFrom( toolBar );
    QObject::disconnect( action, SIGNAL( destroyed() ), toolBar, SLOT( actionRemoved() ) );
    if ( !::qt_cast<QActionGroup*>(action) || ( (QActionGroup*)action )->usesDropDown()) {
	action->removeEventFilter( toolBar );
    } else {
	if ( action->children() ) {
	    QObjectListIt it( *action->children() );
	    while ( it.current() ) {
		QObject *o = it.current();
		++it;
		if ( !::qt_cast<QAction*>(o) )
		    continue;
		if ( ::qt_cast<QDesignerAction*>(o) ) {
		    o->removeEventFilter( toolBar );
		    toolBar->removeAction( (QAction*)o );
		}
		QObject::disconnect( o, SIGNAL( destroyed() ), toolBar, SLOT( actionRemoved() ) );
	    }
	}
    }
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

AddToolBarCommand::AddToolBarCommand( const QString &n, FormWindow *fw, QMainWindow *mw )
    : Command( n, fw ), toolBar( 0 ), mainWindow( mw )
{
}

void AddToolBarCommand::execute()
{
    if ( !toolBar ) {
	toolBar = new QDesignerToolBar( mainWindow );
	QString n = "Toolbar";
	formWindow()->unify( toolBar, n, TRUE );
	toolBar->setName( n );
	mainWindow->addToolBar( toolBar, n );
    } else {
	toolBar->show();
	QString s = toolBar->name();
	s.remove( 0, QString( "qt_dead_widget_" ).length() );
	toolBar->setName( s );
    }
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void AddToolBarCommand::unexecute()
{
    toolBar->hide();
    QString s = toolBar->name();
    s.prepend( "qt_dead_widget_" );
    toolBar->setName( s );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

#ifdef QT_CONTAINER_CUSTOM_WIDGETS
AddContainerPageCommand::AddContainerPageCommand( const QString &n, FormWindow *fw,
						  QWidget *c, const QString &label )
    : Command( n, fw ), container( c ), index( -1 ), pageLabel( label ), page( 0 )
{
    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( WidgetFactory::classNameOf( container ), &iface );
    if ( !iface )
	return;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&wiface );
    if ( !wiface )
	return;
    wClassName = WidgetFactory::classNameOf( container );
}

AddContainerPageCommand::~AddContainerPageCommand()
{
    if ( wiface )
	wiface->release();
}

void AddContainerPageCommand::execute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;
    if ( index == -1 )
	index = wiface->count( wClassName, container );
    if ( !page )
	page = wiface->addPage( wClassName, container, pageLabel, index );
    else
	wiface->insertPage( wClassName, container, pageLabel, index, page );

    MetaDataBase::addEntry( page );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}

void AddContainerPageCommand::unexecute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;
    wiface->removePage( wClassName, container, index );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}

// ------------------------------------------------------------

DeleteContainerPageCommand::DeleteContainerPageCommand( const QString &n, FormWindow *fw,
							QWidget *c, int idx )
    : Command( n, fw ), container( c ), index( idx )
{
    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( WidgetFactory::classNameOf( container ), &iface );
    if ( !iface )
	return;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&wiface );
    if ( !wiface )
	return;
    wClassName = WidgetFactory::classNameOf( container );
    page = wiface->page( wClassName, container, index );
    pageLabel = wiface->pageLabel( wClassName, container, index );
}

DeleteContainerPageCommand::~DeleteContainerPageCommand()
{
    if ( wiface )
	wiface->release();
}

void DeleteContainerPageCommand::execute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;

    wiface->removePage( wClassName, container, index );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}

void DeleteContainerPageCommand::unexecute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;
    if ( index == -1 )
	index = wiface->count( wClassName, container );

    wiface->insertPage( wClassName, container, pageLabel, index, page );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}

// ------------------------------------------------------------

RenameContainerPageCommand::RenameContainerPageCommand( const QString &n, FormWindow *fw,
							QWidget *c, int idx,
							const QString &label )
    : Command( n, fw ), container( c ), index( idx ), newLabel( label )
{
    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( WidgetFactory::classNameOf( container ), &iface );
    if ( !iface )
	return;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&wiface );
    if ( !wiface )
	return;
    wClassName = WidgetFactory::classNameOf( container );
    oldLabel = wiface->pageLabel( wClassName, container, index );
}

RenameContainerPageCommand::~RenameContainerPageCommand()
{
    if ( wiface )
	wiface->release();
}

void RenameContainerPageCommand::execute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;
    wiface->renamePage( wClassName, container, index, newLabel );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}

void RenameContainerPageCommand::unexecute()
{
    if ( !wiface || !wiface->supportsPages( wClassName ) )
	return;
    wiface->renamePage( wClassName, container, index, oldLabel );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    // #### show and update pages in object hierarchy view
}
#endif // QT_CONTAINER_CUSTOM_WIDGETS
// ------------------------------------------------------------

AddActionToPopupCommand::AddActionToPopupCommand( const QString &n,
						  FormWindow *fw,
						  PopupMenuEditor *m,
						  PopupMenuEditorItem *i,
						  int idx )
    : Command( n, fw ), menu( m ), item( i ), index( idx )
{ }

void AddActionToPopupCommand::execute()
{
    menu->insert( item, index );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void AddActionToPopupCommand::unexecute()
{
    item->hideMenu();
    int i = menu->find( item->action() );
    menu->remove( i );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

RemoveActionFromPopupCommand::RemoveActionFromPopupCommand( const QString &n,
							    FormWindow *fw,
							    PopupMenuEditor *m,
							    int idx )
    : AddActionToPopupCommand( n, fw, m, 0, idx )
{
    item = menu->at( index );
}

void RemoveActionFromPopupCommand::execute()
{
    AddActionToPopupCommand::unexecute();
}

void RemoveActionFromPopupCommand::unexecute()
{
    AddActionToPopupCommand::execute();
}

// ------------------------------------------------------------

ExchangeActionInPopupCommand::ExchangeActionInPopupCommand( const QString &n,
							    FormWindow *fw,
							    PopupMenuEditor *m,
							    int a,
							    int b )
    : Command( n, fw ), menu( m ), c( a ), d( b )
{ }

void ExchangeActionInPopupCommand::execute()
{
    menu->exchange( c, d );
}

void ExchangeActionInPopupCommand::unexecute()
{
    execute();
}

// ------------------------------------------------------------

ActionEditor *ActionCommand::actionEditor()
{
    // FIXME: handle multiple action editors
    return (ActionEditor *) formWindow()->mainWindow()->child( 0, "ActionEditor" );
}

// ------------------------------------------------------------

RenameActionCommand::RenameActionCommand( const QString &n,
					  FormWindow *fw,
					  QAction *a,
					  PopupMenuEditor *m,
					  QString nm )
    : ActionCommand( n, fw, a ), menu( m ), newName( nm )
{
    oldName = action->menuText();
}

void RenameActionCommand::execute()
{
    QString actionText = newName;
    actionText.replace("&&", "&");
    QString menuText = newName;
    action->setText( actionText );
    action->setMenuText( menuText );

    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void RenameActionCommand::unexecute()
{
    action->setMenuText( oldName );
    action->setText( oldName );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

SetActionIconsCommand::SetActionIconsCommand( const QString &n,
					      FormWindow *fw,
					      QAction *a,
					      PopupMenuEditor *m,
					      QIconSet &icons )
    : ActionCommand( n, fw, a ), menu( m ), newIcons( icons )
{
    oldIcons = a->iconSet();
}

void SetActionIconsCommand::execute()
{
    action->setIconSet( newIcons );
    ActionEditor *ae = actionEditor();
    if ( ae )
	ae->updateActionIcon( action );
    MetaDataBase::setPropertyChanged( action, "iconSet", TRUE );
}

void SetActionIconsCommand::unexecute()
{
    action->setIconSet( oldIcons );
    ActionEditor *ae = actionEditor();
    if ( ae )
	ae->updateActionIcon( action );
    MetaDataBase::setPropertyChanged( action, "iconSet", TRUE );
}

// ------------------------------------------------------------

AddMenuCommand::AddMenuCommand( const QString &n,
				FormWindow *fw,
				MenuBarEditor *b,
				MenuBarEditorItem *i,
				int idx )
    : Command( n, fw ), mb( b ), item( i ), name( 0 ), index( idx )
{ }

AddMenuCommand::AddMenuCommand( const QString &n,
				FormWindow *fw,
				QMainWindow *mw,
				const QString &nm )
    : Command( n, fw ), mb( 0 ), item( 0 ), name( nm ), index( -1 )
{
    if ( mw )
	mb = (MenuBarEditor *)mw->child( 0, "MenuBarEditor" );
}

void AddMenuCommand::execute()
{
    QString n;
    QMainWindow *mw = (QMainWindow*)formWindow()->mainContainer();
    if ( !mb ) {
	mb = new MenuBarEditor( formWindow(), mw );
	mb->setName( "MenuBarEditor" );
	formWindow()->insertWidget( mb, TRUE );
    }
    if ( !item ) {
	PopupMenuEditor *popup = new PopupMenuEditor( formWindow(), mw );
	popup->setName( "PopupMenuEditor" );
	formWindow()->insertWidget( popup, TRUE );
	mb->insertItem( name, popup, index );
	index = mb->findItem( popup );
	item = mb->item( index );
    } else {
	PopupMenuEditor *popup = item->menu();
	popup->setName( item->menuText() );
	formWindow()->insertWidget( popup, TRUE );
	mb->insertItem( item, index );
    }
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void AddMenuCommand::unexecute()
{
    if ( !item ) {
	qWarning( "The AddMenuCommand was created without a menu item." );
	return;
    }
    item->menu()->hide();
    int i = mb->findItem( item );
    formWindow()->removeWidget( item->menu() );
    mb->removeItemAt( i );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

RemoveMenuCommand::RemoveMenuCommand( const QString &n,
				      FormWindow *fw,
				      MenuBarEditor *b,
				      int idx )
    : AddMenuCommand( n, fw, b, 0, idx )
{
    item = mb->item( index );
}

void RemoveMenuCommand::execute()
{
    mb->hideItem( index );
    AddMenuCommand::unexecute();
}

void RemoveMenuCommand::unexecute()
{
    AddMenuCommand::execute();
}

// ------------------------------------------------------------

ExchangeMenuCommand::ExchangeMenuCommand( const QString &n,
					  FormWindow *fw,
					  MenuBarEditor *b,
					  int i,
					  int j )
    : Command( n, fw ), bar( b ), k( i ), l( j )
{ }

void ExchangeMenuCommand::execute()
{
    bar->exchange( k, l );
}

void ExchangeMenuCommand::unexecute()
{
    execute();
}

// ------------------------------------------------------------

MoveMenuCommand::MoveMenuCommand( const QString &n,
				  FormWindow *fw,
				  MenuBarEditor *b,
				  int i,
				  int j )
    : Command( n, fw ), bar( b ), from( i ), to( j )
{ }

void MoveMenuCommand::execute()
{
    bar->hideItem( from );
    MenuBarEditorItem * i = bar->item( from );
    bar->removeItemAt( from );
    int t = ( from > to ? to : to - 1 );
    bar->insertItem( i, t );
}

void MoveMenuCommand::unexecute()
{
    bar->hideItem( to );
    int t = ( from > to ? to : to - 1 );
    MenuBarEditorItem * i = bar->item( t );
    bar->removeItemAt( t );
    bar->insertItem( i, from );
}

// ------------------------------------------------------------

RenameMenuCommand::RenameMenuCommand( const QString &n,
				      FormWindow *fw,
				      MenuBarEditor *b,
				      QString nm,
				      MenuBarEditorItem *i )
    : Command( n, fw ), bar( b ), item( i ), newName( nm )
{
    oldName = item->menu()->name();
}

QString RenameMenuCommand::makeLegal( const QString &str )
{
    // remove illegal characters
    QString d;
    char c = 0, i = 0;
    while ( !str.at(i).isNull() ) {
	c = str.at(i).latin1();
	if ( c == '-' || c == ' ' )
	    d += '_';
	else if ( ( c >= '0' && c <= '9') || ( c >= 'A' && c <= 'Z' )
                  || c == '_' || ( c >= 'a' && c <= 'z' ) )
	    d += c;
	++i;
    }
    return d;
}

void RenameMenuCommand::execute()
{
    PopupMenuEditor *popup = item->menu();
    item->setMenuText( newName );
    QString legal = makeLegal( newName );
    formWindow()->unify( popup, legal, TRUE );
    popup->setName( legal );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void RenameMenuCommand::unexecute()
{
    PopupMenuEditor *popup = item->menu();
    item->setMenuText( oldName );
    popup->setName( oldName );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

AddToolBoxPageCommand::AddToolBoxPageCommand( const QString &n, FormWindow *fw,
				      QToolBox *tw, const QString &label )
    : Command( n, fw ), toolBox( tw ), toolBoxLabel( label )
{
    toolBoxPage = new QDesignerWidget( formWindow(), toolBox, "page" );
    toolBoxPage->hide();
    index = -1;
    MetaDataBase::addEntry( toolBoxPage );
}

void AddToolBoxPageCommand::execute()
{
    index = toolBox->insertItem( index, toolBoxPage, toolBoxLabel);
    toolBox->setCurrentIndex( index );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void AddToolBoxPageCommand::unexecute()
{
    toolBox->removeItem( toolBoxPage );
    toolBoxPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

// ------------------------------------------------------------

DeleteToolBoxPageCommand::DeleteToolBoxPageCommand( const QString &n, FormWindow *fw,
					    QToolBox *tw, QWidget *page )
    : Command( n, fw ), toolBox( tw ), toolBoxPage( page )
{
    toolBoxLabel = toolBox->itemLabel( toolBox->currentIndex() );
    index = toolBox->currentIndex();
}

void DeleteToolBoxPageCommand::execute()
{
    toolBox->removeItem( toolBoxPage );
    toolBoxPage->hide();
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

void DeleteToolBoxPageCommand::unexecute()
{
    index = toolBox->insertItem( index, toolBoxPage, toolBoxLabel );
    toolBox->setCurrentIndex( index );
    formWindow()->emitUpdateProperties( formWindow()->currentWidget() );
    formWindow()->mainWindow()->objectHierarchy()->rebuild();
}

/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "customwidgeteditorimpl.h"
#include "defs.h"
#include "pixmapchooser.h"
#include "mainwindow.h"
#include "asciivalidator.h"
#include "resource.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"

#include <kfiledialog.h>
#include <klineedit.h>

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdom.h>
#include <qtextcodec.h>
#include <qcheckbox.h>

#include <klocale.h>

CustomWidgetEditor::CustomWidgetEditor( QWidget *parent, MainWindow *mw )
    : CustomWidgetEditorBase( parent, 0, TRUE ), mainWindow( mw )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    checkTimer = new QTimer( this );
    connect( checkTimer, SIGNAL( timeout() ),
	     this, SLOT( checkWidgetName() ) );

    editClass->setEnabled( FALSE );
    editHeader->setEnabled( FALSE );
    buttonChooseHeader->setEnabled( FALSE );
    buttonChoosePixmap->setEnabled( FALSE );
    spinWidth->setEnabled( FALSE );
    spinHeight->setEnabled( FALSE );
    sizeHor->setEnabled( FALSE );
    sizeVer->setEnabled( FALSE );
    checkContainer->setEnabled( FALSE );
    localGlobalCombo->setEnabled( FALSE );
    editClass->setValidator( new AsciiValidator( QString(":"), editClass ) );
    editSignal->setValidator( new AsciiValidator( TRUE, editSignal ) );
    editSlot->setValidator( new AsciiValidator( TRUE, editSignal ) );
    editProperty->setValidator( new AsciiValidator( editSignal ) );
    editSignal->setEnabled( FALSE );
    buttonRemoveSignal->setEnabled( FALSE );

    setupDefinition();
    setupSignals();
    setupSlots();
    setupProperties();
}

void CustomWidgetEditor::setupDefinition()
{
    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	QListBoxItem *i;
	if ( w->pixmap )
	    i = new QListBoxPixmap( boxWidgets, *w->pixmap, w->className );
	else
	    i = new QListBoxText( boxWidgets, w->className );
	customWidgets.insert( i, w );
    }

    if ( boxWidgets->firstItem() ) {
	boxWidgets->setCurrentItem( boxWidgets->firstItem() );
	boxWidgets->setSelected( boxWidgets->firstItem(), TRUE );
    }
    oldItem = 0;
}

void CustomWidgetEditor::setupSignals()
{
    editSignal->setEnabled( FALSE );
    buttonRemoveSignal->setEnabled( FALSE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w )
	return;
    listSignals->clear();
    for ( QValueList<QCString>::Iterator it = w->lstSignals.begin(); it != w->lstSignals.end(); ++it )
	listSignals->insertItem( QString( *it ) );
    if ( listSignals->firstItem() ) {
	listSignals->setCurrentItem( listSignals->firstItem() );
	listSignals->setSelected( listSignals->firstItem(), TRUE );
    }
}

void CustomWidgetEditor::setupSlots()
{
    editSlot->setEnabled( FALSE );
    comboAccess->setEnabled( FALSE );
    buttonRemoveSlot->setEnabled( FALSE );

    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w )
	return;
    listSlots->clear();
    for ( QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.begin(); it != w->lstSlots.end(); ++it )
	(void)new QListViewItem( listSlots, (*it).function, (*it).access );

    if ( listSlots->firstChild() ) {
	listSlots->setCurrentItem( listSlots->firstChild() );
	listSlots->setSelected( listSlots->firstChild(), TRUE );
    }
}

void CustomWidgetEditor::setupProperties()
{
    editProperty->setEnabled( FALSE );
    comboType->setEnabled( FALSE );
    buttonRemoveProperty->setEnabled( FALSE );

    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w )
	return;
    listProperties->clear();
    for ( QValueList<MetaDataBase::Property>::Iterator it = w->lstProperties.begin(); it != w->lstProperties.end(); ++it )
	(void)new QListViewItem( listProperties, (*it).property, (*it).type );

    if ( listProperties->firstChild() ) {
	listProperties->setCurrentItem( listProperties->firstChild() );
	listProperties->setSelected( listProperties->firstChild(), TRUE );
    }
}

void CustomWidgetEditor::currentWidgetChanged( QListBoxItem *i )
{
    checkTimer->stop();
    checkWidgetName();

    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w ) {
	buttonChooseHeader->setEnabled( FALSE );
	editClass->setEnabled( FALSE );
	editHeader->setEnabled( FALSE );
	buttonChoosePixmap->setEnabled( FALSE );
	spinWidth->setEnabled( FALSE );
	spinHeight->setEnabled( FALSE );
	localGlobalCombo->setEnabled( FALSE );
	sizeHor->setEnabled( FALSE );
	sizeVer->setEnabled( FALSE );
	checkContainer->setEnabled( FALSE );
	return;
    }

    buttonChooseHeader->setEnabled( TRUE );
    editClass->setEnabled( TRUE );
    editHeader->setEnabled( TRUE );
    buttonChoosePixmap->setEnabled( TRUE );
    spinWidth->setEnabled( TRUE );
    spinHeight->setEnabled( TRUE );
    localGlobalCombo->setEnabled( TRUE );
    sizeHor->setEnabled( TRUE );
    sizeVer->setEnabled( TRUE );
    checkContainer->setEnabled( TRUE );

    editClass->blockSignals( TRUE );
    editClass->setText( w->className );
    editClass->blockSignals( FALSE );
    editHeader->setText( w->includeFile );
    localGlobalCombo->setCurrentItem( (int)w->includePolicy );
    if ( w->pixmap )
	previewPixmap->setPixmap( *w->pixmap );
    else
	previewPixmap->setText( "" );
    spinWidth->setValue( w->sizeHint.width() );
    spinHeight->setValue( w->sizeHint.height() );
    sizeHor->setCurrentItem( size_type_to_int( w->sizePolicy.horData() ) );
    sizeVer->setCurrentItem( size_type_to_int( w->sizePolicy.verData() ) );
    checkContainer->setChecked( w->isContainer );

    setupSignals();
    setupSlots();
    setupProperties();
}

void CustomWidgetEditor::addWidgetClicked()
{
    oldItem = 0;
    checkTimer->stop();
    checkWidgetName();

    MetaDataBase::CustomWidget *w = new MetaDataBase::CustomWidget;
    QString s = w->className;
    if ( !MetaDataBase::addCustomWidget( w ) ) {
	QMessageBox::information( this, i18n( "Adding Custom Widget" ),
				  i18n( "Custom widget names must be unique.\n"
				      "A custom widget called '%1' already exists, so it is not possible "
				      "to add another widget with this name." ).arg( s ) );
	return;
    }

    QListBoxPixmap *i = new QListBoxPixmap( boxWidgets, *w->pixmap, w->className );
    customWidgets.insert( i, w );

    boxWidgets->setCurrentItem( i );
    boxWidgets->setSelected( i, TRUE );
}

void CustomWidgetEditor::classNameChanged( const QString &s )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    WidgetDatabase::customWidgetClassNameChanged( w->className, s );

    checkTimer->stop();
    boxWidgets->blockSignals( TRUE );
    oldName = w->className;
    w->className = s;
    QListBoxItem *old = i;
    if ( w->pixmap )
	boxWidgets->changeItem( *w->pixmap, s, boxWidgets->currentItem() );
    else
	boxWidgets->changeItem( s, boxWidgets->currentItem() );
    i = boxWidgets->item( boxWidgets->currentItem() );
    oldItem = i;
    customWidgets.insert( i, w );
    customWidgets.remove( old );
    boxWidgets->blockSignals( FALSE );
    checkTimer->start( 1000, TRUE );
}

void CustomWidgetEditor::deleteWidgetClicked()
{
    oldItem = 0;
    checkTimer->stop();
    checkWidgetName();

    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );

    if ( mainWindow->isCustomWidgetUsed( w ) ) {
	QMessageBox::information( mainWindow, i18n( "Removing Custom Widget" ),
				  i18n( "The custom widget '%1' is in use, so it cannot be removed." ).
				  arg( w->className ) );
	return;
    }

    if ( !i || !w )
	return;

    MetaDataBase::CustomWidget *cw = MetaDataBase::customWidget( mainWindow->currentTool() );
    if ( cw == w )
	mainWindow->resetTool();

    MetaDataBase::removeCustomWidget( w );
    customWidgets.remove( i );
    delete i;

    i = boxWidgets->item( boxWidgets->currentItem() );
    if ( i ) {
	boxWidgets->setCurrentItem( i );
	boxWidgets->setSelected( i, TRUE );
    }
}

void CustomWidgetEditor::headerFileChanged( const QString &s )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    w->includeFile = s;
}

void CustomWidgetEditor::heightChanged( int h )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    w->sizeHint.setHeight( h );
    updateCustomWidgetSizes();
}

void CustomWidgetEditor::includePolicyChanged( int p )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    w->includePolicy = (MetaDataBase::CustomWidget::IncludePolicy)p;
}

void CustomWidgetEditor::pixmapChoosen()
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    QPixmap pix = qChoosePixmap( this );
    if ( pix.isNull() )
	return;
    delete w->pixmap;
    w->pixmap = new QPixmap( pix );

    boxWidgets->blockSignals( TRUE );
    QListBoxItem *old = i;
    boxWidgets->changeItem( *w->pixmap, w->className, boxWidgets->currentItem() );
    i = boxWidgets->item( boxWidgets->currentItem() );
    customWidgets.insert( i, w );
    customWidgets.remove( old );
    boxWidgets->blockSignals( FALSE );
    previewPixmap->setPixmap( *w->pixmap );
}

void CustomWidgetEditor::widthChanged( int wid )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    w->sizeHint.setWidth( wid );
    updateCustomWidgetSizes();
}

MetaDataBase::CustomWidget *CustomWidgetEditor::findWidget( QListBoxItem *i )
{
    if ( !i )
	return 0;

    QMap<QListBoxItem*, MetaDataBase::CustomWidget*>::Iterator it = customWidgets.find( i );
    if ( it == customWidgets.end() )
	return 0;
    return *it;
}

void CustomWidgetEditor::chooseHeader()
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    QString h = KFileDialog::getOpenFileName( QString::null, i18n( "*.h *.h++ *.hxx *.hh|Header Files" ), this );
    if ( h.isEmpty() )
	return;
    editHeader->setText( h );
    localGlobalCombo->setCurrentItem( (int)MetaDataBase::CustomWidget::Global );
}

void CustomWidgetEditor::checkWidgetName()
{
    QListBoxItem *i = oldItem ? oldItem : boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    oldItem = 0;
    if ( !i || !w )
	return;

    if ( MetaDataBase::isWidgetNameUsed( w ) ) {
	QString s = w->className;
	w->className = oldName;
	QMessageBox::information( this, i18n( "Renaming Custom Widget" ),
				  i18n( "Custom widget names must be unique.\n"
				      "A custom widget called '%1' already exists, so it is not possible "
				      "to rename this widget with this name." ).arg( s ) );
	if ( i != boxWidgets->item( boxWidgets->currentItem() ) ) {
	    boxWidgets->setCurrentItem( i );
	    qApp->processEvents();
	}
	editClass->setText( w->className );
	classNameChanged( w->className );
    }
}

void CustomWidgetEditor::closeClicked()
{
    oldItem = 0;
    checkTimer->stop();
    checkWidgetName();
    accept();
}

void CustomWidgetEditor::currentSignalChanged( QListBoxItem *i )
{
    editSignal->blockSignals( TRUE );
    editSignal->setText( "" );
    editSignal->blockSignals( FALSE );

    if ( !i ) {
	editSignal->setEnabled( FALSE );
	buttonRemoveSignal->setEnabled( FALSE );
	return;
    }

    editSignal->blockSignals( TRUE );
    editSignal->setEnabled( TRUE );
    buttonRemoveSignal->setEnabled( TRUE );
    editSignal->setText( i->text() );
    editSignal->blockSignals( FALSE );
}

void CustomWidgetEditor::addSignal()
{
    QListBoxItem *i = new QListBoxText( listSignals, "signal()" );
    listSignals->setCurrentItem( i );
    listSignals->setSelected( i, TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w )
	w->lstSignals.append( i->text().latin1() );
}

void CustomWidgetEditor::removeSignal()
{
    QString s = listSignals->currentText();
    delete listSignals->item( listSignals->currentItem() );
    if ( listSignals->currentItem() != -1 )
	listSignals->setSelected( listSignals->currentItem(), TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w )
	w->lstSignals.remove( s.latin1() );
}

void CustomWidgetEditor::signalNameChanged( const QString &s )
{
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w || listSignals->currentItem() == -1 )
	return;

    QValueList<QCString>::Iterator it = w->lstSignals.find( listSignals->currentText().latin1() );
    if ( it != w->lstSignals.end() )
	w->lstSignals.remove( it );
    listSignals->blockSignals( TRUE );
    listSignals->changeItem( s, listSignals->currentItem() );
    listSignals->blockSignals( FALSE );
    w->lstSignals.append( s.latin1() );
}

void CustomWidgetEditor::slotAccessChanged( const QString &s )
{
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w || !listSlots->currentItem() )
	return;

    MetaDataBase::Function slot;
    slot.function = listSlots->currentItem()->text( 0 );
    slot.access = listSlots->currentItem()->text( 1 );
    QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.find( slot );
    if ( it != w->lstSlots.end() )
	w->lstSlots.remove( it );
    listSlots->currentItem()->setText( 1, s );
    slot.function = listSlots->currentItem()->text( 0 );
    slot.access = listSlots->currentItem()->text( 1 );
    w->lstSlots.append( slot );
}

void CustomWidgetEditor::slotNameChanged( const QString &s )
{
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w || !listSlots->currentItem() )
	return;

    MetaDataBase::Function slot;
    slot.function = listSlots->currentItem()->text( 0 );
    slot.access = listSlots->currentItem()->text( 1 );
    slot.type = "slot";
    QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.find( slot );
    if ( it != w->lstSlots.end() )
	w->lstSlots.remove( it );
    listSlots->currentItem()->setText( 0, s );
    slot.function = listSlots->currentItem()->text( 0 );
    slot.access = listSlots->currentItem()->text( 1 );
    w->lstSlots.append( slot );
}

void CustomWidgetEditor::addSlot()
{
    QListViewItem *i = new QListViewItem( listSlots, "slot()", "public" );
    listSlots->setCurrentItem( i );
    listSlots->setSelected( i, TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w ) {
	MetaDataBase::Function slot;
	slot.function = "slot()";
	slot.access = "public";
	slot.type = "slot";
	w->lstSlots.append( slot );
    }
}

void CustomWidgetEditor::removeSlot()
{
    MetaDataBase::Function slot;
    slot.function = "1 2 3";
    if ( listSlots->currentItem() ) {
	slot.function = listSlots->currentItem()->text( 0 );
	slot.access = listSlots->currentItem()->text( 1 );
    }
    delete listSlots->currentItem();
    if ( listSlots->currentItem() )
	listSlots->setSelected( listSlots->currentItem(), TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w && slot.function != "1 2 3" )
	w->lstSlots.remove( slot );
}

void CustomWidgetEditor::currentSlotChanged( QListViewItem *i )
{
    editSlot->blockSignals( TRUE );
    editSlot->setText( "" );
    editSignal->blockSignals( FALSE );

    if ( !i ) {
	editSlot->setEnabled( FALSE );
	comboAccess->setEnabled( FALSE );
	buttonRemoveSlot->setEnabled( FALSE );
	return;
    }

    editSlot->setEnabled( TRUE );
    comboAccess->setEnabled( TRUE );
    buttonRemoveSlot->setEnabled( TRUE );
    editSlot->blockSignals( TRUE );
    comboAccess->blockSignals( TRUE );
    editSlot->setText( i->text( 0 ) );
    if ( i->text( 1 ) == i18n( "protected" ) )
	comboAccess->setCurrentItem( 1 );
    else
	comboAccess->setCurrentItem( 0 );
    editSlot->blockSignals( FALSE );
    comboAccess->blockSignals( FALSE );
}

void CustomWidgetEditor::propertyTypeChanged( const QString &s )
{
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w || !listProperties->currentItem() )
	return;

    MetaDataBase::Property property;
    property.property = listProperties->currentItem()->text( 0 );
    property.type = listProperties->currentItem()->text( 1 );
    QValueList<MetaDataBase::Property>::Iterator it = w->lstProperties.find( property );
    if ( it != w->lstProperties.end() )
	w->lstProperties.remove( it );
    listProperties->currentItem()->setText( 1, s );
    property.property = listProperties->currentItem()->text( 0 );
    property.type = listProperties->currentItem()->text( 1 );
    w->lstProperties.append( property );
}

void CustomWidgetEditor::propertyNameChanged( const QString &s )
{
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( !w || !listProperties->currentItem() )
	return;

    MetaDataBase::Property property;
    property.property = listProperties->currentItem()->text( 0 );
    property.type = listProperties->currentItem()->text( 1 );
    QValueList<MetaDataBase::Property>::Iterator it = w->lstProperties.find( property );
    if ( it != w->lstProperties.end() )
	w->lstProperties.remove( it );
    listProperties->currentItem()->setText( 0, s );
    property.property = listProperties->currentItem()->text( 0 );
    property.type = listProperties->currentItem()->text( 1 );
    w->lstProperties.append( property );
}

void CustomWidgetEditor::addProperty()
{
    QListViewItem *i = new QListViewItem( listProperties, "property", "String" );
    listProperties->setCurrentItem( i );
    listProperties->setSelected( i, TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w ) {
	MetaDataBase::Property prop;
	prop.property = "property";
	prop.type = "String";
	w->lstProperties.append( prop );
    }
}

void CustomWidgetEditor::removeProperty()
{
    MetaDataBase::Property property;
    property.property = "1 2 3";
    if ( listProperties->currentItem() ) {
	property.property = listProperties->currentItem()->text( 0 );
	property.type = listProperties->currentItem()->text( 1 );
    }
    delete listProperties->currentItem();
    if ( listProperties->currentItem() )
	listProperties->setSelected( listProperties->currentItem(), TRUE );
    MetaDataBase::CustomWidget *w = findWidget( boxWidgets->item( boxWidgets->currentItem() ) );
    if ( w && property.property != "1 2 3" )
	w->lstProperties.remove( property );
}

void CustomWidgetEditor::currentPropertyChanged( QListViewItem *i )
{
    editProperty->blockSignals( TRUE );
    editProperty->setText( "" );
    editSignal->blockSignals( FALSE );

    if ( !i ) {
	editProperty->setEnabled( FALSE );
	comboType->setEnabled( FALSE );
	buttonRemoveProperty->setEnabled( FALSE );
	return;
    }

    editProperty->setEnabled( TRUE );
    comboType->setEnabled( TRUE );
    buttonRemoveProperty->setEnabled( TRUE );
    editProperty->blockSignals( TRUE );
    comboType->blockSignals( TRUE );
    editProperty->setText( i->text( 0 ) );

    for ( int j = 0; j < comboType->count(); ++j ) {
	if ( i->text( 1 ) == comboType->text( j ) ) {
	    comboType->setCurrentItem( j );
	    break;
	}
    }
    editProperty->blockSignals( FALSE );
    comboType->blockSignals( FALSE );
}

static QString makeIndent2( int indent )
{
    QString s;
    s.fill( ' ', indent * 4 );
    return s;
}

static QString entitize2( const QString &s )
{
    QString s2 = s;
    s2 = s2.replace( "\"", "&quot;" );
    s2 = s2.replace( "&", "&amp;" );
    s2 = s2.replace( ">", "&gt;" );
    s2 = s2.replace( "<", "&lt;" );
    s2 = s2.replace( "'", "&apos;" );
    return s2;
}

void CustomWidgetEditor::saveDescription()
{
    QString fn = KFileDialog::getSaveFileName( QString::null, i18n( "*.cw|Custom-Widget Description\n*|All Files" ), this );
    if ( fn.isEmpty() )
	return;

    if ( QFileInfo( fn ).extension() != "cw" )
	fn += ".cw";
    QFile f( fn );
    if ( !f.open( IO_WriteOnly ) )
	return;

    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    int indent = 0;

    ts << "<!DOCTYPE CW><CW>" << endl;

    ts << makeIndent2( indent ) << "<customwidgets>" << endl;
    indent++;

    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	ts << makeIndent2( indent ) << "<customwidget>" << endl;
	indent++;
	ts << makeIndent2( indent ) << "<class>" << w->className << "</class>" << endl;
	ts << makeIndent2( indent ) << "<header location=\""
	   << ( w->includePolicy == MetaDataBase::CustomWidget::Local ? "local" : "global" )
	   << "\">" << w->includeFile << "</header>" << endl;
	ts << makeIndent2( indent ) << "<sizehint>" << endl;
	indent++;
	ts << makeIndent2( indent ) << "<width>" << w->sizeHint.width() << "</width>" << endl;
	ts << makeIndent2( indent ) << "<height>" << w->sizeHint.height() << "</height>" << endl;
	indent--;
	ts << makeIndent2( indent ) << "</sizehint>" << endl;
	ts << makeIndent2( indent ) << "<container>" << (int)w->isContainer << "</container>" << endl;
	ts << makeIndent2( indent ) << "<sizepolicy>" << endl;
	indent++;
	ts << makeIndent2( indent ) << "<hordata>" << (int)w->sizePolicy.horData() << "</hordata>" << endl;
	ts << makeIndent2( indent ) << "<verdata>" << (int)w->sizePolicy.verData() << "</verdata>" << endl;
	indent--;
	ts << makeIndent2( indent ) << "</sizepolicy>" << endl;
	ts << makeIndent2( indent ) << "<pixmap>" << endl;
	indent++;
	Resource::saveImageData( w->pixmap->convertToImage(), ts, indent );
	indent--;
	ts << makeIndent2( indent ) << "</pixmap>" << endl;
	if ( !w->lstSignals.isEmpty() ) {
	    for ( QValueList<QCString>::Iterator it = w->lstSignals.begin(); it != w->lstSignals.end(); ++it )
		ts << makeIndent2( indent ) << "<signal>" << entitize2( *it ) << "</signal>" << endl;
	}
	if ( !w->lstSlots.isEmpty() ) {
	    for ( QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.begin(); it != w->lstSlots.end(); ++it )
		ts << makeIndent2( indent ) << "<slot access=\"" << (*it).access << "\">" << entitize2( (*it).function ) << "</slot>" << endl;
	}
	if ( !w->lstProperties.isEmpty() ) {
	    for ( QValueList<MetaDataBase::Property>::Iterator it = w->lstProperties.begin(); it != w->lstProperties.end(); ++it )
		ts << makeIndent2( indent ) << "<property type=\"" << (*it).type << "\">" << entitize2( (*it).property ) << "</property>" << endl;
	}
	indent--;
	ts << makeIndent2( indent ) << "</customwidget>" << endl;
    }

    indent--;
    ts << makeIndent2( indent ) << "</customwidgets>" << endl;
    ts << "</CW>" << endl;
}

void CustomWidgetEditor::loadDescription()
{
    QString fn = KFileDialog::getOpenFileName( QString::null, i18n( "*.cw|Custom-Widget Description\n*|All Files" ), this );
    if ( fn.isEmpty() )
	return;

    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QDomDocument doc;
    QString errMsg;
    int errLine;
    if ( !doc.setContent( &f, &errMsg, &errLine ) ) {
	qDebug( QString("Parse error: ") + errMsg + QString(" in line %d"), errLine );
	return;
    }

    QDomElement firstWidget = doc.firstChild().toElement().firstChild().toElement();

    while ( firstWidget.tagName() != "customwidgets" )
	firstWidget = firstWidget.nextSibling().toElement();

    Resource::loadCustomWidgets( firstWidget, 0 );
    boxWidgets->clear();
    setupDefinition();
    setupSignals();
    setupSlots();
    setupProperties();
}

void CustomWidgetEditor::updateCustomWidgetSizes()
{
    if ( cwLst.isEmpty() )
	cwLst = *mainWindow->queryList( "CustomWidget" );
    for ( QObject *o = cwLst.first(); o; o = cwLst.next() )
	( (QWidget*)o )->updateGeometry();
}

void CustomWidgetEditor::horDataChanged( int a )
{
    QSizePolicy::SizeType st = int_to_size_type( a );
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    QSizePolicy osp = w->sizePolicy;
    w->sizePolicy.setHorData( st );
    if ( cwLst.isEmpty() )
	cwLst = *mainWindow->queryList( "CustomWidget" );
    for ( QObject *o = cwLst.first(); o; o = cwLst.next() ) {
	CustomWidget *cw = (CustomWidget*)o;
	if ( cw->realClassName() == boxWidgets->currentText() ) {
	    if ( cw->sizePolicy() == osp )
		cw->setSizePolicy( w->sizePolicy );
	}
    }
}

void CustomWidgetEditor::verDataChanged( int a )
{
    QSizePolicy::SizeType st = int_to_size_type( a );
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    QSizePolicy osp = w->sizePolicy;
    w->sizePolicy.setVerData( st );
    if ( cwLst.isEmpty() )
	cwLst = *mainWindow->queryList( "CustomWidget" );
    for ( QObject *o = cwLst.first(); o; o = cwLst.next() ) {
	CustomWidget *cw = (CustomWidget*)o;
	if ( cw->realClassName() == boxWidgets->currentText() ) {
	    if ( cw->sizePolicy() == osp )
		cw->setSizePolicy( w->sizePolicy );
	}
    }
}

void CustomWidgetEditor::widgetIsContainer( bool b )
{
    QListBoxItem *i = boxWidgets->item( boxWidgets->currentItem() );
    MetaDataBase::CustomWidget *w = findWidget( i );
    if ( !i || !w )
	return;

    w->isContainer = b;
    WidgetDatabaseRecord *r = WidgetDatabase::at( w->id );
    if ( r )
	r->isContainer = b;
}

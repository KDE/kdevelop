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
#include <kiconloader.h>

#include "sourceeditor.h"

#include "formwindow.h"
#include "metadatabase.h"
#include "project.h"
#include "mainwindow.h"
#include "../interfaces/languageinterface.h"
#include <qregexp.h>
#include "project.h"
#include "sourcefile.h"
#include "hierarchyview.h"
#include <qmessagebox.h>
#include <qtextstream.h>
#include "kdevdesigner_part.h"

SourceEditor::SourceEditor( QWidget *parent, EditorInterface *iface, LanguageInterface *liface )
    : QVBox( parent, 0, WDestructiveClose ), iFace( iface ), lIface( liface ), obj( 0 ), pro( 0 )
{
    iFace->addRef();
    lIface->addRef();
    editor = iFace->editor( MainWindow::self->areEditorsReadOnly(),
			    this, MainWindow::self->designerInterface() );
    iFace->onBreakPointChange( MainWindow::self, SLOT( breakPointsChanged() ) );
    resize( 600, 400 );
    setIcon( SmallIcon( "designer_filenew.png" , KDevDesignerPartFactory::instance()) );
}

SourceEditor::~SourceEditor()
{
    saveBreakPoints();
    editor = 0;
    if ( formWindow() ) {
	formWindow()->formFile()->setCodeEdited( FALSE );
	formWindow()->formFile()->setEditor( 0 );
    } else if ( sourceFile() ) {
	sourceFile()->setEditor( 0 );
	if ( MainWindow::self->objectHierarchy()->sourceEditor() == this )
	    MainWindow::self->objectHierarchy()->setFormWindow( 0, 0 );
    }
    iFace->release();
    lIface->release();
    MainWindow::self->editorClosed( this );
}

void SourceEditor::setObject( QObject *o, Project *p )
{
    if ( sourceFile() )
	sourceFile()->setEditor( 0 );
    if ( formWindow() ) {
	formWindow()->formFile()->setCodeEdited( FALSE );
	formWindow()->formFile()->setEditor( 0 );
    }
    if ( ::qt_cast<FormWindow*>(o) )
	( (FormWindow*)o )->formFile()->setCodeEdited( TRUE );
    save();
    bool changed = FALSE;
    if ( &(*obj) != o ) {
	saveBreakPoints();
	changed = TRUE;
    }
    obj = o;
    pro = p;
    if ( formWindow() ) {
	if ( formWindow()->isFake() )
	    setCaption( formWindow()->project()->objectForFakeForm( formWindow() )->name() );
	else
	    setCaption( obj->name() );
    } else {
	setCaption( sourceFile()->fileName() );
    }
    if ( sourceFile() )
	sourceFile()->setEditor( this );
    else if ( formWindow() )
	formWindow()->formFile()->setEditor( this );
    iFace->setText( sourceOfObject( obj, lang, iFace, lIface ) );
    if ( pro && formWindow() ) {
	if ( formWindow()->isFake() )
	    iFace->setContext( pro->objectForFakeFormFile( formWindow()->formFile() ) );
	else
	    iFace->setContext( formWindow()->mainContainer() );
    } else {
	iFace->setContext( 0 );
    }
    if ( changed || sourceFile() )
	iFace->setBreakPoints( MetaDataBase::breakPoints( o ) );
    MainWindow::self->objectHierarchy()->showClasses( this );
}

QString SourceEditor::sourceOfObject( QObject *o, const QString &,
				      EditorInterface *, LanguageInterface * )
{
    QString txt;
    if ( !o )
	return txt;
    if ( ::qt_cast<FormWindow*>(o) )
	txt = ( (FormWindow*)o )->formFile()->code();
    else if ( ::qt_cast<SourceFile*>(o) )
	txt = ( (SourceFile*)o )->text();
    return txt;
}

void SourceEditor::setFunction( const QString &func, const QString &clss )
{
    iFace->scrollTo( lIface->createFunctionStart( obj->name(), func, "", "" ), clss );
}

void SourceEditor::setClass( const QString &clss )
{
    iFace->scrollTo( clss, QString::null );
}

void SourceEditor::closeEvent( QCloseEvent *e )
{
    e->accept();
    if ( !obj )
	return;
    if ( formWindow() ) {
	save();
	formWindow()->formFile()->cm = formWindow()->formFile()->isModified();
    } else {
	if ( !sourceFile()->closeEvent() )
	    e->ignore();
    }
}

void SourceEditor::save()
{
    if ( !obj )
	return;
    if ( formWindow() )
	formWindow()->formFile()->syncCode();
    else if ( sourceFile() && sourceFile()->editor() )
	sourceFile()->setText( iFace->text() );
}

QString SourceEditor::language() const
{
    return lang;
}

void SourceEditor::setLanguage( const QString &l )
{
    lang = l;
}

void SourceEditor::editCut()
{
    iFace->cut();
}

void SourceEditor::editCopy()
{
    iFace->copy();
}

void SourceEditor::editPaste()
{
    iFace->paste();
}

bool SourceEditor::editIsUndoAvailable()
{
    return iFace->isUndoAvailable();
}

bool SourceEditor::editIsRedoAvailable()
{
    return iFace->isRedoAvailable();
}

void SourceEditor::editUndo()
{
    iFace->undo();
}

void SourceEditor::editRedo()
{
    iFace->redo();
}

void SourceEditor::editSelectAll()
{
    iFace->selectAll();
}

void SourceEditor::configChanged()
{
    iFace->readSettings();
}

void SourceEditor::setModified( bool b )
{
    iFace->setModified( b );
}

void SourceEditor::refresh( bool allowSave )
{
    if ( allowSave )
	save();
    bool oldMod = iFace->isModified();
    iFace->setText( sourceOfObject( obj, lang, iFace, lIface ) );
    iFace->setModified( oldMod );
}

void SourceEditor::resetContext()
{
    if ( pro && formWindow() ) {
	if ( formWindow()->isFake() )
	    iFace->setContext( pro->objectForFakeFormFile( formWindow()->formFile() ) );
	else
	    iFace->setContext( formWindow()->mainContainer() );
    } else {
	iFace->setContext( 0 );
    }
}

void SourceEditor::setFocus()
{
    if ( formWindow() )
	formWindow()->formFile()->setCodeEdited( TRUE );
    if ( editor )
	editor->setFocus();
}

int SourceEditor::numLines() const
{
    return iFace->numLines();
}

void SourceEditor::saveBreakPoints()
{
    if ( !obj )
	return;
    QValueList<uint> l;
    iFace->breakPoints( l );
    MetaDataBase::setBreakPoints( obj, l );
}

void SourceEditor::clearStep()
{
    iFace->clearStep();
}

void SourceEditor::clearStackFrame()
{
    iFace->clearStackFrame();
}

void SourceEditor::resetBreakPoints()
{
    iFace->setBreakPoints( MetaDataBase::breakPoints( obj ) );
}

QString SourceEditor::text() const
{
    return iFace->text();
}

bool SourceEditor::isModified() const
{
    return iFace->isModified();
}

void SourceEditor::checkTimeStamp()
{
    if ( formWindow() )
	formWindow()->formFile()->checkTimeStamp();
    else if ( sourceFile() )
	sourceFile()->checkTimeStamp();
}

bool SourceEditor::saveAs()
{
    if ( formWindow() )
	return formWindow()->formFile()->saveAs();
    else if ( sourceFile() )
	return sourceFile()->saveAs();
    return FALSE;
}

SourceFile *SourceEditor::sourceFile() const
{
    if ( !::qt_cast<SourceFile*>((QObject *)obj) )
	return 0;
    return (SourceFile*)(QObject*)obj;
}

FormWindow *SourceEditor::formWindow() const
{
    if ( !::qt_cast<FormWindow*>((QObject *)obj) )
	return 0;
    return (FormWindow*)(QObject*)obj;
}

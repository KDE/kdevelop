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

#include "formsettingsimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "command.h"
#include "asciivalidator.h"
#include "mainwindow.h"
#include "project.h"

#include <q3multilineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

FormSettings::FormSettings( QWidget *parent, FormWindow *fw )
    : FormSettingsBase( parent, 0, TRUE ), formwindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    MetaDataBase::MetaInfo info = MetaDataBase::metaInfo( fw );
    if ( info.classNameChanged && !info.className.isEmpty() )
	editClassName->setText( info.className );
    else
	editClassName->setText( fw->name() );
    editComment->setText( info.comment );
    editAuthor->setText( info.author );

    editClassName->setValidator( new AsciiValidator( QString( ":" ), editClassName ) );
    editPixmapFunction->setValidator( new AsciiValidator( QString( ":" ), editPixmapFunction ) );

    if ( formwindow->savePixmapInline() )
	radioPixmapInline->setChecked( TRUE );
    else if ( formwindow->savePixmapInProject() )
	radioProjectImageFile->setChecked( TRUE );
    else
	radioPixmapFunction->setChecked( TRUE );
    editPixmapFunction->setText( formwindow->pixmapLoaderFunction() );
    radioProjectImageFile->setEnabled( !fw->project()->isDummy() );
    spinSpacing->setValue( formwindow->layoutDefaultSpacing() );
    spinMargin->setValue( formwindow->layoutDefaultMargin() );
    editSpacingFunction->setValidator( new AsciiValidator( QString( ":" ), editSpacingFunction ) );
    editMarginFunction->setValidator( new AsciiValidator( QString( ":" ), editMarginFunction ) ); 
    checkLayoutFunctions->setChecked( formwindow->hasLayoutFunctions() );
    editSpacingFunction->setText( formwindow->spacingFunction() );
    editMarginFunction->setText( formwindow->marginFunction() );
}

void FormSettings::okClicked()
{
    MetaDataBase::MetaInfo info;
    info.className = editClassName->text();
    info.classNameChanged = info.className != QString( formwindow->name() );
    info.comment = editComment->text();
    info.author = editAuthor->text();
    MetaDataBase::setMetaInfo( formwindow, info );

    formwindow->commandHistory()->setModified( TRUE );

    if ( formwindow->savePixmapInline() ) {
	MetaDataBase::clearPixmapArguments( formwindow );
	MetaDataBase::clearPixmapKeys( formwindow );
    } else if ( formwindow->savePixmapInProject() ) {
	MetaDataBase::clearPixmapArguments( formwindow );
    } else {
	MetaDataBase::clearPixmapKeys( formwindow );
    }

    if ( radioPixmapInline->isChecked() ) {
	formwindow->setSavePixmapInline( TRUE );
	formwindow->setSavePixmapInProject( FALSE );
    } else if ( radioProjectImageFile->isChecked() ){
	formwindow->setSavePixmapInline( FALSE );
	formwindow->setSavePixmapInProject( TRUE );
    } else {
	formwindow->setSavePixmapInline( FALSE );
	formwindow->setSavePixmapInProject( FALSE );
    }
    
    if ( checkLayoutFunctions->isChecked() )
	formwindow->hasLayoutFunctions( TRUE );
    else
	formwindow->hasLayoutFunctions( FALSE );

    formwindow->setPixmapLoaderFunction( editPixmapFunction->text() );
    formwindow->setLayoutDefaultSpacing( spinSpacing->value() );
    formwindow->setSpacingFunction( editSpacingFunction->text() );
    formwindow->setLayoutDefaultMargin( spinMargin->value() );
    formwindow->setMarginFunction( editMarginFunction->text() );

    accept();
}

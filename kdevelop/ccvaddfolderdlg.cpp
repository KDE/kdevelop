/***************************************************************************
               cclassaddattributedlg.cpp  -  description

                             -------------------

    begin                : Fri Mar 19 1999

    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ccvaddfolderdlg.h"
#include <kmsgbox.h>
#include <kapp.h>
#include <qlayout.h>
#define LAYOUT_BORDER (10)

CCVAddFolderDlg::CCVAddFolderDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true ),
		folderLbl( this, "folderLbl" ),
	  folderEdit( this, "folderEdit" ),
	  okBtn( this, "okBtn" ),
	  cancelBtn( this, "cancelBtn" )
{
  setCaption(i18n("Add folder") );

  setWidgetValues();
  setCallbacks();
}

void CCVAddFolderDlg::setWidgetValues()
{
	QGridLayout * layout = new QGridLayout( this, 2, 3, LAYOUT_BORDER );
	
  folderLbl.setText( i18n("Folder name:") );
 	folderLbl.setMinimumSize( folderLbl.sizeHint() );
	layout->addWidget( &folderLbl, 0, 0 );

  folderEdit.setMinimumSize( folderEdit.sizeHint() );
	layout->addWidget( &folderEdit, 0, 1 );

  okBtn.setFocusPolicy( QWidget::TabFocus );
  okBtn.setBackgroundMode( QWidget::PaletteBackground );
  okBtn.setFontPropagation( QWidget::NoChildren );
  okBtn.setPalettePropagation( QWidget::NoChildren );
  okBtn.setText( i18n("OK") );
  okBtn.setAutoRepeat( FALSE );
  okBtn.setAutoResize( FALSE );
	okBtn.setDefault( true );

  cancelBtn.setFocusPolicy( QWidget::TabFocus );
  cancelBtn.setBackgroundMode( QWidget::PaletteBackground );
  cancelBtn.setFontPropagation( QWidget::NoChildren );
  cancelBtn.setPalettePropagation( QWidget::NoChildren );
  cancelBtn.setText( i18n("Cancel") );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );
	
	okBtn.setFixedSize( cancelBtn.sizeHint() );
	cancelBtn.setFixedSize( cancelBtn.sizeHint() );

	layout->addWidget( &okBtn, 0, 2 );
	layout->addWidget( &cancelBtn, 1, 2 );

	layout->setRowStretch( 0, 1 );
	layout->setRowStretch( 1, 1 );
	layout->setColStretch( 0, 0 );
	layout->setColStretch( 1, 1 );
	layout->setColStretch( 2, 0 );

	layout->activate();
  adjustSize();
	
  folderEdit.setFocus();
}

void CCVAddFolderDlg::setCallbacks()
{

  // Ok and cancel buttons.
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}

void CCVAddFolderDlg::OK()
{

  if( strlen( folderEdit.text() ) == 0 )
    KMsgBox::message( this, i18n("No name"),
                      i18n("You have to specify a foldername.") );
  else
    accept();
}






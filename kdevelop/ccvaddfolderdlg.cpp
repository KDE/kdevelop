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
  folderLbl.setGeometry( 10, 10, 80, 30 );
  folderLbl.setText( i18n("Folder name:") );

  folderEdit.setGeometry( 90, 10, 160, 30 );

  okBtn.setGeometry( 260, 10, 100, 30 );
  okBtn.setMinimumSize( 0, 0 );
  okBtn.setMaximumSize( 32767, 32767 );
  okBtn.setFocusPolicy( QWidget::TabFocus );
  okBtn.setBackgroundMode( QWidget::PaletteBackground );
  okBtn.setFontPropagation( QWidget::NoChildren );
  okBtn.setPalettePropagation( QWidget::NoChildren );
  okBtn.setText( i18n("OK") );
  okBtn.setAutoRepeat( FALSE );
  okBtn.setAutoResize( FALSE );
	okBtn.setDefault(true);

  cancelBtn.setGeometry( 260, 50, 100, 30 );
  cancelBtn.setMinimumSize( 0, 0 );
  cancelBtn.setMaximumSize( 32767, 32767 );
  cancelBtn.setFocusPolicy( QWidget::TabFocus );
  cancelBtn.setBackgroundMode( QWidget::PaletteBackground );
  cancelBtn.setFontPropagation( QWidget::NoChildren );
  cancelBtn.setPalettePropagation( QWidget::NoChildren );
  cancelBtn.setText( i18n("Cancel") );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );

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



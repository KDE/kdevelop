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
#include <kmessagebox.h>
#include <kapp.h>
#include <klocale.h>
#include <qlayout.h>
#include <kbuttonbox.h>

#define LAYOUT_BORDER (10)

CCVAddFolderDlg::CCVAddFolderDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true )
{
  setCaption(i18n("Add Folder") );

  setWidgetValues();
  setCallbacks();
}

void CCVAddFolderDlg::setWidgetValues()
{
  QGridLayout * layout = new QGridLayout( this, 2, 2, LAYOUT_BORDER );
  folderLbl=new QLabel( this, "folderLbl" );
  folderEdit=new QLineEdit( this, "folderEdit" );


  folderLbl->setText( i18n("Folder name:") );
  folderLbl->setMinimumSize( folderLbl->sizeHint() );
  layout->addWidget( folderLbl, 0, 0 );

  folderEdit->setMinimumSize( folderEdit->sizeHint() );
  layout->addWidget( folderEdit, 0, 1 );

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  okBtn = bb->addButton( i18n("OK") );
  okBtn->setFocusPolicy( QWidget::TabFocus );
  okBtn->setBackgroundMode( QWidget::PaletteBackground );
  okBtn->setAutoRepeat( FALSE );
  okBtn->setAutoResize( FALSE );
  okBtn->setDefault( true );
  cancelBtn = bb->addButton( i18n( "Close" ) );
  cancelBtn->setFocusPolicy( QWidget::TabFocus );
  cancelBtn->setBackgroundMode( QWidget::PaletteBackground );
  cancelBtn->setAutoRepeat( FALSE );
  cancelBtn->setAutoResize( FALSE );
  bb->layout();
  layout->addWidget(bb,1,1);

  
  layout->setRowStretch( 0, 1 );
  layout->setRowStretch( 1, 1 );
  layout->setColStretch( 0, 0 );
  layout->setColStretch( 1, 1 );
  
  layout->activate();
  adjustSize();
	
  folderEdit->setFocus();
}

void CCVAddFolderDlg::setCallbacks()
{

  // Ok and cancel buttons.
  connect( okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}

void CCVAddFolderDlg::OK()
{

  if( folderEdit->text().isEmpty() )
    KMessageBox::error( this,
                        i18n("You have to specify a foldername."),
                        i18n("No Name") );
  else
    accept();
}






#include "ccvaddfolderdlg.moc"

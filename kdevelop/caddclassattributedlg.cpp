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


#include "caddclassattributedlg.h"
#include <kmsgbox.h>

CAddClassAttributeDlg::CAddClassAttributeDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true ),
    modifierGrp( this, "modifierGrp" ),
    varGrp( this, "functionGrp" ),
    accessGrp( this, "accessGrp" ),
    typeLbl( this, "typeLbl" ),
    typeEdit( this, "typeEdit" ),
    nameLbl( this, "nameLbl" ),
    nameEdit( this, "nameEdit" ),
    publicRb( this, "publicRb" ),
    protectedRb( this, "protectedRb" ),
    privateRb( this, "privateRb" ),
    staticCb( this, "staticCb" ),
    constCb( this, "constCb" ),
    okBtn( this, "okBtn" ),
    cancelBtn( this, "cancelBtn" )
{
  setCaption( "Add member variable" );

  setWidgetValues();
  setCallbacks();
}

void CAddClassAttributeDlg::setWidgetValues()
{
  modifierGrp.setGeometry( 10, 240, 260, 60 );
  modifierGrp.setMinimumSize( 0, 0 );
  modifierGrp.setMaximumSize( 32767, 32767 );
  modifierGrp.setFocusPolicy( QWidget::NoFocus );
  modifierGrp.setBackgroundMode( QWidget::PaletteBackground );
  modifierGrp.setFontPropagation( QWidget::NoChildren );
  modifierGrp.setPalettePropagation( QWidget::NoChildren );
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( "Modifiers" );
  modifierGrp.setAlignment( 1 );

  varGrp.setGeometry( 10, 10, 260, 150 );
  varGrp.setMinimumSize( 0, 0 );
  varGrp.setMaximumSize( 32767, 32767 );
  varGrp.setFocusPolicy( QWidget::NoFocus );
  varGrp.setBackgroundMode( QWidget::PaletteBackground );
  varGrp.setFontPropagation( QWidget::NoChildren );
  varGrp.setPalettePropagation( QWidget::NoChildren );
  varGrp.setFrameStyle( 49 );
  varGrp.setTitle( "Variable" );
  varGrp.setAlignment( 1 );

  accessGrp.setGeometry( 10, 170, 260, 60 );
  accessGrp.setMinimumSize( 0, 0 );
  accessGrp.setMaximumSize( 32767, 32767 );
  accessGrp.setFocusPolicy( QWidget::NoFocus );
  accessGrp.setBackgroundMode( QWidget::PaletteBackground );
  accessGrp.setFontPropagation( QWidget::NoChildren );
  accessGrp.setPalettePropagation( QWidget::NoChildren );
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( "Access" );
  accessGrp.setAlignment( 1 );

  typeLbl.setGeometry( 20, 30, 40, 20 );
  typeLbl.setMinimumSize( 0, 0 );
  typeLbl.setMaximumSize( 32767, 32767 );
  typeLbl.setFocusPolicy( QWidget::NoFocus );
  typeLbl.setBackgroundMode( QWidget::PaletteBackground );
  typeLbl.setFontPropagation( QWidget::NoChildren );
  typeLbl.setPalettePropagation( QWidget::NoChildren );
  typeLbl.setText( "Type:" );
  typeLbl.setAlignment( 289 );
  typeLbl.setMargin( -1 );

  typeEdit.setGeometry( 20, 50, 240, 30 );
  typeEdit.setMinimumSize( 0, 0 );
  typeEdit.setMaximumSize( 32767, 32767 );
  typeEdit.setFocusPolicy( QWidget::StrongFocus );
  typeEdit.setBackgroundMode( QWidget::PaletteBase );
  typeEdit.setFontPropagation( QWidget::NoChildren );
  typeEdit.setPalettePropagation( QWidget::NoChildren );
  typeEdit.setText( "" );
  typeEdit.setMaxLength( 32767 );
  typeEdit.setEchoMode( QLineEdit::Normal );
  typeEdit.setFrame( TRUE );

  nameLbl.setGeometry( 20, 90, 70, 20 );
  nameLbl.setMinimumSize( 0, 0 );
  nameLbl.setMaximumSize( 32767, 32767 );
  nameLbl.setFocusPolicy( QWidget::NoFocus );
  nameLbl.setBackgroundMode( QWidget::PaletteBackground );
  nameLbl.setFontPropagation( QWidget::NoChildren );
  nameLbl.setPalettePropagation( QWidget::NoChildren );
  nameLbl.setText( "Name:" );
  nameLbl.setAlignment( 289 );
  nameLbl.setMargin( -1 );

  nameEdit.setGeometry( 20, 110, 240, 30 );
  nameEdit.setMinimumSize( 0, 0 );
  nameEdit.setMaximumSize( 32767, 32767 );
  nameEdit.setFocusPolicy( QWidget::StrongFocus );
  nameEdit.setBackgroundMode( QWidget::PaletteBase );
  nameEdit.setFontPropagation( QWidget::NoChildren );
  nameEdit.setPalettePropagation( QWidget::NoChildren );
  nameEdit.setText( "" );
  nameEdit.setMaxLength( 32767 );
  nameEdit.setEchoMode( QLineEdit::Normal );
  nameEdit.setFrame( TRUE );

  publicRb.setGeometry( 20, 190, 70, 30 );
  publicRb.setMinimumSize( 0, 0 );
  publicRb.setMaximumSize( 32767, 32767 );
  publicRb.setFocusPolicy( QWidget::TabFocus );
  publicRb.setBackgroundMode( QWidget::PaletteBackground );
  publicRb.setFontPropagation( QWidget::NoChildren );
  publicRb.setPalettePropagation( QWidget::NoChildren );
  publicRb.setText( "Public" );
  publicRb.setAutoRepeat( FALSE );
  publicRb.setAutoResize( FALSE );

  protectedRb.setGeometry( 110, 190, 80, 30 );
  protectedRb.setMinimumSize( 0, 0 );
  protectedRb.setMaximumSize( 32767, 32767 );
  protectedRb.setFocusPolicy( QWidget::TabFocus );
  protectedRb.setBackgroundMode( QWidget::PaletteBackground );
  protectedRb.setFontPropagation( QWidget::NoChildren );
  protectedRb.setPalettePropagation( QWidget::NoChildren );
  protectedRb.setText( "Protected" );
  protectedRb.setAutoRepeat( FALSE );
  protectedRb.setAutoResize( FALSE );

  privateRb.setGeometry( 200, 190, 60, 30 );
  privateRb.setMinimumSize( 0, 0 );
  privateRb.setMaximumSize( 32767, 32767 );
  privateRb.setFocusPolicy( QWidget::TabFocus );
  privateRb.setBackgroundMode( QWidget::PaletteBackground );
  privateRb.setFontPropagation( QWidget::NoChildren );
  privateRb.setPalettePropagation( QWidget::NoChildren );
  privateRb.setText( "Private" );
  privateRb.setAutoRepeat( FALSE );
  privateRb.setAutoResize( FALSE );
 
  staticCb.setGeometry( 20, 260, 60, 30 );
  staticCb.setMinimumSize( 0, 0 );
  staticCb.setMaximumSize( 32767, 32767 );
  staticCb.setFocusPolicy( QWidget::TabFocus );
  staticCb.setBackgroundMode( QWidget::PaletteBackground );
  staticCb.setFontPropagation( QWidget::NoChildren );
  staticCb.setPalettePropagation( QWidget::NoChildren );
  staticCb.setText( "Static" );
  staticCb.setAutoRepeat( FALSE );
  staticCb.setAutoResize( FALSE );
 
  constCb.setGeometry( 110, 260, 60, 30 );
  constCb.setMinimumSize( 0, 0 );
  constCb.setMaximumSize( 32767, 32767 );
  constCb.setFocusPolicy( QWidget::TabFocus );
  constCb.setBackgroundMode( QWidget::PaletteBackground );
  constCb.setFontPropagation( QWidget::NoChildren );
  constCb.setPalettePropagation( QWidget::NoChildren );
  constCb.setText( "Const" );
  constCb.setAutoRepeat( FALSE );
  constCb.setAutoResize( FALSE );

  okBtn.setGeometry( 290, 10, 100, 30 );
  okBtn.setMinimumSize( 0, 0 );
  okBtn.setMaximumSize( 32767, 32767 );
  okBtn.setFocusPolicy( QWidget::TabFocus );
  okBtn.setBackgroundMode( QWidget::PaletteBackground );
  okBtn.setFontPropagation( QWidget::NoChildren );
  okBtn.setPalettePropagation( QWidget::NoChildren );
  okBtn.setText( "OK" );
  okBtn.setAutoRepeat( FALSE );
  okBtn.setAutoResize( FALSE );

  cancelBtn.setGeometry( 290, 50, 100, 30 );
  cancelBtn.setMinimumSize( 0, 0 );
  cancelBtn.setMaximumSize( 32767, 32767 );
  cancelBtn.setFocusPolicy( QWidget::TabFocus );
  cancelBtn.setBackgroundMode( QWidget::PaletteBackground );
  cancelBtn.setFontPropagation( QWidget::NoChildren );
  cancelBtn.setPalettePropagation( QWidget::NoChildren );
  cancelBtn.setText( "Cancel" );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );

  typeEdit.setFocus();
  publicRb.setChecked( true );

  modifierGrp.insert( &staticCb );
  modifierGrp.insert( &constCb );

  accessGrp.insert( &publicRb );
  accessGrp.insert( &protectedRb );
  accessGrp.insert( &privateRb );
}

void CAddClassAttributeDlg::setCallbacks()
{

  // Ok and cancel buttons.
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}


void CAddClassAttributeDlg::OK()
{

  if( strlen( typeEdit.text() ) == 0 )
    KMsgBox::message( this, "No type",
                      "You have to specify a variable type." );
  else if( strlen( nameEdit.text() ) == 0 )
    KMsgBox::message( this, "No name",
                      "You have to specify a variable name." );
  else
    accept();
}

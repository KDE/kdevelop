/***************************************************************************
               cclassaddmethoddlg.cpp  -  description

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


#include "caddclassmethoddlg.h"
#include <kmsgbox.h>
#include <kapp.h>

CAddClassMethodDlg::CAddClassMethodDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true ),
    modifierGrp( this, "modifierGrp" ),
    functionGrp( this, "functionGrp" ),
    accessGrp( this, "accessGrp" ),
    typeLbl( this, "typeLbl" ),
    typeEdit( this, "typeEdit" ),
    declLbl( this, "declLbl" ),
    declEdit( this, "declEdit" ),
    docLbl( this, "docLbl" ),
    docEdit( this, "docEdit" ),
    publicRb( this, "publicRb" ),
    protectedRb( this, "protectedRb" ),
    privateRb( this, "privateRb" ),
    virtualCb( this, "virtualCb" ),
    staticCb( this, "staticCb" ),
    constCb( this, "constCb" ),
    okBtn( this, "okBtn" ),
    cancelBtn( this, "cancelBtn" )
{
  setCaption( i18n("Add member function") );

  setWidgetValues();
  setCallbacks();
}

void CAddClassMethodDlg::setWidgetValues()
{
  modifierGrp.setGeometry( 10, 310, 260, 50 );
  modifierGrp.setMinimumSize( 0, 0 );
  modifierGrp.setMaximumSize( 32767, 32767 );
  modifierGrp.setFocusPolicy( QWidget::NoFocus );
  modifierGrp.setBackgroundMode( QWidget::PaletteBackground );
  modifierGrp.setFontPropagation( QWidget::NoChildren );
  modifierGrp.setPalettePropagation( QWidget::NoChildren );
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( i18n("Modifiers") );
  modifierGrp.setAlignment( 1 );

  functionGrp.setGeometry( 10, 10, 260, 230 );
  functionGrp.setMinimumSize( 0, 0 );
  functionGrp.setMaximumSize( 32767, 32767 );
  functionGrp.setFocusPolicy( QWidget::NoFocus );
  functionGrp.setBackgroundMode( QWidget::PaletteBackground );
  functionGrp.setFontPropagation( QWidget::NoChildren );
  functionGrp.setPalettePropagation( QWidget::NoChildren );
  functionGrp.setFrameStyle( 49 );
  functionGrp.setTitle( i18n("Function") );
  functionGrp.setAlignment( 1 );

  accessGrp.setGeometry( 10, 250, 260, 50 );
  accessGrp.setMinimumSize( 0, 0 );
  accessGrp.setMaximumSize( 32767, 32767 );
  accessGrp.setFocusPolicy( QWidget::NoFocus );
  accessGrp.setBackgroundMode( QWidget::PaletteBackground );
  accessGrp.setFontPropagation( QWidget::NoChildren );
  accessGrp.setPalettePropagation( QWidget::NoChildren );
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( i18n("Access") );
  accessGrp.setAlignment( 1 );

  typeLbl.setGeometry( 20, 30, 40, 20 );
  typeLbl.setMinimumSize( 0, 0 );
  typeLbl.setMaximumSize( 32767, 32767 );
  typeLbl.setFocusPolicy( QWidget::NoFocus );
  typeLbl.setBackgroundMode( QWidget::PaletteBackground );
  typeLbl.setFontPropagation( QWidget::NoChildren );
  typeLbl.setPalettePropagation( QWidget::NoChildren );
  typeLbl.setText( i18n("Type:") );
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

  declLbl.setGeometry( 20, 80, 70, 20 );
  declLbl.setMinimumSize( 0, 0 );
  declLbl.setMaximumSize( 32767, 32767 );
  declLbl.setFocusPolicy( QWidget::NoFocus );
  declLbl.setBackgroundMode( QWidget::PaletteBackground );
  declLbl.setFontPropagation( QWidget::NoChildren );
  declLbl.setPalettePropagation( QWidget::NoChildren );
  declLbl.setText( i18n("Declaration:") );
  declLbl.setAlignment( 289 );
  declLbl.setMargin( -1 );

  declEdit.setGeometry( 20, 100, 240, 30 );
  declEdit.setMinimumSize( 0, 0 );
  declEdit.setMaximumSize( 32767, 32767 );
  declEdit.setFocusPolicy( QWidget::StrongFocus );
  declEdit.setBackgroundMode( QWidget::PaletteBase );
  declEdit.setFontPropagation( QWidget::NoChildren );
  declEdit.setPalettePropagation( QWidget::NoChildren );
  declEdit.setText( "" );
  declEdit.setMaxLength( 32767 );
  declEdit.setEchoMode( QLineEdit::Normal );
  declEdit.setFrame( TRUE );

  docLbl.setGeometry( 20, 130, 100, 20 );
  docLbl.setMinimumSize( 0, 0 );
  docLbl.setMaximumSize( 32767, 32767 );
  docLbl.setFocusPolicy( QWidget::NoFocus );
  docLbl.setBackgroundMode( QWidget::PaletteBackground );
  docLbl.setFontPropagation( QWidget::NoChildren );
  docLbl.setPalettePropagation( QWidget::NoChildren );
  docLbl.setText( i18n("Documentation:") );
  docLbl.setAlignment( 289 );
  docLbl.setMargin( -1 );

  docEdit.setGeometry( 20, 150, 240, 80 );
  docEdit.setMinimumSize( 0, 0 );
  docEdit.setMaximumSize( 32767, 32767 );
  docEdit.setFocusPolicy( QWidget::StrongFocus );
  docEdit.setBackgroundMode( QWidget::PaletteBase );
  docEdit.setFontPropagation( QWidget::SameFont );
  docEdit.setPalettePropagation( QWidget::SameFont );
  docEdit.insertLine( "" );
  docEdit.setReadOnly( FALSE );
  docEdit.setOverwriteMode( FALSE );

  publicRb.setGeometry( 20, 270, 70, 20 );
  publicRb.setMinimumSize( 0, 0 );
  publicRb.setMaximumSize( 32767, 32767 );
  publicRb.setFocusPolicy( QWidget::TabFocus );
  publicRb.setBackgroundMode( QWidget::PaletteBackground );
  publicRb.setFontPropagation( QWidget::NoChildren );
  publicRb.setPalettePropagation( QWidget::NoChildren );
  publicRb.setText( "Public" );
  publicRb.setAutoRepeat( FALSE );
  publicRb.setAutoResize( FALSE );

  protectedRb.setGeometry( 110, 270, 80, 20 );
  protectedRb.setMinimumSize( 0, 0 );
  protectedRb.setMaximumSize( 32767, 32767 );
  protectedRb.setFocusPolicy( QWidget::TabFocus );
  protectedRb.setBackgroundMode( QWidget::PaletteBackground );
  protectedRb.setFontPropagation( QWidget::NoChildren );
  protectedRb.setPalettePropagation( QWidget::NoChildren );
  protectedRb.setText( "Protected" );
  protectedRb.setAutoRepeat( FALSE );
  protectedRb.setAutoResize( FALSE );

  privateRb.setGeometry( 200, 270, 60, 20 );
  privateRb.setMinimumSize( 0, 0 );
  privateRb.setMaximumSize( 32767, 32767 );
  privateRb.setFocusPolicy( QWidget::TabFocus );
  privateRb.setBackgroundMode( QWidget::PaletteBackground );
  privateRb.setFontPropagation( QWidget::NoChildren );
  privateRb.setPalettePropagation( QWidget::NoChildren );
  privateRb.setText( "Private" );
  privateRb.setAutoRepeat( FALSE );
  privateRb.setAutoResize( FALSE );
 
  virtualCb.setGeometry( 20, 330, 60, 20 );
  virtualCb.setMinimumSize( 0, 0 );
  virtualCb.setMaximumSize( 32767, 32767 );
  virtualCb.setFocusPolicy( QWidget::TabFocus );
  virtualCb.setBackgroundMode( QWidget::PaletteBackground );
  virtualCb.setFontPropagation( QWidget::NoChildren );
  virtualCb.setPalettePropagation( QWidget::NoChildren );
  virtualCb.setText( "Virtual" );
  virtualCb.setAutoRepeat( FALSE );
  virtualCb.setAutoResize( FALSE );

  staticCb.setGeometry( 110, 330, 60, 20 );
  staticCb.setMinimumSize( 0, 0 );
  staticCb.setMaximumSize( 32767, 32767 );
  staticCb.setFocusPolicy( QWidget::TabFocus );
  staticCb.setBackgroundMode( QWidget::PaletteBackground );
  staticCb.setFontPropagation( QWidget::NoChildren );
  staticCb.setPalettePropagation( QWidget::NoChildren );
  staticCb.setText( "Static" );
  staticCb.setAutoRepeat( FALSE );
  staticCb.setAutoResize( FALSE );
 
  constCb.setGeometry( 200, 330, 60, 20 );
  constCb.setMinimumSize( 0, 0 );
  constCb.setMaximumSize( 32767, 32767 );
  constCb.setFocusPolicy( QWidget::TabFocus );
  constCb.setBackgroundMode( QWidget::PaletteBackground );
  constCb.setFontPropagation( QWidget::NoChildren );
  constCb.setPalettePropagation( QWidget::NoChildren );
  constCb.setText( "Const" );
  constCb.setAutoRepeat( FALSE );
  constCb.setAutoResize( FALSE );

  okBtn.setGeometry( 10, 370, 100, 30 );
  okBtn.setMinimumSize( 0, 0 );
  okBtn.setMaximumSize( 32767, 32767 );
  okBtn.setFocusPolicy( QWidget::TabFocus );
  okBtn.setBackgroundMode( QWidget::PaletteBackground );
  okBtn.setFontPropagation( QWidget::NoChildren );
  okBtn.setPalettePropagation( QWidget::NoChildren );
  okBtn.setText( i18n("OK") );
  okBtn.setAutoRepeat( FALSE );
  okBtn.setAutoResize( FALSE );
	okBtn.setDefault( TRUE );

  cancelBtn.setGeometry( 170, 370, 100, 30 );
  cancelBtn.setMinimumSize( 0, 0 );
  cancelBtn.setMaximumSize( 32767, 32767 );
  cancelBtn.setFocusPolicy( QWidget::TabFocus );
  cancelBtn.setBackgroundMode( QWidget::PaletteBackground );
  cancelBtn.setFontPropagation( QWidget::NoChildren );
  cancelBtn.setPalettePropagation( QWidget::NoChildren );
  cancelBtn.setText( i18n("Cancel") );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );

  typeEdit.setFocus();
  publicRb.setChecked( true );

  modifierGrp.insert( &virtualCb );
  modifierGrp.insert( &staticCb );
  modifierGrp.insert( &constCb );

  accessGrp.insert( &publicRb );
  accessGrp.insert( &protectedRb );
  accessGrp.insert( &privateRb );
}

void CAddClassMethodDlg::setCallbacks()
{

  // Ok and cancel buttons.
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}

CParsedMethod *CAddClassMethodDlg::asSystemObj()
{
  int lpPos;
  QString decl;
  CParsedMethod *aMethod = new CParsedMethod();
  QString comment;

  aMethod->setType( typeEdit.text() );
  
  decl = declEdit.text();

  lpPos = decl.find( '(' );

  // If no arguments we add ().
  if( lpPos == -1 )
    aMethod->setName( decl + "()" );
  else // Else just set the whole thing as the name
    aMethod->setName( decl );
  
  // Set the export 
  if( publicRb.isChecked() )
    aMethod->setExport( PIE_PUBLIC );
  else if( protectedRb.isChecked() )
    aMethod->setExport( PIE_PROTECTED );
  else if( privateRb.isChecked() )
    aMethod->setExport( PIE_PRIVATE );
  
  // Set modifiers
  aMethod->setIsStatic( staticCb.isChecked() );
  aMethod->setIsConst( constCb.isChecked() );
  aMethod->setIsVirtual( virtualCb.isChecked() );

  // Set comment
  comment = "/** " + docEdit.text() + " */";
  aMethod->setComment( comment );

  return aMethod;
}

void CAddClassMethodDlg::OK()
{
  if( strlen( typeEdit.text() ) == 0 )
    KMsgBox::message( this, i18n("No type"),
                      i18n("You have to specify a function type.") );
  else if( strlen( declEdit.text() ) == 0 )
    KMsgBox::message( this, i18n("No name"),
                      i18n("You have to specify a function name.") );
  else
    accept();
}

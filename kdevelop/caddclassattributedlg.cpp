
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
#include "classstore.h"

#include <kmessagebox.h>
#include <kapp.h>
#include <qwhatsthis.h>
#include <klocale.h>

CAddClassAttributeDlg::CAddClassAttributeDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true ),
    topLayout( this, 5 ),
    varLayout( 9, 3, 5, "functionLayout" ),
    accessLayout( 3, 5, 5, "accessLayout" ),
    modifierLayout( 3, 4, 5, "modifierLayout" ),
    buttonLayout( 5, "buttonLayout" ),
    modifierGrp( this, "modifierGrp" ),
    varGrp( this, "functionGrp" ),
    accessGrp( this, "accessGrp" ),
    typeLbl( this, "typeLbl" ),
    typeEdit( this, "typeEdit" ),
    nameLbl( this, "nameLbl" ),
    nameEdit( this, "nameEdit" ),
    docLbl( this, "docLbl" ),
    docEdit( this, "docEdit" ),
    publicRb( this, "publicRb" ),
    protectedRb( this, "protectedRb" ),
    privateRb( this, "privateRb" ),
    staticCb( this, "staticCb" ),
    constCb( this, "constCb" ),
    okBtn( this, "okBtn" ),
    cancelBtn( this, "cancelBtn" )
{
  setCaption( i18n("Add Member Variable") );

  setWidgetValues();
  setCallbacks();
}

void CAddClassAttributeDlg::setWidgetValues()
{
  // Top layout
  topLayout.addLayout( &varLayout );
  topLayout.addLayout( &accessLayout );
  topLayout.addLayout( &modifierLayout );
  topLayout.addLayout( &buttonLayout );

  // Variable group
  varGrp.setFrameStyle( 49 );
  varGrp.setTitle( i18n("Variable") );

  // Modifier group
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( i18n("Modifiers") );
  QWhatsThis::add(&modifierGrp,i18n("You can set modifiers for the member variable here."));

  // Access group
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( i18n("Access") );
  QWhatsThis::add(&accessGrp,i18n(
  "You can choose here whether you want the member variable\n"
  "be declared as public, protected or private."));

  typeLbl.setMinimumSize( 40, 20 );
  typeLbl.setFixedHeight( 20 );
  typeLbl.setText( i18n("Type:") );

  typeEdit.setMinimumSize( 40, 30 );
  typeEdit.setFixedHeight( 30 );
  typeEdit.setFrame( TRUE );
  QWhatsThis::add(&typeLbl, i18n("Enter the type of the member variable here."));
  QWhatsThis::add(&typeEdit,i18n("Enter the type of the member variable here."));

  nameLbl.setMinimumSize( 70, 20 );
  nameLbl.setFixedHeight( 20 );
  nameLbl.setText( i18n("Name:") );

  nameEdit.setMinimumSize( 240, 30 );
  nameEdit.setFixedHeight( 30 );
  nameEdit.setFrame( TRUE );
  QWhatsThis::add(&nameLbl,i18n("Enter the name of the member variable here."));
  QWhatsThis::add(&nameEdit,i18n("Enter the name of the member variable here."));

  docLbl.setMinimumSize( 100, 20 );
  docLbl.setFixedHeight( 20 );
  docLbl.setText( i18n("Documentation:") );
  QWhatsThis::add(&docLbl,i18n("You can enter a description of the member variable here."));
  QWhatsThis::add(&docEdit,i18n("You can enter a description of the member variable here."));

  docEdit.setMinimumSize( 240, 80 );

  publicRb.setMinimumSize( 70, 20 );
  publicRb.setFixedHeight( 20 );
  publicRb.setText( "Public" );
  publicRb.setChecked( true );

  protectedRb.setMinimumSize( 80, 20 );
  protectedRb.setFixedHeight( 20 );
  protectedRb.setText( "Protected" );

  privateRb.setMinimumSize( 60, 20 );
  privateRb.setFixedHeight( 20 );
  privateRb.setText( "Private" );

  staticCb.setMinimumSize( 60, 20 );
  staticCb.setFixedHeight( 20 );
  staticCb.setText( "Static" );

  constCb.setMinimumSize( 60, 20 );
  constCb.setFixedHeight( 20 );
  constCb.setText( "Const" );

  okBtn.setGeometry( 10, 370, 100, 30 );
  okBtn.setFixedSize( 100, 30 );
  okBtn.setText( i18n("OK") );
  okBtn.setDefault( TRUE );

  cancelBtn.setGeometry( 170, 370, 100, 30 );
  cancelBtn.setFixedSize( 100, 30 );
  cancelBtn.setText( i18n("Cancel") );
  cancelBtn.setAutoRepeat( FALSE );
  cancelBtn.setAutoResize( FALSE );

  // Access group
  accessGrp.insert( &publicRb );
  accessGrp.insert( &protectedRb );
  accessGrp.insert( &privateRb );

  // Modifier group
  modifierGrp.insert( &staticCb );
  modifierGrp.insert( &constCb );

  // Var layout.
  varLayout.addMultiCellWidget( &varGrp, 0, 8, 0, 2 );
  varLayout.addRowSpacing( 0, 20 );
  varLayout.addWidget( &typeLbl, 2, 1 );
  varLayout.addWidget( &typeEdit, 3, 1 );
  varLayout.addWidget( &nameLbl, 4, 1 );
  varLayout.addWidget( &nameEdit, 5, 1 );
  varLayout.addWidget( &docLbl, 6, 1 );
  varLayout.addWidget( &docEdit, 7, 1 );
  varLayout.addRowSpacing( 8, 10 );

  // Access layout
  accessLayout.addMultiCellWidget( &accessGrp, 0, 2, 0, 4 );
  accessLayout.addRowSpacing( 0, 20 );
  accessLayout.addWidget( &publicRb, 1, 1 );
  accessLayout.addWidget( &protectedRb, 1, 2 );
  accessLayout.addWidget( &privateRb, 1, 3 );
  accessLayout.addRowSpacing( 2, 10 );

  // Modifier layout
  modifierLayout.addMultiCellWidget( &modifierGrp, 0, 2, 0, 3 );
  modifierLayout.addRowSpacing( 0, 20 );
  modifierLayout.addWidget( &staticCb, 1, 1 );
  modifierLayout.addWidget( &constCb, 1, 2 );
  modifierLayout.addRowSpacing( 2, 10 );

  // Button layout
  buttonLayout.addWidget( &okBtn );
  //  buttonLayout.addWidget( &btnFill );
  buttonLayout.addWidget( &cancelBtn );

  // Set the default focus.
  typeEdit.setFocus();
}

void CAddClassAttributeDlg::setCallbacks()
{

  // Ok and cancel buttons.
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}


ParsedAttribute *CAddClassAttributeDlg::asSystemObj()
{
  ParsedAttribute *aAttr = new ParsedAttribute();
  QString comment;

  aAttr->setType( typeEdit.text() );
  aAttr->setName( nameEdit.text() );

  // Set export
  if( publicRb.isChecked() )
    aAttr->setAccess( PIE_PUBLIC );
  else if( protectedRb.isChecked() )
    aAttr->setAccess( PIE_PROTECTED );
  else if( privateRb.isChecked() )
    aAttr->setAccess( PIE_PRIVATE );

  // Set modifiers
  aAttr->setIsStatic( staticCb.isChecked() );
  aAttr->setIsConst( constCb.isChecked() );

  // Set comment
  comment = "/** " + docEdit.text() + " */";
  aAttr->setComment( comment );

  return aAttr;
}

void CAddClassAttributeDlg::OK()
{

  if( strlen( typeEdit.text() ) == 0 )
    KMessageBox::error( this,
                      i18n("You have to specify a variable type."),
                      i18n("No Type") );
  else if( strlen( nameEdit.text() ) == 0 )
    KMessageBox::error( this,
                      i18n("You have to specify a variable name."),
                      i18n("No Name") );
  else
    accept();
}

void CAddClassAttributeDlg::enterEvent(QEvent* event){
    QDialog::enterEvent(event);
    typeEdit.setFocus();
}
#include "caddclassattributedlg.moc"

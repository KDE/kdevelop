/***************************************************************************
               JavaAddClassMethodDialog.java  -  description

                             -------------------

    begin                : Fri Mar 19 1999

    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
	java conversion      : Richard Dale

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import org.kde.qt.*;
import org.kde.koala.*;

/** Dialog to create a new method for a class.
 * @author Jonas Nordin
 */
class JavaAddClassMethodDialog extends QDialog
{
// Private widgets

  ///////////////////////////////
  // Layouts
  ///////////////////////////////

  /** Main layout for the dialog. */
  QVBoxLayout topLayout;

  /** Layout for function definition. */
  QGridLayout functionLayout;

  /** Layout for choosing access. */
  QGridLayout accessLayout;

  /** Layout for choosing type. */
  QGridLayout typeLayout;

  /** Layout for choosing modifier. */
  QGridLayout modifierLayout;

  /** Layout for the ok/cancel buttons. */
  QHBoxLayout buttonLayout;

  ///////////////////////////////
  // Button groups
  //////////////////////////////

  QButtonGroup modifierGrp;
  QButtonGroup typeGrp;
  QButtonGroup functionGrp;
  QButtonGroup accessGrp;

  QLabel typeLbl;
  QLineEdit typeEdit;
  QLabel declLbl;
  QLineEdit declEdit;
  QLabel docLbl;
  QMultiLineEdit docEdit;

  /** Public method. */
  QRadioButton publicRb;
  /** Protected method rb. */
  QRadioButton protectedRb;
  /** Private method rb. */
  QRadioButton privateRb;

  /** This is a method rb. */
  QRadioButton methodRb;

  /** This method is final. */
  QCheckBox finalCb;
  /** This method is abstract-final. */
  QCheckBox abstractCb;
  /** This method is static. */
  QCheckBox staticCb;
  /** This method is native. */
  QCheckBox nativeCb;

  QPushButton okBtn;
  QPushButton cancelBtn;

public JavaAddClassMethodDialog( QWidget parent, String name )
{
	super( parent, name, true );
    topLayout = new QVBoxLayout( this, 5 );
    functionLayout = new QGridLayout( 9, 3, 10, "functionLayout" );
    accessLayout = new QGridLayout( 3, 5, 1, "accessLayout" );
    typeLayout = new QGridLayout( 3, 5, 1, "typeLayout" );
    modifierLayout = new QGridLayout( 3, 6, 1, "modifierLayout" );
    buttonLayout = new QHBoxLayout( 5, "buttonLayout" );
    modifierGrp = new QButtonGroup( this, "modifierGrp" );
    typeGrp = new QButtonGroup( this, "typeGrp" );
    functionGrp = new QButtonGroup( this, "functionGrp" );
    accessGrp = new QButtonGroup( this, "accessGrp" );
    typeLbl = new QLabel( this, "typeLbl" );
    typeEdit = new QLineEdit( this, "typeEdit" );
    declLbl = new QLabel( this, "declLbl" );
    declEdit = new QLineEdit( this, "declEdit" );
    docLbl = new QLabel( this, "docLbl" );
    docEdit = new QMultiLineEdit( this, "docEdit" );
    publicRb = new QRadioButton( this, "publicRb" );
    protectedRb = new QRadioButton( this, "protectedRb" );
    privateRb = new QRadioButton( this, "privateRb" );
    methodRb = new QRadioButton( this, "methodRb" );
    finalCb = new QCheckBox( this, "finalCb" );
    abstractCb = new QCheckBox( this, "abstractCb" );
    staticCb = new QCheckBox( this, "staticCb" );
    nativeCb = new QCheckBox( this, "nativeCb" );
    okBtn = new QPushButton( this, "okBtn" );
    cancelBtn = new QPushButton( this, "cancelBtn" );
  setCaption( tr("Add class member") );

  setWidgetValues();
  setCallbacks();
}

private void setWidgetValues()
{
  // Top layout
  topLayout.addLayout( functionLayout );
  topLayout.addLayout( accessLayout );
  topLayout.addLayout( typeLayout );
  topLayout.addLayout( modifierLayout );
  topLayout.addLayout( buttonLayout );

  // Function grp
  functionGrp.setFrameStyle( 49 );
  functionGrp.setTitle( tr("Function") );

  // Accessgrp
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( tr("Access") );

  String text;
  text = tr("You can choose here whether you want the method\n"
              + "be declared as public, protected or private.");
  QWhatsThis.add(accessGrp, text);

  typeGrp.setFrameStyle( 49 );
  typeGrp.setTitle( tr( "Type" ) );

  text = tr("Choose the type of method you want to create.\n"
              + "The type can be method.");
  QWhatsThis.add(typeGrp, text);

  // Modifier grp
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( tr("Modifiers") );
  modifierGrp.setAlignment( 1 );

  text = tr("You can set modifiers for the method here.");
  QWhatsThis.add(modifierGrp, text);

  typeLbl.setText( tr("Type:") );

  text = tr("Enter the type of the method here.");
  QWhatsThis.add(typeLbl, text);
  QWhatsThis.add(typeEdit, text);

  declLbl.setText( tr("Declaration:") );

  text = tr("Enter the declaration of the method here.");
  QWhatsThis.add(declLbl, text);
  QWhatsThis.add(declEdit, text);

  docLbl.setText( tr("Documentation:") );

  QFontMetrics fm = new QFontMetrics(docEdit.fontMetrics());
  docEdit.setMinimumWidth(fm.width("0")*30);

  text = tr("You can enter a description of the method here.");
  QWhatsThis.add(docLbl, text);
  QWhatsThis.add(docEdit, text);

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

  methodRb.setMinimumSize( 60, 20 );
  methodRb.setFixedHeight( 20 );
  methodRb.setText( tr( "Method" ) );
  methodRb.setChecked( true );

  finalCb.setMinimumSize( 60, 20 );
  finalCb.setFixedHeight( 20 );
  finalCb.setText( "Final" );

  abstractCb.setMinimumSize( 60, 20 );
  abstractCb.setFixedHeight( 20 );
  abstractCb.setText( "Abstract" );

  staticCb.setMinimumSize( 60, 20 );
  staticCb.setFixedHeight( 20 );
  staticCb.setText( "Static" );

  nativeCb.setMinimumSize( 60, 20 );
  nativeCb.setFixedHeight( 20 );
  nativeCb.setText( "Native" );

  okBtn.setText( tr("OK") );
  okBtn.setDefault( true );
  cancelBtn.setText( tr("Cancel") );

  // Access group
  accessGrp.insert( publicRb );
  accessGrp.insert( protectedRb );
  accessGrp.insert( privateRb );

  // Type group
  typeGrp.insert( methodRb );

  // Modifier group
  modifierGrp.insert( finalCb );
  modifierGrp.insert( abstractCb );
  modifierGrp.insert( staticCb );
  modifierGrp.insert( nativeCb );

  // Function layout.
  functionLayout.addMultiCellWidget( functionGrp, 0, 8, 0, 2 );
  functionLayout.addRowSpacing( 0, 10 );
  functionLayout.addWidget( typeLbl, 2, 1 );
  functionLayout.addWidget( typeEdit, 3, 1 );
  functionLayout.addWidget( declLbl, 4, 1 );
  functionLayout.addWidget( declEdit, 5, 1 );
  functionLayout.addWidget( docLbl, 6, 1 );
  functionLayout.addWidget( docEdit, 7, 1 );
  functionLayout.addRowSpacing( 8, 10 );

  // Access layout
  accessLayout.addMultiCellWidget( accessGrp, 0, 2, 0, 4 );
  accessLayout.addRowSpacing( 0, 20 );
  accessLayout.addWidget( publicRb, 1, 1 );
  accessLayout.addWidget( protectedRb, 1, 2 );
  accessLayout.addWidget( privateRb, 1, 3 );
  accessLayout.addRowSpacing( 2, 10 );

  // Type layout
  typeLayout.addMultiCellWidget( typeGrp, 0, 2, 0, 4 );
  typeLayout.addRowSpacing( 0, 20 );
  typeLayout.addWidget( methodRb, 1, 1 );
  typeLayout.addRowSpacing( 2, 10 );

  // Modifier layout
  modifierLayout.addMultiCellWidget( modifierGrp, 0, 2, 0, 5 );
  modifierLayout.addRowSpacing( 0, 20 );
  modifierLayout.addColSpacing( 0, 10 );
  modifierLayout.addWidget( finalCb, 1, 1 );
  modifierLayout.addWidget( abstractCb, 1, 2 );
  modifierLayout.addWidget( staticCb, 1, 3 );
  modifierLayout.addWidget( nativeCb, 1, 4 );
  modifierLayout.addColSpacing( 5, 10 );
  modifierLayout.addRowSpacing( 2, 10 );

  // Button layout
  buttonLayout.addWidget( okBtn );
  buttonLayout.addStretch();
  buttonLayout.addWidget( cancelBtn );

  // Set the default focus.
  typeEdit.setFocus();
}

private void setCallbacks()
{
  connect( methodRb, SIGNAL( "clicked()" ), SLOT( "slotToggleModifier()" ) );
  connect( finalCb, SIGNAL ( "clicked()" ), SLOT( "slotFinalClicked()" ) );

  // Ok and cancel buttons.
  connect( okBtn, SIGNAL( "clicked()" ), SLOT( "OK()" ) );
  connect( cancelBtn, SIGNAL( "clicked()" ), SLOT( "reject()" ) );
 }

public ParsedMethod asSystemObj()
{
  int lpPos;
  String decl;
  ParsedMethod aMethod = new ParsedMethod();
  String comment;

  aMethod.setType( typeEdit.text() );

  decl = declEdit.text();

  lpPos = decl.indexOf( '(' );

  // If no arguments we add ().
  if( lpPos == -1 )
    aMethod.setName( decl + "()" );
  else // Else just set the whole thing as the name
    aMethod.setName( decl );


  // Set the export.
  if( publicRb.isChecked() )
    aMethod.setAccess( ParsedItem.PIE_PUBLIC );
  else if( protectedRb.isChecked() )
    aMethod.setAccess( ParsedItem.PIE_PROTECTED );
  else if( privateRb.isChecked() )
    aMethod.setAccess( ParsedItem.PIE_PRIVATE );

  // Set the modifiers if they are enabled.
  if( abstractCb.isEnabled() )
    aMethod.setIsPure( abstractCb.isChecked() );
  if( staticCb.isEnabled() )
    aMethod.setIsStatic( staticCb.isChecked() );
  if( finalCb.isEnabled() )
    aMethod.setIsVirtual( ! finalCb.isChecked() );
  if( nativeCb.isEnabled())
    aMethod.setIsConst( nativeCb.isChecked() );

  // Set comment
  comment = "/** " + docEdit.text() + " */";
  aMethod.setComment( comment );

  return aMethod;
}

protected void slotToggleModifier()
{
    finalCb.setEnabled( true );
    abstractCb.setEnabled( false );
    slotFinalClicked();
}

protected void slotFinalClicked()
{
  abstractCb.setEnabled(  ! finalCb.isChecked() );
//  staticCb.setEnabled( finalCb.isChecked() );
}

protected void OK()
{
  if( typeEdit.text().length() == 0 )
    QMessageBox.information( this, tr("No type"),
                      tr("You have to specify a method return type.") );
  else if( declEdit.text().length() == 0 )
    QMessageBox.information( this, tr("No name"),
                      tr("You have to specify a method name.") );
  else
    accept();
}

}



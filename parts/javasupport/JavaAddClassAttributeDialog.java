/***************************************************************************
               JavaAddClassAttributeDialog.java  -  description

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

/** Dialog to create a new attibute for a class.
 * @author Jonas Nordin
 */
class JavaAddClassAttributeDialog extends QDialog
{

  ///////////////////////////////
  // Layouts
  ///////////////////////////////

  /** Main layout for the dialog. */
public QVBoxLayout topLayout;

  /** Layout for function definition. */
public QGridLayout varLayout;

  /** Layout for choosing access. */
public QGridLayout accessLayout;

  /** Layout for choosing modifier. */
public QGridLayout modifierLayout;

  /** Layout for the ok/cancel buttons. */
public QHBoxLayout buttonLayout;

  ///////////////////////////////
  // Groups
  ///////////////////////////////

public QButtonGroup modifierGrp;
public QButtonGroup varGrp;
public QButtonGroup accessGrp;

public QLabel typeLbl;
public QLineEdit typeEdit;
public QLabel nameLbl;
public QLineEdit nameEdit;
public QLabel docLbl;
public QMultiLineEdit docEdit;

public   QRadioButton publicRb;
public   QRadioButton protectedRb;
public   QRadioButton privateRb;

public   QCheckBox staticCb;
public   QCheckBox constCb;

public   QPushButton okBtn;
public   QPushButton cancelBtn;


public JavaAddClassAttributeDialog( QWidget parent, String name )
{
  	super( parent, name, true );
    topLayout = new QVBoxLayout( this, 5 );
    varLayout = new QGridLayout( 9, 3, 10, "functionLayout" );
    accessLayout = new QGridLayout( 3, 5, 1, "accessLayout" );
    modifierLayout = new QGridLayout( 3, 4, 1, "modifierLayout" );
    buttonLayout = new QHBoxLayout( 5, "buttonLayout" );
    modifierGrp = new QButtonGroup( this, "modifierGrp" );
    varGrp = new QButtonGroup( this, "functionGrp" );
    accessGrp = new QButtonGroup( this, "accessGrp" );
    typeLbl = new QLabel( this, "typeLbl" );
    typeEdit = new QLineEdit( this, "typeEdit" );
    nameLbl = new QLabel( this, "nameLbl" );
    nameEdit = new QLineEdit( this, "nameEdit" );
    docLbl = new QLabel( this, "docLbl" );
    docEdit = new QMultiLineEdit( this, "docEdit" );
    publicRb = new QRadioButton( this, "publicRb" );
    protectedRb = new QRadioButton( this, "protectedRb" );
    privateRb = new QRadioButton( this, "privateRb" );
    staticCb = new QCheckBox( this, "staticCb" );
    constCb = new QCheckBox( this, "constCb" );
    okBtn = new QPushButton( this, "okBtn" );
    cancelBtn = new QPushButton( this, "cancelBtn" );
  setCaption( tr("Add member variable") );

  setWidgetValues();
  setCallbacks();
}

private void setWidgetValues()
{
  // Top layout
  topLayout.addLayout( varLayout );
  topLayout.addLayout( accessLayout );
  topLayout.addLayout( modifierLayout );
  topLayout.addLayout( buttonLayout );

  // Variable group
  varGrp.setFrameStyle( 49 );
  varGrp.setTitle( tr("Variable") );

  // Modifier group
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( tr("Modifiers") );

  String text;
  text = tr("You can set modifiers for the member variable here.");
  QWhatsThis.add(modifierGrp, text);

  // Access group
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( tr("Access") );

  text = tr("You can choose here whether you want the member variable\n"
              + "be declared as public, protected or private.");
  QWhatsThis.add(accessGrp, text);

  typeLbl.setText( tr("Type:") );

  text = tr("Enter the type of the member variable here.");
  QWhatsThis.add(typeLbl, text);
  QWhatsThis.add(typeEdit, text);

  nameLbl.setText( tr("Name:") );

  text = tr("Enter the name of the member variable here.");
  QWhatsThis.add(nameLbl, text);
  QWhatsThis.add(nameEdit, text);

  docLbl.setText( tr("Documentation:") );

  text = tr("You can enter a description of the member variable here.");
  QWhatsThis.add(docLbl, text);
  QWhatsThis.add(docEdit, text);

  QFontMetrics fm = new QFontMetrics(docEdit.fontMetrics());
  docEdit.setMinimumWidth(fm.width("0")*30);

  publicRb.setText( "Public" );
  publicRb.setChecked( true );

  protectedRb.setText( "Protected" );
  privateRb.setText( "Private" );
  staticCb.setText( "Static" );
  constCb.setText( "Const" );

  okBtn.setText( tr("OK") );
  okBtn.setDefault( true );
  cancelBtn.setText( tr("Cancel") );

  // Access group
  accessGrp.insert( publicRb );
  accessGrp.insert( protectedRb );
  accessGrp.insert( privateRb );

  // Modifier group
  modifierGrp.insert( staticCb );
  modifierGrp.insert( constCb );

  // Var layout.
  varLayout.addMultiCellWidget( varGrp, 0, 8, 0, 2 );
  varLayout.addRowSpacing( 0, 10 );
  varLayout.addWidget( typeLbl, 2, 1 );
  varLayout.addWidget( typeEdit, 3, 1 );
  varLayout.addWidget( nameLbl, 4, 1 );
  varLayout.addWidget( nameEdit, 5, 1 );
  varLayout.addWidget( docLbl, 6, 1 );
  varLayout.addWidget( docEdit, 7, 1 );
  varLayout.addRowSpacing( 8, 10 );

  // Access layout
  accessLayout.addMultiCellWidget( accessGrp, 0, 2, 0, 4 );
  accessLayout.addRowSpacing( 0, 20 );
  accessLayout.addWidget( publicRb, 1, 1 );
  accessLayout.addWidget( protectedRb, 1, 2 );
  accessLayout.addWidget( privateRb, 1, 3 );
  accessLayout.addRowSpacing( 2, 10 );

  // Modifier layout
  modifierLayout.addMultiCellWidget( modifierGrp, 0, 2, 0, 3 );
  modifierLayout.addRowSpacing( 0, 15 );
  modifierLayout.addWidget( staticCb, 1, 1 );
  modifierLayout.addWidget( constCb, 1, 2 );
  modifierLayout.addRowSpacing( 2, 15 );

  // Button layout
  buttonLayout.addWidget( okBtn );
  buttonLayout.addStretch();
  buttonLayout.addWidget( cancelBtn );

  // Set the default focus.
  typeEdit.setFocus();
}

private void setCallbacks()
{

  // Ok and cancel buttons.
  connect( okBtn, SIGNAL( "clicked()" ), SLOT( "OK()" ) );
  connect( cancelBtn, SIGNAL( "clicked()" ), SLOT( "reject()" ) );
}


public ParsedAttribute asSystemObj()
{
  ParsedAttribute aAttr = new ParsedAttribute();
  String comment;

  aAttr.setType( typeEdit.text() );
  aAttr.setName( nameEdit.text() );

  // Set export
  if( publicRb.isChecked() )
    aAttr.setAccess( ParsedItem.PIE_PUBLIC );
  else if( protectedRb.isChecked() )
    aAttr.setAccess( ParsedItem.PIE_PROTECTED );
  else if( privateRb.isChecked() )
    aAttr.setAccess( ParsedItem.PIE_PRIVATE );

  // Set modifiers
  aAttr.setIsStatic( staticCb.isChecked() );
  aAttr.setIsConst( constCb.isChecked() );

  // Set comment
  comment = "/** " + docEdit.text() + " */";
  aAttr.setComment( comment );

  return aAttr;
}

protected void OK()
{

  if( typeEdit.text().length() == 0 )
    QMessageBox.information( this, tr("No type"),
                      tr("You have to specify a variable type.") );
  else if( nameEdit.text().length() == 0 )
    QMessageBox.information( this, tr("No name"),
                      tr("You have to specify a variable name.") );
  else
    accept();
}

}

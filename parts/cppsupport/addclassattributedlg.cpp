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

#include "addclassattributedlg.h"

#include <qwhatsthis.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>


AddClassAttributeDialog::AddClassAttributeDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true),
      topLayout( this, 5 ),
      varLayout( 9, 3, 10, "functionLayout" ),
      accessLayout( 3, 5, 1, "accessLayout" ),
      modifierLayout( 3, 4, 1, "modifierLayout" ),
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


void AddClassAttributeDialog::setWidgetValues()
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
    
    QString text;
    text = i18n("You can set modifiers for the member variable here.");
    QWhatsThis::add(&modifierGrp, text);
    
    // Access group
    accessGrp.setFrameStyle( 49 );
    accessGrp.setTitle( i18n("Access") );
    
    text = i18n("You can choose here whether you want the member variable\n"
                "be declared as public, protected or private.");
    QWhatsThis::add(&accessGrp, text);
    
    typeLbl.setText( i18n("Type:") );
    
    text = i18n("Enter the type of the member variable here.");
    QWhatsThis::add(&typeLbl, text);
    QWhatsThis::add(&typeEdit, text);
    
    nameLbl.setText( i18n("Name:") );
    
    text = i18n("Enter the name of the member variable here.");
    QWhatsThis::add(&nameLbl, text);
    QWhatsThis::add(&nameEdit, text);
    
    docLbl.setText( i18n("Documentation:") );
    
    text = i18n("You can enter a description of the member variable here.");
    QWhatsThis::add(&docLbl, text);
    QWhatsThis::add(&docEdit, text);
    
    QFontMetrics fm(docEdit.fontMetrics());
    docEdit.setMinimumWidth(fm.width("0")*30);
    
    publicRb.setText( "Public" );
    publicRb.setChecked( true );
    
    protectedRb.setText( "Protected" );
    privateRb.setText( "Private" );
    staticCb.setText( "Static" );
    constCb.setText( "Const" );
    
    okBtn.setText( i18n("OK") );
    okBtn.setDefault( TRUE );
    cancelBtn.setText( i18n("Cancel") );
    
    // Access group
    accessGrp.insert( &publicRb );
    accessGrp.insert( &protectedRb );
    accessGrp.insert( &privateRb );
    
    // Modifier group
    modifierGrp.insert( &staticCb );
    modifierGrp.insert( &constCb );
    
    // Var layout.
    varLayout.addMultiCellWidget( &varGrp, 0, 8, 0, 2 );
    varLayout.addRowSpacing( 0, 10 );
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
    modifierLayout.addRowSpacing( 0, 15 );
    modifierLayout.addWidget( &staticCb, 1, 1 );
    modifierLayout.addWidget( &constCb, 1, 2 );
    modifierLayout.addRowSpacing( 2, 15 );
    
    // Button layout
    buttonLayout.addWidget( &okBtn );
    buttonLayout.addStretch();
    buttonLayout.addWidget( &cancelBtn );
    
    // Set the default focus.
    typeEdit.setFocus();
}


void AddClassAttributeDialog::setCallbacks()
{
    
    // Ok and cancel buttons.
    connect( &okBtn, SIGNAL( clicked() ), SLOT( accept() ) );
    connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}


ParsedAttribute *AddClassAttributeDialog::asSystemObj()
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


void AddClassAttributeDialog::accept()
{
    if (typeEdit.text().isEmpty()) {
        KMessageBox::sorry(this, i18n("No type"),
                           i18n("You have to specify a variable type.") );
        return;
    }
    
    if (nameEdit.text().isEmpty()) {
        KMessageBox::sorry(this, i18n("No name"),
                           i18n("You have to specify a variable name.") );
        return;
    }
    
    QDialog::accept();
}

#include "addclassattributedlg.moc"

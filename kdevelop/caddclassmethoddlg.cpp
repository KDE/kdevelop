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
#include "cclonefunctiondlg.h"
#include "classstore.h"

#include <kmessagebox.h>
#include <qwhatsthis.h>
#include <klocale.h>

CAddClassMethodDlg::CAddClassMethodDlg(CClassView* class_tree, QWidget *parent, const char *name ) :
  QDialog( parent, name, true ),
  topLayout( this, 5 ),
  functionLayout( 9, 3, 5, "functionLayout" ),
  accessLayout( 3, 5,5, "accessLayout" ),
  typeLayout( 3, 5, 5, "typeLayout" ),
  modifierLayout( 3, 6,5, "modifierLayout" ),
  buttonLayout( 5, "buttonLayout" ),
  modifierGrp( this, "modifierGrp" ),
  typeGrp( this, "typeGrp" ),
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
  methodRb( this, "methodRb" ),
  slotRb( this, "slotRb" ),
  signalRb( this, "signalRb" ),
  virtualCb( this, "virtualCb" ),
  pureCb( this, "pureCb" ),
  staticCb( this, "staticCb" ),
  constCb( this, "constCb" ),
  cloneBtn( this, "cloneBtn" ),
  okBtn( this, "okBtn" ),
  cancelBtn( this, "cancelBtn" ),
  btnFill( this, "btnFill" ),
  classtree(class_tree)
{
  setCaption( i18n("Add Class Member") );

  setWidgetValues();
  setCallbacks();
}

void CAddClassMethodDlg::setWidgetValues()
{
  // Top layout
  topLayout.addWidget( &cloneBtn );
  topLayout.addLayout( &functionLayout );
  topLayout.addLayout( &accessLayout );
  topLayout.addLayout( &typeLayout );
  topLayout.addLayout( &modifierLayout );
  topLayout.addLayout( &buttonLayout );

  // Function grp
  functionGrp.setFrameStyle( 49 );
  functionGrp.setTitle( i18n("Function") );

  // Accessgrp
  accessGrp.setFrameStyle( 49 );
  accessGrp.setTitle( i18n("Access") );
  QWhatsThis::add( &accessGrp,
                   i18n(
                     "You can choose here whether you want the member function\n"
                     "be declared as public, protected or private."));

  typeGrp.setFrameStyle( 49 );
  typeGrp.setTitle( i18n( "Type" ) );
  QWhatsThis::add( &typeGrp,
                   i18n( "Choose the type of member object you want to create.\nThe type can be signal, slot or method." ) );

  // Modifier grp
  modifierGrp.setFrameStyle( 49 );
  modifierGrp.setTitle( i18n("Modifiers") );
  modifierGrp.setAlignment( 1 );
  QWhatsThis::add( &modifierGrp,
                   i18n("You can set modifiers for the member function here."));

  typeLbl.setMinimumSize( 40, 20 );
  typeLbl.setFixedHeight( 20 );
  typeLbl.setText( i18n("Type:") );

  typeEdit.setMinimumSize( 240, 30 );
  typeEdit.setFixedHeight( 30 );
  typeEdit.setFrame( TRUE );
  QWhatsThis::add(&typeLbl,i18n("Enter the type of the member function here."));
  QWhatsThis::add(&typeEdit,i18n("Enter the type of the member function here."));

  declLbl.setMinimumSize( 70, 20 );
  declLbl.setFixedHeight( 20 );
  declLbl.setText( i18n("Declaration:") );

  declEdit.setMinimumSize( 240, 30 );
  declEdit.setFixedHeight( 30 );
  declEdit.setFrame( TRUE );
  QWhatsThis::add(&declLbl,i18n("Enter the declaration of the member function here."));
  QWhatsThis::add(&declEdit,i18n("Enter the declaration of the member function here."));

  docLbl.setMinimumSize( 100, 20 );
  docLbl.setFixedHeight( 20 );
  docLbl.setText( i18n("Documentation:") );

  docEdit.setMinimumSize( 240, 80 );
  QWhatsThis::add(&docLbl,i18n("You can enter a description of the member function here."));
  QWhatsThis::add(&docEdit,i18n("You can enter a description of the member function here."));

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
  methodRb.setText( i18n( "Method" ) );
  methodRb.setChecked( true );

  slotRb.setMinimumSize( 60, 20 );
  slotRb.setFixedHeight( 20 );
  slotRb.setText( "Slot" );

  signalRb.setMinimumSize( 60, 20 );
  signalRb.setFixedHeight( 20 );
  signalRb.setText( "Signal" );

  virtualCb.setMinimumSize( 60, 20 );
  virtualCb.setFixedHeight( 20 );
  virtualCb.setText( "Virtual" );

  pureCb.setMinimumSize( 60, 20 );
  pureCb.setFixedHeight( 20 );
  pureCb.setText( "Pure" );
  pureCb.setEnabled( false );

  staticCb.setMinimumSize( 60, 20 );
  staticCb.setFixedHeight( 20 );
  staticCb.setText( "Static" );

  constCb.setMinimumSize( 60, 20 );
  constCb.setFixedHeight( 20 );
  constCb.setText( "Const" );

  //cloneBtn.setGeometry( 10, 370, 100, 30 );
  cloneBtn.setFixedSize( 100, 30 );
  cloneBtn.setText( i18n("Clone") );
  cloneBtn.setDefault( TRUE );

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

  // Type group
  typeGrp.insert( &methodRb );
  typeGrp.insert( &slotRb );
  typeGrp.insert( &signalRb );

  // Modifier group
  modifierGrp.insert( &virtualCb );
  modifierGrp.insert( &pureCb );
  modifierGrp.insert( &staticCb );
  modifierGrp.insert( &constCb );

  // Function layout.
  functionLayout.addMultiCellWidget( &functionGrp, 0, 8, 0, 2 );
  functionLayout.addRowSpacing( 0, 20 );
  functionLayout.addWidget( &typeLbl, 2, 1 );
  functionLayout.addWidget( &typeEdit, 3, 1 );
  functionLayout.addWidget( &declLbl, 4, 1 );
  functionLayout.addWidget( &declEdit, 5, 1 );
  functionLayout.addWidget( &docLbl, 6, 1 );
  functionLayout.addWidget( &docEdit, 7, 1 );
  functionLayout.addRowSpacing( 8, 10 );

  // Access layout
  accessLayout.addMultiCellWidget( &accessGrp, 0, 2, 0, 4 );
  accessLayout.addRowSpacing( 0, 20 );
  accessLayout.addWidget( &publicRb, 1, 1 );
  accessLayout.addWidget( &protectedRb, 1, 2 );
  accessLayout.addWidget( &privateRb, 1, 3 );
  accessLayout.addRowSpacing( 2, 10 );

  // Type layout
  typeLayout.addMultiCellWidget( &typeGrp, 0, 2, 0, 4 );
  typeLayout.addRowSpacing( 0, 20 );
  typeLayout.addWidget( &methodRb, 1, 1 );
  typeLayout.addWidget( &slotRb, 1, 2 );
  typeLayout.addWidget( &signalRb, 1, 3 );
  typeLayout.addRowSpacing( 2, 10 );

  // Modifier layout
  modifierLayout.addMultiCellWidget( &modifierGrp, 0, 2, 0, 5 );
  modifierLayout.addRowSpacing( 0, 20 );
  modifierLayout.addWidget( &virtualCb, 1, 1 );
  modifierLayout.addWidget( &pureCb, 1, 2 );
  modifierLayout.addWidget( &staticCb, 1, 3 );
  modifierLayout.addWidget( &constCb, 1, 4 );
  modifierLayout.addRowSpacing( 2, 10 );

  // Button layout
  buttonLayout.addWidget( &okBtn );
  buttonLayout.addWidget( &btnFill );
  buttonLayout.addWidget( &cancelBtn );

  // Set the default focus.
  typeEdit.setFocus();
}

void CAddClassMethodDlg::setCallbacks()
{
  connect( &methodRb, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( &slotRb, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( &signalRb, SIGNAL( clicked() ), SLOT( slotToggleModifier() ) );
  connect( &virtualCb, SIGNAL ( clicked() ), SLOT( slotVirtualClicked() ) );

  // Ok and cancel buttons.
  connect( &cloneBtn, SIGNAL( clicked() ), SLOT( slotCloneClicked() ) );
  connect( &okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( &cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );
}

ParsedMethod *CAddClassMethodDlg::asSystemObj()
{
  int lpPos;
  QString decl;
  ParsedMethod *aMethod = new ParsedMethod();
  QString comment;

  aMethod->setType( typeEdit.text() );

  decl = declEdit.text();

  lpPos = decl.find( '(' );

  // If no arguments we add ().
  if( lpPos == -1 )
    aMethod->setName( decl + "()" );
  else // Else just set the whole thing as the name
    aMethod->setName( decl );

  // Set the type.
  if( slotRb.isChecked() )
    aMethod->setIsSlot( true );
  else if( signalRb.isChecked() )
    aMethod->setIsSignal( true );

  // Set the export.
  if( publicRb.isChecked() )
    aMethod->setAccess( PIE_PUBLIC );
  else if( protectedRb.isChecked() )
    aMethod->setAccess( PIE_PROTECTED );
  else if( privateRb.isChecked() )
    aMethod->setAccess( PIE_PRIVATE );

  // Set the modifiers if they are enabled.
  if( pureCb.isEnabled() )
    aMethod->setIsPure( pureCb.isChecked() );
  if( staticCb.isEnabled() )
    aMethod->setIsStatic( staticCb.isChecked() );
  if( constCb.isEnabled() )
    aMethod->setIsConst( constCb.isChecked() );
  if( virtualCb.isEnabled())
    aMethod->setIsVirtual( virtualCb.isChecked() );

  // Set comment
  comment = "/** " + docEdit.text() + " */";
  aMethod->setComment( comment );

  return aMethod;
}

void CAddClassMethodDlg::slotToggleModifier()
{
  if( slotRb.isChecked() || signalRb.isChecked() )
  {
    staticCb.setEnabled( false );
    constCb.setEnabled( false );
    virtualCb.setEnabled( false );
    pureCb.setEnabled( false );
  }
  else
  {
    constCb.setEnabled( true );
    virtualCb.setEnabled( true );

    slotVirtualClicked();
  }
}

void CAddClassMethodDlg::slotVirtualClicked()
{
  pureCb.setEnabled( virtualCb.isChecked() );
  staticCb.setEnabled( !virtualCb.isChecked() );
}

void CAddClassMethodDlg::OK()
{
  if( strlen( typeEdit.text() ) == 0 )
    KMessageBox::error( this,
                      i18n("You have to specify a function type."),
                      i18n("No Type"));
  else if( strlen( declEdit.text() ) == 0 )
    KMessageBox::error( this,
                      i18n("You have to specify a function name."),
                      i18n("No Name"));
  else
    accept();
}

void CAddClassMethodDlg::enterEvent(QEvent* event){
    QDialog::enterEvent(event);
    typeEdit.setFocus();
}
/** look up method to overload */
void CAddClassMethodDlg::slotCloneClicked(){

//#warning "FIXME: this file isn't used anymore - AH"
#if 0
  CCloneFunctionDlg volDlg(classtree, this, "volnameDlg");

  if (volDlg.exec()) {
    ParsedMethod* res = volDlg.getMethod();
    if (! res)
    	return;

    // copy type and declaration
    QString str;
    typeEdit.setText(res->type());;
    declEdit.setText(res->asString());

    // the comment needs some adjustment
    str = res->comment;
    // remove /** and */
    str.replace( QRegExp("^/\\** *"), "" );
    str.replace( QRegExp(" *\\**/$"), "" );
    // clean up line breaks
    str.replace( QRegExp("\n *\\** *"), "\n" );
    docEdit.setText(str);

    // all the buttons
   	privateRb.setChecked( res->isPrivate() );
    protectedRb.setChecked( res->isProtected() );
    publicRb.setChecked( res->isPublic() );
    constCb.setChecked( res->isConst );

    methodRb.setChecked( true ); // ??
//??    slotRb.setChecked( res->isSlot );
//??    signalRb.setChecked( res->isSignal );

    constCb.setChecked( res->isConst);
    staticCb.setChecked( res->isStatic );
    pureCb.setChecked( res->isPure );
    virtualCb.setChecked( res->isVirtual );
  }
#endif
}

#include "caddclassmethoddlg.moc"

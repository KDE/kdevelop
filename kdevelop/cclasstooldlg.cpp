/***************************************************************************
               cclasstooldlg.cpp  -  description

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


#include "cclasstooldlg.h"
#include <kmsgbox.h>
#include <kapp.h>
#include <qtooltip.h>
#include <assert.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CClassToolDlg::CClassToolDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, true ),
    classTree( this, "classTree" ),
    parentsBtn( this, "parentsBtn" ),
    childrenBtn( this, "childrenBtn" ),
    clientsBtn( this, "clientsBtn" ),
    suppliersBtn( this, "suppliersBtn" ),
    attributesBtn( this, "attributesBtn" ),
    methodsBtn( this, "methodsBtn" ),
    virtualsBtn( this, "virtualsBtn" ),
    exportCombo( false, this, "exportCombo" )
{
  export = 0;
  onlyVirtual = false;
  store = NULL;
  setCaption( "Class Tool" );

  setWidgetValues();
  setIcons();
  setCallbacks();
  setTooltips();
}


/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

void CClassToolDlg::setWidgetValues()
{
  classTree.setGeometry( 10, 50, 400, 230 );
  classTree.setIndentSpacing(15);
  classTree.setFocusPolicy(QWidget::NoFocus);

  parentsBtn.setGeometry( 10, 10, 30, 30 );
  parentsBtn.setMinimumSize( 0, 0 );
  parentsBtn.setMaximumSize( 32767, 32767 );
  parentsBtn.setFocusPolicy( QWidget::TabFocus );
  parentsBtn.setBackgroundMode( QWidget::PaletteBackground );
  parentsBtn.setFontPropagation( QWidget::NoChildren );
  parentsBtn.setPalettePropagation( QWidget::NoChildren );
  parentsBtn.setAutoRepeat( FALSE );
  parentsBtn.setAutoResize( FALSE );

  childrenBtn.setGeometry( 50, 10, 30, 30 );
  childrenBtn.setMinimumSize( 0, 0 );
  childrenBtn.setMaximumSize( 32767, 32767 );
  childrenBtn.setFocusPolicy( QWidget::TabFocus );
  childrenBtn.setBackgroundMode( QWidget::PaletteBackground );
  childrenBtn.setFontPropagation( QWidget::NoChildren );
  childrenBtn.setPalettePropagation( QWidget::NoChildren );
  childrenBtn.setText( "" );
  childrenBtn.setAutoRepeat( FALSE );
  childrenBtn.setAutoResize( FALSE );

  clientsBtn.setGeometry( 90, 10, 30, 30 );
  clientsBtn.setMinimumSize( 0, 0 );
  clientsBtn.setMaximumSize( 32767, 32767 );
  clientsBtn.setFocusPolicy( QWidget::TabFocus );
  clientsBtn.setBackgroundMode( QWidget::PaletteBackground );
  clientsBtn.setFontPropagation( QWidget::NoChildren );
  clientsBtn.setPalettePropagation( QWidget::NoChildren );
  clientsBtn.setText( "" );
  clientsBtn.setAutoRepeat( FALSE );
  clientsBtn.setAutoResize( FALSE );

  suppliersBtn.setGeometry( 130, 10, 30, 30 );
  suppliersBtn.setMinimumSize( 0, 0 );
  suppliersBtn.setMaximumSize( 32767, 32767 );
  suppliersBtn.setFocusPolicy( QWidget::TabFocus );
  suppliersBtn.setBackgroundMode( QWidget::PaletteBackground );
  suppliersBtn.setFontPropagation( QWidget::NoChildren );
  suppliersBtn.setPalettePropagation( QWidget::NoChildren );
  suppliersBtn.setText( "" );
  suppliersBtn.setAutoRepeat( FALSE );
  suppliersBtn.setAutoResize( FALSE );

  attributesBtn.setGeometry( 190, 10, 30, 30 );
  attributesBtn.setMinimumSize( 0, 0 );
  attributesBtn.setMaximumSize( 32767, 32767 );
  attributesBtn.setFocusPolicy( QWidget::TabFocus );
  attributesBtn.setBackgroundMode( QWidget::PaletteBackground );
  attributesBtn.setFontPropagation( QWidget::NoChildren );
  attributesBtn.setPalettePropagation( QWidget::NoChildren );
  attributesBtn.setText( "" );
  attributesBtn.setAutoRepeat( FALSE );
  attributesBtn.setAutoResize( FALSE );

  methodsBtn.setGeometry( 230, 10, 30, 30 );
  methodsBtn.setMinimumSize( 0, 0 );
  methodsBtn.setMaximumSize( 32767, 32767 );
  methodsBtn.setFocusPolicy( QWidget::TabFocus );
  methodsBtn.setBackgroundMode( QWidget::PaletteBackground );
  methodsBtn.setFontPropagation( QWidget::NoChildren );
  methodsBtn.setPalettePropagation( QWidget::NoChildren );
  methodsBtn.setText( "" );
  methodsBtn.setAutoRepeat( FALSE );
  methodsBtn.setAutoResize( FALSE );

  virtualsBtn.setGeometry( 270, 10, 30, 30 );
  virtualsBtn.setMinimumSize( 0, 0 );
  virtualsBtn.setMaximumSize( 32767, 32767 );
  virtualsBtn.setFocusPolicy( QWidget::TabFocus );
  virtualsBtn.setBackgroundMode( QWidget::PaletteBackground );
  virtualsBtn.setFontPropagation( QWidget::NoChildren );
  virtualsBtn.setPalettePropagation( QWidget::NoChildren );
  virtualsBtn.setText( "" );
  virtualsBtn.setAutoRepeat( FALSE );
  virtualsBtn.setAutoResize( FALSE );

  exportCombo.setGeometry( 310, 10, 100, 30 );
  exportCombo.setMinimumSize( 0, 0 );
  exportCombo.setMaximumSize( 32767, 32767 );
  exportCombo.setFocusPolicy( QWidget::StrongFocus );
  exportCombo.setBackgroundMode( QWidget::PaletteBackground );
  exportCombo.setFontPropagation( QWidget::AllChildren );
  exportCombo.setPalettePropagation( QWidget::AllChildren );
  exportCombo.setSizeLimit( 10 );
  exportCombo.setAutoResize( FALSE );
  exportCombo.insertItem( "All" );
  exportCombo.insertItem( "Public" );
  exportCombo.insertItem( "Protected" );
  exportCombo.insertItem( "Private" );
}

void CClassToolDlg::setIcons()
{
  QString PIXPREFIX = "/kdevelop/pics/mini/";
  QString pixDir = KApplication::kde_datadir() + PIXPREFIX;
  QPixmap pm;
  
  pm.load( pixDir + "CTparents.xpm" );
  parentsBtn.setPixmap( pm );
  
  pm.load( pixDir + "CTchildren.xpm" );
  childrenBtn.setPixmap( pm );

  pm.load( pixDir + "CTclients.xpm" );
  clientsBtn.setPixmap( pm );

  pm.load( pixDir + "CTsuppliers.xpm" );
  suppliersBtn.setPixmap( pm );

  pm.load( pixDir + "CVpublic_var.xpm" );
  attributesBtn.setPixmap( pm );

  pm.load( pixDir + "CVpublic_meth.xpm" );
  methodsBtn.setPixmap( pm );

  classPm = new QPixmap( pixDir + "CVclass.xpm" );
}

void CClassToolDlg::setTooltips()
{
  QToolTip::add( &parentsBtn, "View parents" );
  QToolTip::add( &childrenBtn, "View children" );
  QToolTip::add( &clientsBtn, "View clients" );
  QToolTip::add( &suppliersBtn, "View suppliers" );
  QToolTip::add( &attributesBtn, "View attributes" );
  QToolTip::add( &methodsBtn, "View methods" );
  QToolTip::add( &virtualsBtn, "View virtual methods" );
}

void CClassToolDlg::setCallbacks()
{
  connect( &parentsBtn, SIGNAL(clicked()), SLOT(slotParents()));
  connect( &childrenBtn, SIGNAL(clicked()), SLOT(slotChildren()));
  connect( &clientsBtn, SIGNAL(clicked()), SLOT(slotClients()));
  connect( &suppliersBtn, SIGNAL(clicked()), SLOT(slotSuppliers()));
  connect( &methodsBtn, SIGNAL(clicked()), SLOT(slotMethods()));
  connect( &attributesBtn, SIGNAL(clicked()), SLOT(slotAttributes()));
  connect( &virtualsBtn, SIGNAL(clicked()), SLOT(slotVirtuals()));
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void CClassToolDlg::setStore( CClassStore *aStore )
{
  assert( aStore != NULL );

  store = aStore;
}

void CClassToolDlg::setClass( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );
  assert( store != NULL );

  currentClass = store->getClassByName( aName );
}

void CClassToolDlg::setClass( CParsedClass *aClass )
{
  assert( aClass != NULL );

  currentClass = aClass;
}

void CClassToolDlg::addClassAndAttributes( CParsedClass *aClass )
{
  CParsedAttribute *aAttr;
  KPath classPath;
  
  // Insert root item(the current class);
  classTree.insertItem( aClass->name, classPm );
  classPath.push( &aClass->name );

  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current();
       ++aClass->attributeIterator )
  {
    aAttr = aClass->attributeIterator.current();
    if( export == 0 || export == aAttr->export )
      classTree.addChildItem( aAttr->name, classPm, &classPath );
  }
}

void CClassToolDlg::addClassAndMethods( CParsedClass *aClass )
{
  CParsedMethod *aMethod;
  QList<CParsedMethod> *list;
  KPath classPath;
  QString str;
  
  // Insert root item(the current class);
  classTree.insertItem( aClass->name, classPm );
  classPath.push( &aClass->name );

  list = aClass->getMethods();
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    if( ( export == 0 || export == aMethod->export ) &&
        onlyVirtual == aMethod->isVirtual )
    {
      aMethod->toString( str );
      classTree.addChildItem( str, classPm, &classPath );
    }
  }
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/** View the parents of the current class. */
void CClassToolDlg::viewParents()
{
  CParsedParent *aParent;
  QString root = currentClass->name;
  KPath classPath;
  
  setCaption( "Parents" );
  classTree.clear();
  
  // Insert root item(the current class);
  classTree.insertItem( root, classPm );
  classPath.push( &root );

  for( aParent = currentClass->parents.first();
       aParent != NULL;
       aParent = currentClass->parents.next() )
  {
    if( export == 0 || aParent->export == export )
      classTree.addChildItem( aParent->name, classPm, &classPath );
  }

  classTree.setExpandLevel( 1 );
}

/** View the children of the current class. */
void CClassToolDlg::viewChildren()
{
  QString root = currentClass->name;
  KPath classPath;
  
  setCaption( "Children" );
  classTree.clear();
  
  // Insert root item(the current class);
  classTree.insertItem( root, classPm );
  classPath.push( &root );

  classTree.setExpandLevel( 1 );
}

/** View methods in this class and parents. */
void CClassToolDlg::viewMethods()
{
  CParsedParent *aParent;
  CParsedClass *aClass;

  setCaption( "Methods" );
  classTree.clear();
  
  // First treat all parents.
  for( aParent = currentClass->parents.first();
       aParent != NULL;
       aParent = currentClass->parents.next() )
  {
    aClass = store->getClassByName( aParent->name );
    if( aClass != NULL )
      addClassAndMethods( aClass );
  }

  // Add the current class
  addClassAndMethods( currentClass );

  classTree.setExpandLevel( 1 );
}

/** View attributes in this class and parents. */
void CClassToolDlg::viewAttributes()
{
  CParsedParent *aParent;
  CParsedClass *aClass;

  setCaption( "Attributes" );
  classTree.clear();
  
  // First treat all parents.
  for( aParent = currentClass->parents.first();
       aParent != NULL;
       aParent = currentClass->parents.next() )
  {
    aClass = store->getClassByName( aParent->name );
    if( aClass != NULL )
      addClassAndAttributes( aClass );
  }

  // Add the current class
  addClassAndAttributes( currentClass );

  classTree.setExpandLevel( 1 );

}

void CClassToolDlg::viewVirtuals()
{
  onlyVirtual = true;
  viewMethods();
  onlyVirtual = false;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CClassToolDlg::slotParents()
{
  viewParents();
}
void CClassToolDlg::slotChildren()
{
  viewChildren();
}
void CClassToolDlg::slotClients()
{
}
void CClassToolDlg::slotSuppliers()
{
}
void CClassToolDlg::slotAttributes()
{
  viewAttributes();
}
void CClassToolDlg::slotMethods()
{
  viewMethods();
}
void CClassToolDlg::slotVirtuals()
{
  viewVirtuals();
}

void CClassToolDlg::OK()
{
  accept();
}

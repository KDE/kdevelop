/***************************************************************************
               cclasstooldlg.cpp  -  implementation
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se

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
#include <qmessagebox.h>
#include <kmsgbox.h>
#include <kapp.h>
#include <klocale.h>
#include <qtooltip.h>
#include <qlistbox.h>
#include <qheader.h>
#include <qregexp.h>
#include <iostream.h>
#include "classparser/ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CClassToolDlg::CClassToolDlg()
 * CClassToolDlg()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassToolDlg::CClassToolDlg( QWidget *parent, const char *name )
  : QDialog( parent, name, false ),
    classTree( this, "classTree" ),
    classLbl( this, "classLbl" ),
    classCombo( false, this, "classCombo" ),
    topLayout( this, 5 ),
    comboLayout( 1, 2 ),
    parentsBtn( this, "parentsBtn" ),
    childrenBtn( this, "childrenBtn" ),
    clientsBtn( this, "clientsBtn" ),
    suppliersBtn( this, "suppliersBtn" ),
    filler( this, "filler" ),
    attributesBtn( this, "attributesBtn" ),
    methodsBtn( this, "methodsBtn" ),
    //    virtualsBtn( this, "virtualsBtn" ),
    exportCombo( false, this, "exportCombo" )
{
  currentOperation = CTNONE;
  comboExport = CTHALL;
  onlyVirtual = false;
  store = NULL;

  setCaption( i18n("Class Tool") );

  setWidgetValues();
  readIcons();
  setCallbacks();
  setTooltips();
}


/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------- CClassToolDlg::setWidgetValues()
 * CClassToolDlg()
 *   Set all initial values of all widgets in the dialog.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setWidgetValues()
{
  classLbl.setFixedHeight( 30 );
  classLbl.setText( i18n("Class:") );

  classCombo.setMinimumWidth( 260 );
  classCombo.setFixedHeight( 30 );
  classCombo.setSizeLimit( 10 );
  
  parentsBtn.setFixedSize( 30, 30 );
  childrenBtn.setFixedSize( 30, 30 );
  clientsBtn.setFixedSize( 30, 30 );
  suppliersBtn.setFixedSize( 30, 30 );

  attributesBtn.setFixedSize( 30, 30 );
  methodsBtn.setFixedSize( 30, 30 );
  //  virtualsBtn.setFixedSize( 30, 30 );

  exportCombo.setMinimumWidth( 100 );
  exportCombo.setFixedHeight( 30 );
  exportCombo.setSizeLimit( 10 );
  exportCombo.insertItem( i18n("All") );
  exportCombo.insertItem(i18n("Public"));
  exportCombo.insertItem(i18n("Protected"));
  exportCombo.insertItem(i18n("Private"));

  classTree.setMinimumSize( 500, 400 );
  classTree.setRootIsDecorated( true );
  classTree.addColumn( "classes" );
  classTree.header()->hide();
  classTree.setSorting(-1,false);
  //  classTree.setFrameStyle( QFrame::WinPanel | QFrame::Sunken );

  // Top layout
  topLayout.addLayout( &comboLayout );
  topLayout.addLayout( &btnLayout );

  // Combo layout
  comboLayout.addWidget( &classLbl, 0, 0, AlignLeft );
  comboLayout.addWidget( &classCombo, 0, 1, AlignLeft );

  // Button layout
  btnLayout.addWidget( &parentsBtn );
  btnLayout.addWidget( &childrenBtn );
  btnLayout.addWidget( &clientsBtn );
  btnLayout.addWidget( &suppliersBtn );
  btnLayout.addWidget( &filler );
  btnLayout.addWidget( &attributesBtn );
  btnLayout.addWidget( &methodsBtn );
  //  btnLayout.addWidget( &virtualsBtn );
  btnLayout.addWidget( &exportCombo );

  topLayout.addWidget( &classTree, 10);
}

/*----------------------------------------- CClassToolDlg::readIcons()
 * readIcons()
 *   Read and set the icons of all the buttons in the dialog.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::readIcons()
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

  //  pm.load( pixDir + "CTvirtuals.xpm" );
  //  virtualsBtn.setPixmap( pm );
}

/*---------------------------------- CClassToolDlg::setTooltips()
 * setTooltips()
 *   Set tooltip strings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setTooltips()
{
  QToolTip::add( &parentsBtn, i18n("Show parents") );
  QToolTip::add( &childrenBtn, i18n("Show children") );
  QToolTip::add( &clientsBtn, i18n("Show clients") );
  QToolTip::add( &suppliersBtn, i18n("Show suppliers") );
  QToolTip::add( &attributesBtn, i18n("Show attributes") );
  QToolTip::add( &methodsBtn, i18n("Show methods") );
  //  QToolTip::add( &virtualsBtn, i18n("Show virtual methods") );
}

/*-------------------------------------- CClassToolDlg::setCallbacks()
 * setCallbacks()
 *   Set all signal<->slot mappings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setCallbacks()
{
  connect( &classCombo, SIGNAL(activated(int)), SLOT(slotClassComboChoice(int)));
  connect( &exportCombo, SIGNAL(activated(int)), SLOT(slotExportComboChoice(int)));
  connect( &parentsBtn, SIGNAL(clicked()), SLOT(slotParents()));
  connect( &childrenBtn, SIGNAL(clicked()), SLOT(slotChildren()));
  connect( &clientsBtn, SIGNAL(clicked()), SLOT(slotClients()));
  connect( &suppliersBtn, SIGNAL(clicked()), SLOT(slotSuppliers()));
  connect( &methodsBtn, SIGNAL(clicked()), SLOT(slotMethods()));
  connect( &attributesBtn, SIGNAL(clicked()), SLOT(slotAttributes()));
  //  connect( &virtualsBtn, SIGNAL(clicked()), SLOT(slotVirtuals()));
  connect( &classTree, 
           SIGNAL( signalViewDeclaration(const char *, const char *, THType,THType ) ),
           SLOT(slotCTViewDecl(const char *, const char *, THType,THType ) ) );
                   
  connect( &classTree, 
           SIGNAL( signalViewDefinition(const char *, const char *, THType,THType ) ),
           SLOT(slotCTViewDef(const char *, const char *, THType,THType ) ) );
}

/*------------------------------------ CClassToolDlg::setActiveClass()
 * setActiveClass()
 *   Make the supplied class the selected one in the classcombo.
 *
 * Parameters:
 *   aName           Class to be selected in the classcombo.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setActiveClass( const char *aName )
{
  QListBox *lb;
  uint i;

  lb = classCombo.listBox();

  for( i=0; i < lb->count() && strcmp( lb->text( i ), aName ) != 0; i++ )
    ;

  classCombo.setCurrentItem( i );
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------- CClassToolDlg::setStore()
 * setStore()
 *   Set the store to use to search for classes.
 *
 * Parameters:
 *   store     Store to search for classes in.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setStore( CClassStore *aStore )
{
  REQUIRE( "Valid store", aStore != NULL );

  QStrList *list;

  store = aStore;

  // Set the store in the treehandler as well.
  ((CClassTreeHandler *)classTree.treeH)->setStore( store );

  // Fetch the list and update the combobox.
  list = store->getSortedClassNameList();
  classCombo.insertStrList( list );
  delete list;
}

/*---------------------------------------- CClassToolDlg::setClass()
 * setClass()
 *   Set the class to view.
 *
 * Parameters:
 *   aName     Name of the class to view.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setClass( const char *aName )
{
  REQUIRE( "Valid name", aName != NULL && strlen( aName ) > 0 );

  setActiveClass( aName );
  currentClass = store->getClassByName( aName );

  // If we can't find the class something is very wring
  if( currentClass == NULL )
  {
    QString warning = "Couldn't find class: ";
    warning += aName;
    QMessageBox::warning( this, "Class error", warning );
  }
}

/*---------------------------------------- CClassToolDlg::setClass()
 * setClass()
 *   Set the class to view.
 *
 * Parameters:
 *   aClass    The class to view.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::setClass( CParsedClass *aClass )
{
  if (aClass == NULL ) return;
  
  setActiveClass( aClass->name );
  currentClass = aClass;
}

/*---------------------------------------- CClassToolDlg::addClasses()
 * addClasses()
 *   Add a list of classes as subitems to the current class.
 *
 * Parameters:
 *   list      List of classes to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolDlg::addClasses( QList<CParsedClass> *list )
{
  CParsedClass *aClass;
  QListViewItem *root;

  // Clear all previous items in the tree.
  classTree.treeH->clear();

  // Insert root item(the current class);
  root = classTree.treeH->addRoot( currentClass->name, THCLASS );

  for( aClass = list->first();
       aClass != NULL;
       aClass = list->next() )
  {
    ((CClassTreeHandler *)classTree.treeH)->addClass( aClass, root );
  }
  classTree.setOpen( root, true );
}

void CClassToolDlg::addClassAndAttributes( CParsedClass *aClass )
{
  REQUIRE( "Valid class", currentClass != NULL );

  QListViewItem *root;
  
  // Insert root item(the current class);
  root = classTree.treeH->addRoot( aClass->name, THCLASS );

  ((CClassTreeHandler *)classTree.treeH)->addAttributesFromClass( aClass, root, comboExport );
  
  classTree.setOpen( root, true );
}

void CClassToolDlg::addClassAndMethods( CParsedClass *aClass )
{
  REQUIRE( "Valid class", currentClass != NULL );

  QListViewItem *root;
  
  // Insert root item(the current class);
  root = classTree.treeH->addRoot( aClass->name, THCLASS );

  ((CClassTreeHandler *)classTree.treeH)->addMethodsFromClass( aClass, root, comboExport );

  classTree.setOpen( root, true );
}

void CClassToolDlg::addAllClassMethods()
{
  REQUIRE( "Valid class", currentClass != NULL );

  CParsedParent *aParent;
  CParsedClass *aClass;

  // Clear all previous items in the tree.
  classTree.treeH->clear();

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
}

void CClassToolDlg::addAllClassAttributes()
{
  REQUIRE( "Valid class", currentClass != NULL );

  CParsedParent *aParent;
  CParsedClass *aClass;

  // Clear all previous items in the tree.
  classTree.treeH->clear();
  
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
}

/** Change the caption depending on the current operation. */
void CClassToolDlg::changeCaption()
{
  QString caption;

  switch( currentOperation )
  {
    case CTPARENT:
      caption = i18n("Parents");
      break;
    case CTCHILD:
      caption = i18n("Children");
      break;
    case CTCLIENT:
      caption = i18n("Clients");
      break;
    case CTSUPP:
      caption = i18n("Suppliers");
      break;
    case CTATTR:
      caption = exportCombo.currentText();
      caption += i18n(" attributes");
      break;
    case CTMETH:
      caption = exportCombo.currentText();
      caption += i18n(" methods");
      break;
    case CTVIRT:
      caption = exportCombo.currentText();
      caption += i18n(" virtual methods");
      break;
    default:
      caption = i18n("Class Tool Dialog");
      break;
  }

  caption += QString(" ") + i18n(" of class ") + " ";
  if( currentClass != NULL )
    caption += currentClass->name;

  setCaption( caption );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/** View the parents of the current class. */
void CClassToolDlg::viewParents()
{
  REQUIRE( "Valid class", currentClass != NULL );

  CParsedParent *aParent;
  QListViewItem *root;
  
  currentOperation = CTPARENT;

  changeCaption();

  classTree.treeH->clear();
  
  // Insert root item(the current class);
  root = classTree.treeH->addRoot( currentClass->name, THCLASS );

  for( aParent = currentClass->parents.first();
       aParent != NULL;
       aParent = currentClass->parents.next() )
  {
    ((CClassTreeHandler *)classTree.treeH)->addClass( aParent->name, root );
  }

  classTree.setOpen( root, true );
}

/** View the children of the current class. */
void CClassToolDlg::viewChildren()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  QList<CParsedClass> *list;
  
  currentOperation = CTCHILD;
  changeCaption();

  list = store->getClassesByParent( currentClass->name );
  addClasses( list );
  delete list;
}

/** View all classes that has this class as an attribute. */
void CClassToolDlg::viewClients()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  QList<CParsedClass> *list;
  
  currentOperation = CTCLIENT;
  changeCaption();

  list = store->getClassClients( currentClass->name );
  addClasses( list );
  delete list;
}

/** View all classes that this class has as attributes. */
void CClassToolDlg::viewSuppliers()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  QList<CParsedClass> *list;

  currentOperation = CTSUPP;
  changeCaption();

  list = store->getClassSuppliers( currentClass->name );
  addClasses( list );
  delete list;
}

/** View methods in this class and parents. */
void CClassToolDlg::viewMethods()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  currentOperation = CTMETH;

  changeCaption();  
  addAllClassMethods();
}

/** View attributes in this class and parents. */
void CClassToolDlg::viewAttributes()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  currentOperation = CTATTR;

  changeCaption();
  addAllClassAttributes();
}

void CClassToolDlg::viewVirtuals()
{
  REQUIRE( "Valid current class", currentClass != NULL );

  currentOperation = CTVIRT;

  changeCaption();

  onlyVirtual = true;
  addAllClassMethods();
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
  viewClients();
}

void CClassToolDlg::slotSuppliers()
{
  viewSuppliers();
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

void CClassToolDlg::slotExportComboChoice(int idx)
{
  QString str;

  str = exportCombo.currentText();

  // Check exporttype
  if( str == i18n("All") )
    comboExport = CTHALL;
  else if( str == "Public" )
    comboExport = CTHPUBLIC;
  else if( str == "Protected" )
    comboExport = CTHPROTECTED;
  else if( str == "Private" )
    comboExport = CTHPRIVATE;

  // Update the view if the choice affected the data.
  switch( currentOperation )
  {
    case CTATTR:
      viewAttributes();
      break;
    case CTMETH:
      viewMethods();
      break;
    case CTVIRT:
      viewVirtuals();
      break;
    default:
      break;
  }
}

void CClassToolDlg::slotClassComboChoice(int idx)
{
  QString className=classCombo.currentText();
  // className.replace(QRegExp("::"), ".");
  setClass( className );

  // Update the view if the choice affected the data.
  switch( currentOperation )
  {
    case CTPARENT:
      viewParents();
      break;
    case CTCHILD:
      viewChildren();
      break;
    case CTCLIENT:
      viewClients();
      break;
    case CTSUPP:
      viewSuppliers();
      break;
    case CTATTR:
      viewAttributes();
      break;
    case CTMETH:
      viewMethods();
      break;
    case CTVIRT:
      viewVirtuals();
      break;
    default:
      break;
  }
}

/** Slot from the classtree telling that the user wants to view a
 * declation. */
void CClassToolDlg::slotCTViewDecl( const char *className, const char *declName, THType type,THType type1)
{
  emit signalViewDeclaration( className, declName, type, type1 );
}

/** Slot from the classtree telling that the user wants to view a
 * defintion. */
void CClassToolDlg::slotCTViewDef( const char *className, const char *declName, THType type,THType type1 )
{
  emit signalViewDefinition( className, declName, type,type1 );
}

void CClassToolDlg::OK()
{
  accept();
}

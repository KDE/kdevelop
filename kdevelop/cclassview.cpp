/***************************************************************************
                          cclassview.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
    based on             : cclassview.cpp by Sandy Meier
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cclassview.h"
#include <assert.h>
#include <kmsgbox.h>
#include <qheader.h>
#include <qprogressdialog.h> 
#include <qmessagebox.h>

#include "cclasstooldlg.h"
#include "ccvaddfolderdlg.h"
#include "./gfxview/GfxClassTreeWindow.h"
#include "resource.h"
#include "./classwizard/cclasswizarddlg.h"

// Initialize static members
QString CClassView::CLASSROOTNAME = "Classes";
QString CClassView::GLOBALROOTNAME = "Globals";

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CClassView::CClassView()
 * CClassView()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::CClassView(QWidget* parent /* = 0 */,const char* name /* = 0 */)
  : CTreeView (parent, name)
{
  // Create the popupmenus.
  initPopups();

  // Set the store.
  store = &cp.store;

  setTreeHandler( new CClassTreeHandler() );
  ((CClassTreeHandler *)treeH)->setStore( store );

  definitionCmd.setClassView( this );
  declarationCmd.setClassView( this );
  
  connect(this, SIGNAL(selectionChanged()), SLOT(slotClassViewSelected()));
}

/*------------------------------------------ CClassView::~CClassView()
 * ~CClassView()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::~CClassView()
{
}

/*------------------------------------------ CClassView::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::initPopups()
{
  int id;
  // Project popup
  projectPopup.setTitle(i18n ("Project"));
  projectPopup.insertItem(i18n("New file..."), this, SLOT(slotFileNew()),0, ID_FILE_NEW);
  projectPopup.insertItem(i18n("New class..."), this, SLOT(slotClassNew()), 0, ID_PROJECT_NEW_CLASS);
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Add Folder..."), this, SLOT( slotFolderNew()),0, ID_CV_FOLDER_NEW);
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Options..."), this, SLOT(slotProjectOptions()),0, ID_PROJECT_OPTIONS);
  projectPopup.insertItem(i18n("Graphical classview.."), this, SLOT(slotGraphicalView()), 0, ID_CV_GRAPHICAL_VIEW);



  // Class popup
  classPopup.setTitle( i18n("Class"));
  classPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()),ID_CV_VIEW_DEFINITION);
  classPopup.insertItem( i18n("Add member function..."), this, SLOT(slotMethodNew()), ID_CV_METHOD_NEW);
  classPopup.insertItem( i18n("Add member variable..."), this, SLOT(slotAttributeNew()),0, ID_CV_ATTRIBUTE_NEW);
  id = classPopup.insertItem( i18n("Implement virtual function..."), this, SLOT(slotImplementVirtual()),0, ID_CV_IMPLEMENT_VIRTUAL);
  classPopup.setItemEnabled( id, false );

  classPopup.insertSeparator();
  classPopup.insertItem( i18n("Parent classes..."), this, SLOT(slotClassBaseClasses()),0, ID_CV_CLASS_BASE_CLASSES);
  classPopup.insertItem( i18n("Child classes..."), this, SLOT(slotClassDerivedClasses()),0, ID_CV_CLASS_DERIVED_CLASSES);
  classPopup.insertItem( i18n("Classtool..."), this, SLOT(slotClassTool()),0, ID_CV_CLASS_TOOL);
  classPopup.insertSeparator();
  id = classPopup.insertItem( i18n( "Add slot for signal" ), this, SLOT(slotAddSlotSignal()),0, ID_CV_ADD_SLOT_SIGNAL);
  classPopup.setItemEnabled( id, false );
  id = classPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete class"), this, SLOT(slotClassDelete()), ID_CV_CLASS_DELETE);
  classPopup.setItemEnabled(id, false );
  classPopup.insertSeparator();
  classPopup.insertItem( i18n( "ClassWizard" ), this, SLOT( slotClassWizard()),0, ID_CV_CLASSWIZARD );

  // Method popup
  methodPopup.setTitle( i18n( "Method" ) );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDeclaration()), 0, ID_CV_VIEW_DECLARATION);
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDefinition() ),0,ID_CV_VIEW_DEFINITION);
	methodPopup.insertSeparator();
  id = methodPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete method" ), this, SLOT(slotMethodDelete()),0, ID_CV_METHOD_DELETE);
  methodPopup.setItemEnabled( id, false );

  // Attribute popup
  attributePopup.setTitle( i18n( "Attribute" ) );
  attributePopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDefinition()),0, ID_CV_VIEW_DEFINITION);
  attributePopup.insertSeparator();
  id = attributePopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete attribute" ), this, SLOT(slotAttributeDelete()),0, ID_CV_ATTRIBUTE_DELETE);
  attributePopup.setItemEnabled( id, false );

  // Slot popup
  slotPopup.setTitle( i18n( "Slot" ) );
  slotPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDeclaration()),0, ID_CV_VIEW_DECLARATION);
  slotPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDefinition()),0, ID_CV_VIEW_DEFINITION);
  slotPopup.insertSeparator();
  id = slotPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete slot" ), this, SLOT(slotMethodDelete()),0,ID_CV_METHOD_DELETE);
  slotPopup.setItemEnabled( id, false );

  // Folder popup
  folderPopup.setTitle( i18n( "Folder" ) );
  folderPopup.insertItem( i18n("Add Folder..."), this, SLOT( slotFolderNew()),0, ID_CV_FOLDER_NEW);
  id = folderPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete Folder..."), this, SLOT( slotFolderDelete()),0, ID_CV_FOLDER_DELETE);

  connect(&attributePopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&classPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&folderPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&methodPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&projectPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&slotPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Add all classes from the project. Reparse and redraw all classes 
 *   in the view.
 *
 * Parameters:
 *   proj          The project specification.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh( CProject *proj )
{
  QProgressDialog progressDlg(NULL, "progressDlg", true );
  QStrList src;
  QStrList header;
  char *str;
  int totalCount=0;
  int currentCount = 0;
  bool popupClassItemsEnable;

  debug( "CClassView::refresh( proj )" );

  project = proj;

  // Reset the classparser and the view.
  ((CClassTreeHandler *)treeH)->clear();
  cp.wipeout();

  // Get all header and src filenames.
  header = proj->getHeaders();
  src = proj->getSources();

  // Initialize progressbar.
  totalCount = header.count() + src.count();
  emit resetStatusbarProgress();
  emit setStatusbarProgress( 0 );
  emit setStatusbarProgressSteps( totalCount );

  // Parse headerfiles.
  for( str = header.first(); str != NULL; str = header.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    emit setStatusbarProgress( ++currentCount );
  }
	
  // Parse sourcefiles.
  for( str = src.first(); str != NULL; str = src.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    emit setStatusbarProgress( ++currentCount );
  }

  refresh();

  // disable certain popup items if it is a C Project
  popupClassItemsEnable=proj->getProjectType()!="normal_c";
  projectPopup.setItemEnabled(ID_PROJECT_NEW_CLASS, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_FOLDER_NEW, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_GRAPHICAL_VIEW, popupClassItemsEnable);

}

/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Reparse and redraw all classes in the view.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh()
{
  QString str;
  QListViewItem *item;
  QString treeStr;
  QList<CParsedMethod> *methodList;
  QList<CParsedAttribute> *attributeList;
  QList<CParsedStruct> *structList;

  debug( "CClassView::refresh()" );

  // Try to fetch a stored classview tree.
  treeStr = project->getClassViewTree();

  // If there doesn't exists a stored tree we build one.
  if( treeStr.isEmpty() )
    buildInitalClassTree();
  else // Build the tree using the stored string.
    buildTree( treeStr );

  // Add the globals folder.
  str = i18n( GLOBALROOTNAME );
  globalsItem = treeH->addRoot( str, THFOLDER );

  // Add all global items.
  item = treeH->addItem( i18n( "Structures" ), THFOLDER, globalsItem );
  structList = store->globalContainer.getSortedStructList();
  ((CClassTreeHandler *)treeH)->addGlobalStructs( structList, item );
  delete structList;

  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Functions" ), THFOLDER, globalsItem );
  methodList = store->globalContainer.getSortedMethodList();
  ((CClassTreeHandler *)treeH)->addGlobalFunctions( methodList, item );
  delete methodList;

  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Variables" ), THFOLDER, globalsItem );
  attributeList = store->globalContainer.getSortedAttributeList();
  ((CClassTreeHandler *)treeH)->addGlobalVariables( attributeList, item );
  delete attributeList;

  treeH->setLastItem( item );

  // Open the classes and globals folder.
  setOpen( classesItem, true );
  setOpen( globalsItem, true );
}

/*---------------------------------------------- CClassView::addFile()
 * addFile()
 *   Add a source file, parse it and rebuild the tree.
 *
 * Parameters:
 *   aName          The absolute filename.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addFile( const char *aName )
{
  // Reset the tree.
  ((CClassTreeHandler *)treeH)->clear();

  debug( "Adding file %s", aName );

  // Parse the file.
  cp.parse( aName );

  // Build the new classtree.
  refresh();
}

/*---------------------------------- CClassView::refreshClassByName()
 * refreshClassByName()
 *   Reparse and redraw a class by using its' name.
 *
 * Parameters:
 *   aName          The classname
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refreshClassByName( const char *aName )
{
  QListViewItem *classItem;
  CParsedClass *aClass;

  classItem = firstChild();
  while( classItem != NULL && strcmp( classItem->text(0), aName ) != 0 )
    classItem = classItem->nextSibling();

  // If the item was found we reparse and update.
  if( classItem )
  {
    // cp->reparseClass( aName );
    aClass = store->getClassByName( aName );
    ((CClassTreeHandler *)treeH)->updateClass( aClass , classItem );
  }
}

/*------------------------------------- CClassView::viewGraphicalTree()
 * viewGraphicalTree()
 *   View graphical classtree.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::viewGraphicalTree()
{
  QList<CClassTreeNode> *forest = store->asForest();
  CGfxClassTreeWindow *cb = new CGfxClassTreeWindow(NULL);
  cb->setCaption(i18n("Graphical classview"));
  cb->InitializeTree(forest);
  cb->show();
}

/*------------------------------------- CClassView::viewDefinition()
 * viewDefinition()
 *   Views a definition of an item.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::viewDefinition( const char *className, 
                                 const char *declName, 
                                 THType type )
{
  if( className == NULL && declName == NULL )
    QMessageBox::warning( this, i18n( "Not found" ), i18n( "This item could not be viewed. The item isn't parsed." ) );
  else
    emit selectedViewDefinition( className, declName, type );
}

/*------------------------------------- CClassView::viewDefinition()
 * viewDefinition()
 *   Views a declaration of an item.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::viewDeclaration( const char *className, 
                                  const char *declName, 
                                  THType type )
{
  if( className == NULL && declName == NULL )
    QMessageBox::warning( this, i18n( "Not found" ), i18n( "This item could not be viewed. The item isn't parsed." ) );
  else
    emit selectedViewDeclaration( className, declName, type );
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*********************************************************************
 *                                                                   *
 *                        PROTECTED QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassView::getCurrentClass()
 * getCurrentClass()
 *   Fetches the class currently selected in the tree.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass *CClassView::getCurrentClass()
{
  return store->getClassByName( currentItem()->text(0) );
}


/*--------------------------------- CClassView::getCurrentPopup()
 * getCurrentPopup()
 *   Get the current popupmenu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
KPopupMenu *CClassView::getCurrentPopup()
{
  KPopupMenu *popup = NULL;

  switch( treeH->itemType() )
  {
    case THFOLDER:
      if( strcmp( currentItem()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
        popup = &projectPopup;
      else
        popup = &folderPopup;
      break;
    case THCLASS:
      popup = &classPopup;
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
    case THGLOBAL_FUNCTION:
      popup = &methodPopup;
      break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
    case THGLOBAL_VARIABLE:
      popup = &attributePopup;
      break;
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      popup = &slotPopup;
      break;
    default:
      break;
  }

  return popup;
}

int CClassView::getTreeStrItem( const char *str, int pos, char *buf )
{
  int idx = 0;

  // Skip first '.
  pos++;
  while( str[pos] != '\'' )
  {
    buf[idx]=str[pos];
    idx++;
    pos++;
  }

  // Add a null termination.
  buf[ idx ] = '\0';

  // Skip to next ' or to ')'.
  pos++;
  
  return pos;
}

void CClassView::buildTree( const char *str )
{
  uint pos=0;
  QListViewItem *root=NULL;
  QListViewItem *parent=NULL;
  QListViewItem *ci;
  CParsedClass *aPC;
  char buf[50];

  debug( "CClassView::buildtree( treeStr )" );

  while( pos < strlen( str ) )
  {
    if( str[ pos ] == '(' )
    {
      pos++;
      pos = getTreeStrItem( str, pos, buf );

      if( parent == NULL )
      {
        parent = ((CClassTreeHandler *)treeH)->addRoot( buf, THFOLDER );
        root = parent;
      }
      else
        parent = ((CClassTreeHandler *)treeH)->addItem( buf, THFOLDER, parent );
    }

    while( str[ pos ] == '\'' )
    {
      pos = getTreeStrItem( str, pos, buf );
      aPC = store->getClassByName( buf );
      if( aPC )
      {
        ci = ((CClassTreeHandler *)treeH)->addClass( aPC, parent );
        ((CClassTreeHandler *)treeH)->updateClass( aPC, ci );
        treeH->setLastItem( ci );
      }
      else
        ((CClassTreeHandler *)treeH)->addItem( buf, THCLASS, parent );
    }

    if( str[ pos ] == ')' )
    {
      pos++;
      parent = parent->parent();

      if( parent != NULL && parent != root )
        treeH->setLastItem( parent );
    }
  }
}

/*----------------------------------------- CClassView::buildTreeStr()
 * buildTreeStr()
 *   Make a tree(as a string).
 *
 * Parameters:
 *   item           The root item.
 *   str            The string to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildTreeStr( QListViewItem *item, QString &str )
{
  THType type;

  if( item != NULL )
  {
    while( item != NULL )
    {
      type = treeH->itemType( item );
      if( item != NULL && type == THFOLDER )
      {
        str += "('";
        str += item->text( 0 );
        str += "'";
        
        buildTreeStr( item->firstChild(), str );
        str += ")";
      }
      else if( type == THCLASS )
      {
        str += "'";
        str += item->text( 0 );
        str += "'";
      }

      // Ignore globals folder.
      if( item->parent() == NULL )
        item = NULL;
      else
        item = item->nextSibling();
    }
  }
}

/*-------------------------------------------- CClassView::asTreeStr()
 * asTreeStr()
 *   Return this view as a treestring.
 *
 * Parameters:
 *   -
 * Returns:
 *   QString        The tree as a string.
 *-----------------------------------------------------------------*/
const char *CClassView::asTreeStr()
{
  QString str="";

  buildTreeStr( classesItem, str );

  return str;
}

void CClassView::buildInitalClassTree()
{
  QString str;
  CParsedClass *aPC;
  QListViewItem *folder;
  QList<CParsedClass> *list;
  QList<CParsedClass> *iterlist;
  QString projDir;
  QDict< QList<CParsedClass> > dict;
  QDictIterator< QList<CParsedClass> > dictI( dict );
  QList<CParsedClass> rootList;

  debug( "buildInitalClassTree" );

  dict.setAutoDelete( true );

  // Insert root item
  str = i18n( CLASSROOTNAME );
  classesItem = treeH->addRoot( str, THFOLDER );

  list = store->getSortedClasslist();
  projDir = project->getProjectDir();

  // Add all parsed classes to the correct list;
  for( aPC = list->first();
       aPC != NULL;
       aPC = list->next() )
  {
    // Try to determine if this is a subdirectory.
    str = aPC->definedInFile;
    str = str.remove( 0, projDir.length() );
    str = str.remove( 0, str.find( '/', 1 ) );
    str = str.remove( str.findRev( '/' ), 10000 );

    if( str.isEmpty() )
      rootList.append( aPC );
    else
    {
      // Remove heading /
      str = str.remove( 0, 1 );

      iterlist = dict.find( str );

      if( iterlist == NULL )
      {
        iterlist = new QList<CParsedClass>();
        dict.insert( str, iterlist );
      }

      iterlist->append( aPC );
    }
  }

  delete list;

  // Add all classes with a folder.
  for( dictI.toFirst();
       dictI.current();
       ++dictI )
  {
    // Add folder.
    folder = treeH->addItem( dictI.currentKey(), THFOLDER, classesItem );
    ((CClassTreeHandler *)treeH)->addClasses( dictI.current(), folder );
    treeH->setLastItem( folder );
  }

  // Add all classes without a folder.
  ((CClassTreeHandler *)treeH)->addClasses( &rootList, classesItem );

  // Save the tree. FOR SOME REASON ONLY 1974 CHARACTERS ARE SAVED. :-(
  //  str = asTreeStr();
  //  project->setClassViewTree( str );
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CClassView::slotProjectOptions()
{
  emit selectedProjectOptions();
}

void CClassView::slotGraphicalView()
{
  viewGraphicalTree();
}

void CClassView::slotFileNew()
{
  emit selectedFileNew();
}

void CClassView::slotClassNew()
{
  emit selectedClassNew();
}

void CClassView::slotClassDelete()
{
  if( KMsgBox::yesNo( this, i18n("Delete class"),
                      i18n("Are you sure you want to delete this class?"),
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
                      
}

void CClassView::slotClassViewSelected()
{
  THType type;

  type = treeH->itemType();

  // Only react on clicks on the left mousebutton.
  if( mouseBtn == LeftButton && type != THFOLDER )
  {
    if( type == THCLASS || type == THSTRUCT ||
        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR || 
        type == THPRIVATE_ATTR || type == THPUBLIC_SIGNAL ||
        type == THPROTECTED_SIGNAL || type == THPRIVATE_SIGNAL )
      slotViewDefinition();
    else
      slotViewDeclaration();
  }
  else if( mouseBtn == MidButton && type != THFOLDER )
  {
    if( type == THCLASS || type == THSTRUCT ||
        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR || 
        type == THPRIVATE_ATTR  || type == THPUBLIC_SIGNAL ||
        type == THPROTECTED_SIGNAL || type == THPRIVATE_SIGNAL)
      slotViewDeclaration();
    else
      slotViewDefinition();
  }

  // Set it back, so next time only if user clicks again we react.
  mouseBtn = RightButton; 
}

void CClassView::slotMethodNew()
{
  emit signalAddMethod( currentItem()->text( 0 ) );
}

void CClassView::slotMethodDelete()
{
  if( KMsgBox::yesNo( this, i18n("Delete method"),
                      i18n("Are you sure you want to delete this method?"),
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
}

void CClassView::slotAttributeNew()
{
  emit signalAddAttribute( currentItem()->text( 0 ) );
}

void CClassView::slotAttributeDelete()
{
  if( KMsgBox::yesNo( this, i18n("Delete attribute"), 
                      i18n("Are you sure you want to delete this attribute?"),
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
}

void CClassView::slotAddSlotSignal()
{
}

void CClassView::slotImplementVirtual()
{
}

void CClassView::slotFolderNew() 
{
  CCVAddFolderDlg dlg;
  QListViewItem *item;

  if( dlg.exec() )
  {
    item = ((CClassTreeHandler *)treeH)->addItem( dlg.folderEdit.text(),
                                                  THFOLDER,
                                                  currentItem() );
    setOpen( item, true );
  }
}

void CClassView::slotFolderDelete() 
{
  QListViewItem *parent;

  if( KMsgBox::yesNo( this, i18n("Delete folder"), 
                      i18n("Are you sure you want to delete this folder?"),
                      KMsgBox::QUESTION ) == 1 )
  {
    parent = currentItem()->parent();
    parent->removeItem( currentItem() );
  }
}

void CClassView::slotClassBaseClasses()
{
  CClassToolDlg ctDlg( this, "classToolDlg" );

  ctDlg.setStore( store );
  ctDlg.setViewDefinitionCmd( &definitionCmd );
  ctDlg.setViewDeclarationCmd( &declarationCmd );  
  ctDlg.setClass( getCurrentClass() );
  ctDlg.viewParents();
  ctDlg.show();
}

void CClassView::slotClassDerivedClasses() 
{
  CClassToolDlg ctDlg( this, "classToolDlg" );

  ctDlg.setStore( store );
  ctDlg.setClass( getCurrentClass() );
  ctDlg.setViewDefinitionCmd( &definitionCmd );
  ctDlg.setViewDeclarationCmd( &declarationCmd );  
  ctDlg.viewChildren();
  ctDlg.show();
}

void CClassView::slotClassTool()
{
  CClassToolDlg ctDlg( this, "classToolDlg" );

  ctDlg.setStore( store );
  ctDlg.setClass( getCurrentClass() );
  ctDlg.setViewDefinitionCmd( &definitionCmd );
  ctDlg.setViewDeclarationCmd( &declarationCmd );  
  ctDlg.show();
}

void CClassView::slotViewDefinition() 
{
  const char *className;
  const char *otherName;
  THType idxType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( &className, &otherName, &idxType );

  viewDefinition( className, otherName, idxType );
}

void CClassView::slotViewDeclaration()
{
  const char *className;
  const char *otherName;
  THType idxType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( &className, &otherName, &idxType );

  viewDeclaration( className, otherName, idxType );
}

void CClassView::slotClassWizard()
{
  CClassWizardDlg dlg;

  dlg.setStore( store );
  dlg.exec();
}

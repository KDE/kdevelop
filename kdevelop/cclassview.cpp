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

#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
#include "cclasstooldlg.h"
#include "ccvaddfolderdlg.h"
//#include "ccvadminfolderdlg.h"


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
  projectPopup.insertItem(i18n("New file..."), this, SLOT(slotFileNew()));
  projectPopup.insertItem(i18n("New class..."), this, SLOT(slotClassNew()));
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Add Folder..."), this, SLOT( slotFolderNew()));
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Options..."), this, SLOT(slotProjectOptions()));

  // Class popup
  classPopup.setTitle( i18n("Class"));
  classPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()));
  classPopup.insertItem( i18n("Add member function..."), this, SLOT(slotMethodNew()));
  classPopup.insertItem( i18n("Add member variable..."), this, SLOT(slotAttributeNew()));
  classPopup.insertSeparator();
  classPopup.insertItem( i18n("Parent classes..."), this, SLOT(slotClassBaseClasses()));
  classPopup.insertItem( i18n("Child classes..."), this, SLOT(slotClassDerivedClasses()));
  classPopup.insertItem( i18n("Classtool..."), this, SLOT(slotClassTool()));
  classPopup.insertSeparator();
  id = classPopup.insertItem( i18n( "Add slot for signal" ), this, SLOT(slotAddSlotSignal()));
  classPopup.setItemEnabled( id, false );
  id = classPopup.insertItem( i18n("Delete class"), this, SLOT(slotClassDelete()));
  classPopup.setItemEnabled(id, false );

  // Method popup
  methodPopup.setTitle( i18n( "Method" ) );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()));
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()));
  methodPopup.insertSeparator();
  id = methodPopup.insertItem( i18n( "Delete method" ), this, SLOT(slotMethodDelete()));
  methodPopup.setItemEnabled( id, false );

  // Attribute popup
  attributePopup.setTitle( i18n( "Attribute" ) );
  attributePopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()));
  attributePopup.insertSeparator();
  id = attributePopup.insertItem( i18n( "Delete attribute" ), this, SLOT(slotAttributeDelete()));
  attributePopup.setItemEnabled( id, false );

  // Slot popup
  slotPopup.setTitle( i18n( "Slot" ) );
  slotPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()));
  slotPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()));
  slotPopup.insertSeparator();
  id = slotPopup.insertItem( i18n( "Delete slot" ), this, SLOT(slotMethodDelete()));
  slotPopup.setItemEnabled( id, false );

  // Folder popup
  folderPopup.setTitle( i18n( "Folder" ) );
  folderPopup.insertItem( i18n("Add Folder..."), this, SLOT( slotFolderNew()));
  folderPopup.insertItem( i18n("Delete Folder..."), this, SLOT( slotFolderDelete()));
  folderPopup.insertSeparator();
  folderPopup.insertItem( i18n( "Move items to/from folder..." ), this, SLOT(slotMoveToFolder()));

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
  int i;

  debug( "CClassView::refresh( proj )" );

  project = proj;

  // Reset the classparser and the view.
  ((CClassTreeHandler *)treeH)->clear();
  cp.wipeout();

  // Parse headerfiles.
  header = proj->getHeaders();
  progressDlg.setLabelText( i18n("Parsing headers...") );
  progressDlg.setTotalSteps( header.count() );
  progressDlg.setProgress( 0 );
  progressDlg.show();
  i=0;
  for( str = header.first(); str != NULL; str = header.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    i++;
    progressDlg.setProgress( i );
  }
  
  // Parse sourcefiles.
  src = proj->getSources();
  progressDlg.setLabelText( i18n("Parsing sources...") );
  progressDlg.setTotalSteps( src.count() );
  progressDlg.setProgress( 0 );
  progressDlg.show();
  i=0;
  for( str = src.first(); str != NULL; str = src.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    i++;
    progressDlg.setProgress( i );
  }

  refresh();
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
  ((CClassTreeHandler *)treeH)->addGlobalStructs( store->getSortedGlobalStructList(), item );
  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Functions" ), THFOLDER, globalsItem );
  ((CClassTreeHandler *)treeH)->addGlobalFunctions( store->getGlobalFunctions(), item );
  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Variables" ), THFOLDER, globalsItem );
  ((CClassTreeHandler *)treeH)->addGlobalVariables( store->getSortedGlobalVarList(), item );
  treeH->setLastItem( item );

  // Open the classes and globals folder.
  setOpen( classesItem, true );
  setOpen( globalsItem, true );
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
    str = aPC->hFilename;
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
  if( KMsgBox::yesNo( this, "Delete class", 
                      "Are you sure you want to delete this class?",
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
                      
}

void CClassView::slotMethodNew()
{
  CParsedMethod *aMethod;
  CAddClassMethodDlg dlg(this, "methodDlg" );
  QString str;
  
  if( dlg.exec() )
  {
    aMethod = dlg.asSystemObj();
    aMethod->setDeclaredInClass( currentItem()->text( 0 ) );

    emit signalAddMethod( aMethod );
  }
}

void CClassView::slotMethodDelete()
{
  if( KMsgBox::yesNo( this, "Delete method", 
                      "Are you sure you want to delete this method?",
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
}

void CClassView::slotAttributeNew()
{
  CAddClassAttributeDlg dlg(this, "attrDlg" );
  CParsedAttribute *aAttr;

  if( dlg.exec() )
  {
    aAttr = dlg.asSystemObj();

    emit signalAddAttribute( aAttr );
  }
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
  CClassToolDlg dlg(this, "classToolDlg" );

  dlg.setStore( store );
  dlg.setClass( getCurrentClass() );
  dlg.viewParents();
  dlg.show();
}

void CClassView::slotClassDerivedClasses() 
{
  CClassToolDlg dlg(this, "classToolDlg" );

  dlg.setStore( store );
  dlg.setClass( getCurrentClass() );
  dlg.viewChildren();
  dlg.show();
}

void CClassView::slotClassTool()
{
  CClassToolDlg dlg(this, "classToolDlg" );

  dlg.setStore( store );
  dlg.setClass( getCurrentClass() );
  dlg.show();
}

void CClassView::slotViewDefinition() 
{
  emit selectedViewDefinition();
}

void CClassView::slotViewDeclaration()
{
  emit selectedViewDeclaration();
}

void CClassView::slotMoveToFolder()
{
//   CParsedClass *aClass;
//   QList<CParsedClass> *list;
//   CCVAdminFolderDlg dlg;

//   list = store->getSortedClasslist();

//   for( aClass = list->first();
//        aClass != NULL;
//        aClass = list->next() )
//   {
//     dlg.outsideLb.inSort( aClass->name );
//   }

//   if( dlg.exec() )
//   {
//     debug( "Moving items." );
//   }
}

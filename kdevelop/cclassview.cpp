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
  id = projectPopup.insertItem(i18n("New Folder..."), this, SLOT( slotFolderNew()));
  projectPopup.setItemEnabled(id, false );
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
  id = classPopup.insertItem( i18n("Delete class"), this, SLOT(slotClassDelete()));
  classPopup.setItemEnabled(id, false );
  id = classPopup.insertItem(i18n("New Folder..."), this, SLOT( slotFolderNew()));
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
  QStrList list;
  QStrList src;
  QStrList header;
  char *str;
  QString projDir;
  int i;

  debug( "CClassView::refresh( proj )" );

  // Reset the classparser and the view.
  ((CClassTreeHandler *)treeH)->clear();
  cp.wipeout();

  projDir = proj->getProjectDir();
  
  // Get the lists containing the files for the project.
  proj->getAllFiles( list );
  src = proj->getSources();
  header = proj->getHeaders();

  // Parse headerfiles.
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
  QList<CParsedClass> *list;
  CParsedClass *aPC;
  QString str;
  QListViewItem *ci;
  QListViewItem *classes; 
  QListViewItem *globals;

  // Insert root item
  str = i18n( CLASSROOTNAME );
  classes = treeH->addRoot( str, THFOLDER );

  list = store->getSortedClasslist();

  // Add all parsed classes to the view
  for( aPC = list->first();
       aPC != NULL;
       aPC = list->next() )
  {
    ci = ((CClassTreeHandler *)treeH)->addClass( aPC, classes );

    ((CClassTreeHandler *)treeH)->updateClass( aPC, ci );
    treeH->setLastItem( ci );
  }

  // Add the globals folder.
  str = i18n( GLOBALROOTNAME );
  globals = treeH->addRoot( str, THFOLDER );

  // Add all global functions and variables
  ((CClassTreeHandler *)treeH)->addGlobalFunctions( store->getGlobalFunctions(), globals );
  ((CClassTreeHandler *)treeH)->addGlobalVariables( store->getSortedGlobalVarList(), globals );

  // Open the classes and globals folder.
  setOpen( classes, true );
  setOpen( globals, true );
}

/*---------------------------------- CClassView::refreshClassByName()
 * refreshClassByName()
 *   Reparse and redraw a classes by using its' name.
 *
 * Parameters:
 *   aName          The classname
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refreshClassByName( const char *aName )
{
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
    default:
      break;
  }

  return popup;
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
  if( KMsgBox::yesNo( this, "Delete attribute", 
                      "Are you sure you want to delete this attribute?",
                      KMsgBox::QUESTION ) == 1 )
  {
    KMsgBox::message( this, "Not implemented",
                      "This function isn't implemented yet." );
  }
}

void CClassView::slotFolderNew() 
{
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

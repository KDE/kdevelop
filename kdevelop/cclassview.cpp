/***************************************************************************
                          cclassview.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
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

#include "cclasstooldlg.h"
#include "ccvaddfolderdlg.h"
#include "cproject.h"

#include "./gfxview/GfxClassTreeWindow.h"
#include "./classparser/ClassParser.h"
#include "./classwizard/cclasswizarddlg.h"
#include "wzconnectdlgimpl.h"

#include "resource.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include <qheader.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qstrlist.h>
#include <qsortedlist.h>

#include <time.h>
#include <assert.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CClassView::CCVToolTip::CCVToolTip()
 * CCVToolTip()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::CCVToolTip::CCVToolTip( QWidget *parent )
  : QToolTip( parent )
{
}

/*********************************************************************
 *                                                                   *
 *                        PROTECTED METHODS                          *
 *                                                                   *
 ********************************************************************/

/*-------------------------------- CClassView::CCVToolTip::maybeTip()
 * maybeTip()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::CCVToolTip::maybeTip( const QPoint &p )
{
  CClassView *cv;
  QString str;
  QRect r;

  cv = (CClassView *)parentWidget();
  cv->tip( p, r, str );

  if( !str.isEmpty() && r.isValid() )
    tip( r, str );
}

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
CClassView::CClassView(QWidget* parent, const char* name) :
  CTreeView (parent, name),
  cp(new CClassParser)
{
  CLASSROOTNAME = "Classes";
  GLOBALROOTNAME = "Globals";


  project = NULL;		//by default initialize it to null;

  // Create the popupmenus.
  initPopups();

  // Set the store.
  store = &cp->store;

  // Create the tooltip;
  toolTip = new CCVToolTip( this );

  setTreeHandler( new CClassTreeHandler() );
  ((CClassTreeHandler *)treeH)->setStore( store );

  connect (this, SIGNAL(executed(QListViewItem *, const QPoint &, int )),
           SLOT(slotClassViewSelected()));
  connect (this, SIGNAL(returnPressed(QListViewItem *)),
           SLOT(slotClassViewSelected()));
  // the signal used to be reemitted from CTreeView, it doesnt make sense
  // any longer and it's therefore replaced (rokrau 6/18/01)
  //connect(this, SIGNAL(selectionChanged()), SLOT(slotClassViewSelected()));
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
  delete toolTip;
  delete cp;
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
  projectPopup.insertTitle(i18n ("Project"));
  projectPopup.insertItem(SmallIconSet("filenew"),i18n("New file..."), this, SLOT(slotFileNew()),0, ID_FILE_NEW);
  projectPopup.insertItem(SmallIconSet("classnew"),i18n("New class..."), this, SLOT(slotClassNew()), 0, ID_PROJECT_NEW_CLASS);
  projectPopup.insertSeparator();
  projectPopup.insertItem(SmallIconSet("configure"),i18n("Options..."), this, SLOT(slotProjectOptions()),0, ID_PROJECT_OPTIONS);
  projectPopup.insertItem(SmallIconSet("graphview"),i18n("Graphical classview.."), this, SLOT(slotGraphicalView()), 0, ID_CV_GRAPHICAL_VIEW);



  // Class popup
  classPopup.insertTitle(SmallIcon("CVclass"), i18n("Class"));
  classPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()),0, ID_CV_VIEW_CLASS_DECLARATION);
//  classPopup.insertItem(SmallIcon("CVclass"), i18n("Class Properties & tool"),this, SLOT( slotViewDeclaration()),0, ID_CV_VIEW_CLASS_DECLARATION);
  classPopup.insertSeparator();
  classPopup.insertItem(SmallIconSet("methodnew"), i18n("Add member function..."), this, SLOT(slotMethodNew()),0, ID_CV_METHOD_NEW);
  classPopup.insertItem(SmallIconSet("variablenew"), i18n("Add member variable..."), this, SLOT(slotAttributeNew()),0,ID_CV_ATTRIBUTE_NEW);
  classPopup.insertItem(SmallIconSet("CVpublic_signal"), i18n("Add signal..."), this, SLOT(slotSignalNew()),0, ID_CV_SIGNAL_NEW);
  classPopup.insertItem(SmallIconSet("CVpublic_slot"), i18n("Add slot..."), this, SLOT(slotSlotNew()),0,ID_CV_SLOT_NEW);
//  id = classPopup.insertItem( i18n("Implement virtual function..."), this, SLOT(slotImplementVirtual()),0,ID_CV_IMPLEMENT_VIRTUAL);
//  classPopup.setItemEnabled( id, false );
  classPopup.insertSeparator();
  classPopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  classPopup.insertSeparator();
  classPopup.insertItem( i18n("Parent classes..."), this, SLOT(slotClassBaseClasses()),0, ID_CV_CLASS_BASE_CLASSES);
  classPopup.insertItem( i18n("Child classes..."), this, SLOT(slotClassDerivedClasses()),0, ID_CV_CLASS_DERIVED_CLASSES);
  classPopup.insertItem(SmallIconSet("CVclass"), i18n("Properties"), this, SLOT(slotClassTool()),0, ID_CV_CLASS_TOOL);
  //  classPopup.insertSeparator();
  //  id = classPopup.insertItem( i18n( "Add slot for signal" ), this, SLOT(slotAddSlotSignal()),0, ID_CV_ADD_SLOT_SIGNAL);
  //  classPopup.setItemEnabled( id, false );
  //  id = classPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete class"), this, SLOT(slotClassDelete()), ID_CV_CLASS_DELETE);
  //  classPopup.setItemEnabled(id, false );
  //  classPopup.insertSeparator();
  //  classPopup.insertItem( i18n( "ClassWizard" ), this, SLOT( slotClassWizard()),0, ID_CV_CLASSWIZARD );

  // Struct popup
  structPopup.insertTitle(SmallIcon("CVstruct"), i18n( "Struct" ) );
  structPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration() ),0,ID_CV_VIEW_DEFINITION);
  structPopup.insertSeparator();
  structPopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);

  // Method popup
  methodPopup.insertTitle(SmallIcon("CVpublic_meth"), i18n( "Method" ),1 );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()), 0, ID_CV_VIEW_DEFINITION);
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration() ),0,ID_CV_VIEW_DECLARATION);
  methodPopup.insertSeparator();
  methodPopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  methodPopup.insertSeparator();
  methodPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete method" ), this, SLOT(slotMethodDelete()),0, ID_CV_METHOD_DELETE);

  // Attribute popup
  attributePopup.insertTitle(SmallIcon("CVpublic_var"), i18n( "Attribute" ), 1);
  attributePopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()),0, ID_CV_VIEW_DEFINITION);
  attributePopup.insertSeparator();
  attributePopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  //  attributePopup.insertSeparator();
  //  id = attributePopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete attribute" ), this, SLOT(slotAttributeDelete()),0, ID_CV_ATTRIBUTE_DELETE);
  //  attributePopup.setItemEnabled( id, false );

  // Slot popup
  slotPopup.insertTitle(SmallIcon("CVpublic_slot"), i18n( "Slot" ),1 );
  slotPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()),0, ID_CV_VIEW_DEFINITION);
  slotPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration()),0, ID_CV_VIEW_DECLARATION);
  slotPopup.insertSeparator();
  slotPopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  slotPopup.insertSeparator();
  slotPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete slot" ), this, SLOT(slotMethodDelete()),0,ID_CV_METHOD_DELETE);

  // Signal popup
  signalPopup.insertTitle(SmallIcon("CVpublic_signal"), i18n( "Signal" ) );
  signalPopup.insertItem( i18n( "Go to declaration" ), this, SLOT(slotViewDeclaration()),0, ID_CV_VIEW_DEFINITION );
  signalPopup.insertSeparator();
  signalPopup.insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  signalPopup.insertSeparator();
  signalPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete signal" ), this, SLOT(slotMethodDelete()),0,ID_CV_METHOD_DELETE);

  // Folder popup
  folderPopup.insertTitle(SmallIcon("folder"), i18n( "Folder" ) );
  folderPopup.insertItem(SmallIconSet("filenew"),i18n("New file..."), this, SLOT(slotFileNew()),0, ID_FILE_NEW);
  folderPopup.insertItem(SmallIconSet("classnew"),i18n("New class..."), this, SLOT(slotClassNew()), 0, ID_PROJECT_NEW_CLASS);
  folderPopup.insertSeparator();
  id = folderPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete Folder..."), this, SLOT( slotFolderDelete()),0, ID_CV_FOLDER_DELETE);

  connect(&attributePopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&classPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&folderPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&methodPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&projectPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&slotPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&signalPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
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
//  QProgressDialog progressDlg(NULL, "progressDlg", true );
  QStrList src1;
  QStrList src;
  QStrList header;
  char *str;
  int totalCount=0;
  int currentCount = 0;
  bool popupClassItemsEnable;

  kdDebug() << "CClassView::refresh( proj )" << endl;

  project = proj;

  // memorize which parts of the tree are open and also the selected item
  QStringList pathList = treeH->pathListOfAllOpenedItems();
  QString curSelectedPath = treeH->pathToSelectedItem();

  // Reset the classparser and the view.
  ((CClassTreeHandler *)treeH)->clear();
  cp->wipeout();

  // Get all header and src filenames.
  header = proj->getHeaders();
  src1 = proj->getSources();
  
  
  // a really cool hack.:-), unfortunaly this make the refresh a little bit slower. :-( 
  //we revert the order of the src's to get the data files (dlgs) first parsed. It's important for the addMethod dlg...
  // -Sandy
  QString filestr;
  for(filestr = src1.last();filestr!=0;filestr = src1.prev()){
      src.append(filestr);
  }

  
  // Initialize progressbar.
  totalCount = header.count() + src.count();
  emit resetStatusbarProgress();
  emit setStatusbarProgress( 0 );
  emit setStatusbarProgressSteps( totalCount );

  // Parse headerfiles.
  for( str = header.first(); str != NULL; str = header.next() )
  {
    kdDebug() << "  parsing:[" << str << "]" << endl;
    cp->parse( str );
    emit setStatusbarProgress( ++currentCount );
  }
	
  // Parse sourcefiles.
  QString cur;
  for( cur = src.first(); cur != NULL; cur = src.next() )
  {
		if (cur.contains(".ui"))
			continue;
    kdDebug() << "  parsing:[" << cur << "]" << endl;
    cp->parse(static_cast<const char*>(cur));
    emit setStatusbarProgress( ++currentCount );
  }

  refresh();

  // disable certain popup items if it is a C Project
  popupClassItemsEnable=proj->getProjectType()!="normal_c";
  projectPopup.setItemEnabled(ID_PROJECT_NEW_CLASS, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_FOLDER_NEW, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_GRAPHICAL_VIEW, popupClassItemsEnable);

  // reopen the tree and select the item again
  treeH->openItems(pathList);
  treeH->activateItem(curSelectedPath);
}


/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Reparse the file and redraw the view.
 *
 * Parameters:
 *   aFile        The file to reparse.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh( QStrList &iHeaderList, QStrList &iSourceList)
{
//  cp->getDependentFiles( iHeaderList, iSourceList);

  kdDebug() << "before ui file removal: " << iSourceList.count() << endl;
  QString cur;
  for (cur = iSourceList.first(); cur; cur = iSourceList.next())
  {
      if(cur.contains(".ui")){
        iSourceList.remove(iSourceList.current());
        iSourceList.prev(); // set the previous item the current so the next item will be the next after the removed ui file
      }
  }
  kdDebug() <<"after ui file removal: " << iSourceList.count() << endl;

  // Initialize progressbar.
  int lTotalCount = 0;
	lTotalCount += iHeaderList.count()+iSourceList.count();
		
  emit resetStatusbarProgress();
  int lCurCount = 0;
	emit setStatusbarProgress( lCurCount );
  emit setStatusbarProgressSteps( lTotalCount );

  const char* lCurFile;
  // Remove all references to the files in the lists
//  for (lCurFile = iHeaderList.first(); lCurFile; lCurFile = iHeaderList.next())
//    cp->removeWithReferences( lCurFile );
  		
//  for (lCurFile = iSourceList.first(); lCurFile; lCurFile = iSourceList.next())
//    cp->removeWithReferences( lCurFile );

  // Now parse the each file and add the data back.
  for (lCurFile = iHeaderList.first(); lCurFile; lCurFile = iHeaderList.next())
  {
    kdDebug() << "  parsing:[" << lCurFile << "]" << endl;
	  cp->parse( lCurFile );
    emit setStatusbarProgress( ++lCurCount );
  }

  for (lCurFile = iSourceList.first(); lCurFile; lCurFile = iSourceList.next())
  {
    kdDebug() <<  "  parsing:[" << lCurFile << "]" << endl;
    cp->parse( lCurFile );
    emit setStatusbarProgress( ++lCurCount );
  }

  // memorize which parts of the tree are open and also the selected item
  QStringList pathList = treeH->pathListOfAllOpenedItems();
  QString curSelectedPath = treeH->pathToSelectedItem();

  //reset and refresh the tree
	((CClassTreeHandler *)treeH)->clear();	
	refresh();

  // reopen the tree and select the item again
  treeH->openItems(pathList);
  treeH->activateItem(curSelectedPath);
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
  QList<CParsedScopeContainer> *scopeList;
  QList<CParsedMethod> *methodList;
  QList<CParsedAttribute> *attributeList;
  QList<CParsedStruct> *structList;

  kdDebug() << "CClassView::refresh()" << endl;

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

  // ----------------
  // Add global items
  // ----------------

  // Add global namespaces
  item = treeH->addItem( i18n( "Namespaces" ), THFOLDER, globalsItem );
  scopeList = store->globalContainer.getSortedScopeList();
  ((CClassTreeHandler *)treeH)->addScopes( scopeList, item );
  item->sortChildItems(0,true);
  delete scopeList;

  // Add global Structures
  item = treeH->addItem( i18n( "Structures" ), THFOLDER, globalsItem );
  structList = store->getSortedStructList();
  ((CClassTreeHandler *)treeH)->addGlobalStructs( structList, item );
  item->sortChildItems(0,true);
  delete structList;

  // Add global functions
  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Functions" ), THFOLDER, globalsItem );
  methodList = store->globalContainer.getSortedMethodList();
  kdDebug() << "Got " << methodList->count() << " methods" << endl;
  ((CClassTreeHandler *)treeH)->addGlobalFunctions( methodList, item );
  item->sortChildItems(0,true);
  delete methodList;

  // Add global variables
  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Variables" ), THFOLDER, globalsItem );
  attributeList = store->globalContainer.getSortedAttributeList();
  ((CClassTreeHandler *)treeH)->addGlobalVariables( attributeList, item );
  item->sortChildItems(0,true);
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
  // memorize which parts of the tree are open and also the selected item
  QStringList pathList = treeH->pathListOfAllOpenedItems();
  QString curSelectedPath = treeH->pathToSelectedItem();

  // Reset the tree.
  ((CClassTreeHandler *)treeH)->clear();

  kdDebug() << "Adding file " << aName << endl;

  // Parse the file.
  cp->parse( aName );

  // Build the new classtree.
  refresh();

  // reopen the tree and select the item again
  treeH->openItems(pathList);
  treeH->activateItem(curSelectedPath);
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
  connect(cb, SIGNAL(gotoClassDefinition(CParsedClass *)), SLOT(slotViewClassDefinition(CParsedClass *)));
  cb->setCaption(i18n("Graphical classview"));
  cb->InitializeTree(forest);
  cb->show();
}

/*------------------------------------------------- CClassView::tip()
 * tip()
 *   Check and get a tooltip for a point.
 *
 * Parameters:
 *   p          Point to check if we should get a tooltip for.
 *   r          Rectangle of the tooltip item.
 *   str        String that should contain the tooltip.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::tip( const QPoint &p, QRect &r, QString &str )
{
  QListViewItem *i;

  i = itemAt( p );
  r = itemRect( i );

  if( i != NULL && treeH->itemType( i ) != THFOLDER && r.isValid() )
    str = i->text( 0 );
  else
    str = "";
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
void CClassView::slotViewDefinition( const char *parentPath,
                                     const char *itemName,
                                     THType parentType,
                                     THType itemType )
{
  //  if( validClassDecl( className, declName, type ) )
  emit selectedViewDefinition( parentPath, itemName, parentType, itemType );
}

/*---------------------------------- CClassView::slotViewDeclaration()
 * viewDefinition()
 *   Views a declaration of an item.
 *
 * Parameters:
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::slotViewDeclaration( const char *parentPath,
                                      const char *itemName,
                                      THType parentType,
                                      THType itemType )
{
  //  if( validClassDecl( className, declName, type ) )
  emit selectedViewDeclaration( parentPath, itemName, parentType, itemType );
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
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );
  return store->getClassByName( parentPath );
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
  QString text= currentItem()->text(0);
  int start= text.find('(', 1);
  if(start!=-1)
    text.remove(start+1,text.length()-(start+1) );

  switch( treeH->itemType() )
  {
    case THFOLDER:
      if( strcmp( currentItem()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
        popup = &projectPopup;
      else
        popup = &folderPopup;
      break;
    case THCLASS:
      classPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &classPopup;
      break;
    case THSTRUCT:
      structPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &structPopup;
      break;
    case THPUBLIC_METHOD:
      methodPopup.changeTitle(1,SmallIcon("CVpublic_meth"), i18n("Public Method"));
      methodPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &methodPopup;
      break;
    case THPROTECTED_METHOD:
      methodPopup.changeTitle(1,SmallIcon("CVprotected_meth"), i18n("Protected Method"));
      methodPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &methodPopup;
      break;
    case THPRIVATE_METHOD:
      methodPopup.changeTitle(1,SmallIcon("CVprivate_meth"), i18n("Private Method"));
      methodPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &methodPopup;
      break;
    case THGLOBAL_FUNCTION:
      methodPopup.changeTitle(1,SmallIcon("CVglobal_meth"), i18n("Global Method"));
      methodPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &methodPopup;
      break;
    case THPUBLIC_ATTR:
      attributePopup.changeTitle(1,SmallIcon("CVpublic_var"), i18n("Public Variable"));
      attributePopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &attributePopup;
      break;
    case THPROTECTED_ATTR:
      attributePopup.changeTitle(1,SmallIcon("CVprotected_var"), i18n("Protected Variable"));
      attributePopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &attributePopup;
      break;
    case THPRIVATE_ATTR:
      attributePopup.changeTitle(1,SmallIcon("CVprivate_var"), i18n("Private Variable"));
      attributePopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &attributePopup;
      break;
    case THGLOBAL_VARIABLE:
      attributePopup.changeTitle(1,SmallIcon("CVglobal_var"), i18n("Global Variable"));
      attributePopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &attributePopup;
      break;
    case THPUBLIC_SLOT:
      slotPopup.changeTitle(1,SmallIcon("CVpublic_slot"), i18n("Public Slot"));
      slotPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &slotPopup;
      break;
    case THPROTECTED_SLOT:
      slotPopup.changeTitle(1,SmallIcon("CVprotected_slot"), i18n("Protected Slot"));
      slotPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &slotPopup;
      break;
    case THPRIVATE_SLOT:
      slotPopup.changeTitle(1,SmallIcon("CVprivate_slot"), i18n("Private Slot"));
      slotPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &slotPopup;
      break;
    case THSIGNAL:
      signalPopup.changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
      popup = &signalPopup;
      break;
    default:
      break;
  }

  return popup;
}

/*--------------------------------- CClassView::getTreeStrItem()
 * getTreeStrItem()
 *   Fetch one node from a tree string.
 *
 * Parameters:
 *   str        String containing the tree.
 *   pos        Current position.
 *   buf        Resulting string.
 *
 * Returns:
 *   int        The new position.
 *-----------------------------------------------------------------*/
int CClassView::getTreeStrItem( const char *str, int pos, char *buf )
{
  int idx = 0;

  // Skip first '.
  pos++;
  while( str[pos] != '\'' && str[pos] != ')' && str[pos] != '(' )
  {
    buf[idx]=str[pos];
    idx++;
    pos++;
  }

  // Add a null termination.
  buf[ idx ] = '\0';

  //modif Benoit Cerrina 17 Dec 1999
  // Skip to next ' or to ')'.
	//  pos++;      //I commented out this since we're already at the next '
	//end modif
  return pos;
}

/*------------------------------------------- CClassView::buildTree()
 * buildTree()
 *   Make the classtree from a treestring.
 *
 * Parameters:
 *   str            The string holding the classtree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildTree( const char *str )
{
//	time_t lStart = time(NULL);
//	clock_t lStartClock = clock();
  uint pos=0;
  QListViewItem *root=NULL;
  QListViewItem *parent=NULL;
  QListViewItem *ci;
  CParsedClass *aPC;
  char buf[50];

  kdDebug() << "CClassView::buildtree( treeStr )"  << endl;
	uint lStringSize = strlen(str);
  while( pos < lStringSize )
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
      {
        parent = ((CClassTreeHandler *)treeH)->addItem( buf, THFOLDER, parent );
        treeH->setLastItem( parent );
      }
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

    }
    else if (str[pos] != '(')					//the current character is not an '\'' (we are out of the inner loop) nor a '('
    {
      kdDebug() << "invalid tree string trying to recover" << endl;
      pos++;
    }
  }
  classesItem = root;
//  kdDebug() << "buildTree(str) took " << (time(NULL) - lStart) << "ms to complete" << endl;
//  kdDebug() << "buildTree(str) took " << (clock() - lStartClock) << "clocktick to complete" << endl;

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
        //modif Benoit Cerrina 17 dec 1999
        //lets stay simple it's not working yet
        /*
        str += "'";
        str += ( item->isOpen() ? "1" : "0" );
        */
				//end modif
        buildTreeStr( item->firstChild(), str );
        str += ")";
      }
      else if( type == THCLASS )
      {
        str += "'";
        str += item->text( 0 );
        //modif Benoit Cerrina 17 dec 1999
        //lets stay simple it's not working yet
        /*
        str += "'";
        str += ( item->isOpen() ? "1" : "0" );
        */
        //end modif
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
void CClassView::asTreeStr(QString &str)
{
  str="";

  buildTreeStr( classesItem, str );
}

/*-------------------------------- CClassView::buildInitalClassTree()
 * buildInitalClassTree()
 *   Build the classtree without using a treestring.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildInitalClassTree()
{
//	time_t start = time(NULL);
//	clock_t startClock = clock();
  QString str;
  CParsedClass *aPC;
  QList<CParsedClass> *list;
  QString projDir;
  QSortedList<CClassView::SubfolderClassList> listOfClassLists;
  CClassView::SubfolderClassList* pCurClassList;
  QList<CParsedClass> rootList;

  kdDebug() << "buildInitalClassTree" << endl;

  listOfClassLists.setAutoDelete(true);

  // Insert root item
  str = i18n( CLASSROOTNAME );
  classesItem = treeH->addRoot( str, THFOLDER );

  list = store->getSortedClassList();
  projDir = project->getProjectDir();

  // Add all parsed classes to the correct list;
  for( aPC = list->first();
       aPC !=NULL;
       aPC = list->next())
  {
    // Try to determine if this is a subdirectory.
    str = aPC->definedInFile;
    str = str.remove( 0, projDir.length() );
    int p = str.find( '/', 1 );
    if (p == -1) {
      // a source file directly in the project dir
      str = "";
    }
    else {
      QString subDir(project->getSubDir());
      QString strPart = str.left(p+1);
      if ((strPart == subDir) || (strPart == "src/") || (strPart == "include/")) {
         // files in such subdirs have to appear directly under Classes
         str = str.remove( 0, p);
      }
      str = str.remove( str.findRev( '/' ),10000 ); // just the subdir will remain
    }
    if( str.isEmpty() )
      rootList.append( aPC );
    else {
      // Remove heading /
      if (!str.isEmpty() && (str[0] == '/')) {
        str = str.remove( 0, 1 );
      }

      // search if a class list called contents of str already exists
      QList<CParsedClass>* iterlist = 0L;
      bool bFound = false;
      for (pCurClassList = listOfClassLists.first(); !bFound && pCurClassList != 0; pCurClassList = listOfClassLists.next()) {
        if (pCurClassList->subfolderName == str) {
          bFound = true;
          iterlist = pCurClassList->pClassList;
        }
      }
      if (!iterlist) {
        // must create a new class list
        iterlist = new QList<CParsedClass>(); // will be deleted in destructor of pSCL
        pCurClassList = new CClassView::SubfolderClassList( str, iterlist);
        listOfClassLists.append(pCurClassList);
      }

      iterlist->append(aPC);
    }
  }

  delete list;


  // Add all classes with a folder.
  // (it's tricky: loop from end to start to ensure the folder items are above the class items)
  listOfClassLists.sort();
  for (pCurClassList = listOfClassLists.last(); pCurClassList != 0L; pCurClassList = listOfClassLists.prev()) {
    // Add folder.
    pCurClassList->pFolderItem = treeH->addItem(pCurClassList->subfolderName, THFOLDER, classesItem);
    ((CClassTreeHandler *)treeH)->addClasses(pCurClassList->pClassList, pCurClassList->pFolderItem);
  }

  // set to last item for the classes without folder
  QListViewItem* c = classesItem->firstChild();
  QListViewItem* oldC = c;
  while (c) {
    oldC = c;
    c = c->nextSibling();
  }
  if (oldC) {
    treeH->setLastItem(oldC);
  }

  // Add all classes without a folder.
  ((CClassTreeHandler *)treeH)->addClasses( &rootList, classesItem );

  // Save the tree.
//  asTreeStr( str );
  //Modif Benoit Cerrina 16 dec 99
  //I uncommented the following line
//  project->setClassViewTree( str );
  //end modif

//  kdDebug() << "buildInitialClassTree took " << (time(NULL) - start) << " ms to complete" << endl;
//  kdDebug() << "buildInitialClassTree took " << (clock() - startClock) << " clock to complete" << endl;
}

/*----------------------------------------- CClassView::createCTDlg()
 * createCTDlg()
 *   Create a new ClassTool dialog and setup its' attributes.
 *
 * Parameters:
 *   -
 * Returns:
 *   A newly allocated classtool dialog.
 *-----------------------------------------------------------------*/
// CClassToolDlg*
CClassPropertiesDlgImpl *CClassView::createCTDlg(CParsedClass* aClass, int pgn)
{
  CClassPropertiesDlgImpl *ctDlg = new CClassPropertiesDlgImpl( this, (CTPACTION) pgn, NULL );

  kdDebug() << "CClassView::createCTDlg() : creating CClassToolDlg as child of CClassPropertiesDlgImpl::*tpgClassView:"<< endl;
  CClassToolDlg* tool = new CClassToolDlg( ctDlg -> CVLayout );
  tool -> show ();

  connect( tool,
           SIGNAL( signalViewDeclaration(const char *,const char *,THType,THType ) ),
           SLOT(slotViewDeclaration(const char *,const char *,THType,THType ) ) );

  connect( tool,
           SIGNAL( signalViewDefinition(const char *, const char *, THType, THType ) ),
           SLOT(slotViewDefinition(const char *, const char *, THType, THType ) ) );

  connect ( tool,
            SIGNAL(signalClassChanged(CParsedClass*)), ctDlg,
            SLOT(slotClassViewChanged( CParsedClass* ) ));

  connect ( ctDlg, SIGNAL(sigAddAttribute( const char*, CParsedAttribute*)),
                        SLOT( slotAddAttribute( const char*, CParsedAttribute*)));
  connect ( ctDlg, SIGNAL(sigAddMethod( const char*, CParsedMethod*)),
                        SLOT( slotAddMethod( const char*, CParsedMethod*)));
  connect ( ctDlg,
            SIGNAL(sigSigSlotMapImplement ( CParsedClass*, const QString&, CParsedMethod*)),
            SLOT(slotSigSlotMapImplement ( CParsedClass*, const QString&, CParsedMethod*)));

  tool -> setStore( store );
  tool -> setClass( aClass );

  ctDlg -> setCurrentClassName( tool -> classToString() );
  ctDlg -> setStore ( store );
  ctDlg -> setClass( aClass );
  ctDlg -> setClassToolDlg( tool );

  return ctDlg;
}

/*-------------------------------------- CClassView::validClassDecl()
 * validClassDecl()
 *   Create a new ClassTool dialog and setup its' attributes.
 *
 * Parameters:
 *   -
 * Returns:
 *   A newly allocated classtool dialog.
 *-----------------------------------------------------------------*/
bool CClassView::validClassDecl( const char *className,
                                 const char *declName,
                                 THType /*type*/ )
{
  bool retVal = false;
  QString str = i18n( "No item selected." );

  retVal = !( className == NULL && declName == NULL );

  if( retVal && className != NULL )
  {
    str = i18n("The class %1 couldn't be found.").arg(className == NULL ? "" : className);
    retVal = store->hasClass( className );

    if( !retVal )
    {
      str = i18n("The struct %1 couldn't be found.").arg(className == NULL ? "" : className);
      retVal = store->hasStruct( className );
    }
  }

  if( !retVal )
    KMessageBox::error( this, str, i18n( "Not found" ) );

  return retVal;
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
  THType type = treeH->itemType();
  if (type == THCLASS)
  {
    QListViewItem *cur=currentItem();
    if( strcmp( currentItem()->parent()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
      emit selectedFileNew();
    else
    {
      QString dir;
      cur = cur->parent();
      while ((cur != 0) && (strcmp( cur->text(0), i18n( CLASSROOTNAME )) != 0))
      {
        dir = cur->text(0) + ("/" + dir);
        cur = cur->parent();
      }
      emit selectedFileNew(dir);
    }
  }
  if (type == THFOLDER)
  {
    if( strcmp( currentItem()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
      emit selectedFileNew();
    else
    {
      QString dir;
      QListViewItem *cur=currentItem();
      while ((cur != 0) && (strcmp( cur->text(0), i18n( CLASSROOTNAME )) != 0))
      {
        dir = cur->text(0) + ("/" + dir);
        cur = cur->parent();
      }
      emit selectedFileNew(project->getProjectDir() + project->getSubDir() + dir);
    }
  }
}

void CClassView::slotClassNew()
{
  THType type = treeH->itemType();
  QString path;
  if (type == THFOLDER)
  {
    if( strcmp( currentItem()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
      emit selectedClassNew();
    else
    {
      QString dir;
      QListViewItem *cur=currentItem();
      while ((cur != 0) && (strcmp( cur->text(0), i18n( CLASSROOTNAME )) != 0))
      {
        dir = cur->text(0) + ("/" + dir);
        cur = cur->parent();
      }
      emit selectedClassNew(project->getSubDir()+dir);
    }
  }
}

void CClassView::slotClassDelete()
{
  if( KMessageBox::questionYesNo( this,
                      i18n("Are you sure you want to delete this class?"),
                      i18n("Delete class")) == KMessageBox::Yes )
  {
    KMessageBox::error( this,
                      i18n("This function isn't implemented yet."),
                      i18n("Not implemented") );
  }

}

void CClassView::slotClassViewSelected()
{
  THType type = treeH->itemType();

  kdDebug() << "in CClassView::slotClassViewSelected(): \n";

  /*  well, there is no middle mouse button action here any longer, we are now fully
      KDE-2 conform and dont want to confuse our windows lusers */

  // Take care of left-button clicks.
//  if( mouseBtn == LeftButton && type != THFOLDER )
//  {
    if( type == THCLASS || type == THSTRUCT || type == THGLOBAL_VARIABLE ||
        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR ||
        type == THPRIVATE_ATTR || type == THSIGNAL || type == THSCOPE )
      slotViewDeclaration();
    else
      slotViewDefinition();
//  }
//  else if( mouseBtn == MidButton && type != THFOLDER ) // Middle button clicks
//  {
//    if( type == THCLASS || type == THSTRUCT || type == THGLOBAL_VARIABLE ||
//        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR ||
//        type == THPRIVATE_ATTR  || type == THSIGNAL || type == THSCOPE )
//      slotViewDefinition();
//    else
//      slotViewDeclaration();
//  }

  // Set it back, so next time only if user clicks again we react.
  //mouseBtn = RightButton;
}

void CClassView::slotMethodNew()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;
    CParsedClass * aClass;
  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );
  if (itemType==THCLASS)
  {
      kdDebug() << "parentPath = " << parentPath.data() << endl;
      aClass = store -> getClassByName ( parentPath );
      kdDebug() << "got class: " << aClass -> name.data() << endl;
      CClassPropertiesDlgImpl* dlg = createCTDlg(getCurrentClass(), (int) CTPADDMETH);
      dlg -> show();
  }

   //emit signalAddMethod( parentPath );
}

void CClassView::slotMethodDelete()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );

  emit signalMethodDelete( parentPath, itemName );
}

void CClassView::slotAttributeNew()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;
  CParsedClass* aClass;
  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );
  if (itemType==THCLASS)
  {
      kdDebug() << "parentPath = " << parentPath.data() << endl;
      aClass = store -> getClassByName ( parentPath );
      kdDebug() << "got class: " << aClass -> name.data() << endl;
      CClassPropertiesDlgImpl* dlg = createCTDlg(getCurrentClass(), (int) CTPADDATTR);
      dlg -> show();
  }
    // Disabling hold dialog...
   //   emit signalAddAttribute( parentPath );
}

void CClassView::slotAttributeDelete()
{
  if( KMessageBox::questionYesNo( this,
                      i18n("Are you sure you want to delete this attribute?"),
                      i18n("Delete attribute")) == KMessageBox::Yes )
  {
    KMessageBox::error( this,
                      i18n("This function isn't implemented yet."),
                      i18n("Not implemented") );
  }
}

void CClassView::slotSignalNew()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;
  CParsedClass * aClass;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );
  if (itemType==THCLASS)
  {
      kdDebug() << "parentPath = " << parentPath.data() << endl;
      aClass = store -> getClassByName ( parentPath );
      kdDebug() << "got class: " << aClass -> name.data() << endl;
      CClassPropertiesDlgImpl* dlg = createCTDlg(getCurrentClass(), (int) CTPADDSIGNAL);
      dlg -> show();
  }
}

void CClassView::slotSignalDelete()
{
  if( KMessageBox::questionYesNo( this,
                      i18n("Are you sure you want to delete this signal?"),
                      i18n("Delete signal")) == KMessageBox::Yes )
  {
    KMessageBox::error( this,
                      i18n("This function isn't implemented yet."),
                      i18n("Not implemented") );
  }
}

void CClassView::slotSlotNew()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;
  CParsedClass * aClass;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );
  if (itemType==THCLASS)
  {
      kdDebug() << "parentPath = " << parentPath.data() << endl;
      aClass = store -> getClassByName ( parentPath );
      kdDebug() << "got class: " << aClass -> name.data() << endl;
      CClassPropertiesDlgImpl* dlg = createCTDlg(getCurrentClass(), (int) CTPADDSLOT);
      dlg -> show();
  }
}

void CClassView::slotSlotDelete()
{
  if( KMessageBox::questionYesNo( this,
                      i18n("Are you sure you want to delete this slot?"),
                      i18n("Delete slot")) == KMessageBox::Yes )
  {
    KMessageBox::error( this,
                      i18n("This function isn't implemented yet."),
                      i18n("Not implemented") );
  }
}

//void CClassView::slotImplementVirtual()
//{
//}

void CClassView::slotFolderDelete()
{
  QListViewItem *parent;

  if( KMessageBox::questionYesNo( this,
                      i18n("Are you sure you want to delete this folder?"),
                      i18n("Delete folder")) == KMessageBox::Yes )
  {
    parent = currentItem()->parent();
    parent->removeItem( currentItem() );
  }
}

void CClassView::slotClassBaseClasses()
{
  CClassPropertiesDlgImpl *ctDlg = createCTDlg(getCurrentClass(), (int) CTPVIEW);

  ctDlg->viewParents();
  ctDlg->show();
}

void CClassView::slotClassDerivedClasses() 
{
  CClassPropertiesDlgImpl *ctDlg = createCTDlg(getCurrentClass(), (int) CTPVIEW);

  ctDlg->viewChildren();
  ctDlg->show();
}

void CClassView::slotClassTool()
{
  CClassPropertiesDlgImpl *ctDlg = createCTDlg(getCurrentClass(), (int) CTPVIEW);

  ctDlg->show();
}

void CClassView::slotViewDefinition() 
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );

  slotViewDefinition( parentPath, itemName, parentType, itemType );
}

void CClassView::slotViewClassDefinition(CParsedClass *pClass)
{
  QString toFile;
  int toLine=-1;

  //  if( validClassDecl( className, declName, type ) )
  if (pClass)
  {
    toFile = pClass->declaredInFile;
    toLine = pClass->declaredOnLine;

    emit selectFile(toFile, toLine);
  }
}

void CClassView::slotViewDeclaration()
{
  QString parentPath;
  QString itemName;
  THType parentType;
  THType itemType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( parentPath, itemName,
                                                 parentType, itemType );

  slotViewDeclaration( parentPath, itemName, parentType, itemType );
}

void CClassView::slotClassWizard()
{
  CClassWizardDlg dlg;

  dlg.setStore( store );
  dlg.exec();
}


/**  */
void CClassView::slotGrepText(){
  QString text= currentItem()->text(0);
  int start= text.find('(', 1);
  if(start!=-1)
    text.remove(start+1,text.length()-(start+1) );

  emit signalGrepText(text);
}

/** Called from signal CClassPropertiesDlgImpl::sigAddxxx(const char *aClassName, CParsedxxx*)
This method emits signal sigAddxxx(...) for CKdevelop
 */
void CClassView::slotAddMethod ( const char * aClassName, CParsedMethod* aMethod)
{
    emit sigAddMethod ( aClassName, aMethod );
}

void CClassView::slotAddAttribute( const char * aClassName, CParsedAttribute* aAttr)
{
    emit sigAddAttribute( aClassName, aAttr);
}
/**  */
void CClassView::slotSigSlotMapImplement ( CParsedClass* aClass, const QString& toAdd, CParsedMethod* implMethod)
{
    emit sigSigSlotMapImplement ( aClass, toAdd, implMethod );
}

#include "cclassview.moc"


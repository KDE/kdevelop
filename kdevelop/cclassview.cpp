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
#include <kiconloader.h>
#include <kmsgbox.h>

#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
#include "cclasstooldlg.h"

// Initialize static members
QString CClassView::CLASSROOTNAME = "Classes";
QString CClassView::GLOBALROOTNAME = "Global";

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
  : KTreeList (parent, name)
{
  readIcons();
  initPopups();

  // Add callback for clicks in the listview.
  connect( this, 
           SIGNAL( singleSelected( int ) ), 
           SLOT( slotSingleSelected( int ) ) );

  // Set the store.
  store = &cp.store;
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
  for( int i=0; i<END_POS; i++ )
    delete icons[ i ];

  delete []icons;
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
  QStrList list;
  QStrList src;
  QStrList header;
  char *str;
  QString projDir;

  debug( "CClassView::refresh( proj )" );

  // Reset the classparser and the view.
  clear();
  cp.wipeout();

  projDir = proj->getProjectDir();
  
  // Get the lists containing the files for the project.
  proj->getAllFiles( list );
  src = proj->getSources();
  header = proj->getHeaders();

  // Parse headerfiles.
  for( str = header.first(); str != NULL; str = header.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
  }

  // Parse sourcefiles.
  for( str = src.first(); str != NULL; str = src.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
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
  CParsedClass *aPC;
  QString cstr;
  QString gstr;
  KPath classPath;
  KPath globalPath;

  setUpdatesEnabled( false );

  // Insert root item
  cstr = i18n( CLASSROOTNAME );
  insertItem( cstr, icons[ PROJECT ] );

  classPath.push( &cstr );

  // Add all parsed classes to the view
  for( store->classIterator.toFirst(); 
       store->classIterator.current(); 
       ++store->classIterator )
  {
    aPC = store->classIterator.current();

    // Add the class.
    addChildItem( aPC->name, icons[ CVCLASS ], &classPath );
    classPath.push( &aPC->name );

    updateClass( aPC, &classPath );
    
    classPath.pop();
  }

  // Add all global functions and variables
  gstr = i18n( GLOBALROOTNAME );
  insertItem( gstr, icons[ PROJECT ] );
  globalPath.push( &gstr );
  addMethods( store->getGlobalFunctions(), globalPath );
  addAttributes( store->gvIterator, globalPath );

  // Redraw the view.
  setExpandLevel( 1 );
  setUpdatesEnabled( true );
  repaint();
}

/*------------------------------------- CClassView::refreshClassById()
 * refreshClassById()
 *   Reparse and redraw a class by using its' menuid.
 *
 * Parameters:
 *   aID             The menuid.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refreshClassById( int aID )
{
}

/*-------------------------------------- CClassView::refreshClassById()
 * refreshClassById()
 *   Reparse and redraw a classes by using its' name.
 *
 * Parameters:
 *   aName          The classname
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refreshClassByName( QString &aName )
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CClassView::indexType()
 * indexType()
 *   Return the type of a certain index.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
int CClassView::indexType( int aIdx )
{
  KTreeListItem *item;
  KTreeListItem *parent;
  CParsedClass *aClass;
  int retVal = -1;

  item = itemAt( aIdx );

  // Should add cases for global functions and variables.
  if( strcmp( item->getText(), i18n( "Classes" ) ) == 0 ) // Root
    retVal = PROJECT;
  else if( cp.store.getClassByName( item->getText() ) )
    retVal = CVCLASS;
  else // Check for methods and attributes.
  {
    parent = item->getParent();
    aClass = cp.store.getClassByName( parent->getText() );
    if( aClass && aClass->getMethodByNameAndArg( item->getText() ) )
      retVal = METHOD;
    else if( aClass && aClass->getAttributeByName( item->getText() ) )
      retVal = ATTRIBUTE;
  }

  // Check for globals if nothing else has worked.
  if( retVal == -1 )
    if( store->getGlobalFunctionByNameAndArg( item->getText() ) != NULL )
      retVal = CVGLOBAL_FUNCTION;
    else if( store->getGlobalVarByName( item->getText() ) != NULL )
      retVal = CVGLOBAL_VARIABLE;

  return retVal;
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CClassView::readIcons()
 * readIcons()
 *   Read the icons from disk and store them in the class.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::readIcons()
{
  QString PIXPREFIX = "/kdevelop/pics/mini/";
  QString projIcon = "folder.xpm";
  QString pixDir;
  KIconLoader *il;

  // Allocate the array.
  icons = new QPixmap *[ END_POS ];

  pixDir = KApplication::kde_datadir () + PIXPREFIX;
  debug( "Fetching pixmaps from: %s", pixDir.data() );

  il = KApplication::getKApplication()->getIconLoader();

  // Load the icons
  icons[ PROJECT ] = new QPixmap( il->loadMiniIcon( "folder.xpm" ) );
  icons[ CVCLASS ] = new QPixmap(pixDir + "CVclass.xpm");
  icons[ STRUCT ] = new QPixmap(pixDir + "CVstruct.xpm");
  icons[ PUBLIC_ATTR ] = new QPixmap(pixDir + "CVpublic_var.xpm");
  icons[ PROTECTED_ATTR ] = new QPixmap(pixDir + "CVprotected_var.xpm");
  icons[ PRIVATE_ATTR ] = new QPixmap(pixDir + "CVprivate_var.xpm");
  icons[ PUBLIC_METHOD ] = new QPixmap(pixDir + "CVpublic_meth.xpm");
  icons[ PROTECTED_METHOD ] = new QPixmap(pixDir + "CVprotected_meth.xpm");
  icons[ PRIVATE_METHOD ] = new QPixmap(pixDir + "CVprivate_meth.xpm");
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
void  CClassView::initPopups()
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

void CClassView::updateClass( CParsedClass *aClass, KPath *aPath )
{
  KTreeListItem *top;
  KTreeListItem *current;
  KTreeListItem *next;
  
  top = itemAt( aPath );
  current = top->getChild();
  
  while( current != NULL )
  {
    next = current->getSibling();
    top->removeChild( current );
    current = next;
  }

  // Add parts of the class
  addMethods( aClass->getMethods(), *aPath );
  addAttributes( aClass->attributeIterator, *aPath );
  addSlots( aClass, *aPath );
  addSignals( aClass, *aPath );
}

/*------------------------------------------ CClassView::addMethods()
 * addMethods()
 *   Add all methods from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addMethods( QList<CParsedMethod> *list, KPath &path )
{
  CParsedMethod *aMethod;
  QPixmap *icon;
  QString str;

  // Add the methods
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    if( aMethod->isPublic() )
      icon = icons[ PUBLIC_METHOD ];
    else if( aMethod->isProtected() )
      icon = icons[ PROTECTED_METHOD ];
    else if( aMethod->isPrivate() )
      icon = icons[ PRIVATE_METHOD ];
    else // Global
      icon = icons[ PUBLIC_METHOD ];
    
    aMethod->toString( str );
    addChildItem( str, icon, &path );
  }
}

/*------------------------------------------ CClassView::addAttributes()
 * addAttributes()
 *   Add all attributes from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addAttributes( QDictIterator<CParsedAttribute> &iter,
                                KPath &path )
{
  QPixmap *icon;
  CParsedAttribute *aAttr;

  // Add the methods
  for( iter.toFirst();
       iter.current();
       ++iter )
  {
    aAttr = iter.current();
    if( aAttr->isPublic() )
      icon = icons[ PUBLIC_ATTR ];
    else if( aAttr->isProtected() )
      icon = icons[ PROTECTED_ATTR ];
    else if( aAttr->isPrivate() )
      icon = icons[ PRIVATE_ATTR ];
    else // Global
      icon = icons[ PUBLIC_ATTR ];
    
    addChildItem( aAttr->name, icon, &path );
  }
}

/*------------------------------------------ CClassView::addSlots()
 * addSlots()
 *   Add all slots from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addSlots( CParsedClass *aPC, KPath &path )
{
  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = aPC->slotList.first();
       aMethod != NULL;
       aMethod = aPC->slotList.next() )
    addChildItem( aMethod->name, icons[ STRUCT ], &path );
}

/*------------------------------------------ CClassView::addSignals()
 * addSignals()
 *   Add all signals from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addSignals( CParsedClass *aPC, KPath &path )
{
  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = aPC->signalList.first();
       aMethod != NULL;
       aMethod = aPC->signalList.next() )
    addChildItem( aMethod->name, icons[ STRUCT ], &path );
}

/*********************************************************************
 *                                                                   *
 *                              EVENTS                               *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassView::mousePressEvent()
 * mousePressEvent()
 *   Handles mousepressevents(duh!). If the left or right mouse 
 *   button is pressed the coordinate and the mousebutton is saved.
 *
 * Parameters:
 *   event           The event.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::mousePressEvent(QMouseEvent * event)
{
  // Save the mousebutton.
  mouseBtn = event->button();

  if( mouseBtn == LeftButton || mouseBtn == RightButton )
    mousePos = event->pos();

  KTreeList::mousePressEvent( event );
}

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
  KTreeListItem *item;
  item = itemAt( currentItem() );
  return store->getClassByName( item->getText() );
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassView::slotSingleSelected()
 * slotSingleSelected()
 *   Event when a user selects someting in the tree.
 *
 * Parameters:
 *   index           Index of the selected item
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::slotSingleSelected (int index)
{
  KPopupMenu *popup;

  // If the right button is pressed we show a popupmenu.
  if( mouseBtn == RightButton )
  {
    switch( indexType( index ) )
    {
      case PROJECT:
        popup = &projectPopup;
        break;
      case CVCLASS:
        popup = &classPopup;
        break;
      case METHOD:
        popup = &methodPopup;
        break;
      case ATTRIBUTE:
        popup = &attributePopup;
        break;
      default:
        popup = NULL;
        break;
    }

    if( popup )
      popup->popup( this->mapToGlobal( mousePos ) );
  }
}

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
  CParsedClass *aClass;
  CParsedMethod *aMethod;
  CAddClassMethodDlg dlg(this, "methodDlg" );
  QString str;
  
  if( dlg.exec() )
  {
    debug( "Adding method." );
    aMethod = dlg.asSystemObj();
    aMethod->out();

    aClass = getCurrentClass();
    if( aClass )
    {
      aClass->addMethod( aMethod );
      updateClass( aClass, itemPath( currentItem() ) );
    }

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
  CParsedClass *aClass;
  CAddClassAttributeDlg dlg(this, "attrDlg" );
  CParsedAttribute *aAttr;

  if( dlg.exec() )
  {
    debug( "Adding attribute:" );
    aAttr = dlg.asSystemObj();
    aAttr->out();

    aClass = getCurrentClass();
    if( aClass )
    {
      aClass->addAttribute( aAttr );
      updateClass( aClass, itemPath( currentItem() ) );
    }

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
  emit selectedViewDefinition( currentItem() );
}

void CClassView::slotViewDeclaration()
{
  emit selectedViewDeclaration( currentItem() );
}

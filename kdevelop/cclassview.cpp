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

/*------------------------------------------ CClassView::addClass()
 * addClass()
 *   Add a class to be shown in the view.
 *
 * Parameters:
 *   aClass         The classdefintion.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addClass( CCVClass *aClass )
{
  assert( aClass != NULL && !hasClass( aClass->name ) );

  classes.append( aClass );
}

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
  char *s1;
  QString projDir;

  debug( "CClassView::refresh( proj )" );

  // Reset the classparser.
  cp.wipeout();

  projDir = proj->getProjectDir();
  
  // Get the lists containing the files for the project.
  proj->getAllFiles( list );
  src = proj->getSources();
  header = proj->getHeaders();

  // Parse headerfiles.
  for( str = header.first(); str != NULL; str = header.next() )
  {
    // Find a fullpath containing the filename.
    for( s1 = list.first(); 
         s1 != NULL && strstr( s1, str ) == NULL; 
         s1 = list.next() )
      ;

    debug( "  parsing:[%s%s]", projDir.data(), s1 );
    cp.parse( projDir + s1 );
  }

  // Parse sourcefiles.
  for( str = src.first(); str != NULL; str = src.next() )
  {
    // Find a fullpath containing the filename.
    for( s1 = list.first(); 
         s1 != NULL && strstr( s1, str ) == NULL; 
         s1 = list.next() )
      ;

    debug( "  parsing:[%s%s]", projDir.data(), s1 );
    cp.parse( projDir + s1 );
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

    debug( "  Added class %s", aPC->name.data() );

    // Add parts of the class
    addMethods( aPC->getMethods(), classPath );
    addAttributes( aPC->attributeIterator, classPath );
    addSlots( aPC, classPath );
    addSignals( aPC, classPath );
    
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

/*------------------------------------------ CClassView::hasClass()
 * hasClass()
 *   Check if a class has been added to the view
 *
 * Parameters:
 *   aName          Name of the class to check for.
 *
 * Returns:
 *   bool           If the class exists or not.
 *-----------------------------------------------------------------*/
bool CClassView::hasClass( QString &aName )
{
  CCVClass *aClass;

  for( aClass = classes.first();
       aClass != NULL && aClass->name != aName;
       aClass = classes.next() )
    ;

  return aClass != NULL;
}

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
  classPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotClassDefinition()));
  classPopup.insertItem( i18n("Add member method..."), this, SLOT(slotMethodNew()));
  classPopup.insertItem( i18n("Add member attribute..."), this, SLOT(slotAttributeNew()));
  classPopup.insertSeparator();
  id = classPopup.insertItem( i18n("Base classes..."), this, SLOT(slotClassBaseClasses()));
  classPopup.setItemEnabled(id, false );
  classPopup.insertItem( i18n("Derived classes..."), this, SLOT(slotClassDerivedClasses()));
  classPopup.insertSeparator();
  classPopup.insertItem( i18n("Delete class"), this, SLOT(slotClassDelete()));
  classPopup.insertItem(i18n("New Folder..."), this, SLOT( slotFolderNew()));

  // Method popup
  methodPopup.setTitle( i18n( "Method" ) );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotMethodDefinition()));
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotMethodDeclaration()));
  methodPopup.insertSeparator();
  methodPopup.insertItem( i18n( "Delete method" ), this, SLOT(slotMethodDelete()));

  // Attribute popup
  attributePopup.setTitle( i18n( "Attribute" ) );
  attributePopup.insertItem( i18n("Go to definition" ), this, SLOT( slotAttributeDefinition()));
  attributePopup.insertItem( i18n( "Delete attribute" ), this, SLOT(slotAttributeDelete()));
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

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CClassView::slotSingleSelected (int index)
{
  KPopupMenu *popup;

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

void CClassView::slotProjectOptions ()
{
  emit selectedProjectOptions ();
}

void CClassView::slotFileNew ()
{
  emit selectedFileNew ();
}

void CClassView::slotClassNew ()
{
  emit selectedClassNew ();
}

void CClassView::slotClassDelete ()
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

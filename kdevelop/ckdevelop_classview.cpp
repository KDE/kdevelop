/***************************************************************************
                          cclassview.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
    based on             : ckdevelop_classview.cpp by Sandy Meier
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ckdevelop.h"
#include "cclassview.h"

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- CKDevelop::slotClassTreeSelected()
 * slotClassTreeSelected()
 *   Event when the user clicks on an item in the classtree.
 *
 * Parameters:
 *   index           Index of the clicked item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassTreeSelected(int index)
{
  int idxT;

  // Only react on clicks on the left mousebutton.
  if( class_tree->mouseBtn == LeftButton )
  {
    idxT = class_tree->indexType( index );

    // If this is an function of some sort, go to the declaration.
    if( idxT == CVGLOBAL_FUNCTION || 
        idxT == METHOD )
      CVGotoDeclaration( index );
    else // Goto the definition.
      CVGotoDefinition( index );
  }
}

/*--------------------------------- CKDevelop::slotClassChoiceCombo()
 * slotClassChoiceCombo()
 *   Event when the user selects an item in the classcombo.
 *
 * Parameters:
 *   index           Index of the selected item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassChoiceCombo(int index)
{
  CParsedClass *aClass;
  KCombo* classCombo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);  
  QString classname = classCombo->text( index );

  aClass = class_tree->store->getClassByName( classname );
  refreshMethodCombo( aClass );
}

/*-------------------------------- CKDevelop::slotMethodChoiceCombo()
 * slotMethodChoiceCombo()
 *   Event when the user selects an item in the methodcombo.
 *
 * Parameters:
 *   index           Index of the selected item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotMethodChoiceCombo(int index)
{
  CParsedClass *aClass;
  CParsedMethod *aMethod;
  QString toFile;
  int toLine;
  KCombo* classCombo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->text( index );

  aClass = class_tree->store->getClassByName( classname );
  if( aClass )
  {
    aMethod = aClass->getMethodByNameAndArg( methodname );

    if( aMethod )
    {
      toFile = ( aMethod->isInHFile ? aClass->hFilename : aClass->implFilename );
      toLine = aMethod->declaredOnLine;
      switchToFile( toFile, toLine );
    }
  }
}

/*-------------------------------- CKDevelop::slotCVViewDeclaration()
 * slotCVViewDeclaration()
 *   Event when the user wants a declaration for an item in the tree.
 *
 * Parameters:
 *   index           Index of the item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVViewDeclaration(int index)
{
  CVGotoDeclaration( index );
}

/*-------------------------------- CKDevelop::slotCVViewDefinition()
 * slotCVViewDefinition()
 *   Event when the user wants a definition for an item in the tree.
 *
 * Parameters:
 *   index           Index of the item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVViewDefinition(int index)
{
  CVGotoDefinition( index );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CKDevelop::CVGotoDefinition()
 * CVGotoDefinition()
 *   Goto the definition of the item with the index in the tree.
 *
 * Parameters:
 *   index          Index in the tree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVGotoDefinition(int index)
{
  CParsedClass *aClass;
  CParsedAttribute *aAttr;
  KTreeListItem *item;
  KTreeListItem *parent;
  QString toFile;
  int idxType;
  int toLine = -1;

  // Fetch the clicked item.
  item = class_tree->itemAt( index );

  // Get the type of declaration at the index.
  idxType = class_tree->indexType( index );
  switch( idxType )
  {
    case CVCLASS:
      aClass = class_tree->store->getClassByName( item->getText() );
      toFile = aClass->hFilename;
      toLine = aClass->definedOnLine;
      break;
    case ATTRIBUTE:
    case METHOD:
      parent = item->getParent();
      aClass = class_tree->store->getClassByName( parent->getText() );
      
      // Fetch the attribute/method.
      aAttr = ( idxType == ATTRIBUTE ? 
                aClass->getAttributeByName( item->getText() ) :
                aClass->getMethodByNameAndArg( item->getText() ) );
      
      toFile = aAttr->declaredInFile;
      toLine = aAttr->definedOnLine;
      break;
    case CVGLOBAL_FUNCTION:
    case CVGLOBAL_VARIABLE:
      aAttr = ( idxType == CVGLOBAL_FUNCTION ?
                class_tree->store->getGlobalFunctionByNameAndArg( item->getText() ) :
                class_tree->store->getGlobalVarByName( item->getText() ) );
      toFile = aAttr->declaredInFile;
      toLine = aAttr->definedOnLine;
      break;
    default:
      debug( "Clicked on unknown type." );
  }

  if( toLine != -1 )
  {
    debug( "  Switching to file %s @ line %d", toFile.data(), toLine );
    switchToFile( toFile, toLine );
  }
}

/*-------------------------------------- CKDevelop::CVGotoDeclaration()
 * CVGotoDeclaration()
 *   Goto the declaration of the item with the index in the tree.
 *
 * Parameters:
 *   index          Index in the tree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVGotoDeclaration(int index)
{
  CParsedClass *aClass;
  CParsedMethod *aMethod = NULL;
  KTreeListItem *item;
  KTreeListItem *parent;

  // Fetch the clicked item.
  item = class_tree->itemAt( index );

  switch( class_tree->indexType( index ) )
  {
    case METHOD:
      parent = item->getParent();
      aClass = class_tree->store->getClassByName( parent->getText() );
      if( aClass )
        aMethod = aClass->getMethodByNameAndArg( item->getText() );
      break;
    case CVGLOBAL_FUNCTION:
      aMethod = class_tree->store->getGlobalFunctionByNameAndArg( item->getText() );
      break;
  }
  
  if( aMethod )
    switchToFile( aMethod->declaredInFile, aMethod->declaredOnLine );
}

/*----------------------------------- CKDevelop::refreshClassCombo()
 * refreshClassCombo()
 *   Update the class combo with all classes.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshClassCombo()
{
  CParsedClass *aClass;
  KCombo* classCombo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);

  // Clear the combos.
  classCombo->clear();
  methodCombo->clear();

  // Add all classes.
  for( class_tree->store->classIterator.toFirst(); 
       class_tree->store->classIterator.current(); 
       ++class_tree->store->classIterator )
  {
    aClass = class_tree->store->classIterator.current();

    // Add the class.
    classCombo->insertItem( aClass->name );
  }

  // Update the method combo with the class from the classcombo.
  aClass = class_tree->store->getClassByName( classCombo->currentText() );
  if( aClass )
    refreshMethodCombo( aClass );
}

/*----------------------------------- CKDevelop::refreshMethodCombo()
 * refreshMethodCombo()
 *   Update the method combo with the methods from the selected
 *   class.
 *
 * Parameters:
 *   aClass           The selected class.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshMethodCombo( CParsedClass *aClass )
{
  CParsedMethod *aMethod;
  QList<CParsedMethod> *list;
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  QString str;

  methodCombo->clear();

  // Add all methods, slots and signals of this class.
  list = aClass->getMethods();
  for( aMethod = list->first(); 
       aMethod != NULL;
       aMethod = list->next() )
  {
    aMethod->toString( str );
    methodCombo->insertItem( str );
  }
  
  for( aMethod = aClass->slotList.first(); 
       aMethod != NULL;
       aMethod = aClass->slotList.next() )
  {
    aMethod->toString( str );
    methodCombo->insertItem( str );
  }
  
  for( aMethod = aClass->signalList.first(); 
       aMethod != NULL;
       aMethod = aClass->signalList.next() )
  {
    aMethod->toString( str );
    methodCombo->insertItem( str );
  }
}

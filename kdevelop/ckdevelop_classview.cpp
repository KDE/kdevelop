/***************************************************************************
                          cclassview.cpp  -  implementation
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
#include <assert.h>


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
void CKDevelop::slotClassTreeSelected()
{
  int idxT;

  // Only react on clicks on the left mousebutton.
  if( class_tree->mouseBtn == LeftButton )
  {
    idxT = class_tree->treeH->itemType();

    // If this is an function of some sort, go to the declaration.
    if( idxT == THGLOBAL_FUNCTION || 
        idxT == THPUBLIC_METHOD ||
        idxT == THPROTECTED_METHOD ||
        idxT == THPRIVATE_METHOD || 
        idxT == THPUBLIC_SLOT || 
        idxT == THPROTECTED_SLOT || 
        idxT == THPRIVATE_SLOT )
      CVGotoDeclaration( class_tree->currentItem() );
    else // Goto the definition.
      CVGotoDefinition( class_tree->currentItem() );
  }
    class_tree->mouseBtn = RightButton; // set it back, so next time only if user clicks again we react
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

  if (classname == "") return;
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

  if (methodname == "") return;
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
void CKDevelop::slotCVViewDeclaration()
{
  CVGotoDeclaration( class_tree->currentItem() );
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
void CKDevelop::slotCVViewDefinition()
{
  CVGotoDefinition( class_tree->currentItem() );
}

/*-------------------------------------- CKDevelop::slotCVAddMethod()
 * slotCVAddMethod()
 *   Event when the user adds a method to a class.
 *
 * Parameters:
 *   aMethod         The method to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVAddMethod( CParsedMethod *aMethod )
{
  CParsedClass *aClass;
  QListViewItem *item;
  CParsedMethod *meth = NULL;
  QString toAdd;
  int atLine = -1;

  // Fetch the current item from the tree.
  item = class_tree->currentItem();

  // Fetch the current class.
  aClass = class_tree->store->getClassByName( item->text(0) );

  // Search for a method with the same export as the one being added.
  for( aClass->methodIterator.toFirst();
       aClass->methodIterator.current() && meth == NULL;
       ++aClass->methodIterator )
  {
    if( aClass->methodIterator.current()->export == aMethod->export )
      meth = aClass->methodIterator.current();
  }

  // Switch to the .h file.
  CVGotoDefinition( item );  

  aMethod->asHeaderCode( toAdd );

  if( meth )
    atLine = meth->definedOnLine + 1;
  else
  {
    switch( aMethod->export )
    {
      case PUBLIC:
        toAdd = "public: // Public methods\n" + toAdd;
        break;
      case PROTECTED:
        toAdd = "protected: // Protected methods\n" + toAdd;
        break;
      case PRIVATE:
        toAdd = "private: // Private methods\n" + toAdd;
        break;
    }

    atLine = aClass->definedOnLine + 2;
  }

  // Add the declaration.
  edit_widget->insertAtLine( toAdd, atLine );
  edit_widget->setCursorPosition( atLine, 0 );
  edit_widget->toggleModified( true );

  // Switch to the .cpp file.
  switchToFile( aClass->implFilename );

  aMethod->asCppCode( toAdd );
  edit_widget->append( toAdd );
  edit_widget->setCursorPosition( edit_widget->lines() - 1, 0 );
  edit_widget->toggleModified( true );
}

/*------------------------------------- CKDevelop::slotCVAddAttribute()
 * slotCVAddAttribute()
 *   Event when the user adds an attribute to a class.
 *
 * Parameters:
 *   aAttr           The attribute to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVAddAttribute( CParsedAttribute *aAttr )
{
  QListViewItem *item;
  CParsedClass *aClass;
  CParsedAttribute *attr = NULL;
  QString toAdd;
  int atLine = -1;
  
  // Fetch the current item.
  item = class_tree->currentItem();

  // Switch to the correct file.
  CVGotoDefinition( item );  

  // Fetch the current class.
  aClass = class_tree->store->getClassByName( item->text(0) );
  
  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current() && attr == NULL;
       ++aClass->attributeIterator )
  {
    if( aClass->attributeIterator.current()->export == aAttr->export )
      attr = aClass->attributeIterator.current();
  }

  // If we find an attribute with the same export we don't need to output
  // the label as well.
  aAttr->asHeaderCode( toAdd );
  if( attr )
    atLine = attr->definedOnLine + 1;
  else
  {
    switch( aAttr->export )
    {
      case PUBLIC:
        toAdd = "public: // Public attributes\n" + toAdd;
        break;
      case PROTECTED:
        toAdd = "protected: // Protected attributes\n" + toAdd;
        break;
      case PRIVATE:
        toAdd = "private: // Private attributes\n" + toAdd;
        break;
    }

    atLine = aClass->definedOnLine + 2;
  }

  edit_widget->insertAtLine( toAdd, atLine );
  edit_widget->setCursorPosition( atLine, 0 );
  edit_widget->toggleModified( true );
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
void CKDevelop::CVGotoDefinition(QListViewItem *item)
{
  CParsedClass *aClass;
  CParsedAttribute *aAttr = NULL;
  CParsedStruct *aStruct;
  QListViewItem *parent;
  QString toFile;
  int idxType;
  int toLine = -1;

  // Get the type of declaration at the index.
  idxType = class_tree->treeH->itemType();
  switch( idxType )
  {
    case THCLASS:
      aClass = class_tree->store->getClassByName( item->text(0) );
      toFile = aClass->hFilename;
      toLine = aClass->definedOnLine;
      break;
    case THSTRUCT:
      aStruct = class_tree->store->getGlobalStructByName( item->text(0) );
      toFile = aStruct->definedInFile;
      toLine = aStruct->definedOnLine;
      break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
    case THGLOBAL_FUNCTION:
    case THGLOBAL_VARIABLE:
    case THPUBLIC_SIGNAL:
    case THPROTECTED_SIGNAL:
    case THPRIVATE_SIGNAL:
      parent = item->parent();
      aClass = class_tree->store->getClassByName( parent->text(0) );
      
      // Fetch the attribute/method.
      if( idxType == THPUBLIC_ATTR || 
          idxType == THPROTECTED_ATTR ||
          idxType == THPRIVATE_ATTR )
      {
        aAttr = aClass->getAttributeByName( item->text(0) );
      }
      else if( idxType == THPUBLIC_METHOD ||
               idxType == THPROTECTED_METHOD ||
               idxType == THPRIVATE_METHOD )
      {
        aAttr = aClass->getMethodByNameAndArg( item->text(0) );
      }
      else if( idxType == THPUBLIC_SLOT || 
               idxType == THPROTECTED_SLOT || 
               idxType == THPRIVATE_SLOT  )
      {
        aAttr = aClass->getSlotByNameAndArg( item->text(0) );
      }
      else if( idxType == THPUBLIC_SIGNAL ||
               idxType == THPROTECTED_SIGNAL ||
               idxType == THPRIVATE_SIGNAL )
        aAttr = aClass->getSignalByNameAndArg( item->text(0) );
      else if( idxType == THGLOBAL_FUNCTION )
      {
        aAttr = class_tree->store->getGlobalFunctionByNameAndArg( item->text(0) );
      }
      else if( idxType == THGLOBAL_VARIABLE )
      {
        aAttr = class_tree->store->getGlobalVarByName( item->text(0) );
      }

      assert( aAttr != NULL);

      toFile = aAttr->definedInFile;
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
void CKDevelop::CVGotoDeclaration(QListViewItem *item)
{
  CParsedClass *aClass;
  CParsedMethod *aMethod = NULL;
  QListViewItem *parent;

  switch( class_tree->treeH->itemType() )
  {
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      parent = item->parent();
      aClass = class_tree->store->getClassByName( parent->text(0) );
      if( aClass )
        aMethod = aClass->getSlotByNameAndArg( item->text(0) );      
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
      parent = item->parent();
      aClass = class_tree->store->getClassByName( parent->text(0) );
      if( aClass )
        aMethod = aClass->getMethodByNameAndArg( item->text(0) );
      break;
    case THGLOBAL_FUNCTION:
      aMethod = class_tree->store->getGlobalFunctionByNameAndArg( item->text(0) );
      break;
    default:
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
  QListBox *lb;
  KCombo* classCombo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);

  // Clear the combos.
  classCombo->clear();
  methodCombo->clear();

  lb = classCombo->listBox();
  // Add all classes.
  for( class_tree->store->classIterator.toFirst(); 
       class_tree->store->classIterator.current(); 
       ++class_tree->store->classIterator )
  {
    aClass = class_tree->store->classIterator.current();

    // Add the class.
    lb->inSort( aClass->name );
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
  QListBox *lb;
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  QString str;

  methodCombo->clear();
  lb = methodCombo->listBox();

  // Add all methods, slots and signals of this class.
  for( aClass->methodIterator.toFirst(); 
       aClass->methodIterator.current();
       ++aClass->methodIterator )
  {
    aClass->methodIterator.current()->toString( str );
    lb->inSort( str );
  }
  
  for( aClass->slotIterator.toFirst(); 
       aClass->slotIterator.current();
       ++aClass->slotIterator )
  {
    aClass->slotIterator.current()->toString( str );
    lb->inSort( str );
  }
}

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
  CParsedClass *aClass;
  CParsedAttribute *aAttr;
  KTreeListItem *item;
  KTreeListItem *parent;
  QString toFile;
  int idxType;
  int toLine = -1;

  if( class_tree->mouseBtn == LeftButton )
  {
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
      switchToFile( toFile );
      edit_widget->setCursorPosition( toLine, 0 );
      edit_widget->setFocus();
    }
  }
}

void CKDevelop::slotClassChoiceCombo(int index)
{
  debug( "slotClassChoiceCombo\n" );
}

void CKDevelop::slotMethodChoiceCombo(int index)
{
  debug( "slotMethodChoiceCombo\n" );
}

void CKDevelop::slotCVViewDeclaration(int index)
{
  debug( "slotCVViewDeclaration\n" );
}

void CKDevelop::slotCVViewDefinition(int index)
{
  debug( "slotCVViewDefinition\n" );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

int CKDevelop::CVGotoClassDecl(QString classname)
{
  debug( "CVGotoClassDecl(" + classname +")\n" );
}

void CKDevelop::CVGotoMethodeImpl(QString classname,QString meth_name)
{
  debug( "CVGotoMethodeImpl(" + classname + "," + meth_name + ")\n" );
}

void CKDevelop::CVGotoClassVarDecl(QString classname,QString var_name)
{
  debug( "CVGotoClassVarDecl(" + classname + "," + var_name + ")\n" );
}

void CKDevelop::refreshClassCombos()
{
  QList<CParsedMethod> *list;
  CParsedClass *aClass;
  CParsedMethod *aMethod;
  KCombo* classCombo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);

  // Clear the combos.
  classCombo->clear();
  methodCombo->clear();

  // Fetch all classes.
  // Add all classes and their methods.
  for( class_tree->store->classIterator.toFirst(); 
       class_tree->store->classIterator.current(); 
       ++class_tree->store->classIterator )
  {
    aClass = class_tree->store->classIterator.current();

    // Add the class.
    classCombo->insertItem( aClass->name );

    // Add all methods, slots and signals of this class.
    list = aClass->getMethods();
    for( aMethod = list->first(); 
         aMethod != NULL;
         aMethod = list->next() )
      methodCombo->insertItem( aMethod->name );

    for( aMethod = aClass->slotList.first(); 
         aMethod != NULL;
         aMethod = aClass->slotList.next() )
      methodCombo->insertItem( aMethod->name );

    for( aMethod = aClass->signalList.first(); 
         aMethod != NULL;
         aMethod = aClass->signalList.next() )
      methodCombo->insertItem( aMethod->name );
  }
}

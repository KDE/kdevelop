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
#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
#include <assert.h>


/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- CKDevelop::slotClassbrowserViewTree()
 * slotClassbrowserViewTree()
 *   Event when the user clicks the graphical view toolbar button.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserViewTree()
{
  class_tree->viewGraphicalTree();
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
  KCombo* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->text( index );

  if ( !classname.isEmpty() )
  {
    aClass = class_tree->store->getClassByName( classname );
    refreshMethodCombo( aClass );
  }
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
  KCombo* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->text( index );

  // Only bother if the methodname is non-empty.
  if( !methodname.isEmpty() )
  {
    // Make sure the next click on the wiz-button switch to declaration.
    cv_decl_or_impl = true;

    // Switch to the method defintin
    CVGotoDefinition( classname, methodname, THPUBLIC_METHOD );
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
void CKDevelop::slotCVViewDeclaration( const char *className, 
                                       const char *declName, 
                                       THType type )
{
  CVGotoDeclaration( className, declName, type );
  CVMethodSelected( declName );
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
void CKDevelop::slotCVViewDefinition(  const char *className, 
                                       const char *declName, 
                                       THType type )
{
  CVGotoDefinition( className, declName, type );
  CVMethodSelected( declName );
}

/*-------------------------------------- CKDevelop::slotCVAddMethod()
 * slotCVAddMethod()
 *   Event when the user adds a method to a class. Brings up a dialog
 *   and lets the user fill it out.
 *
 * Parameters:
 *   aClassName      The class to add the method to.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVAddMethod( const char *aClassName )
{
  CParsedMethod *aMethod;
  CAddClassMethodDlg dlg(this, "methodDlg" );
  
  // Show the dialog and let the user fill it out.
  if( dlg.exec() )
  {
    aMethod = dlg.asSystemObj();
    aMethod->setDeclaredInClass( aClassName );

    slotCVAddMethod( aClassName, aMethod );

    delete aMethod;
  }
}

/*-------------------------------------- CKDevelop::slotCVAddMethod()
 * slotCVAddMethod()
 *   Event when the user adds a method to a class. 
 *
 * Parameters:
 *   aClassName      The class to add the method to.
 *   aMethod         The method to add to the class.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVAddMethod( const char *aClassName, CParsedMethod *aMethod )
{
  CParsedClass *aClass;
  QString toAdd;
  int atLine = -1;
  CParsedMethod *meth = NULL;

  // Fetch the current class.
  aClass = class_tree->store->getClassByName( aClassName );

  if( aMethod->isSignal )     // Signals
  {
    // Search for a signal with the same export as the one being added.
    for( aClass->signalIterator.toFirst();
         aClass->signalIterator.current();
         ++aClass->signalIterator )
    {
      meth = aClass->signalIterator.current();
      if( meth->exportScope == aMethod->exportScope && 
          atLine < meth->declarationEndsOnLine )
        atLine = meth->declarationEndsOnLine;
    }
  }
  else if( aMethod->isSlot )  // Slots
  {
    // Search for a slot with the same export as the one being added.
    for( aClass->slotIterator.toFirst();
         aClass->slotIterator.current();
         ++aClass->slotIterator )
    {
      meth = aClass->slotIterator.current();
      if( meth->exportScope == aMethod->exportScope && 
          atLine < meth->declarationEndsOnLine )
        atLine = meth->declarationEndsOnLine;
    }
  }
  else                        // Methods
  {
    // Search for a method with the same export as the one being added.
    for( aClass->methodIterator.toFirst();
         aClass->methodIterator.current();
         ++aClass->methodIterator )
    {
      meth = aClass->methodIterator.current();
      if( meth->exportScope == aMethod->exportScope && 
          atLine < meth->declarationEndsOnLine )
        atLine = meth->declarationEndsOnLine;
    }
  }

  // Switch to the .h file.
  CVGotoDeclaration( aClass->name, NULL, THCLASS );  

  aMethod->asHeaderCode( toAdd );

  if( atLine == -1 )
  {
    switch( aMethod->exportScope )
    {
      case PIE_PUBLIC:
        toAdd = "public: // Public methods\n" + toAdd;
        break;
      case PIE_PROTECTED:
        toAdd = "protected: // Protected methods\n" + toAdd;
        break;
      case PIE_PRIVATE:
        toAdd = "private: // Private methods\n" + toAdd;
        break;
      default:
        break;
    }

    atLine = aClass->declarationEndsOnLine;
  }

  // Add the declaration.
  edit_widget->insertAtLine( toAdd, atLine );
  edit_widget->setCursorPosition( atLine, 0 );
  edit_widget->toggleModified( true );

  // Switch to the .cpp file.
  switchToFile( aClass->definedInFile );

  // Add the code to the file.
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
void CKDevelop::slotCVAddAttribute( const char *aClassName )
{
  CParsedClass *aClass;
  CParsedAttribute *attr = NULL;
  QString toAdd;
  int atLine = -1;
  CAddClassAttributeDlg dlg(this, "attrDlg" );
  CParsedAttribute *aAttr;

  if( dlg.exec() )
  {
    aAttr = dlg.asSystemObj();

    aAttr->setDeclaredInClass( aClassName );
  }
  else
    return;

  // Fetch the current class.
  aClass = class_tree->store->getClassByName( aClassName );
  
  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current() && attr == NULL;
       ++aClass->attributeIterator )
  {
    if( aClass->attributeIterator.current()->exportScope == aAttr->exportScope )
      attr = aClass->attributeIterator.current();
  }

  // Switch to the .h file.
  CVGotoDefinition( aClass->name, NULL, THCLASS );  

  // If we find an attribute with the same export we don't need to output
  // the label as well.
  aAttr->asHeaderCode( toAdd );
  if( attr )
    atLine = attr->declaredOnLine + 1;
  else
  {
    switch( aAttr->exportScope )
    {
      case PIE_PUBLIC:
        toAdd = "public: // Public attributes\n" + toAdd;
        break;
      case PIE_PROTECTED:
        toAdd = "protected: // Protected attributes\n" + toAdd;
        break;
      case PIE_PRIVATE:
        toAdd = "private: // Private attributes\n" + toAdd;
        break;
      default:
        break;
    }

    atLine = aClass->declarationEndsOnLine;
  }

  // Add the code to the file.
  edit_widget->insertAtLine( toAdd, atLine );
  edit_widget->setCursorPosition( atLine, 0 );
}

/*------------------------------------- CKDevelop::slotCVDeleteMethod()
 * slotCVDeleteMethod()
 *   Event when the user wants to delete a method.
 *
 * Parameters:
 *   aClassName      Name of the class that holds the method. NULL
 *                   for global functions.
 *   aMethod         The method to delete.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotCVDeleteMethod( const char *aClassName,const char *aMethodName )
{
  CParsedClass *aClass;
  CParsedMethod *aMethod;
  int line;

  aClass = class_tree->store->getClassByName( aClassName );

  if( aClass != NULL )
    aMethod = aClass->getMethodByNameAndArg( aMethodName );

  if( aClass != NULL && aMethod != NULL )
  {
    if( KMsgBox::yesNo( this, i18n("Delete method"),
                        i18n("Are you sure you want to delete this method?"),
                        KMsgBox::QUESTION ) == 1 )
    {
      // Start by deleting the declaration.
      switchToFile( aMethod->declaredInFile, aMethod->declaredOnLine );
      edit_widget->deleteInterval( aMethod->declaredOnLine, 
                                   aMethod->declarationEndsOnLine );

      // Comment out the definition.
      switchToFile( aMethod->definedInFile, aMethod->definedOnLine );
      for( line = aMethod->definedOnLine; 
           line <= aMethod->definitionEndsOnLine;
           line++ )
        edit_widget->insertAtLine( i18n("//Del by KDevelop: "), line );
    }
  }
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CKDevelop::CVClassSelected()
 * CVClassSelected()
 *   The class has been selected, make sure the classcombo updates.
 *
 * Parameters:
 *   aName          Name of the class.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVClassSelected( const char *aName )
{
  KCombo* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  bool found = false;
  int i;

  // Only bother if the text has changed.
  if( strcmp( classCombo->currentText(), aName ) != 0 )
  {

    for( i=0; i< classCombo->count() && !found; i++ )
      found = ( strcmp( classCombo->text( i ), aName ) == 0 );
    
    if( found )
    {
      i--;
      classCombo->setCurrentItem( i );
      slotClassChoiceCombo( i );
    }
  }
}

/*-------------------------------------- CKDevelop::CVMethodSelected()
 * CVMethodSelected()
 *   A method has been selected, make sure the methodcombo updates.
 *
 * Parameters:
 *   aName          Name of the method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVMethodSelected( const char *aName )
{
  KCombo* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  bool found = false;
  int i;

  // Only bother if the text has changed.
  for( i=0; i< methodCombo->count() && !found; i++ )
    found = ( strcmp( methodCombo->text( i ), aName ) == 0 );
    
  if( found )
  {
    i--;
    methodCombo->setCurrentItem( i );
  }
}

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
void CKDevelop::CVGotoDefinition( const char *className, 
                                  const char *declName, 
                                  THType type )
{
  CParsedClass *aClass = NULL;
  CParsedMethod *aMethod = NULL;

  // Fetch a class if one is passed.
  if( className != NULL && strlen( className ) > 0 )
  {
    aClass = class_tree->store->getClassByName( className );

    if( aClass != NULL )
      CVClassSelected( className );
  }

  // Get the type of declaration at the index.
  switch( type )
  {
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      if( aClass )
        aMethod = aClass->getSlotByNameAndArg( declName );      
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
      if( aClass )
      {
        aMethod = aClass->getMethodByNameAndArg( declName );

        // If at first we don't succeed...
        if( aMethod == NULL )
          aMethod = aClass->getSlotByNameAndArg( declName ); 
      }
      break;
    case THGLOBAL_FUNCTION:
      aMethod = class_tree->store->globalContainer.getMethodByNameAndArg( declName );
      break;
    default:
      debug( "Unknown type %d in CVGotoDefinition.", type );
  }

  if( aMethod )
    switchToFile( aMethod->definedInFile, aMethod->definedOnLine );
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
void CKDevelop::CVGotoDeclaration( const char *className, 
                                   const char *declName, 
                                   THType type )
{
  CParsedClass *aClass = NULL;
  CParsedAttribute *aAttr = NULL;
  CParsedStruct *aStruct = NULL;
  QString toFile;
  int toLine = -1;

  if( className != NULL && strlen( className ) > 0 )
  {
    aClass = class_tree->store->getClassByName( className );

    if( aClass != NULL )
      CVClassSelected( className );
  }

  switch( type )
  {
    case THCLASS:
      toFile = aClass->declaredInFile;
      toLine = aClass->declaredOnLine;
      break;
    case THSTRUCT:
      if( aClass != NULL )
        aClass->getStructByName( declName );
      else
        aStruct = class_tree->store->globalContainer.getStructByName( declName );

      toFile = aStruct->declaredInFile;
      toLine = aStruct->declaredOnLine;
      break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
      if( aClass != NULL )
        aAttr = aClass->getAttributeByName( declName );
      else
      {
        aStruct = class_tree->store->globalContainer.getStructByName( className );
        if( aStruct != NULL )
          aAttr = aStruct->getAttributeByName( declName );
      }
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
      aAttr = aClass->getMethodByNameAndArg( declName );

      // If at first we don't succeed...
      if( aAttr == NULL )
        aAttr = aClass->getSlotByNameAndArg( declName );      
      break;
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      aAttr = aClass->getSlotByNameAndArg( declName );
      break;
    case THPUBLIC_SIGNAL:
    case THPROTECTED_SIGNAL:
    case THPRIVATE_SIGNAL:
      aAttr = aClass->getSignalByNameAndArg( declName );
      break;
    case THGLOBAL_FUNCTION:
      aAttr = class_tree->store->globalContainer.getMethodByNameAndArg( declName );
      break;
    case THGLOBAL_VARIABLE:
      aAttr = class_tree->store->globalContainer.getAttributeByName( declName );
      break;
    default:
      debug( "Unknown type %d in CVGotoDeclaration.", type );
      break;
  }
  
  // Fetch the line and file from the attribute if the value is set.
  if( aAttr != NULL )
  {
    toFile = aAttr->declaredInFile;
    toLine = aAttr->declaredOnLine;
  }

  if( toLine != -1 )
  {
    debug( "  Switching to file %s @ line %d", toFile.data(), toLine );
    switchToFile( toFile, toLine );
  }
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
  QList<CParsedClass> *classList;
  KCombo* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KCombo* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString savedClass;
  int savedIdx = -1;
  int i;

  savedClass = classCombo->currentText();

  // Clear the combos.
  classCombo->clear();

  // Add all classes.
  classList = class_tree->store->getSortedClassList();
  for( aClass = classList->first(),i=0;
       aClass != NULL;
       aClass = classList->next(), i++ )
  {
    classCombo->insertItem( aClass->name );
    if( aClass->name == savedClass )
      savedIdx = i;
  }
  delete classList;

  // Update the method combo with the class from the classcombo.
  aClass = class_tree->store->getClassByName( savedClass );
  if( aClass && savedIdx != -1 )
  {
    classCombo->setCurrentItem( savedIdx );
    refreshMethodCombo( aClass );
  }
  else
    methodCombo->clear();
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
  KCombo* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString str;
  QString savedMethod;

  // Save the current value.
  savedMethod = methodCombo->currentText();

  methodCombo->clear();
  lb = methodCombo->listBox();
  lb->setAutoUpdate( false );

  // Add all methods, slots and signals of this class.
  for( aClass->methodIterator.toFirst(); 
       aClass->methodIterator.current();
       ++aClass->methodIterator )
  {
    aClass->methodIterator.current()->asString( str );
    lb->inSort( str );
  }
  
  for( aClass->slotIterator.toFirst(); 
       aClass->slotIterator.current();
       ++aClass->slotIterator )
  {
    aClass->slotIterator.current()->asString( str );
    lb->inSort( str );
  }

  lb->setAutoUpdate( true );

  // Try to restore the saved value.
  for(int i=0; i<methodCombo->count(); i++ )
  {
    if( savedMethod == methodCombo->text( i ) )
      methodCombo->setCurrentItem( i );
  }
}

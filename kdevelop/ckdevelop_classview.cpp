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
#include "api.h"
#include "cclassview.h"
#include "caddclassmethoddlg.h"
#include "caddclassattributedlg.h"
//#include <ceditwidget.h>
#include "programmingbycontract.h"
#include "docviewman.h"

#include <kate/document.h>
#include <kate/view.h>

#include <qlistbox.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include "wzconnectdlgimpl.h"

using namespace std;

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
void CKDevelop::slotClassChoiceCombo(const QString& text)
{
  ParsedClass *aClass=NULL;
  aClass = API::getInstance()->classStore()->getClassByName( text );

  if ( (!text.isEmpty()) && aClass)
  {
    CVRefreshMethodCombo( aClass );
  }
  if(text==i18n("(Globals)"))
  {
    CVRefreshMethodCombo( aClass );
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
void CKDevelop::slotMethodChoiceCombo(const QString& text)
{
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  ParsedClass *aClass=NULL;
  aClass = API::getInstance()->classStore()->getClassByName(classname );

  if(classCombo->currentText()==i18n("(Globals)"))
  {
    cv_decl_or_impl = true;
    CVGotoDefinition( classname, text, THFOLDER, THGLOBAL_FUNCTION );
  }

  // Only bother if the methodname is non-empty.
  if( (!text.isEmpty()) && aClass)
  {
    // Make sure the next click on the wiz-button switch to declaration.
    cv_decl_or_impl = true;

    // Switch to the method defintion
    CVGotoDefinition( classname, text, THCLASS, THPUBLIC_METHOD );
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
void CKDevelop::slotCVViewDeclaration( const char *parentPath,
                                       const char *itemName,
                                       THType parentType,
                                       THType itemType )
{
  REQUIRE( "Valid parent path", parentPath != NULL );
  REQUIRE( "Valid item name", itemName != NULL );

  CVGotoDeclaration( parentPath, itemName, parentType, itemType );
  CVMethodSelected( itemName );
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
void CKDevelop::slotCVViewDefinition( const char *parentPath,
                                      const char *itemName,
                                      THType parentType,
                                      THType itemType )
{
  REQUIRE( "Valid parent path", parentPath != NULL );
  REQUIRE( "Valid item name", itemName != NULL );

  CVGotoDefinition( parentPath, itemName, parentType, itemType );
  CVMethodSelected( itemName );
}

/*---------------------------- CClassView::slotViewClassDeclaration()
 * viewClassDefinition()
 *   Views a declaration of a specified class.
 *
 * Parameters:
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotSwitchToFile(const QString & toFile, int toLine)
{
  /* this can not be checked here
   * should be done outside the slot
   * before calling, so its gone
   * (rokrau 05/10/01)
   */
  //if (!toFile.isEmpty() && toLine!=-1)
  //{
    switchToFile( toFile, toLine );
  //}
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
  REQUIRE( "Valid class name", aClassName != NULL );

  CAddClassMethodDlg dlg( class_tree, this, "methodDlg");

  if (bAutosave)
    saveTimer->stop();
  // Show the dialog and let the user fill it out.
  if( dlg.exec() )
  {
    ParsedMethod *aMethod = dlg.asSystemObj();
    aMethod->setDeclaredInScope( aClassName );

    slotCVAddMethod( aClassName, aMethod );

    delete aMethod;
  }
  if (bAutosave)
    saveTimer->start(saveTimeout);
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
void CKDevelop::slotCVAddMethod( const char *aClassName,
                                 ParsedMethod *aMethod )
{
  REQUIRE( "Valid class name", aClassName != NULL );
  REQUIRE( "Valid method", aMethod != NULL );

  ParsedClass *aClass;
  QString toAdd;
  QString headerCode;
  int atLine = -1;
  ParsedMethod *meth = NULL;

  // Fetch the current class.
  aClass = API::getInstance()->classStore()->getClassByName( aClassName );
  REQUIRE( "Valid class", aClass != NULL );

  if( aMethod->isSignal() )     // Signals
  {
    // Search for a signal with the same export as the one being added.
    for( aClass->signalIterator.toFirst();
         aClass->signalIterator.current();
         ++aClass->signalIterator )
    {
      meth = aClass->signalIterator.current();
      if( meth->access() == aMethod->access() &&
          atLine < meth->declarationEndsOnLine() )
        atLine = meth->declarationEndsOnLine();
    }
  }
  else if( aMethod->isSlot() )  // Slots
  {
    // Search for a slot with the same export as the one being added.
    for( aClass->slotIterator.toFirst();
         aClass->slotIterator.current();
         ++aClass->slotIterator )
    {
      meth = aClass->slotIterator.current();
      if( meth->access() == aMethod->access() &&
          atLine < meth->declarationEndsOnLine() )
        atLine = meth->declarationEndsOnLine();
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
      if( meth->access() == aMethod->access() &&
          atLine < meth->declarationEndsOnLine() )
        atLine = meth->declarationEndsOnLine();
    }
  }

  // Switch to the .h file.
  CVGotoDeclaration( aClassName, "", THCLASS, THCLASS );

//  aMethod->asHeaderCode( headerCode ); ROBE

  if( atLine == -1 )
  {
    if( aMethod->isSignal() )
      toAdd = "signals: // Signals\n" + headerCode;
    else // Methods and slots
    {
      switch( aMethod->access() )
      {
        case PIE_PUBLIC:
          toAdd.sprintf( "public%s: // Public %s\n%s",
                         aMethod->isSlot() ? " slots" : "",
                         aMethod->isSlot() ? "slots" : "methods",
                         headerCode.data() );
          break;
        case PIE_PROTECTED:
          toAdd.sprintf( "protected%s: // Protected %s\n%s",
                         aMethod->isSlot() ? " slots" : "",
                         aMethod->isSlot() ? "slots" : "methods",
                         headerCode.data() );
          break;
        case PIE_PRIVATE:
          toAdd.sprintf( "private%s: // Private %s\n%s",
                         aMethod->isSlot() ? " slots" : "",
                         aMethod->isSlot() ? "slots" : "methods",
                         headerCode.data() );
          break;
        default:
          break;
      }
    }

    atLine = aClass->declarationEndsOnLine();
  }
  else
  {
    toAdd = headerCode;
    atLine++;
  }

  // Add the declaration.
  Kate::View* pView = m_docViewManager->currentEditView();
  Kate::Document* pDoc = pView->getDoc();
  pDoc->insertLine(atLine,toAdd);
  pView->setCursorPosition( atLine, 0 );
  pDoc->setModified( true );

  // Get the code for the .cpp file.
//  aMethod->asCppCode( toAdd ); ROBE

  // Switch to the .cpp file and add the code if some code was generated.
  if( !toAdd.isEmpty() )
  {
    switchToFile( aClass->definedInFile() );
    QString extra = CClassPropertiesDlgImpl::CppCodeExtra;
//	 aMethod->asCppCode( toAdd ); ROBE
	 kdDebug() << "addmethod: toadd=" << toAdd .data() << endl;
	 if( !extra.isEmpty() )
	 {
	    int ix;
	    if ( ( ix = toAdd.find('{')) != -1 )
	        toAdd.insert(ix+1, extra);
	 }
	 //int pos = toAdd.find( aMethod -> name );
    //kdDebug() << aMethod -> name.data() << " 's pos=" << pos << endl;
	 //if(pos > -1 ) toAdd.insert(pos, QString(QString(aClassName)+"::"));
	 //kdDebug() << "fixed cpp code : " << toAdd.data() << endl;
    // Add the code to the file.

    Kate::View* pView = m_docViewManager->currentEditView();
    Kate::Document* pDoc = pView->getDoc();
    pDoc->insertLine( -1 , toAdd );
    pView->setCursorPosition( pDoc->numLines()-1 , 0 );
    pDoc->setModified( true );
  }
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
  REQUIRE( "Valid class name", aClassName != NULL );

  ParsedClass *aClass;
  ParsedAttribute *attr = NULL;
  QString toAdd;
  int atLine = -1;
  CAddClassAttributeDlg dlg(this, "attrDlg" );
  ParsedAttribute *aAttr;

  if (bAutosave)
    saveTimer->stop();

  if( !dlg.exec() )
    return;

  aAttr = dlg.asSystemObj();
  aAttr->setDeclaredInScope( aClassName );

  if (bAutosave)
    saveTimer->start(saveTimeout);

  // Fetch the current class.
  aClass = API::getInstance()->classStore()->getClassByName( aClassName );
  REQUIRE( "Valid class", aClass != NULL );

  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current();
       ++aClass->attributeIterator )
  {
    attr = aClass->attributeIterator.current();
    if( attr->access() == aAttr->access() &&
        atLine < attr->declarationEndsOnLine() )
      atLine = attr->declarationEndsOnLine() + 1;
  }

  // Switch to the .h file.
  CVGotoDeclaration( aClass->name(), "", THCLASS, THCLASS );

  // Get the code for the new attribute
//  aAttr->asHeaderCode( toAdd );

  // If we found an attribute with the same export we don't need to output
  // the label as well.
  if( atLine == -1 )
  {
    switch( aAttr->access() )
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

    atLine = aClass->declarationEndsOnLine();
  }

  // Add the code to the file.
  Kate::View* pView = m_docViewManager->currentEditView();
  Kate::Document* pDoc = pView->getDoc();
  pDoc->insertLine( atLine , toAdd );
  pView->setCursorPosition( atLine, 0 );

  // Delete the genererated attribute
  delete aAttr;
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
void CKDevelop::slotCVAddAttribute( const char *aClassName, ParsedAttribute* aAttr )
{
  REQUIRE( "Valid class name", aClassName != NULL );

  ParsedClass *aClass;
  ParsedAttribute *attr = NULL;
  QString toAdd;
  int atLine = -1;
/*
  CAddClassAttributeDlg dlg(this, "attrDlg" );
  ParsedAttribute *aAttr;
*/
  if (bAutosave)
    saveTimer->stop();

  aAttr->setDeclaredInScope( aClassName );

  if (bAutosave)
    saveTimer->start(saveTimeout);

  // Fetch the current class.
  aClass = API::getInstance()->classStore()->getClassByName( aClassName );
  REQUIRE( "Valid class", aClass != NULL );

  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current();
       ++aClass->attributeIterator )
  {
    attr = aClass->attributeIterator.current();
    if( attr->access() == aAttr->access() &&
        atLine < attr->declarationEndsOnLine() )
      atLine = attr->declarationEndsOnLine() + 1;
  }

  // Switch to the .h file.
  CVGotoDeclaration( aClass->name(), "", THCLASS, THCLASS );

  // Get the code for the new attribute
//  aAttr->asHeaderCode( toAdd ); ROBE

  // If we found an attribute with the same export we don't need to output
  // the label as well.
  if( atLine == -1 )
  {
    switch( aAttr->access() )
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

    atLine = aClass->declarationEndsOnLine();
  }

  // Add the code to the file.
  Kate::View* pView = m_docViewManager->currentEditView();
  Kate::Document* pDoc = pView->getDoc();
  pDoc->insertLine( atLine , toAdd );
  pView->setCursorPosition( atLine, 0 );
  slotFileSave();
  if( !aAttr -> isStatic() )
  {
    delete aAttr;
    return;
  }
  /* We have a static attribute member... Thus I added the code below
     to instanciate the attribute into the cpp file.
     - First I have to find a place to add the code.
     - I choose to find the position at the first method...
  */
  ParsedMethod *aMethod;
  QValueList <ParsedMethod*>  lMethod = aClass -> getSortedMethodList();
  QString File="";
  int n, Line = 32767;

  QValueList<ParsedMethod*>::ConstIterator lMethodIt;
  for (lMethodIt = lMethod.begin(); lMethodIt != lMethod.end(); ++lMethodIt)
  {
    aMethod = *lMethodIt;
    kdDebug() << "in file " << aMethod -> definedInFile().data() << endl;
    if( ( aMethod -> definedInFile().find(".h")) == -1)
    {
      File = aMethod -> definedInFile();
      if ( ((n=aMethod -> definedOnLine()) < Line) && ( n > 0) )
      Line = n;
    }
  }
  if( Line != 32767 )
  {
    kdDebug() << "found a place " << File.data() << " at line# " << Line << endl;
    if( !File.isEmpty() && (Line != 32767) )
    {
        if(Line <= 2) Line = 2;
        Line -= 1;
        switchToFile( File, Line );
        toAdd = aAttr -> type() + " " + aClass -> name() + "::" + aAttr -> name() + ";\n";
        pDoc->insertLine( atLine , toAdd );
        pView->setCursorPosition( atLine, 0 );
    }
  }

  // Delete the genererated attribute
  delete aAttr;
}

/**  */
void CKDevelop::slotCVSigSlotMapImplement ( ParsedClass* aClass, const QString& toAdd, ParsedMethod* implMethod )
{
    if ( implMethod == NULL || aClass == NULL ) return;
    int atLine = implMethod -> definitionEndsOnLine() ;
    if ( atLine ==-1 )
    {
        kdDebug() << "Line# not stored in implement method! Aborting SigSlotMapImplement!" << endl;
        return;
    }
   QString str = "\t" + toAdd + "\n";
    switchToFile ( aClass -> definedInFile() );
    Kate::View* pView = m_docViewManager->currentEditView();
    Kate::Document* pDoc = pView->getDoc();
    pDoc->insertLine( atLine , toAdd );
    pView->setCursorPosition( atLine, 0 );
    slotFileSave();
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
void CKDevelop::slotCVDeleteMethod( const char *aClassName,
                                    const char *aMethodName )
{
  ParsedClass *aClass;
  ParsedMethod *aMethod;
  int line;

  aClass = API::getInstance()->classStore()->getClassByName( aClassName );
  REQUIRE( "Valid class", aClass != NULL );

  if( aClass != NULL )
  {
    aMethod = aClass->getMethodByNameAndArg( aMethodName );

    // If we don't find the method, we try to find a slot with the same name.
    if( aMethod == NULL )
      aMethod = aClass->getSlotByNameAndArg( aMethodName );

    // If it isn't a method or a slot we go for a signal.
    if( aMethod == NULL )
      aMethod = aClass->getSignalByNameAndArg( aMethodName );

    // If everything fails notify the user.
    if( aMethod != NULL )
    {
      if( KMessageBox::questionYesNo( this,
                          i18n("Are you sure you want to delete this method?"),
                          i18n("Delete method")) == KMessageBox::Yes )
      {
        // Start by deleting the declaration.
        switchToFile( aMethod->declaredInFile(), aMethod->declaredOnLine() );
        Kate::View* pView = m_docViewManager->currentEditView();
        Kate::Document* pDoc = pView->getDoc();
        for (int il=aMethod->declaredOnLine(); il<aMethod->declarationEndsOnLine(); ++il)
        {
          pDoc->removeLine( il );
        }
        // Comment out the definition if it isn't a signal.
        if( !aMethod->isSignal() )
        {
          switchToFile( aMethod->definedInFile(), aMethod->definedOnLine() );
          for( line = aMethod->definedOnLine();
               line <= aMethod->definitionEndsOnLine();
               line++ )
            pDoc->insertLine( line , i18n("//Del by KDevelop: ") );
        }
      }
    }
    else
        KMessageBox::error( NULL, i18n( "Couldn't find the method to delete." ),
                                i18n( "Method missing" ) );
  }
  else
    KMessageBox::error( NULL, i18n( "Couldn't find the class which has the method to delete." ),
                                i18n( "Class missing" ) );

}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
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
      slotClassChoiceCombo( classCombo->currentText() );
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
  KComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
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
void CKDevelop::CVGotoDefinition( const char *parentPath,
                                  const char *itemName,
                                  THType parentType,
                                  THType itemType )
{
  ParsedContainer *aContainer = NULL;
  ParsedMethod *aMethod = NULL;
  ParsedAttribute *aAttr = NULL;
  QString toFile;
  int toLine = -1;
  // to get around the method combo selection, we change the type to attribute if there is one of that name
  aContainer = CVGetContainer( parentPath, parentType );
  if(!aContainer){
    itemType=THGLOBAL_FUNCTION;
  }

  // Get the type of declaration at the index.
  switch( itemType )
  {
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      if( aContainer )
        aMethod = ((ParsedClass *)aContainer)->getSlotByNameAndArg( itemName );
      if(aMethod)
        break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
      if( aContainer ){
          aMethod = aContainer->getMethodByNameAndArg( itemName );
        if(!aMethod)
          aMethod = ((ParsedClass *)aContainer)->getSlotByNameAndArg( itemName );
        if(!aMethod)
          aMethod = ((ParsedClass *)aContainer)->getSignalByNameAndArg( itemName );
      }
      break;
    case THGLOBAL_FUNCTION:
      aMethod = API::getInstance()->classStore()->globalContainer.getMethodByNameAndArg( itemName );
      break;
    case THCLASS:
      switchToFile( aContainer->definedInFile(), aContainer->definedOnLine() );
      return;
    default:
      debug( "Unknown type %d in CVGotoDefinition.", itemType );
  }
  if( aMethod ){
    switchToFile( aMethod->definedInFile(), aMethod->definedOnLine() );
    return;
  }
  // now a new switch for the attributes
  else if(aContainer && aContainer->hasAttribute(itemName) ){
    aAttr = aContainer->getAttributeByName( itemName );
  }
  else{
    CVGotoDeclaration( parentPath,itemName,parentType,itemType );
    debug( "Couldn't find method %s::%s", parentPath, itemName );
    return;
  }

  // Fetch the line and file from the attribute if the value is set.
  if( aAttr )
  {
    toFile = aAttr->declaredInFile();
    toLine = aAttr->declaredOnLine();
    if( toLine != -1 )
    {
      debug( "  Switching to file %s @ line %d", toFile.data(), toLine );
      switchToFile( toFile, toLine );
    }
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
void CKDevelop::CVGotoDeclaration( const char *parentPath,
                                   const char *itemName,
                                   THType parentType,
                                   THType itemType )
{
  ParsedContainer *aContainer = NULL;
  ParsedAttribute *aAttr = NULL;
  ParsedStruct *aStruct = NULL;
  QString toFile;
  int toLine = -1;

  aContainer = CVGetContainer( parentPath, parentType );
  if(!aContainer){
    itemType=THGLOBAL_FUNCTION;
  }

  switch( itemType )
  {
    case THCLASS:
    case THSTRUCT:
    case THSCOPE:
      if( aContainer != NULL )
      {
        toFile = aContainer->declaredInFile();
        toLine = aContainer->declaredOnLine();
      }
      break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
      if( aContainer != NULL )
        aAttr = aContainer->getAttributeByName( itemName );
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
      if( aContainer != NULL )
        aAttr = aContainer->getMethodByNameAndArg( itemName );
      if(!aAttr && aContainer !=NULL)
        aAttr = ((ParsedClass *)aContainer)->getSlotByNameAndArg( itemName );
      if(!aAttr && aContainer != NULL )
        aAttr = ((ParsedClass *)aContainer)->getSignalByNameAndArg( itemName );
      break;
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      if( aContainer != NULL )
        aAttr = ((ParsedClass *)aContainer)->getSlotByNameAndArg( itemName );
      break;
    case THSIGNAL:
      if( aContainer != NULL )
        aAttr = ((ParsedClass *)aContainer)->getSignalByNameAndArg( itemName );
      break;
    case THGLOBAL_FUNCTION:
    case THGLOBAL_VARIABLE:
      aAttr = API::getInstance()->classStore()->globalContainer.getMethodByNameAndArg( itemName );
      if(!aAttr)
        aAttr = API::getInstance()->classStore()->globalContainer.getAttributeByName( itemName );
      if(!aAttr)
        aStruct = API::getInstance()->classStore()->globalContainer.getStructByName( itemName );
      break;
    default:
      debug( "Unknown type %d in CVGotoDeclaration.", itemType );
      break;
  }

  // Fetch the line and file from the attribute if the value is set.
  if( aAttr != NULL )
  {
    toFile = aAttr->declaredInFile();
    toLine = aAttr->declaredOnLine();
  }
  else if( aStruct != NULL )
  {
    toFile = aStruct->declaredInFile();
    toLine = aStruct->declaredOnLine();
  }

  if( toLine != -1 )
  {
    debug( "  Switching to file %s @ line %d", toFile.data(), toLine );
    switchToFile( toFile, toLine );
  }
}

/*----------------------------------- CKDevelop::CVRefreshClassCombo()
 * CVRefreshClassCombo()
 *   Update the class combo with all classes.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVRefreshClassCombo()
{
  ParsedClass *aClass;
  QValueList<ParsedClass*> classList;
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString savedClass;
  int savedIdx = -1;
  int i;

  savedClass = classCombo->currentText();

  // Clear the combos.
  classCombo->clear();

  // Add all classes.
  classCombo->insertItem(i18n("(Globals)") );
  class_comp->addItem(i18n("(Globals)"));

  classList = API::getInstance()->classStore()->getSortedClassList();
  QValueList<ParsedClass*>::ConstIterator classListIt;
  for (classListIt = classList.begin(), i=0; classListIt != classList.end(); ++classListIt, i++)
  {
    aClass = *classListIt;
    classCombo->insertItem(SmallIcon("CVclass"), aClass->name() );
    class_comp->addItem(aClass->name());
    if( aClass->name() == savedClass )
      savedIdx = i;

  }

  if (!savedClass.isEmpty())
  {

		// Update the method combo with the class from the classcombo.
		aClass = API::getInstance()->classStore()->getClassByName( savedClass );
		if( aClass && savedIdx != -1 )
		{
			classCombo->setCurrentItem( savedIdx );
			CVRefreshMethodCombo( aClass );
			return;
		}

	}
	methodCombo->clear();
}

/*----------------------------------- CKDevelop::CVRefreshMethodCombo()
 * CVRefreshMethodCombo()
 *   Update the method combo with the methods from the selected
 *   class.
 *
 * Parameters:
 *   aClass           The selected class.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::CVRefreshMethodCombo( ParsedClass *aClass )
{
  QListBox *lb;
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString str;
  QString savedMethod;

  // Save the current value.
  if(methodCombo->count() > 0)
    savedMethod = methodCombo->currentText();

  methodCombo->clear();
  lb = methodCombo->listBox();
  lb->setAutoUpdate( false );

  if((classCombo->currentText()==i18n("(Globals)"))){
    QValueList<ParsedMethod*> globalmeth=API::getInstance()->classStore()->globalContainer.getSortedMethodList( );
    QValueList<ParsedMethod*>::ConstIterator globalmethit;
    for (globalmethit = globalmeth.begin(); globalmethit != globalmeth.end(); ++globalmethit)
      {
        str = (*globalmethit)->asString();
        methodCombo->insertItem(SmallIcon("CVglobal_meth"), str );
        method_comp->addItem(str);
      }
    QValueList<ParsedAttribute*> globalattr=API::getInstance()->classStore()->globalContainer.getSortedAttributeList( );
    QValueList<ParsedAttribute*>::ConstIterator  globalattrit;
    for (globalattrit = globalattr.begin(); globalattrit != globalattr.end(); ++globalattrit)
      {
        str=(*globalattrit)->name();
        methodCombo->insertItem(SmallIcon("CVglobal_var"), str );
        method_comp->addItem(str);
      }
    QValueList<ParsedStruct*> globalstruct=API::getInstance()->classStore()->globalContainer.getSortedStructList( );
    QValueList<ParsedStruct*>::ConstIterator globalstructit;
    for (globalstructit = globalstruct.begin(); globalstructit != globalstruct.end(); ++globalstructit)
      {
        str=(*globalstructit)->name();
        methodCombo->insertItem(SmallIcon("CVstruct"), str );
        method_comp->addItem(str);
      }
    lb->sort();
    lb->setAutoUpdate( true );
    return;
  }

  // Add all methods, slots and signals of this class.
  for( aClass->methodIterator.toFirst();
       aClass->methodIterator.current();
       ++aClass->methodIterator )
  {
    str = aClass->methodIterator.current()->asString();
    method_comp->addItem(str);
    if(aClass->methodIterator.current()->isPublic())
      methodCombo->insertItem(SmallIcon("CVpublic_meth"), str );
    if(aClass->methodIterator.current()->isProtected())
      methodCombo->insertItem(SmallIcon("CVprotected_meth"), str );
    if(aClass->methodIterator.current()->isPrivate())
      methodCombo->insertItem(SmallIcon("CVprivate_meth"), str );
  }
  lb->sort();

  for( aClass->slotIterator.toFirst();
       aClass->slotIterator.current();
       ++aClass->slotIterator )
  {
    str = aClass->slotIterator.current()->asString();
    method_comp->addItem(str);
    if(aClass->slotIterator.current()->isPublic())
      methodCombo->insertItem(SmallIcon("CVpublic_slot"), str );
    if(aClass->slotIterator.current()->isProtected())
      methodCombo->insertItem(SmallIcon("CVprotected_slot"), str );
    if(aClass->slotIterator.current()->isPrivate())
      methodCombo->insertItem(SmallIcon("CVprivate_slot"), str );

  }
  for( aClass->signalIterator.toFirst();
       aClass->signalIterator.current();
       ++aClass->signalIterator )
  {
    str = aClass->signalIterator.current()->asString();
    methodCombo->insertItem(SmallIcon("CVpublic_signal"), str );
    method_comp->addItem(str);
  }

  // ADD ATTRIBUTES
  QValueList<ParsedAttribute*> list;
  list = aClass->getSortedAttributeList();
  QValueList<ParsedAttribute*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    str=(*listit)->name();
    method_comp->addItem(str);
    if((*listit)->isPublic())
      methodCombo->insertItem(SmallIcon("CVpublic_var"), str );
    if((*listit)->isProtected())
      methodCombo->insertItem(SmallIcon("CVprotected_var"), str );
    if((*listit)->isPrivate())
      methodCombo->insertItem(SmallIcon("CVprivate_var"), str );
  }
  lb->sort();
  lb->setAutoUpdate( true );

  // Try to restore the saved value.
  for(int i=0; i<methodCombo->count(); i++ )
  {
    if( savedMethod == methodCombo->text( i ) )
      methodCombo->setCurrentItem( i );
  }
}

/*----------------------------------- CKDevelop::CVRefreshMethodCombo()
 * CVRefreshMethodCombo()
 *   Returns the class for the supplied classname.
 *
 * Parameters:
 *   className        Name of the class to fetch.
 *
 * Returns:
 *   Pointer to the class or NULL if not found.
 *-----------------------------------------------------------------*/
ParsedContainer *CKDevelop::CVGetContainer( const char *containerPath,
                                             THType containerType )
{
  REQUIRE1( "Valid container path", containerPath != NULL, NULL );
  REQUIRE1( "Valid container path length", strlen( containerPath ) > 0, NULL );

  ParsedContainer *aContainer;

  switch( containerType )
  {
    case THCLASS:
      // Try to fetch the class.
      aContainer = API::getInstance()->classStore()->getClassByName( containerPath );

      // If we found the class and it isn't a subclass we update the combo.
      if( aContainer != NULL && aContainer->declaredInScope().isEmpty() )
        CVClassSelected( containerPath );
      break;
    case THSTRUCT:
      aContainer = API::getInstance()->classStore()->globalContainer.getStructByName( containerPath );
      break;
    case THSCOPE:
      aContainer = API::getInstance()->classStore()->globalContainer.getScopeByName( containerPath );
      break;
    default:
      debug( "Didn't find class/struct/scope %s[%d]", containerPath, containerType );
      aContainer = NULL;
      break;
  }

  return aContainer;
}

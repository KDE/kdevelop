/***************************************************************************
                          cclasstooltreeview.cpp  -  implementation
                             -------------------
    begin                : Fri May 23 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
 ***************************************************************************/
 
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "cclasstooltreeview.h"
#include "cclasstreehandler.h"
#include <assert.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
 
/*-------------------------- CClassToolTreeView::CClassToolTreeView()
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
CClassToolTreeView::CClassToolTreeView(QWidget* parent /* = 0 */,const char* name /* = 0 */)
  : CTreeView (parent, name)                                                  
{
  // Create the popupmenus.
  initPopups();

  setTreeHandler( new CClassTreeHandler() );

  definitionCmd = NULL;
  declarationCmd = NULL;
}

/*----------------------------------- CClassToolTreeView::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolTreeView::initPopups()
{
  // Class popup
  classPopup.setTitle( i18n("Class"));
  classPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()));

  // Method popup
  methodPopup.setTitle( i18n( "Method" ) );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()));
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration())); 

  // Attribute popup
  attributePopup.setTitle( i18n( "Attribute" ) );
  attributePopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()));
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CClassToolTreeView::setDefinitionCmd()
 * setDefinitionCmd()
 *   Set the definition command.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolTreeView::setDefinitionCmd( CCommand *aCmd )
{
  assert( aCmd != NULL );

  definitionCmd = aCmd;
}

/*------------------------------ CClassToolTreeView::setDeclarationCmd()
 * setDeclarationCmd()
 *   Set the declaration command.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassToolTreeView::setDeclarationCmd( CCommand *aCmd )
{
  assert( aCmd != NULL );

  declarationCmd = aCmd;
}

/*********************************************************************
 *                                                                   *
 *                        PROTECTED QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CClassToolTreeView::getCurrentPopup()
 * getCurrentPopup()
 *   Get the current popupmenu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
KPopupMenu *CClassToolTreeView::getCurrentPopup()
{
  KPopupMenu *popup = NULL;
 
  switch( treeH->itemType() )
  {
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

void CClassToolTreeView::slotViewDefinition() 
{
  const char *className;
  const char *declName;
  THType type;

  ((CClassTreeHandler *)(treeH))->getCurrentNames( &className, &declName, &type );

  emit signalViewDefinition( className, declName, type );
}

void CClassToolTreeView::slotViewDeclaration()
{
  const char *className;
  const char *declName;
  THType type;

  if( treeH->itemType() == THCLASS )
  {
    className = currentItem()->text(0);
    emit signalViewDeclaration( className, className, THCLASS );
  }
  else
  {
    ((CClassTreeHandler *)(treeH))->getCurrentNames( &className, &declName, &type );

    emit signalViewDeclaration( className, declName, type );
  }
}

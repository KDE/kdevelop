/***************************************************************************
               cclasstooltreeview.h  -  description

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
#ifndef _CCLASSTOOLTREEVIEW_H_INCLUDED
#define _CCLASSTOOLTREEVIEW_H_INCLUDED

#include "ctreeview.h"
#include "cproject.h"
#include "ccommand.h"

class CClassToolTreeView : public CTreeView
{
  Q_OBJECT

public: // Constructor and Destructor

  CClassToolTreeView(QWidget* parent = 0,const char* name = 0);
  ~CClassToolTreeView() {}

public: // Public methods

  /** Set the definition command. */
  void setDefinitionCmd( CCommand *aCmd );

  /** Set the declaration command. */
  void setDeclarationCmd( CCommand *aCmd );

protected: // Implementations of virtual methods.
 
  /** Initialize popupmenus. */
  void initPopups();
 
  /** Get the current popupmenu. */
  KPopupMenu *getCurrentPopup();                                               

  /** Refresh this view using the current project. */
  void refresh( CProject *proj ) {}

private: // Private attributes

  /** Command to execute to view a definition. */
  CCommand *definitionCmd;

  /** Command to execute to view a declaration. */
  CCommand *declarationCmd;

private: // Popupmenus
 
  /** Popupmenu for classes. */
  KPopupMenu classPopup;
 
  /** Popupmenu for methods. */
  KPopupMenu methodPopup;
 
  /** Popupmenu for attributes. */
  KPopupMenu attributePopup;                                                   

protected slots:
  void slotViewDefinition();
  void slotViewDeclaration();
};

#endif

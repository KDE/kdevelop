/***************************************************************************
                cctviewdefinitioncmd.h  -  description
                             -------------------
    begin                : Sun May 23 1999
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

#ifndef _CCTVIEWDECLARATIONCMD_H_INCLUDED
#define _CCTVIEWDECLARATIONCMD_H_INCLUDED

#include "ccommand.h"

class CClassView;

class CCTViewDeclarationCmd : public CCommand
{
public: // Constructor & destructor
  CCTViewDeclarationCmd();
  virtual ~CCTViewDeclarationCmd();

public: // Methods to set attribute values.

  /** Set a pointer to the classview. */
  void setClassView( CClassView *aView ) { view = aView; }

public: // Implementation of virtual methods.

  /** Views a definition of an item. */
  void execute( void *anArg );

private: // Private attributes

  /** The classview. */
  CClassView *view;
};

#include "cclassview.h"

#endif

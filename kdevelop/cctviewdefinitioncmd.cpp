/***************************************************************************
                cctviewdefinitioncmd.cpp  -  implementation
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
#include "cctviewdefinitioncmd.h"
#include "cclasstooldlg.h"
#include <assert.h>

CCTViewDefinitionCmd::CCTViewDefinitionCmd()
{
  view = NULL;
}

CCTViewDefinitionCmd::~CCTViewDefinitionCmd()
{
}

void CCTViewDefinitionCmd::execute( void *anArg )
{
  assert( anArg != NULL );
  assert( view != NULL );

  CClassToolTreeView *treeView;
  const char *className;
  const char *declName;
  THType type;

  treeView = (CClassToolTreeView *)anArg;

  ((CClassTreeHandler *)(treeView->treeH))->getCurrentNames( &className, &declName, &type );
  view->viewDefinition( className, declName, type );
}

/***************************************************************************
                          ckdevelop_kdlgedinit.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


// This file contains the construction of the kdialogedit views including menubar and toolbar
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qsplitter.h>
#include <kmenubar.h>
#include "ckdevelop.h"
#include "componentmanager.h"

#include "./kwrite/kwview.h"
#include "./kwrite/kguicommand.h"

extern KGuiCmdManager cmdMngr;



  


/***************************************************************************
                          cctags.cpp  -  description
                             -------------------
    begin                : Wed Feb 21 2001
    copyright            : (C) 2001 by kdevelop-team
    email                : kdevelop-team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cctags.h"

CCtags::CCtags(){
    debug("in ctags constructor\n");
}
CCtags::~CCtags(){
}
/** Create a tags file for the current project. The file is stored in the project parent 
directory. Later this should be configurable.  */
void CCtags::make_tags(){
}
/** Open, read and parse the tags file. */
void CCtags::load_tags(){
}

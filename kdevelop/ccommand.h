/***************************************************************************
                          ccommand.h  -  description
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
#ifndef _CCOMMAND_H_INCLUDED
#define _CCOMMAND_H_INCLUDED

class CCommand
{
public: // The execute method

  /** This method is executed whenever the callback is executed. */
  virtual void execute( void *anArg ) = 0;
};

#endif

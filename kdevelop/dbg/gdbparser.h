/***************************************************************************
                          gdbparser.h  -  description                              
                             -------------------                                         
    begin                : Tue Aug 17 1999                                           
    copyright            : (C) 1999 by John Birch                         
    email                : jb.nz@writeme.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef GDBPARSER_H
#define GDBPARSER_H

#include "vartree.h"

/**
  *@author John Birch
  */

class GDBParser
{
public:
	GDBParser();
	~GDBParser();

  void parseData(TrimmableItem* parent, char* buf,
                      bool requested, bool params);
  DataType  determineType(char* buf) const;

  char* skipString(char* buf) const;
  char* skipQuotes(char* buf, char quote) const;
  char* skipDelim(char* buf, char open, char close) const;

private:
  TrimmableItem* getItem(TrimmableItem* parent, DataType itemType,
                          const QString& varName, bool requested);

  void parseArray(TrimmableItem* parent, const QString& value);

  char*     skipTokenEnd(char* buf) const;
  char*     skipTokenValue(char* buf) const;
  char*     skipNextTokenStart(char* buf) const;

  QString getName(char** buf);
  QString getValue(char** buf, bool requested);
  void setItem(TrimmableItem* parent, const QString& varName, DataType dataType,
                const QString& value, bool requested, bool params);
};

#endif

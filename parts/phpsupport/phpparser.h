/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann,Sandy Meier                      *
 *   bernd@kdevelop.org,smeier@kdevelop.org                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PHPPARSER_H
#define PHPPARSER_H

#include <qstring.h>
#include <qstringlist.h>
class KDevCore;
class ClassStore;
/**
  *@author Sandy Meier, Bernd Gehrmann
  */

class PHPParser {
public: 
	PHPParser(KDevCore* core,ClassStore* store);
	~PHPParser();
	void parseFile(const QString& fileName);
	void parseLines(QStringList* lines,const QString& fileName);

 private:
  ClassStore* m_classStore;
  KDevCore* m_core;
};

#endif

/***************************************************************************
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "phpparser.h"

#include <kdevcore.h>
#include <codemodel.h>

#include <qregexp.h>
#include <kdebug.h>

#include <qfileinfo.h>
#include <qtextstream.h>

#include <iostream>

using namespace std;

PHPParser::PHPParser(KDevCore* core,CodeModel* model){
  m_core = core;
  m_model = model;
}
PHPParser::~PHPParser(){
}
void PHPParser::parseLines(QStringList* lines,const QString& fileName){
  kdDebug(9018) << "enter parsedLines" << endl;
  QRegExp classre("^[ \t]*class[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*(extends[ \t]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*))?.*$");
  QRegExp methodre("^[ \t]*function[ \t&]*([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*\\(([a-zA-Z_\x7f-\xff]*[0-9A-Za-z_\x7f-\xff\\$\\, \t=&\\'\\\"]*)\\).*$");
  QRegExp varre("^[ \t]*var[ \t]*\\$([a-zA-Z_\x7f-\xff][0-9A-Za-z_\x7f-\xff]*)[ \t;=].*$");
  QRegExp createMemberRe("\\$this->([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)[ \t]*=[ \t&]*new[ \t]+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)");

  ClassDom lastClass = 0;
  QString rawline;
  QCString line;
  int lineNo = 0;
  int bracketOpen = 0;
  int bracketClose = 0;
  bool inClass = false;

  for ( QStringList::Iterator it = lines->begin(); it != lines->end(); ++it ) {
    line = (*it).local8Bit();
    if(!line.isNull()){
      //    cerr << "LINE" << line << endl;
      bracketOpen += line.contains("{");
      bracketClose += line.contains("}");
      if(bracketOpen == bracketClose && bracketOpen !=0 && bracketClose !=0){
	inClass = false; // ok we are out ouf class
      }
      //cerr << "kdevelop (phpsupport): try match line: " << line << endl;
      if (classre.search(line) != -1 ) {

	//  cerr << "kdevelop (phpsupport): regex match line: " << line << endl;
	inClass= true;
	bracketOpen = line.contains("{");
	bracketClose = line.contains("}");
	lastClass = m_model->create<ClassModel>();
	lastClass->setName(classre.cap(1));

	lastClass->setFileName(fileName);
	lastClass->setStartPosition(lineNo, 0);

	QString parentStr = classre.cap(3);
	if(!parentStr.isEmpty()){
	  lastClass->addBaseClass(parentStr);
	}

	m_file->addClass(lastClass);

      } else if (createMemberRe.search(line) != -1 ) {
	if (lastClass && inClass) {
	    if( lastClass->hasVariable(QString("$") + createMemberRe.cap(1)) )
	       lastClass->variableByName(QString("$") + createMemberRe.cap(1))->setType(createMemberRe.cap(2));
	}
      } else if (methodre.search(line) != -1) {
	//	  cerr << "kdevelop (phpsupport): regex match line ( method ): " << line << endl;
	FunctionDom method = m_model->create<FunctionModel>();
	method->setName(methodre.cap(1));

	ArgumentDom anArg = m_model->create<ArgumentModel>();
	QString arguments = methodre.cap(2);
	anArg->setType(arguments.stripWhiteSpace().local8Bit());
	method->addArgument( anArg );

	method->setFileName( fileName );
	method->setStartPosition( lineNo, 0 );

	if (lastClass && inClass) {
	  //	    kdDebug(9018) << "in Class: " << line << endl;
	  if (!lastClass->hasFunction(method->name()))
	    lastClass->addFunction(method);
	} else {
	  if (!m_file->hasFunction(method->name()) )
	    m_file->addFunction(method);
	}
      }
      else if (varre.search(line) != -1) {
	//	  kdDebug(9018) << "###########regex match line ( var ): " << varre.cap(1) << endl;
	if (lastClass && inClass) {
	  VariableDom anAttr = m_model->create<VariableModel>();
	  anAttr->setName(varre.cap(1));
	  anAttr->setFileName(fileName);
	  anAttr->setStartPosition( lineNo, 0 );
	  lastClass->addVariable( anAttr );
	}
      }

      ++lineNo;
    } // end for
  } // end if
}
void PHPParser::parseFile(const QString& fileName){
  kdDebug(9018) << "enter parsedFile" << endl;
  kdDebug(9018) <<  "FileName:" << fileName.latin1() << endl;
  QFile f(QFile::encodeName(fileName));
  if (!f.open(IO_ReadOnly))
    return;
  QTextStream stream(&f);
  QStringList list;
  QString rawline;
  while (!stream.eof()) {
   rawline = stream.readLine();
   list.append(rawline.stripWhiteSpace().local8Bit());
 }
 f.close();

 m_file = m_model->create<FileModel>();
 m_file->setName( fileName );

 this->parseLines(&list,fileName);

 m_model->addFile( m_file );
}

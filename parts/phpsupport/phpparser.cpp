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

#include <iostream>
#include "phpparser.h"
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "kdevcore.h"
#include <qfileinfo.h>
#include <qtextstream.h>
#include <kregexp.h>
#include <kdebug.h>

using namespace std;

PHPParser::PHPParser(KDevCore* core,ClassStore* store){
  m_core = core;
  m_classStore = store;
  m_classStore->hasClass("test");
}
PHPParser::~PHPParser(){
}
void PHPParser::parseLines(QStringList* lines,const QString& fileName){
  kdDebug(9018) << "enter parsedLines" << endl;
  KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  KRegExp methodre("^[ \t]*function[ \t]*([0-9A-Za-z_]*)[ \t]*\\(([0-9A-Za-z_\\$\\, \t=&\\'\\\"]*)\\).*$");
  KRegExp varre("^[ \t]*var[ \t]*([0-9A-Za-z_\\$]+)[ \t;=].*$");
  KRegExp createMemberRe("\\$this->([0-9A-Za-z_]+)[ \t]*=[ \t]*new[ \t]+([0-9A-Za-z_]+)");

  ParsedClass *lastClass = 0;
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
      if (classre.match(line)) {

	//  cerr << "kdevelop (phpsupport): regex match line: " << line << endl;
	inClass= true;
	bracketOpen = line.contains("{");
	bracketClose = line.contains("}");
	lastClass = new ParsedClass;
	lastClass->setName(classre.group(1));

	lastClass->setDeclaredInFile(fileName);
	lastClass->setDeclaredOnLine(lineNo);

	lastClass->setDefinedInFile(fileName);
	lastClass->setDefinedOnLine(lineNo);

	QString parentStr = classre.group(3);
	if(!parentStr.isEmpty()){
	  ParsedParent *parent = new ParsedParent;
	  parent->setName(parentStr);
	  parent->setAccess(PIE_PUBLIC);
	  lastClass->addParent(parent);
	}
	if (m_classStore->hasClass(lastClass->name())) {
	  ParsedClass *old = m_classStore->getClassByName(lastClass->name());
	  old->setDeclaredOnLine(lastClass->declaredOnLine());
	  old->setDeclaredInFile(lastClass->declaredInFile());
	  delete lastClass;
	  lastClass=0;
	} else {
          m_classStore->globalScope()->addClass(lastClass);
	  m_classStore->addClass(lastClass);
	}
      } else if (createMemberRe.match(line)) {
	if (lastClass && inClass) {
	  ParsedAttribute *att = lastClass->getAttributeByName(QString("$") + createMemberRe.group(1));
	if (att){
	  att->setType(createMemberRe.group(2));
	}
	}
      } else if (methodre.match(line)) {
	//	  cerr << "kdevelop (phpsupport): regex match line ( method ): " << line << endl;
	ParsedMethod *method = new ParsedMethod;
	method->setName(methodre.group(1));
	ParsedArgument* anArg = new ParsedArgument();
	QString arguments = methodre.group(2);
	anArg->setType(arguments.stripWhiteSpace().local8Bit());
	method->addArgument( anArg );

	method->setDeclaredInFile(fileName);
	method->setDeclaredOnLine(lineNo);

	method->setDefinedInFile(fileName);
	method->setDefinedOnLine(lineNo);
	if (lastClass && inClass) {
	  //	    kdDebug(9018) << "in Class: " << line << endl;
	  ParsedMethod *old = lastClass->getMethod(method);
	  if (!old)
	    lastClass->addMethod(method);
	} else {
	  ParsedMethod *old = m_classStore->globalScope()->getMethod(method);
	  if (!old)
	    m_classStore->globalScope()->addMethod(method);
	}
      }
      else if (varre.match(line)) {
	//	  kdDebug(9018) << "###########regex match line ( var ): " << varre.group(1) << endl;
	if (lastClass && inClass) {
	  ParsedAttribute* anAttr = new ParsedAttribute();
	  anAttr->setName(varre.group(1));
	  anAttr->setDeclaredInFile(fileName);
	  anAttr->setDeclaredOnLine(lineNo);
	  anAttr->setDefinedInFile(fileName);
	  anAttr->setDefinedOnLine(lineNo);
	  lastClass->addAttribute( anAttr );
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
 this->parseLines(&list,fileName);

 /*
  QFile f(QFile::encodeName(fileName));
  if (!f.open(IO_ReadOnly))
    return;
  QTextStream stream(&f);

  KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
  KRegExp methodre("^[ \t]*function[ \t]*([0-9A-Za-z_]*)[ \t]*\\(([0-9A-Za-z_\\$\\, \t=&\\'\\\"]*)\\).*$");
  KRegExp varre("^[ \t]*var[ \t]*([0-9A-Za-z_\\$]+)[ \t;=].*$");
  KRegExp createMemberRe("\\$this->([0-9A-Za-z_]+)[ \t]*=[ \t]*new[ \t]+([0-9A-Za-z_]+)");

  ParsedClass *lastClass = 0;
  QString rawline;
  QCString line;
  int lineNo = 0;
  int bracketOpen = 0;
  int bracketClose = 0;
  bool inClass = false;

  while (!stream.eof()) {
    rawline = stream.readLine();
    line = rawline.stripWhiteSpace().latin1();
    bracketOpen += line.contains("{");
    bracketClose += line.contains("}");
    if(bracketOpen == bracketClose && bracketOpen !=0 && bracketClose !=0){
      inClass = false; // ok we are out ouf class
    }

    //cerr << "kdevelop (phpsupport): try match line: " << line << endl;
    if (classre.match(line)) {
      //  cerr << "kdevelop (phpsupport): regex match line: " << line << endl;
      inClass= true;
      bracketOpen = line.contains("{");
      bracketClose = line.contains("}");
      lastClass = new ParsedClass;
      lastClass->setName(classre.group(1));
      lastClass->setDefinedInFile(fileName);
      lastClass->setDefinedOnLine(lineNo);

      QString parentStr = classre.group(3);
      if(!parentStr.isEmpty()){
	ParsedParent *parent = new ParsedParent;
	parent->setName(parentStr);
	parent->setAccess(PIE_PUBLIC);
	lastClass->addParent(parent);
      }

      if (m_classStore->hasClass(lastClass->name())) {
	ParsedClass *old = m_classStore->getClassByName(lastClass->name());
	old->setDeclaredOnLine(lastClass->declaredOnLine());
	old->setDeclaredInFile(lastClass->declaredInFile());
	delete lastClass;
	lastClass=0;
      } else {
	m_classStore->addClass(lastClass);
      }

    } else if (createMemberRe.match(line)) {
      if (lastClass && inClass) {
	ParsedAttribute *att = lastClass->getAttributeByName(QString("$") + createMemberRe.group(1));
	if (att){
	  att->setType(createMemberRe.group(2));
	}
      }

    } else if (methodre.match(line)) {
      //	  cerr << "kdevelop (phpsupport): regex match line ( method ): " << line << endl;
      ParsedMethod *method = new ParsedMethod;
      method->setName(methodre.group(1));
      ParsedArgument* anArg = new ParsedArgument();
      QString arguments = methodre.group(2);
      anArg->setType(arguments.stripWhiteSpace().latin1());
      method->addArgument( anArg );

      method->setDefinedInFile(fileName);
      method->setDefinedOnLine(lineNo);

      if (lastClass && inClass) {
	//	    kdDebug(9018) << "in Class: " << line << endl;
	ParsedMethod *old = lastClass->getMethod(method);
	if (!old)
	  lastClass->addMethod(method);
      } else {
	ParsedMethod *old = m_classStore->globalContainer.getMethod(method);
	if (!old)
	  m_classStore->globalContainer.addMethod(method);
      }
    }
    else if (varre.match(line)) {
      //	  kdDebug(9018) << "###########regex match line ( var ): " << varre.group(1) << endl;
      if (lastClass && inClass) {
	ParsedAttribute* anAttr = new ParsedAttribute();
	anAttr->setName(varre.group(1));
	anAttr->setDefinedInFile(fileName);
	anAttr->setDefinedOnLine(lineNo);
	lastClass->addAttribute( anAttr );
      }
    }
    ++lineNo;
  }
  f.close();
  */
}

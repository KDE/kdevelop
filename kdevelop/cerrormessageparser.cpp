/***************************************************************************
                          cerrormessageparser.cpp  -  description                              
                             -------------------                                         
    begin                : Tue Mar 30 1999                                           
    copyright            : (C) 1999 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de              
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "cerrormessageparser.h"
#include <iostream.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qstack.h>

CErrorMessageParser::CErrorMessageParser(){
  m_info_list.setAutoDelete(true);
}
CErrorMessageParser::~CErrorMessageParser(){
}
void CErrorMessageParser::parse(QString makeoutput,QString startdir){
  QStack<QString> stack;
  QStrList outputlist;
  QString str;
  int next =0;
  int pos =0;
  TErrorMessageInfo* error_info;

  cerr << "PARSER";
  m_info_list.clear();
  
  // fill the outputlist
  while(next != -1){
    next = makeoutput.find('\n',pos);
    if(next != -1){
      str = makeoutput.mid(pos,next-pos);
      outputlist.append(str);
    }
    pos = next+1;
  }
  
  QRegExp error_reg(":[0-9]*:"); // is it an error line?, I hope it works
  QRegExp enter_reg(": Entering directory");
  QRegExp leave_reg(": Leaving directory");

  
  int pos1;
  int pos2;
  int error_line;
  bool ok;
  int makeoutputline=0;

  QString* stack_str;
  stack_str = new QString(startdir);
  stack.push(stack_str);
  
  QString error_str;

  for(str = outputlist.first();str != 0;str = outputlist.next()){
    makeoutputline++;
    //enter directory
    if((pos1=enter_reg.match(str)) != -1){
      // extract the enter directory
      pos1 = str.find('`',0);
      pos2 = str.find('\'',pos1+1);
      stack_str = new QString;
      *stack_str = str.mid(pos1+1,pos2-pos1-1);
      stack.push(stack_str);
    }
    //leaving directory
    if((pos1=leave_reg.match(str)) != -1){
      // extract the enter directory
      pos1 = str.find('`',0);
      pos2 = str.find('\'',pos1+1);
      stack_str = new QString;
      *stack_str = str.mid(pos1+1,pos2-pos1-1);
      stack.pop();
    }
    
    //errors/warnings
    if((pos1=error_reg.match(str)) != -1){
      pos2 = str.find(':',pos1+1);
      error_str = str.mid(pos1+1,pos2-pos1-1);
      error_line = error_str.toInt(&ok);
      if(ok){ // was it a number?
	// extract the filename
	pos2 = str.findRev(' ',pos1);
	if (pos2 == -1) {
	  pos2 = 0; // the filename is at the begining of the string
	}
	else { pos2++; }
	error_str = str.mid(pos2,pos1-pos2);

	// ok we will create now a new entry
	error_info = new TErrorMessageInfo;
	stack_str = stack.top();
	if(stack_str->right(1) != "/"){
	  *stack_str += "/";
	}
	error_info->filename = *stack_str + error_str;
	error_info->errorline = error_line;
	error_info->makeoutputline = makeoutputline;
	m_info_list.append( error_info);
	
      }
    }
  } //  end for outputlist
}

TErrorMessageInfo CErrorMessageParser::getInfo(int makeoutputline){
}
TErrorMessageInfo CErrorMessageParser::getNext(){
}
TErrorMessageInfo CErrorMessageParser::getPrev(){
}
void CErrorMessageParser::out(){
  TErrorMessageInfo* info;
  
  for(info = m_info_list.first();info !=0;info = m_info_list.next()){
    cerr << endl << "File:" << info->filename;
    cerr << endl << "Errorline:" << info->errorline;
    cerr << endl << "Makeoutputline:" << info->makeoutputline << endl;
  }
}

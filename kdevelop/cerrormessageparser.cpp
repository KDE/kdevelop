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
  current = -1;
  state = false;
}
CErrorMessageParser::~CErrorMessageParser(){
}
void  CErrorMessageParser::parseInSgml2HtmlMode(QString* sgmloutput,QString sgmlfile){
  if(!isOn()) return;
  QStrList outputlist;
  QString str;
  int next =0;
  int pos =0;
  TErrorMessageInfo* error_info;
  m_info_list.clear();
  
  // fill the outputlist
  while(next != -1){
    next = sgmloutput->find('\n',pos);
    if(next != -1){
      str = sgmloutput->mid(pos,next-pos);
      outputlist.append(str);
    }
    pos = next+1;
  }

  int pos1;
  int pos2;
  int error_line;
  bool ok;
  int makeoutputline=0;
  QString error_str;
  QRegExp error_reg(":*:[0-9]*:*:"); // is it an error line for sgml2html?, I hope it works
  
  for(str = outputlist.first();str != 0;str = outputlist.next()){
      makeoutputline++;
    if((pos1=error_reg.match(str)) != -1){ // error ?
      pos2 = str.find(':',pos1+1);
      error_str = str.mid(pos1+1,pos2-pos1-1);
      error_line = error_str.toInt(&ok);
      if(ok){ // was it a number?
	// ok we will create now a new entry
	error_info = new TErrorMessageInfo;
	error_info->filename = sgmlfile;
	error_info->errorline = error_line;
	error_info->makeoutputline = makeoutputline;
	m_info_list.append( error_info);
      }
    }
  }
  out();
//  cerr << endl << endl << ":::::::::";
}
void CErrorMessageParser::parseInMakeMode(QString* makeoutput){
  if(!isOn()) return;
  QStack<QString> stack;
  QStrList outputlist;
  QString str;
  int next =0;
  int pos =0;
  
  TErrorMessageInfo* error_info;

  m_info_list.clear();
  
  // fill the outputlist
  while(next != -1){
    next = makeoutput->find('\n',pos);
    if(next != -1){
      str = makeoutput->mid(pos,next-pos);
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
  stack_str = new QString(startDir);
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
        // make the parser robust against missing 'Entering ...'
        QString empty;
        if (stack_str == 0)
            stack_str = &empty;
	if(stack_str->right(1) != "/"){
	  *stack_str += "/";
	}
	if(error_str.left(1) == "/"){ // absolute
	  error_info->filename = error_str;
	}
	else{
	error_info->filename = *stack_str + error_str;
	}
	error_info->errorline = error_line;
	error_info->makeoutputline = makeoutputline;
	m_info_list.append( error_info);
	
      }
    }
  } //  end for outputlist
}

TErrorMessageInfo CErrorMessageParser::getInfo(int makeoutputline){
  TErrorMessageInfo * info;
  TErrorMessageInfo temp_info;
  temp_info.filename = "";
  for(info = m_info_list.first();info != 0;info = m_info_list.next()){
    if (info->makeoutputline == makeoutputline ){
      temp_info = *info;
    }
  }
  return temp_info;
}

TErrorMessageInfo CErrorMessageParser::getNext(){
  TErrorMessageInfo * info;
  TErrorMessageInfo temp_info;
  current++;
  
  
  if(int(m_info_list.count()) > current){ // if a next exists  
    info = m_info_list.at(current);
    return *info;
  }
  else{
    current--;
    temp_info.filename = "";
    return temp_info;
  }
}
TErrorMessageInfo CErrorMessageParser::getPrev(){
  TErrorMessageInfo * info;
  TErrorMessageInfo temp_info;
  current--;
  
  if(int(m_info_list.count()) > current && current > -1){ // if a prev exists  
    info = m_info_list.at(current);
    return *info;
  }
  else{
    current++;
    temp_info.filename = "";
    return temp_info;
  }
}

bool CErrorMessageParser::hasNext(){
  int tmp = current +1;
  if(int(m_info_list.count()) > tmp){ // if a next exists  
    return true;
  }
  return false;
}
bool CErrorMessageParser::hasPrev(){
  int tmp = current -1;
  if(int(m_info_list.count()) > current && tmp > -1){ // if a prev exists  
    return true;
  }
  return false;
}
void CErrorMessageParser::out(){
  TErrorMessageInfo* info;
  
  for(info = m_info_list.first();info !=0;info = m_info_list.next()){
    cerr << endl << "File:" << info->filename;
    cerr << endl << "Errorline:" << info->errorline;
    cerr << endl << "Makeoutputline:" << info->makeoutputline << endl;
  }
}
void CErrorMessageParser::reset(){
  current = -1;
  m_info_list.clear();
}
/**toogle the parser on*/
void CErrorMessageParser::toogleOn(TEPMode mode){
  m_mode = mode;
  state = true;
}
/**toogle the parser off, in this state the parse command, return without doing anything*/
void CErrorMessageParser::toogleOff(){
  state = false;
}
/** returns the state*/
bool CErrorMessageParser::isOn(){
  return state;
}
CErrorMessageParser::TEPMode CErrorMessageParser::getMode(){
  return m_mode;
}


/***************************************************************************
             cerrormessageparser.h  -  a small "parser" for the makeoutput                 
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


#ifndef CERRORMESSAGEPARSER_H
#define CERRORMESSAGEPARSER_H

#include <qstring.h>
#include <qlist.h>

/** */
struct TErrorMessageInfo {
  QString filename;
  int errorline;
  int makeoutputline;
};

/** a small "parser" for the makeoutput,sgml2html output ...
  *@author Sandy Meier
  */
class CErrorMessageParser {
public: 
  CErrorMessageParser();
  ~CErrorMessageParser();
  /** the mode in which the parser will operate*/
  typedef  enum {MAKE,SGML2HTML} TEPMode; // ErrorMessageParser Mode
  /** set the counter to -1, called if "make", or sgml2html was started and the outputwidget was cleared*/
  void reset();

  /** parse the string in the  make mode*/
  void parseInMakeMode(QString* makeoutput,QString startdir);
  /** parse the string in the  sgml2html mode*/
  void parseInSgml2HtmlMode(QString* sgmloutput,QString sgmlfile);
  CErrorMessageParser::TEPMode getMode();
  
  /** get the error info for a specific line from the parse "makeoutput")*/
  TErrorMessageInfo getInfo(int makeoutputline);
  
  /** get the next error/warning*/
  TErrorMessageInfo getNext();

  bool hasNext();
  bool hasPrev();
  
  /** get the previous error/warning*/
  TErrorMessageInfo getPrev();
  /** print out the parsed informations*/
  void out();
  
  /**toogle the parser on, default mode is make/compiler*/
  void toogleOn(TEPMode mode = MAKE);
  /**toogle the parser off, in this state the parse command, return without doing anything*/
  void toogleOff();
  /** returns the state*/
  bool isOn();
protected:
  int current; // used to store infos for getNext() ...
  QList<TErrorMessageInfo> m_info_list;
  bool state;
  TEPMode m_mode;
};


#endif

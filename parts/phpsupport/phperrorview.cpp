/***************************************************************************
                          phperrorview.cpp  -  description
                             -------------------
    begin                : Sat May 26 2001
    copyright            : (C) 2001 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "phperrorview.h"
#include <qstringlist.h>
#include <kregexp.h>
#include <iostream>

using namespace std;

PHPErrorView::PHPErrorView(PHPSupportPart *part) : QListBox(0,"PHP Error View") {
  connect(this,SIGNAL(selected(int)),this,SLOT(slotItemSelected(int)));
}

PHPErrorView::~PHPErrorView(){
}

void PHPErrorView::slotItemSelected(int index){
  cerr << "slotItemSelected()" << endl;
  ErrorItem* errorItem = errorDict[index];
  emit fileSelected(errorItem->filename,errorItem->line -1);
}

void PHPErrorView::parse(QString& phpOutput){
  ///  KRegExp parseerror("^<b>Parse error</b>: parse error in<b>(.*)</b> on line<b>(.*)</b>*$");
  cerr << phpOutput;
  errorDict.clear();
  clear();
  ErrorItem* errorItem;
  int currentLine=0;

  // todo, test and add other
  KRegExp parseError("^<b>Parse error</b>:  parse error in <b>(.*)</b> on line <b>(.*)</b>.*$");
  KRegExp undefFunctionError("^<b>Fatal error</b>:  Call to undefined function:  (.*) in <b>(.*)</b> on line <b>(.*)</b>.*$");
  KRegExp warning("^<b>Warning</b>.*<b>(.*)</b> on line <b>(.*)</b>.*$");
  KRegExp generalFatalError("^<b>Fatal error</b>: (.*) in <b>(.*)</b> on line <b>(.*)</b>.*$");
  

  QStringList list = QStringList::split("\n",phpOutput);
  QStringList::Iterator it;
  for( it = list.begin(); it != list.end(); ++it ){
    if(parseError.match(*it)){
      insertItem("parse error in " + QString(parseError.group(1)) + " on line " 
		 + QString(parseError.group(2)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(parseError.group(1));
      errorItem->line = QString(parseError.group(2)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
    }
    else if(undefFunctionError.match(*it)){
      insertItem("call to undefined function " + QString(undefFunctionError.group(1)) + " in " +
		 QString(undefFunctionError.group(2)) + " on line " + QString(undefFunctionError.group(3)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(parseError.group(2));
      errorItem->line = QString(parseError.group(3)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
    }
    else if(warning.match(*it)){
      insertItem("warning in " + QString(warning.group(1)) + " on line " + QString(warning.group(2)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(warning.group(1));
      errorItem->line = QString(warning.group(2)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++; 
    }
    else if(generalFatalError.match(*it)){
      insertItem(QString(generalFatalError.group(1)) + " in " +
		 QString(generalFatalError.group(2)) + " on line " + QString(generalFatalError.group(3)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(generalFatalError.group(2));
      errorItem->line = QString(generalFatalError.group(3)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
      
    }
  }
}
#include "phperrorview.moc"

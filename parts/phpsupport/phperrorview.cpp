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
#include <qregexp.h>
#include <iostream>

#include <klocale.h>

using namespace std;

PHPErrorView::PHPErrorView(PHPSupportPart */*part*/) : QListBox(0,"PHP Error View") {
  connect(this,SIGNAL(selected(int)),this,SLOT(slotItemSelected(int)));
}

PHPErrorView::~PHPErrorView(){
}

void PHPErrorView::slotItemSelected(int index){
  ErrorItem* errorItem = errorDict[index];
  emit fileSelected(errorItem->filename,errorItem->line -1);
}

void PHPErrorView::parse(QString& phpOutput){
  ///  KRegExp parseerror("^<b>Parse error</b>: parse error in<b>(.*)</b> on line<b>(.*)</b>*$");
  errorDict.clear();
  clear();
  ErrorItem* errorItem;
  int currentLine=0;

  // todo, test and add other
  QRegExp parseError("^<b>Parse error</b>:  parse error in <b>(.*)</b> on line <b>(.*)</b>.*$");
  QRegExp undefFunctionError("^<b>Fatal error</b>:  Call to undefined function:  (.*) in <b>(.*)</b> on line <b>(.*)</b>.*$");
  QRegExp warning("^<b>Warning</b>.*<b>(.*)</b> on line <b>(.*)</b>.*$");
  QRegExp generalFatalError("^<b>Fatal error</b>: (.*) in <b>(.*)</b> on line <b>(.*)</b>.*$");


  QStringList list = QStringList::split("\n",phpOutput);
  QStringList::Iterator it;
  for( it = list.begin(); it != list.end(); ++it ){
    if(parseError.search(*it)>=0){
      insertItem(i18n("Parse Error in %1 on Line %2").arg(parseError.cap(1)).arg(parseError.cap(2)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(parseError.cap(1));
      errorItem->line = QString(parseError.cap(2)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
    }
    else if(undefFunctionError.search(*it)>=0){
      insertItem(i18n("Call to Undefined Function %1 in %2 on Line %3")
                 .arg(undefFunctionError.cap(1))
                 .arg(undefFunctionError.cap(2))
                 .arg(undefFunctionError.cap(3)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(parseError.cap(2));
      errorItem->line = QString(parseError.cap(3)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
    }
    else if(warning.search(*it)>=0){
      insertItem(i18n("Warning in %1 on Line %2").arg(warning.cap(1)).arg(warning.cap(2)));
      errorItem = new ErrorItem();
      errorItem->filename = QString(warning.cap(1));
      errorItem->line = QString(warning.cap(2)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;
    }
    else if(generalFatalError.search(*it)>=0){
      insertItem(i18n("%1 in %2 on Line %3")
                 .arg(generalFatalError.cap(1))
                 .arg(generalFatalError.cap(2))
                 .arg((generalFatalError.cap(3))));
      errorItem = new ErrorItem();
      errorItem->filename = QString(generalFatalError.cap(2));
      errorItem->line = QString(generalFatalError.cap(3)).toInt();
      errorDict.insert(currentLine,errorItem);
      currentLine++;

    }
  }
}
#include "phperrorview.moc"

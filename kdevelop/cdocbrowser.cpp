/***************************************************************************
           cdocbrowser.cpp - 
                             -------------------                                         

    version              :                                   
    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
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
#include "cdocbrowser.h"
#include <iostream.h>
#include <kmsgbox.h>
#include <kfm.h>

CDocBrowser::CDocBrowser(QWidget*parent,const char* name) : KHTMLView(parent,name){
}

CDocBrowser::~CDocBrowser(){
}

void CDocBrowser::showURL(QString url){
 //read the htmlfile
  //cerr << "URL:" << url << "\n";
  QString ref = url;
  int pos = ref.findRev('#');
  int len = ref.length();
  ref = ref.right(len - pos - 1);
  //cerr << "REF:" << ref << "\n";
  
  QString str="";
  KFM::download(url,str);

  //cerr << endl << "STR:" << str;
  
  QFile file(str);
  if(!file.open(IO_ReadOnly)) return;
  QTextStream stream(&file);
  QString str2="";
  while(!stream.eof()){
    str2 = str2 + stream.readLine() + "\n";
  }
  begin(str);
  write(str2);
  end();
  parse();
  show();
  KFM::removeTempFile(str);
  file.close();
  
  if (pos != -1){
    gotoAnchor(ref);
  }
    
}

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
#include <qregexp.h>

CDocBrowser::CDocBrowser(QWidget*parent,const char* name) : KHTMLView(parent,name){
}

CDocBrowser::~CDocBrowser(){
}

void CDocBrowser::showURL(QString url,bool reload){
 //read the htmlfile
  //cerr << "URL:" << url << "\n";
  QString ref = url;
  QString url_wo_ref; // without ref
  int pos = ref.findRev('#');
  int len = ref.length();
  ref = ref.right(len - pos - 1);
  
  pos = url.findRev('#');
  url_wo_ref = url.left(pos);
  
  if( (url_wo_ref != old_url) || reload){
    QString str="";
    KFM::download(url,str);
    
    //cerr << endl << "STR:" << str;
    
    char buffer[256];
    int val;
    QFile file(str) ;
    if(file.exists()){
      file.open(IO_ReadOnly);
      begin( url);
      do
	{
	  buffer[0] = '\0';
	  val = file.readLine( buffer, 256 );
	  write(buffer);
	}
      while ( !file.atEnd() );
      
      end();
      parse();
           show();
      KFM::removeTempFile(str);
      file.close();
    }
  }
  


  if (pos != -1){
    gotoAnchor(ref);
  }
  old_url = url_wo_ref;
}

/***************************************************************************
                 cgeneratenewfile.cpp - 
                             -------------------                                         

    version              :                                   
    begin                : 1 Oct 1998                                        
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

#include <iostream.h>
#include <qfileinfo.h>
#include "cgeneratenewfile.h"
#include "cproject.h"
#include <kprocess.h>
#include <kstddirs.h>
#include <qstringlist.h>

CGenerateNewFile::CGenerateNewFile(){
}
CGenerateNewFile::~CGenerateNewFile(){
}

QString  CGenerateNewFile::genCPPFile(QString abs_name,CProject* prj, QString file_name){
  QString name;
  if ( file_name=="") { name  = QFileInfo(abs_name).fileName(); }
   else { name  = QFileInfo(file_name).fileName(); }
   // local cpp_template
  QString cpp_header = prj->getProjectDir() + prj->getSubDir() +"/templates/cpp_template";
  if(!QFile::exists(cpp_header)){
    cpp_header = locate("kdev_template","cpp_template");
  }

  QStrList list;
  QString str;
  
  QFile file(cpp_header);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the header_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){

      str.replace(QRegExp("\\|FILENAME\\|"),name);
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();

}
QString  CGenerateNewFile::genHeaderFile(QString abs_name,CProject* prj, QString file_name){
  QString name;
  if ( file_name=="") { name  = QFileInfo(abs_name).fileName(); }
   else { name  = QFileInfo(file_name).fileName(); }

  // local header_template
  QString template_header = prj->getProjectDir() + prj->getSubDir() +"/templates/header_template";
  if(!QFile::exists(template_header)){
    template_header = locate("kdev_template", "header_template");
  }

  QStrList list;
  QString str;
  
  QFile file(template_header);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the header_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){

      str.replace(QRegExp("\\|FILENAME\\|"),name);
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}

QString  CGenerateNewFile::genLEXICALFile(QString abs_name,CProject* prj){
  QString name  = QFileInfo(abs_name).fileName();
  QString lexical_header = locate("kdev_template", "lexical_template");

  QStrList list;
  QString str;
  
  QFile file(lexical_header);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the header_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){
      str.replace(QRegExp("\\|FILENAME\\|"),name);
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}

QString  CGenerateNewFile::genEngDocbook(QString abs_name,CProject* prj){
  QString template_handbook = locate("kdev_template","docbook_en_template");

  QStrList list;
  QString str;

  QFile file(template_handbook);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the handbook_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){

      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();
  return file.name();

}

QString  CGenerateNewFile::genEngHandbook(QString abs_name,CProject* prj){

  QString template_handbook = locate("kdev_template","handbook_en_template");

  QStrList list;
  QString str;

  QFile file(template_handbook);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the handbook_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){

      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}
QString  CGenerateNewFile::genDesktopFile(QString abs_name,CProject* prj){  
  QString template_desktop = locate("kdev_template", "desktop_template");
  QString project_name = prj->getProjectName().lower();
  QString str;
  QStrList list;
  
  QFile file(template_desktop);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the desktop_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();

  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){
      str.replace(QRegExp("\\|PRJNAME\\|"),project_name);
      stream << str << "\n";
    }
  }
  file.close();  
  return file.name();
  
}
QString  CGenerateNewFile::genLSMFile(QString abs_name,CProject* prj){
  QString template_lsm = locate("kdev_template", "lsm_template");


  QStrList list;
  QString str;
  
  QFile file(template_lsm);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the lsm_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){

      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}

QString CGenerateNewFile::genIcon(QString abs_name){
  QStrList list;
  QString str;
  QString icon_template = locate("kdev_template", "icon_template");
  QFile file(icon_template);
  QTextStream stream(&file);

  if(file.open(IO_ReadOnly)){ // read the handbook_template
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  file.setName(abs_name);
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str !=0;str = list.next()){
      stream << str << "\n";
    }
  }
  file.close();  
  return file.name();
}

QString CGenerateNewFile::genNifFile(QString abs_name){
    KShellProcess process;
    QString nif_template = locate("kdev_template", "nif_template");

    process.clearArguments();
    process << "cp"; // copy is your friend :-)
    process << nif_template;
    process << abs_name;
    process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
}






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

CGenerateNewFile::CGenerateNewFile(){
}
CGenerateNewFile::~CGenerateNewFile(){
}
QString  CGenerateNewFile::genCPPFile(QString abs_name,CProject* prj){
  QString name  = QFileInfo(abs_name).fileName();
   // local cpp_template
  QString cpp_header = prj->getProjectDir() + prj->getSubDir() +"/templates/cpp_template";
  if(!QFile::exists(cpp_header)){
    cpp_header = KApplication::kde_datadir() + "/kdevelop/templates/cpp_template";
  }
/*
     not needed anymore - handled by CProject::setInfosInString()
  QString email = prj->getEmail();
  QString author = prj->getAuthor();
  QString version = prj->getVersion();
  QString date = QDate::currentDate().toString();
  QString year;
  year.setNum(QDate::currentDate().year());
*/
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
/*
     not needed anymore - handled by CProject::setInfosInString()
      str.replace(QRegExp("EMAIL"),email);
      str.replace(QRegExp("AUTHOR"),author);
      str.replace(QRegExp("VERSION"),version);
      str.replace(QRegExp("DATE"),date);
      str.replace(QRegExp("YEAR"),year);
      stream << str << "\n";
*/
      str.replace(QRegExp("\\|FILENAME\\|"),name);
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();

}
QString  CGenerateNewFile::genHeaderFile(QString abs_name,CProject* prj){
  QString name  = QFileInfo(abs_name).fileName();
  // local header_template
  QString template_header = prj->getProjectDir() + prj->getSubDir() +"/templates/header_template";
  if(!QFile::exists(template_header)){
    template_header = KApplication::kde_datadir() + "/kdevelop/templates/header_template";
  }
/*
     not needed anymore - handled by CProject::setInfosInString()
  QString email = prj->getEmail();
  QString author = prj->getAuthor();
  QString version = prj->getVersion();
  QString date = QDate::currentDate().toString();
  QString year;
  year.setNum(QDate::currentDate().year());
*/
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
/*
     not needed anymore - handled by CProject::setInfosInString()
      str.replace(QRegExp("EMAIL"),email);
      str.replace(QRegExp("AUTHOR"),author);
      str.replace(QRegExp("VERSION"),version);
      str.replace(QRegExp("DATE"),date);
      str.replace(QRegExp("YEAR"),year);
      stream << str << "\n";
*/
      str.replace(QRegExp("\\|FILENAME\\|"),name);
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}
QString  CGenerateNewFile::genEngHandbook(QString abs_name,CProject* prj){
  
  QString template_handbook = KApplication::kde_datadir() + 
    "/kdevelop/templates/handbook_en_template";
/*
     not needed anymore - handled by CProject::setInfosInString()
  QString project_name = prj->getProjectName();
  QString email = prj->getEmail();
  QString author = prj->getAuthor();
  QString version = prj->getVersion();
  QString date = QDate::currentDate().toString();
*/
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
/*
     not needed anymore - handled by CProject::setInfosInString()
      str.replace(QRegExp("PROJECT_NAME"),project_name);
      str.replace(QRegExp("EMAIL"),email);
      str.replace(QRegExp("AUTHOR"),author);
      str.replace(QRegExp("VERSION"),version);
      str.replace(QRegExp("DATE"),date);
      stream << str << "\n";
*/
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}
QString  CGenerateNewFile::genKDELnkFile(QString abs_name,CProject* prj){  
  QString template_kdelnk = KApplication::kde_datadir() + "/kdevelop/templates/kdelnk_template";
  QString project_name = prj->getProjectName().lower();
  QString str;
  QStrList list;
  
  QFile file(template_kdelnk);
  QTextStream stream(&file);
  if(file.open(IO_ReadOnly)){ // read the kdelnk_template
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
  QString template_lsm = KApplication::kde_datadir() + "/kdevelop/templates/lsm_template";
/*
     not needed anymore - handled by CProject::setInfosInString()
  QString project_name = prj->getProjectName();
  QString email = prj->getEmail();
  QString author = prj->getAuthor();
  QString version = prj->getVersion();
*/

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
/*
     not needed anymore - handled by CProject::setInfosInString()
      str.replace(QRegExp("PROJECT_NAME"),project_name);
      str.replace(QRegExp("EMAIL"),email);
      str.replace(QRegExp("AUTHOR"),author);
      str.replace(QRegExp("VERSION"),version);
      stream << str << "\n";
*/
      stream << prj->setInfosInString(str) << '\n';
    }
  }
  file.close();  
  return file.name();
}

QString CGenerateNewFile::genIcon(QString abs_name){
  QStrList list;
  QString str;
  QString icon_template = KApplication::kde_datadir() + "/kdevelop/templates/icon_template";
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

/***************************************************************************
                 cproject.cpp - the projectproperties
                             -------------------                                         

    version              :                                   
    begin                : 28 Jul 1998                                        
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

#include "cproject.h"
#include <iostream.h>
#include <qregexp.h>

CProject::CProject(){
  valid = false;
}

CProject::~CProject(){
}

bool CProject::readProject(QString file){
  
  QFile qfile(file);
  QTextStream stream(&qfile);
  qfile.open(IO_ReadOnly);
  QString str = stream.readLine();
  if(str.contains("KDevelop Project File Version 0.3 #DO NOT EDIT#") ){
    return false;

  }
  config = new KSimpleConfig(file);
  
  prjfile = file;
  QFileInfo fileinfo(file);
  dir = fileinfo.dirPath() + "/";
  setSourcesHeaders();
  valid = true;
  return true;
}
void CProject::writeProject(){
   // new 
  if(config != 0){
    config->sync();
  }
}
QString CProject::getProjectFile(){
  return prjfile;
}
QString CProject::getProjectDir(){
  return dir;
}
void CProject::setProjectName(QString name){
  config->setGroup("General");
  config->writeEntry("project_name",name);
}
QString CProject::getProjectName(){
  config->setGroup("General");
  return config->readEntry("project_name");
}
void CProject::setSubDir(QString dirname){
  config->setGroup("General");
  config->writeEntry("sub_dir",dirname);
}
QString CProject::getSubDir(){
  config->setGroup("General");
  return config->readEntry("sub_dir");
}
void CProject::setAuthor(QString name){
  config->setGroup("General");
  config->writeEntry("author",name);
}
QString CProject::getAuthor(){
  config->setGroup("General");
  return config->readEntry("author");
}
void CProject::setEmail(QString email){
  config->setGroup("General");
  config->writeEntry("email",email);
}
QString CProject::getEmail(){
  config->setGroup("General");
  return config->readEntry("email");
}
void CProject::setVersion(QString version){
  config->setGroup("General");
  config->writeEntry("version",version);
}
QString CProject::getVersion(){
  config->setGroup("General");
  return config->readEntry("version");
}
void CProject::setSGMLFile(QString name){
  config->setGroup("General");
  config->writeEntry("sgml_file",name);
}
QString CProject::getSGMLFile(){
  config->setGroup("General");
  return config->readEntry("sgml_file");
}
void CProject::setShortInfo(QStrList short_info) {
  config->setGroup("General");
  config->writeEntry("short_info",short_info);
}
QStrList CProject::getShortInfo(){
  QStrList list;
  config->setGroup("General");
  config->readListEntry("short_info",list);
  return list;
}
void CProject::setMakeOptions(QString options){
  config->setGroup("General");
  config->writeEntry("make_options",options);
}
QString CProject::getMakeOptions(){
  config->setGroup("General");
  return config->readEntry("make_options");
}

QStrList& CProject::getSources(){
  return cpp_files;
} 
QStrList& CProject::getHeaders(){
  return header_files;
}
void CProject::setBinPROGRAM(QString name){
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("bin_program",name);
}
QString CProject::getBinPROGRAM(){
  config->setGroup("Config for BinMakefileAm");
  return config->readEntry("bin_program");
}
void CProject::setLDFLAGS(QString flags){
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("ldflags",flags);
}
QString CProject::getLDFLAGS(){
  config->setGroup("Config for BinMakefileAm");
  return config->readEntry("ldflags");
}
void CProject::setCXXFLAGS(QString flags){
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("cxxflags",flags);
}
QString CProject::getCXXFLAGS(){
  config->setGroup("Config for BinMakefileAm");
  return config->readEntry("cxxflags");
}
void CProject::setAdditCXXFLAGS(QString flags){
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("addcxxflags",flags);
}
QString CProject::getAdditCXXFLAGS(){
  config->setGroup("Config for BinMakefileAm");
  return config->readEntry("addcxxflags");
}
/** set the librarys*/
void CProject::setLDADD(QString libstring){
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("ldadd",libstring);
}
QString CProject::getLDADD(){
  config->setGroup("Config for BinMakefileAm");
  return config->readEntry("ldadd");
}
  
void CProject::getAllFiles(QStrList& list){
  QStrList makefile_list;
  QStrList file_list;
  QString makefile_name;
  QString file_name;
  list.clear();
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);
  
  for(makefile_name=makefile_list.first();makefile_name!=0;makefile_name=makefile_list.next()){
    config->setGroup(makefile_name);
    file_list.clear();
    config->readListEntry("files",file_list);
    for(file_name=file_list.first();file_name!=0;file_name=file_list.next()){
      list.append(file_name);
    }
  }
  
  
}
TFileInfo CProject::getFileInfo(QString rel_filename){
  TFileInfo info;
  config->setGroup(rel_filename);
  info.rel_name = rel_filename;
  info.type = config->readEntry("type");
  info.dist = config->readBoolEntry("dist");
  info.install = config->readBoolEntry("install");
  info.install_location = config->readEntry("install_location");
  return info;
  
}

void CProject::writeFileInfo(TFileInfo info){
  QString rel_name = info.rel_name;
  config->setGroup(rel_name);
  config->writeEntry("type",info.type);
  config->writeEntry("dist",info.dist);
  config->writeEntry("install",info.install);
  // save the $ because kconfig removes one
  info.install_location.replace("[\\$]","$$");
  config->writeEntry("install_location",info.install_location);
}
void CProject::getLFVGroups(QStrList& groups){
  groups.clear();
  config->setGroup("LFV Groups");
  config->readListEntry("groups",groups);
}

void CProject::getFilters(QString group,QStrList& filters){
  filters.clear();
  config->setGroup("LFV Groups");
  config->readListEntry(group,filters);
}
void CProject::addFileToProject(QString rel_name){
  QStrList list_files;
  QString makefile_name;
  int slash_pos = rel_name.findRev('/');
  if(slash_pos == -1) { // not found
    makefile_name = "Makefile.am";
  }
  else{
    makefile_name = rel_name.copy();
    makefile_name.truncate(slash_pos+1);
    makefile_name += "Makefile.am";
  }
  config->setGroup(makefile_name);
  config->readListEntry("files",list_files); 
  list_files.append(rel_name);
  config->writeEntry("files",list_files);
  setSourcesHeaders();
  createMakefilesAm();
  
}
void CProject::removeFileFromProject(QString rel_name){
  QStrList list_files;
  QString makefile_name;
  int slash_pos = rel_name.findRev('/');
  if(slash_pos == -1) { // not found
    makefile_name = "Makefile.am";
  }
  else{
    makefile_name = rel_name.copy();
    makefile_name.truncate(slash_pos+1);
    makefile_name += "Makefile.am";
  }
  config->setGroup(makefile_name);
  config->readListEntry("files",list_files); 
  list_files.remove(rel_name);
  config->writeEntry("files",list_files);

  // remove the fileinfo
  config->deleteGroup(rel_name);
  setSourcesHeaders();
  createMakefilesAm();
}
void CProject::createMakefilesAm(){
  QString makefile;
  QStrList makefile_list;
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);  
  for(makefile = makefile_list.first();makefile !=0;makefile =makefile_list.next()){ // every Makefile
    config->setGroup(makefile);
    createMakefileAm(makefile); 
  }
}
void CProject::createMakefileAm(QString makefile){
  setKDevelopWriteArea(makefile);
  config->setGroup(makefile);

  QString abs_filename = getProjectDir() + makefile;
  QFile file(abs_filename);
  QStrList list;
  QStrList files;
  QStrList subdirs;
  QString str;
  QString str2;
  QString dist_str;
  QTextStream stream(&file);
  bool found=false;
  
  config->readListEntry("files",files);
  config->readListEntry("sub_dirs",subdirs);
  if(file.open(IO_ReadOnly)){ // read the makefileam
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  if(file.open(IO_WriteOnly)){
    for(str = list.first();str != 0;str = list.next()){
      if(str == "####### kdevelop will overwrite this part!!! (begin)##########"){
 	stream << str << "\n";
	
	if (config->readEntry("type") == "prog_main"){
	  QString sources;
	  for(str= getSources().first();str !=0;str = getSources().next()){
	    sources =  str + " " + sources ;
	  }
	  stream << "CXXFLAGS = " << getCXXFLAGS()+" "+getAdditCXXFLAGS() << "\n";
	  stream << "LDFLAGS = " << getLDFLAGS()  << "\n";
	  stream << getBinPROGRAM()  <<  "_SOURCES = " << sources << "\n";
	  stream << getBinPROGRAM()  <<  "_LDADD   = " << getLDADD() << " $(LIBSOCKET)" << "\n";
	}

	//************SUBDIRS***************
	if(!subdirs.isEmpty()){ // the SUBDIRS key
	  stream << "\nSUBDIRS = ";
	  for(str2 = subdirs.first();str2 !=0;str2 = subdirs.next()){
	    stream << str2 << " ";
	  }
	}
	stream << "\n";
	//************EXTRA_DIST************
	dist_str = "\nEXTRA_DIST = ";
	bool dist_write=false;
	for(str2 = files.first();str2 !=0;str2 = files.next()){
	  config->setGroup(str2);
	  if (config->readBoolEntry("dist")){
	    dist_str = dist_str + getName(str2) + " ";
	    dist_write = true;
	  }
	}
	if (dist_write){
	  stream << dist_str << "\n";
	}
	//**************install-data-local****************
	bool install_data=false;
	QString install_data_str = "\ninstall-data-local:\n";
	for(str2 = files.first();str2 !=0;str2 = files.next()){
	  config->setGroup(str2);
	  if (config->readBoolEntry("install") && config->readEntry("type") != "SCRIPT"){
	    install_data_str = install_data_str + "\t$(mkinstalldirs) " 
	      + getDir(config->readEntry("install_location")) + "\n";
	    install_data_str = install_data_str + "\t$(INSTALL_DATA) " +
	      getName(str2) + " " + config->readEntry("install_location") + "\n";
	    
	    install_data = true;
	  }
	}
	if(install_data){
	  stream << install_data_str;
	}
	
	//**************install-exec-local****************
	bool install_exec=false;
	QString install_exec_str = "\ninstall-exec-local:\n";
	for(str2 = files.first();str2 !=0;str2 = files.next()){
	  config->setGroup(str2);
	  if (config->readBoolEntry("install") && config->readEntry("type") == "SCRIPT"){
	    install_exec_str = install_exec_str + "\t$(mkinstalldirs) " 
	      + getDir(config->readEntry("install_location")) + "\n";
	    install_exec_str = install_exec_str + "\t$(INSTALL_EXEC) " +
	      getName(str2) + " " + config->readEntry("install_location") + "\n";
	    install_exec = true;
	  }
	}
	if(install_exec){
	  stream << install_exec_str;
	}
	
	//**************uninstall-local*******************
	bool uninstall_local=false;
	QString uninstall_local_str = "\nuninstall-local:\n";
	for(str2 = files.first();str2 !=0;str2 = files.next()){
	  config->setGroup(str2);
	  if (config->readBoolEntry("install")) {
	    uninstall_local_str = uninstall_local_str + "\t-rm -f " + 
	      config->readEntry("install_location") +"\n";
	    uninstall_local=true;
	  }
	}
	if(uninstall_local){
	  stream << uninstall_local_str;
	}
	stream << "\n";	
	found = true;
      }
      if(found == false){
	stream << str +"\n";
      }
    if(str =="####### kdevelop will overwrite this part!!! (end)############"){
      stream << str + "\n";
      found = false;
    } 
    } // end for
  }// end writeonly
  file.close();
  
}

QString CProject::getDir(QString rel_name){
  int pos = rel_name.findRev('/');
  return rel_name.left(pos+1);
}
QString CProject::getName(QString rel_name){
  int pos = rel_name.findRev('/');
  int len = rel_name.length() - pos - 1;
  return rel_name.right(len);
}
void CProject::setSourcesHeaders(){
  // clear the lists
  header_files.clear();
  cpp_files.clear();
  
  QStrList files;
  QString str,filename;
  config->setGroup(getSubDir() + "Makefile.am");
  config->readListEntry("files",files);
  for(str = files.first();str != 0;str = files.next()){
    filename = getName(str);
    if(filename.right(4) == ".cpp" || filename.right(3) == ".cc" || filename.right(2) == ".C"
       || filename.right(2) == ".c"){
      cpp_files.append(filename);
    }
    if(filename.right(2) == ".h"){
      header_files.append(filename);
    }
  }
}
void CProject::setKDevelopWriteArea(QString makefile){
  QString abs_filename = getProjectDir() + makefile;
  QFile file(abs_filename);
  QStrList list;
  bool found = false;
  QTextStream stream(&file);
  QString str;
  
  if(file.open(IO_ReadOnly)){ // read the makefileam
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();
  for(str = list.first();str != 0;str = list.next()){
    if (str == "####### kdevelop will overwrite this part!!! (begin)##########"){
      found = true;
    }
  }
  if(!found){
    // create the writeable area
    file.open(IO_WriteOnly);
    stream << "####### kdevelop will overwrite this part!!! (begin)##########\n";
    stream << "####### kdevelop will overwrite this part!!! (end)############\n";
    for(str = list.first();str != 0;str = list.next()){
      stream << str + "\n";
    }
  }
}
void CProject::writeMakefileAmInfo(TMakefileAmInfo info){
  QString rel_name = info.rel_name;
  config->setGroup(rel_name);
  config->writeEntry("type",info.type);
  config->writeEntry("sub_dirs",info.sub_dirs);
}
void CProject::setFilters(QString group,QStrList& filters){
  config->setGroup("LFV Groups");
  config->writeEntry(group,filters);
}
void CProject::addLFVGroup(QString name,QString ace_group){
  QStrList groups;
  config->setGroup("LFV Groups");
  config->readListEntry("groups",groups);
  if(ace_group.isEmpty()){
    groups.insert(0,name);
    config->writeEntry("groups",groups);
    return;
  }
  int pos = groups.find(ace_group);
  groups.insert(pos+1,name);
  config->writeEntry("groups",groups);
}
void CProject::removeLFVGroup(QString name){
  QStrList groups;
  config->setGroup("LFV Groups");
  config->readListEntry("groups",groups);
  groups.remove(name);
  config->deleteEntry(name,false);
  config->writeEntry("groups",groups);
}
void CProject::addMakefileAmToProject(QString rel_name){
  
  QStrList makefile_list;
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list); 
  makefile_list.append(rel_name);
  config->writeEntry("makefiles",makefile_list);
}

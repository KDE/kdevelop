/***************************************************************************
                 cproject.cpp - the projectproperties
                             -------------------                                         

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

#include <qdir.h>
#include "cproject.h"
#include <iostream.h>
#include <qregexp.h>
#include <kprocess.h>
#include "debug.h"


CProject::CProject(QString file){
  valid = false;
  config = new KSimpleConfig(file);
  prjfile = file;
}

CProject::~CProject(){
}

bool CProject::readProject(){
  
  QFile qfile(prjfile);
  QTextStream stream(&qfile);
  qfile.open(IO_ReadOnly);
  QString str = stream.readLine();
  if(str.contains("KDevelop Project File Version 0.3 #DO NOT EDIT#") ){
    return false;

  }
  
  QFileInfo fileinfo(prjfile);
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
void CProject::setProjectType(QString type){
  config->setGroup("General");
  config->writeEntry("project_type",type);
}
QString CProject::getProjectType(){
  config->setGroup("General");
  return config->readEntry("project_type");
}
void CProject::setKDevPrjVersion(QString version){
  config->setGroup("General");
  config->writeEntry("kdevprj_version",version);
}

QString CProject::getKDevPrjVersion(){
  config->setGroup("General");
  return config->readEntry("kdevprj_version");
}

void CProject::setLFVOpenGroups(QStrList groups){
  config->setGroup("General");
  config->writeEntry("lfv_open_groups", groups );
}

void CProject::getLFVOpenGroups(QStrList& groups){
  config->setGroup("General");
  config->readListEntry("lfv_open_groups",groups);  
}

QString CProject::getClassViewTree(){
  config->setGroup("General");
  return config->readEntry("classview_tree");
}

void CProject::setClassViewTree( QString &tree ){
  config->setGroup("General");
  config->writeEntry("classview_tree", tree );
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

void CProject::setExecuteArgs(QString args){
  config->setGroup("General");
  config->writeEntry("execute_args",args);
}
QString CProject::getExecuteArgs(){
  config->setGroup("General");
  return config->readEntry("execute_args");
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
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("ldflags",flags);
  config->setDollarExpansion(true);
}
QString CProject::getLDFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  str = config->readEntry("ldflags");
  config->setDollarExpansion(true);
  return str;
}
void CProject::setCXXFLAGS(QString flags){
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("cxxflags",flags);
  config->setDollarExpansion(true);
}
QString CProject::getCXXFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  str = config->readEntry("cxxflags");
  config->setDollarExpansion(true);
  return str;
}
void CProject::setAdditCXXFLAGS(QString flags){
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("addcxxflags",flags);
  config->setDollarExpansion(true);
}
QString CProject::getAdditCXXFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  str = config->readEntry("addcxxflags");
  config->setDollarExpansion(true);
  return str;
}
/** set the librarys*/
void CProject::setLDADD(QString libstring){
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  config->writeEntry("ldadd",libstring);
  config->setDollarExpansion(true);
}

QString CProject::getLDADD(){
  QString str;
  config->setDollarExpansion(false);
  config->setGroup("Config for BinMakefileAm");
  str = config->readEntry("ldadd");
  config->setDollarExpansion(true);
  return str;
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

TMakefileAmInfo CProject::getMakefileAmInfo(QString rel_name){
  TMakefileAmInfo info;
  config->setGroup(rel_name);
  info.rel_name = rel_name;
  info.type = config->readEntry("type");
  config->readListEntry("sub_dirs",info.sub_dirs);
  return info;
  
}

// void CProject::writeMakefileAmInfo(TMakefileAmInfo info){
//   QString rel_name = info.rel_name;
//   config->setGroup(rel_name);
//   config->writeEntry("type",info.type);
//   config->writeEntry("sub_dirs",info.sub_dirs);
// }



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
bool CProject::addDialogFileToProject(QString rel_name,TDialogFileInfo info){
  config->setGroup(info.rel_name);
  config->writeEntry("baseclass",info.baseclass);
  config->writeEntry("widget_files",info.widget_files);
  config->writeEntry("is_toplevel_dialog",info.is_toplevel_dialog);
  config->writeEntry("header_file",info.header_file);
  config->writeEntry("cpp_file",info.header_file);
  config->writeEntry("data_file",info.data_file);
  config->writeEntry("classname",info.classname);
  
  TFileInfo file_info;
  file_info.rel_name = info.rel_name;
  file_info.type = "DIALOG";
  file_info.dist = info.dist;
  file_info.install = info.install;

  return addFileToProject(file_info.rel_name,file_info);
}
bool CProject::addFileToProject(QString rel_name,TFileInfo info){
  
  // normalize it a little bit
  rel_name.replace(QRegExp("///"),"/"); // remove ///
  rel_name.replace(QRegExp("//"),"/"); // remove //
  
  
  QStrList all_files;
  getAllFiles(all_files);

  if(all_files.contains(rel_name) > 0){ // file is already in Project?
    return false; // no new subdir;
  }
  
  QStrList list_files;
  QString makefile_name;
  QStrList sub_dirs;

  // find the correspond. Makefile.am, store it into makefile_name
  int slash_pos = rel_name.findRev('/');
  if(slash_pos == -1) { // not found
    makefile_name = "Makefile.am";
  }
  else {
    makefile_name = rel_name.copy();
    makefile_name.truncate(slash_pos+1);
    makefile_name += "Makefile.am";
  }

  //add the file into the filesentry in the Makefileam group
  config->setGroup(makefile_name);
  config->readListEntry("files",list_files);
  list_files.append(rel_name);
  config->writeEntry("files",list_files);
  
  //++++++++++++++add Makefile.am and the toplevels makefile.ams to the project if needed (begin)
  QStrList makefile_list;
  QStrList check_makefile_list;
  // find the makefiles to check
  int slash2_pos;
  check_makefile_list.append(makefile_name);
  QString makefile_name_org = makefile_name.copy();

  //  cerr << endl << "*check:*" << makefile_name;
  
  while((slash_pos = makefile_name.findRev('/')) != -1){ // if found
    slash2_pos = makefile_name.findRev('/',slash_pos-1);
    if(slash2_pos != -1){
      makefile_name.remove(slash2_pos,slash_pos-slash2_pos);
      check_makefile_list.append(makefile_name);
    } 
    else{
      makefile_name = "";
    }
  }

  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);

  TMakefileAmInfo makefileaminfo;
  
  for(makefile_name=check_makefile_list.first();makefile_name!=0;makefile_name=check_makefile_list.next()){ 
    // check if current makefile exists and all makefile above,if not add it
    if(makefile_list.find(makefile_name) == -1){
      makefileaminfo.rel_name = makefile_name;
      if(makefile_name_org == makefile_name){ // the first makefileam
	if(info.type == "SOURCE") makefileaminfo.type = "static_library"; // cool 
	else makefileaminfo.type = "normal";
	addMakefileAmToProject(makefile_name,makefileaminfo);
      }
      else{
	makefileaminfo.type = "normal";
	addMakefileAmToProject(makefile_name,makefileaminfo);
      }
    }
  }
  //++++++++++++++++add Makefile to the project if needed (end)


  makefile_name = check_makefile_list.first(); // get the complete makefilename


  // change the makefile type if needed
  if(info.type == "SOURCE"){ // a static library is needed?
    config->setGroup(makefile_name);
    if(config->readEntry("type") != "prog_main"){
      config->writeEntry("type","static_library");
    }
  }
  // and at last: modify the subdir entry in every Makefile.am if needed


  QString subdir;
  bool new_subdir=false;

  while((slash_pos = makefile_name.findRev('/')) != -1){ // if found
    slash2_pos = makefile_name.findRev('/',slash_pos-1);
    if(slash2_pos != -1){
      subdir = makefile_name.mid(slash2_pos+1,slash_pos-slash2_pos-1);
      KDEBUG1(KDEBUG_INFO,DIALOG,"SUBDIR %s",subdir.data());
      makefile_name.remove(slash2_pos,slash_pos-slash2_pos);
      config->setGroup(makefile_name);
      sub_dirs.clear();
      config->readListEntry("sub_dirs",sub_dirs);
      
      if(sub_dirs.find(subdir) == -1){
	new_subdir = true;
	sub_dirs.append(subdir);
	config->writeEntry("sub_dirs",sub_dirs);
      }
    }
    else{
      // the subdirs of the topdir are special
      subdir = makefile_name.left(slash_pos);
      config->setGroup("Makefile.am");
      sub_dirs.clear();
      config->readListEntry("sub_dirs",sub_dirs);
      
      if(sub_dirs.find(subdir) == -1){
	new_subdir = true;
	sub_dirs.append(subdir);
	config->writeEntry("sub_dirs",sub_dirs);
      
      }
      makefile_name = "";
    }
  }
  if(new_subdir){
    updateConfigureIn();
  }
//  setSourcesHeaders();   // moved to the end because it's reading the sync'ed file for the classviewer... Ralf
  //  createMakefilesAm(); // do some magic generation

  // write the fileinfo
  config->setGroup(info.rel_name);
  config->writeEntry("type",info.type);
  config->writeEntry("dist",info.dist);
  config->writeEntry("install",info.install);
  // save the $ because kconfig removes one
  info.install_location.replace("[\\$]","$$");
  config->writeEntry("install_location",info.install_location);
  config->sync();
  setSourcesHeaders();
  return new_subdir;
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
  updateMakefilesAm();
}
void CProject::updateMakefilesAm(){

  if ( getProjectType() == "normal_empty" )
    return;

  QString makefile;
  QStrList makefile_list;
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);  
  for(makefile = makefile_list.first();makefile !=0;makefile =makefile_list.next()){ // every Makefile
    config->setGroup(makefile);
    updateMakefileAm(makefile); 
  }
}
void CProject::updateMakefileAm(QString makefile){
  setKDevelopWriteArea(makefile);
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

  QString libname;
  QStrList static_libs;
  getAllStaticLibraries(static_libs);
  
  QString sources;
  QStrList source_files;
  QStrList po_files;
  QString pos;
  config->setGroup(makefile);
  config->readListEntry("files",files);
  config->readListEntry("sub_dirs",subdirs);
  if(file.open(IO_ReadOnly)){ // read the makefileam
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();

  if(file.open(IO_WriteOnly)){
    for(str = list.first();str != 0;str = list.next()){ // every line
      if(str == "####### kdevelop will overwrite this part!!! (begin)##########"){
 	stream << str << "\n";
	
	//***************************generate needed things for the main makefile*********
	if (config->readEntry("type") == "prog_main"){ // the main makefile
	  getSources(makefile,source_files);
	  for(str= source_files.first();str !=0;str = source_files.next()){
	    sources =  str + " " + sources ;
	  }
	  stream << "CXXFLAGS = " << getCXXFLAGS()+" "+getAdditCXXFLAGS() << "\n";
	  stream << "LDFLAGS = " << getLDFLAGS()  << "\n";
	  stream << getBinPROGRAM()  <<  "_SOURCES = " << sources << "\n";
	  if(static_libs.isEmpty()){
	    stream << getBinPROGRAM()  <<  "_LDADD   = " << getLDADD();
	  }
	  else{ // we must link some libs
	    stream << getBinPROGRAM()  <<  "_LDADD   = ";
	    for(libname = static_libs.first();libname != 0;libname = static_libs.next()){
	      stream << libname.replace(QRegExp("^"+getSubDir()),"./") << " "; // remove the subdirname
	    }
	    stream << getLDADD();
	  }
	  
	  if(getProjectType() != "normal_cpp") {
	    stream << " $(LIBSOCKET)" << "\n";
	  }
	  else{
	    stream << "\n";
	  }
	}
	//***************************generate needed things for static_library*********
	config->setGroup(makefile);
	if(config->readEntry("type") == "static_library"){
	  getSources(makefile,source_files);
	  for(str= source_files.first();str !=0;str = source_files.next()){
	    sources =  str + " " + sources ;
	  }
	  QDir dir(getDir(makefile));
	  if (getProjectType() != "normal_cpp"){
	    stream << "\nINCLUDES = $(all_includes)\n\n";
	    stream << "lib" << dir.dirName() << "_a_METASOURCES = USE_AUTOMOC\n\n";
	  }
	  stream << "noinst_LIBRARIES = lib" << dir.dirName() << ".a\n\n";
	  stream << "lib" << dir.dirName() << "_a_SOURCES = " << sources << "\n";
	}

	//***************************generate needed things for a po makefile*********
	if (config->readEntry("type") == "po"){ // a po makefile
	  getPOFiles(makefile,po_files);
	  for(str= po_files.first();str !=0;str = po_files.next()){
	    pos =  str + " " + pos ;
	  }
	  
	  stream <<  "POFILES = " << pos << "\n";
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
void CProject::getAllTopLevelDialogs(QStrList& list){
  list.clear();
  QStrList  all_files;
  TFileInfo info;
  getAllFiles(all_files);
  QString file;
  for(file = all_files.first();file != 0;file = all_files.next()){
    info = getFileInfo(file);
    if(info.type == "DIALOG" && file.right(8) == ".kdevdlg"){
      list.append(info.rel_name);
    }
  }
}
TDialogFileInfo CProject::getDialogFileInfo(QString rel_filename){
  TDialogFileInfo info;
  config->setGroup(rel_filename);
  info.rel_name = rel_filename;
  info.type = config->readEntry("type");
  info.dist = config->readBoolEntry("dist");
  info.install = config->readBoolEntry("install");
  info.install_location = config->readEntry("install_location");
  info.classname = config->readEntry("classname");
  info.baseclass = config->readEntry("baseclass");
  info.is_toplevel_dialog = config->readBoolEntry("is_toplevel_dialog");
  config->readListEntry("widget_files",info.widget_files);
  info.header_file = config->readEntry("header_file");
  info.source_file = config->readEntry("source_file");
  info.data_file = config->readEntry("data_file");
  return info;  
}
void CProject::getSources(QString rel_name_makefileam,QStrList& sources){
  sources.clear();
  QStrList files;
  QString file;
  TFileInfo info;
  config->setGroup(rel_name_makefileam);
  config->readListEntry("files",files);
  
  for(file = files.first();file != 0;file = files.next()){
    if(file.right(4) == ".cpp" || file.right(3) == ".cc" || file.right(2) == ".C"
       || file.right(2) == ".c" || file.right(4) == ".cxx" 
       || file.right(3) == ".ec" || file.right(5) == ".ecpp" ){   
      info = getFileInfo(file);
      if(info.type == "SOURCE"){
	sources.append(getName(file));
      }
    }
  }
}
void CProject::getPOFiles(QString rel_name_makefileam,QStrList& po_files){
  po_files.clear();
  QStrList files;
  QString file;
  TFileInfo info;
  config->setGroup(rel_name_makefileam);
  config->readListEntry("files",files);
  
  for(file = files.first();file != 0;file = files.next()){
    if(file.right(3) == ".po"){   
      info = getFileInfo(file);
      if(info.type == "PO"){
	po_files.append(getName(file));
      }
    }
  }
}
void CProject::setSourcesHeaders(){
  // clear the lists
  header_files.clear();
  cpp_files.clear();
  
  QStrList files;
  QString file;
  TFileInfo info;

  getAllFiles(files);

  for(file = files.first();file != 0;file = files.next()){
    if(file.right(4) == ".cpp" || file.right(3) == ".cc" || file.right(2) == ".C"
       || file.right(2) == ".c" || file.right(4) == ".cxx" || file.right(3) == ".ec" || file.right(5) == ".ecpp" ){    // added .cxx    990204 rnolden
      info = getFileInfo(file);
      if(info.type == "SOURCE"){
	cpp_files.append(getProjectDir()+file);
      }
    }
    if(file.right(2) == ".h" || file.right(3) == ".hh" || file.right(2) == ".H"
       || file.right(2) == ".hxx" ){       // added .hxx    990204 rnolden
      info = getFileInfo(file);
      if(info.type == "HEADER"){
	header_files.append(getProjectDir()+file);
      }
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
void CProject::addMakefileAmToProject(QString rel_name,TMakefileAmInfo info){
  
  config->setGroup(rel_name);
  config->writeEntry("type",info.type);
  config->writeEntry("sub_dirs",info.sub_dirs);

  QStrList makefile_list;
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list); 
  makefile_list.append(rel_name);
  config->writeEntry("makefiles",makefile_list);
}
bool CProject::isDirInProject(QString rel_name){

  KDEBUG(KDEBUG_INFO,CPROJECT,"isDirInProject() Don't use this function,it's not implemented!");
  return true;
  int pos = rel_name.findRev('/');
  QString dir_name;
   
  
  if(pos == -1){ // not found
    dir_name = rel_name.copy();
    rel_name = "Makefile.am";
  }
  else{
    dir_name = rel_name.right(rel_name.length()-pos-1);
    rel_name.truncate(pos+1);
    rel_name.append("Makefile.am");
  }
  
  TMakefileAmInfo info = getMakefileAmInfo(rel_name);
  QString str;
  
  for(str=info.sub_dirs.first();str!=0;str=info.sub_dirs.next()){
    cerr << endl << str;
  }
}


void CProject::updateConfigureIn(){
  QString abs_filename = getProjectDir() + "/configure.in";
  QFile file(abs_filename);
  QStrList list;
  QTextStream stream(&file);
  QString str;
  QStrList makefile_list;
  QString makefile;
    

  if(file.open(IO_ReadOnly)){ // read the configure.in
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();

  file.open(IO_WriteOnly);
  
  for(str = list.first();str != 0;str = list.next()){
    if(str.find("AC_OUTPUT(") != -1){ // if found
      stream << "AC_OUTPUT(";
      config->setGroup("General");
      config->readListEntry("makefiles",makefile_list);  
      for(makefile = makefile_list.first();makefile !=0;makefile =makefile_list.next()){
	stream << makefile.remove(makefile.length()-3,3) << " ";
      }
      stream << ")\n";
      
    }
    else if(str.find("KDE_DO_IT_ALL(") != -1){
      stream << "KDE_DO_IT_ALL(";
      stream << getProjectName().lower() << "," << getVersion();
      stream << ")\n";
    }
    else if(str.find("AM_INIT_AUTOMAKE(") != -1){
      stream << "AM_INIT_AUTOMAKE(";
      stream << getProjectName().lower() << "," << getVersion();
      stream << ")\n";
    }
    else{
      stream << str + "\n";
    }
  }
  
 
}
void  CProject::writeWorkspace(TWorkspace ws){
  switch(ws.id){
  case 1:
    config->setGroup("Workspace_1");
    break;
  case 2:
    config->setGroup("Workspace_2");
    break;
  case 3:
    config->setGroup("Workspace_3");
    break;
  default:
    config->setGroup("Workspace_1");
  }
  config->writeEntry("openfiles",ws.openfiles);
  config->writeEntry("header_file",ws.header_file);
  config->writeEntry("cpp_file",ws.cpp_file);
  config->writeEntry("browser_file",ws.browser_file);
  config->writeEntry("show_treeview",ws.show_treeview);
  config->writeEntry("show_outputview",ws.show_output_view);
  config->sync();
}
void CProject::setCurrentWorkspaceNumber(int id){
   config->setGroup("General");
   config->writeEntry("workspace",id);
   config->sync();
}
int CProject::getCurrentWorkspaceNumber(){
  config->setGroup("General");
  int i = config->readNumEntry("workspace",1);
  return i;
}
TWorkspace CProject::getWorkspace(int id){
  TWorkspace ws;

  switch(id){
  case 1:
    config->setGroup("Workspace_1");
    break;
  case 2:
    config->setGroup("Workspace_2");
    break;
  case 3:
    config->setGroup("Workspace_3");
    break;
  default:
    config->setGroup("Workspace_1");
  }

  config->readListEntry("openfiles",ws.openfiles);
  ws.header_file = config->readEntry("header_file");
  ws.cpp_file = config->readEntry("cpp_file");
  ws.browser_file = config->readEntry("browser_file");
  ws.show_treeview = config->readBoolEntry("show_treeview",true);
  ws.show_output_view =config->readBoolEntry("show_outputview",true);
  return ws;
}

void CProject::getAllStaticLibraries(QStrList& libs){
  QDir dir;
  QStrList makefiles;
  QString makefile;

  libs.clear();
  config->setGroup("General");
  
  config->readListEntry("makefiles",makefiles);
  
  for(makefile=makefiles.first();makefile != 0;makefile=makefiles.next()){
    config->setGroup(makefile);
    if(config->readEntry("type") == "static_library"){
      
      dir.setPath(getDir(makefile));
      libs.append(getDir(makefile) + "lib" + dir.dirName() + ".a");
    }
  }
}




/***************************************************************************
                    cproject.h - the projectproperties
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

#ifndef CPROJECT_H
#define CPROJECT_H

#include <qstring.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <ksimpleconfig.h>

/** info struct for a Makefile.am (used in CProject)
  *@author Sandy Meier
  */
struct TMakefileAmInfo {
  QString rel_name;
  /** normal,prog_main,static_library*/
  QString type;
  QStrList sub_dirs;
};
/** info struct for a file (used in CProject)
  *@author Sandy Meier
  */
struct TFileInfo {
  /** location + name in the projectdir*/
  QString rel_name;
  /** SOURCE,HEADER,SCRIPT,DATA*/
  QString type;
  /** include in distribution?*/
  bool dist;
  /** install?*/
  bool install;
  /** install-location*/
  QString install_location;
};
struct TWorkspace {
  int id; // 1, 2 or 3
  QStrList openfiles;
  QString header_file;
  QString cpp_file;
  QString browser_file;
  bool show_treeview;
  bool show_output_view;  
};

/** this class includes the properties of a project and some methods to read
  * and write these props,all Makefiles.am are registered in the [General] Group,every Makefile.am
  * has it own group and every file in the project too
  * 
  ** format of the general group:**
  * [General]
  * classview_tree: a treestructure of all classes.
  * makefiles: all makefiles in the project
  * project_type: at the moment "normal_cpp","mini_kde","normal_kde","normal_qt"
  * author: name of the author
  * sub_dir: name of main subdir, not "po"
  * project_name: it's clear
  * version: version number (String)
  * email: emailaddress of the author
  * kdevprj_version: internal versions-numbering, not associated with the kdevelop version
  *
  ** format of the Makefile.am groups:**
  * [xxx/Makefile.am]
  * type: at the moment "normal","prog_main" or "po"
  * files: all files in the current dir
  * sub_dirs: the subdirs
  *
  ** format of the file groups:**
  * [xxx/xxx]
  * install_location: install location for "make install"
  * dist: include the file into the distribution at "make dist"
  * install: install it at "make install" ?
  * type: at the moment "DATA","HEADER","SCRIPT","SOURCE" "PO"
  *
  *@author Sandy Meier
  */
class CProject  {

public: 
  /** construtor */
  CProject(QString file); 
  /** destructor */
  ~CProject();

public: // Methods to store project options

  /** Store the version of the kdevprj file format*/
  void setKDevPrjVersion(QString version);
  /** at the moment there exists 3 types: normal_cpp,normal_kde,mini_kde*/
  void setProjectType(QString type);
  /** Store the projectname. */
  void setProjectName(QString name);
  /** the mainsubdir, not "po"*/
  void setSubDir(QString dirname);
  /** Store the authors name. */
  void setAuthor(QString name);
  /** Store the authors eMail-address. */
  void setEmail(QString email);
  /** Store the project version. */
  void setVersion(QString version);
  /** Store the classview tree. */
  void setClassViewTree( QString &tree );
  /** at the moment only an english sgmlfile in docs/en/ */
  void setSGMLFile(QString version);  
  void setShortInfo(QStrList short_info);
  void setBinPROGRAM(QString name);
  /** Store the linked flags. */
  void setLDFLAGS(QString flags);
  /** Store the C++ compiler flags. */
  void setCXXFLAGS(QString flags);
  /** Store the librarys*/
  void setLDADD(QString libstring);
  void setAdditCXXFLAGS(QString flags);
  /** Store options for make( f.e. "-k" for "make -k"*/
  void setMakeOptions(QString options);
  /** Store the commandline execution arguments for the project binary */
  void setExecuteArgs(QString args);

public: // Methods to fetch project options

  /** Fetch the classview tree. */
  QString getClassViewTree();
  /** Fetch the commandline execution arguments for the project binary. */
  QString getExecuteArgs();
  /** Fetch the options for make( i.e "-k" for "make -k". */
  QString getMakeOptions();
  /** Fetch the version of the kdevprj file format*/
  QString getKDevPrjVersion();
  /** Fetch the type: {normal_cpp,normal_kde,mini_kde} */
  QString getProjectType();
  QString getProjectFile();
  QString getProjectDir();
  QString getSubDir();
  /** Fetch the authors name. */
  QString getAuthor();
  /** Fetch the authors eMail-address. */
  QString getEmail();
  /** Fetch the projects version. */
  QString getVersion();
  QString getSGMLFile();
  QString getProjectName();
  QStrList getShortInfo();
  QString getBinPROGRAM();
  QString getLDFLAGS();
  QString getCXXFLAGS();
  /** Fetch the librarys. */
  QString getLDADD();
  QString getAdditCXXFLAGS();
  /** Fetch all groups in the logic file view. */
  void getLFVGroups(QStrList& groups);
  TFileInfo getFileInfo(QString filename);
  TMakefileAmInfo getMakefileAmInfo(QString rel_name);

public: // Public queries

  /* return the sources(.cpp,.c,.C,.cxx) */
  QStrList& getSources() { return cpp_files; }
  /** return the headers(.h.) */
  QStrList& getHeaders() { return header_files; }

  /**the new projectmanagment*/
  void getAllFiles(QStrList& list);

  /** Get all sources for this makefile */
  void getSources(QString rel_name_makefileam,QStrList& sources);
  /** Get all po files for this makefile */
  void getPOFiles(QString rel_name_makefileam,QStrList& po_files);

  /** check if a subdir is in the project (registered)*/
  bool isDirInProject(QString rel_name);

public: // Public methods

  /** if ace_group == empty insert prepend at the grouplist*/
  void addLFVGroup(QString name,QString ace_group);
  void removeLFVGroup(QString name);
  void getFilters(QString group,QStrList& filters);
  void setFilters(QString group,QStrList& filters);
  void writeFileInfo(TFileInfo info);
  //  void writeMakefileAmInfo(TMakefileAmInfo info);
  /** return true if a new subdir was added to the project*/
  bool addFileToProject(QString rel_name,TFileInfo info);
  void removeFileFromProject(QString rel_name);
  void setKDevelopWriteArea(QString makefile);
  void addMakefileAmToProject(QString rel_name,TMakefileAmInfo info);

  void updateConfigureIn();
  void updateMakefilesAm();
  //  void createBinMakefileAm();
  void updateMakefileAm(QString makefile);
  /** read the projectfile */
  bool readProject(); 
  /** write the projectfile to the disk*/
  void writeProject(); 
  /** true if the project was read*/
  bool valid;
  void  writeWorkspace(TWorkspace ws);
  TWorkspace getWorkspace(int id);
  void setCurrentWorkspaceNumber(int id);
  int getCurrentWorkspaceNumber();

protected:

  void setSourcesHeaders();
  void getAllStaticLibraries(QStrList& libs);
  QString getDir(QString rel_name);
  QString getName(QString rel_name);

protected: // Protected attributes
  QString prjfile;
  QString dir;
  
  /** a list of all cpp-files in the project*/
  QStrList cpp_files;
  /** a list of all header-files in the project*/
  QStrList header_files;

  KSimpleConfig* config;
  
};
#endif

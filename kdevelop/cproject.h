/***************************************************************************
                    cproject.h - the projectproperties
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
  /** normal,prog_main*/
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

/** this class includes the properties of a project and some methods to read
  * and write these props
  *@author Sandy Meier
  */
class CProject  {

public: 
  /** construtor */
  CProject(); 
  /** destructor */
  ~CProject();
  /** the version of the kdevprj file format*/
  void setKDevPrjVersion(QString version);
  /** at the moment there exists 3 types: normal_cpp,normal_kde,mini_kde*/
  void setProjectType(QString type);
  void setProjectName(QString name);
  void setSubDir(QString dirname);
  void setAuthor(QString name);
  void setEmail(QString email);
  void setVersion(QString version);
  void setSGMLFile(QString version);  
  void setShortInfo(QStrList short_info);
  void setBinPROGRAM(QString name);
  void setLDFLAGS(QString flags);
  void setCXXFLAGS(QString flags);
  /** set the librarys*/
  void setLDADD(QString libstring);
  void setAdditCXXFLAGS(QString flags);

  /**options for make( f.e. "-k" for "make -k"*/

  void setMakeOptions(QString options);
  QString getMakeOptions();
  
  /**the new projectmanagment*/
  void getAllFiles(QStrList& list);
  void getLFVGroups(QStrList& groups);
  /** if ace_group == empty insert prepend at the gouplist*/
  void addLFVGroup(QString name,QString ace_group);
  void removeLFVGroup(QString name);
  void getFilters(QString group,QStrList& filters);
  void setFilters(QString group,QStrList& filters);
  void writeFileInfo(TFileInfo info);
  void writeMakefileAmInfo(TMakefileAmInfo info);
  void addFileToProject(QString rel_name);
  void removeFileFromProject(QString rel_name);
  void setKDevelopWriteArea(QString makefile);
  void addMakefileAmToProject(QString rel_name);

  QString getKDevPrjVersion();
  QString getProjectType();
  QString getProjectFile();
  QString getProjectDir();
  QString getSubDir();
  QString getAuthor();
  QString getEmail();
  QString getVersion();
  QString getSGMLFile();
  QString getProjectName();
  QStrList getShortInfo();
  QString getBinPROGRAM();
  QString getLDFLAGS();
  QString getCXXFLAGS();
  QString getLDADD();
  QString getAdditCXXFLAGS();
  TFileInfo getFileInfo(QString filename);
  TMakefileAmInfo getMakefileAmInfo(QString rel_name);
  
  /* return the sources(.cpp,.c,.C,.cxx) for the main-makefile*/
  QStrList& getSources();
  /** return the headers(.h.) for the main-makefile*/
  QStrList& getHeaders();
  
  /* usefull information methods*/
  /** check if a subdir is in the project (registered)*/
  bool isDirInProject(QString rel_name);

  void createMakefilesAm();
  //  void createBinMakefileAm();
  void createMakefileAm(QString makefile);
  /** read the projectfile */
  bool readProject(QString file); 
  /** write the projectfile to the disk*/
  void writeProject(); 
  /** true if the project was read*/
  bool valid; 

protected:

  QString getDir(QString rel_name);
  QString getName(QString rel_name);
  void setSourcesHeaders();
  QString prjfile;
  QString dir;
  
 //  /** a list of all cpp-files in the project*/
  QStrList cpp_files;
  //   /** a list of all header-files in the project*/
  QStrList header_files;
  KSimpleConfig* config;
  
};
#endif

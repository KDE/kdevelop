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


class VersionControl;


/** This type defines the different types that can 
  * exist in a project. 
  * @author Jonas Nordin
  */
typedef enum _Project_enum
{
  CORBA_SOURCE,
  CPP_SOURCE,
  CPP_HEADER,
  SCRIPT,
  DATA,
  PO,
  KDEV_DIALOG,
  PT_END_POS
} ProjectFileType;

/** info struct for a Makefile.am (used in CProject)
  *@author Sandy Meier
  */
struct TMakefileAmInfo {
  QString rel_name;
  /** normal,prog_main,static_library*/
  QString type;
  QStrList sub_dirs;
};

/** info struct for a dialogfile (used in CProject) 
 * maybe I will inherited this from TFileInfo in the future
 *@author Sandy Meier
 */
struct TDialogFileInfo {
  /** location + name in the projectdir*/
  QString rel_name;
  /** CPP_SOURCE,CPP_HEADER,SCRIPT,DATA,PO,KDEV_DIALOG*/
  QString type;
  /** include in distribution?*/
  bool dist;
  /** install?*/
  bool install;
  /** install-location*/
  QString install_location;
  
  QString classname;
  /**QWidget,QFrame,QDialog,QTabDialog,Custom*/
  QString baseclass;
  /** toplevel Dialog?, is needed to support QTabDialogs*/
  bool is_toplevel_dialog;
  /** the children if baseclass is a QTabdialog,relative filenames of other *.kdevdlg*/
  QStrList widget_files;
  
  QString header_file;
  QString source_file;
  QString data_file;  
};

/** info struct for a file (used in CProject)
  *@author Sandy Meier
  */
struct TFileInfo {
  /** location + name in the projectdir*/
  QString rel_name;
  /** CPP_SOURCE,CPP_HEADER,SCRIPT,DATA,PO,KDEV_DIALOG*/
  ProjectFileType type;
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
  void setKDevPrjVersion(const char* version) { writeGroupEntry( "General", "kdevprj_version", version ); }

  /** Store the project type: {normal_cpp,normal_kde,mini_kde,normal_empty,normal_qt} */
  void setProjectType(const char *type)       { writeGroupEntry( "General", "project_type", type ); }

  /** Store the projectname. */
  void setProjectName(const char *name)       { writeGroupEntry( "General", "project_name", name ); }

  /** the mainsubdir, not "po"*/
  void setSubDir(const char *dirname)         { writeGroupEntry( "General", "sub_dir", dirname ); }

  /** Store the authors name. */
  void setAuthor(const char *name)            { writeGroupEntry( "General", "author", name ); }

  /** Store the authors eMail-address. */
  void setEmail(const char *email)            { writeGroupEntry( "General", "email", email ); }

  /** Store the project version. */
  void setVersion(const char *version)        { writeGroupEntry( "General", "version",version ); }

  /** Store the name of version control system */
  void setVCSystem(const char *vc)            { writeGroupEntry( "General", "version_control", vc ); }

    /** Store the classview tree. */
  void setClassViewTree( const char *tree )   { writeGroupEntry( "General", "classview_tree", tree ); }

  /** at the moment only an english sgmlfile in docs/en/ */
  void setSGMLFile(const char *name)          { writeGroupEntry( "General", "sgml_file", name ); }

  /** Store options for make( f.e. "-k" for "make -k"*/
  void setMakeOptions(const char *options)    { writeGroupEntry( "General", "make_options", options ); }

  /** Store the commandline execution arguments for the project binary */
  void setExecuteArgs(const char *args)       { writeGroupEntry( "General", "execute_args", args ); }

  void setBinPROGRAM(const char *name)        { writeGroupEntry( "Config for BinMakefileAm", "bin_program", name ); }


  /** Store all open groups in the LFV*/
  void setLFVOpenGroups(QStrList groups);

  void setShortInfo(QStrList short_info);

  /** Store the linker flags. */
  void setLDFLAGS(QString flags);
  void setLDADD(QString libstring);

  /** Store the C++ compiler flags. */
  void setCXXFLAGS(QString flags);
  void setAdditCXXFLAGS(QString flags);

  void setFilters(QString group,QStrList& filters);

public: // Methods to fetch project options

  /** Fetch the version of the kdevprj file format*/
  QString getKDevPrjVersion() { return readGroupEntry( "General", "kdevprj_version" ); }

  /** Fetch the type: {normal_cpp,normal_kde,mini_kde} */
  QString getProjectType()    { return readGroupEntry( "General", "project_type" ); }

  /** Fetch the name of the project. */
  QString getProjectName()    { return readGroupEntry( "General", "project_name" ); }

  /** Fetch the main subdir, not "po". */
  QString getSubDir()         { return readGroupEntry( "General", "sub_dir" ); }

  /** Fetch the authors name. */
  QString getAuthor()         { return readGroupEntry( "General", "author" ); }

  /** Fetch the authors eMail-address. */
  QString getEmail()          { return readGroupEntry( "General", "email" ); }

  /** Fetch the projects' version. */
  QString getVersion()        { return readGroupEntry( "General", "version" ); }

  /** Fetch the name of the version control system */
  QString getVCSystem()       { return readGroupEntry( "General", "version_control" ); }

  /** Fetch the classview tree. */
  QString getClassViewTree()  { return readGroupEntry( "General", "classview_tree" ); }

  /** Fetch the documentation file. */
  QString getSGMLFile()       { return readGroupEntry( "General", "sgml_file" ); }

  /** Fetch the options for make( i.e "-k" for "make -k". */
  QString getMakeOptions()    { return readGroupEntry( "General", "make_options" ); }

  /** Fetch the commandline execution arguments for the project binary. */
  QString getExecuteArgs()    { return readGroupEntry( "General", "execute_args" ); }

  QString getBinPROGRAM()     { return readGroupEntry( "Config for BinMakefileAm", "bin_program" ); }

  /** Fetch all groups in the logic file view. */
  void getLFVGroups(QStrList& groups);

  /** Fetch all open groups in the LFV. */
  void getLFVOpenGroups(QStrList& groups);

  QStrList getShortInfo();

  /** Fetch the librarys. */
  QString getLDFLAGS();
  QString getLDADD();
  QString getCXXFLAGS();
  QString getAdditCXXFLAGS();

  void getFilters(QString group,QStrList& filters);

  TFileInfo getFileInfo(QString filename);

  TMakefileAmInfo getMakefileAmInfo(QString rel_name);

  TDialogFileInfo getDialogFileInfo(QString rel_filename);

public: // Public queries

  /** Return the actual file in which the project is stored. */
  QString getProjectFile() { return prjfile; }

  /** Return the project dir. */
  QString getProjectDir()  { return dir; }

  /** Return the version control object */
  VersionControl *getVersionControl() { return vc; }
  /* return the sources(.cpp,.c,.C,.cxx) */
  QStrList& getSources()   { return cpp_files; }

  /** return the headers(.h.) */
  QStrList& getHeaders()   { return header_files; }

  /**the new projectmanagment*/
  void getAllFiles(QStrList& list);
  void getAllTopLevelDialogs(QStrList& list);

  /** Get all sources for this makefile */
  void getSources(QString rel_name_makefileam,QStrList& sources);

  /** Get all po files for this makefile */
  void getPOFiles(QString rel_name_makefileam,QStrList& po_files);

  bool isKDEProject();
  bool isQtProject();
  bool isCustomProject();
  /** check if a subdir is in the project (registered). */
  bool isDirInProject(QString rel_name);

  

  /** Method that returns the filetype for a given file by looking at 
   * it's extension.
   * @param aFile The filename to check.
   */
  static ProjectFileType getType( const char *aFile );

  /** Return the type matching a string.
   * @param aStr The string representation of a type.
   */
  ProjectFileType getTypeFromString( const char *aStr );

  /** Return a string matching a type. 
   * @param aType The type to get the string for.
   */
  const char *getTypeString( ProjectFileType aType );

public: // Public methods

  /** if ace_group == empty insert prepend at the grouplist*/
  void addLFVGroup(QString name,QString ace_group);
  void removeLFVGroup(QString name);
  void writeFileInfo(TFileInfo info);
  //  void writeMakefileAmInfo(TMakefileAmInfo info);
  /** return true if a new subdir was added to the project*/
  bool addFileToProject(QString rel_name,TFileInfo info);
  bool addDialogFileToProject(QString rel_name,TDialogFileInfo info);
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

/** method to fill up a string template with actual project info
  *@author Walter Tasin
  */
  QString& setInfosInString(QString& strtemplate, bool basics=true);


protected:

  /** Write an entry to the project file. 
   * @param group Name of the group.
   * @param tag   The value-tag e.g version.
   * @param entry The string to store. */
  void writeGroupEntry( const char *group, const char *tag, const char *entry );

  /** Read an entry from the project file and return it.
   * @param group Name of the group.
   * @param tag   The value-tag e.g version. */
  QString readGroupEntry( const char *group, const char *tag );

  void setSourcesHeaders();
  void getAllStaticLibraries(QStrList& libs);
  QString getDir(QString rel_name);
  QString getName(QString rel_name);

protected: // Protected attributes

  /** The actual project file. */
  QString prjfile;

  /** The project directory. */
  QString dir;

  /** Version control object */
  VersionControl *vc;
    
  /** A list of all cpp-files in the project*/
  QStrList cpp_files;

  /** A list of all header-files in the project*/
  QStrList header_files;

  /** Maps a ProjectFileType to a string. */
  QString *ptStringMap;
  
  KSimpleConfig config;
  
};
#endif

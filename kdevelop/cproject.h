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


class VersionControl;
class KSimpleConfig;

// the currently supported version.
#define KDEVELOP_PRJ_VERSION "1.3"

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
  QT_TS,
  KDEV_DIALOG,
  LEXICAL,
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
  /** CPP_SOURCE,CPP_HEADER,SCRIPT,DATA,PO,KDEV_DIALOG, LEXICAL, QT_TS, QT_IF (qt translation(ts), qt interface (ui)*/
  ProjectFileType type;
  /** include in distribution?*/
  bool dist;
  /** install?*/
  bool install;
  /** install-location*/
  QString install_location;
  /** belongs to following targets in the list (for SOURCES and HEADERS)*/
  QStrList targetLst;
};

struct TWorkspace {
  int id; // 1, 2 or 3
  QStrList openfiles;
  QString header_file;
  QString cpp_file;
  QString browser_file;
  bool show_treeview;
  bool show_output_view;
  QString activeTarget;
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
  *                     "normalogl_kde" added by Robert Wheat, 01-22-2000, OpenGL(tm) support
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
class CProject
{

public: 
  /** construtor */
  CProject(const QString& file);
  /** destructor */
  ~CProject();


public: // Methods to store project options

  /** These are not kdevelop spacific, but they are needed for KPP to operate on a
      project by project basis **/

  /** store the RPM revision number **/
  void setKPPRPMVersion(const QString& version) { writeGroupEntry("KPP", "kpp_version", version);}

  /** store the License type **/
  void setKPPLicenceType( const QString& license) { writeGroupEntry("KPP", "kpp_license", license);}

  /** store the URL **/
  void setKPPURL(const QString& url) { writeGroupEntry("KPP", "kpp_url", url);}

  /** store the application group **/
  void setKPPAppGroup(const QString& appgrp) { writeGroupEntry("KPP", "kpp_appgrp", appgrp);}

  /** store the build root **/
  void setKPPBuildRoot(const QString& buildroot) { writeGroupEntry("KPP", "kpp_bldroot", buildroot);}

  /** store the one line summary **/
  void setKPPSummary(const QString& summary) { writeGroupEntry("KPP", "kpp_summary", summary);}

  /** store the icon information **/
  void setKPPIcon(const QString& icon) {writeGroupEntry("KPP", "kpp_icon", icon);}


  /** get the RPM revision number **/
  QString getKPPRPMVersion() { return readGroupEntry( "KPP", "kpp_version" ); }

  /** get the License type **/
  QString getKPPLicenceType() { return readGroupEntry("KPP", "kpp_license");}

  /** get the URL **/
  QString getKPPURL() { return readGroupEntry("KPP", "kpp_url");}

  /** get the application group **/
  QString getKPPAppGroup() { return readGroupEntry("KPP", "kpp_appgrp");}

  /** get the build root **/
  QString getKPPBuildRoot() { return readGroupEntry("KPP", "kpp_bldroot");}

  /** get the one line summary **/
  QString getKPPSummary() { return readGroupEntry("KPP", "kpp_summary");}

  /** get the icon information **/
  QString getKPPIcon() {return readGroupEntry("KPP", "kpp_icon");}

  /** Store the version of the kdevprj file format*/
  void setKDevPrjVersion(const QString& version) { writeGroupEntry( "General", "kdevprj_version", version ); }

  /** Store the project type: {normal_cpp,normal_kde,mini_kde,normal_empty,normal_qt} */
  /**                 {normalogl_kde added by Robert Wheat, 01-22-2000, OpenGL(tm) support} */
  void setProjectType(const QString& type)       { writeGroupEntry( "General", "project_type", type ); }

  /** Store the projectname. */
  void setProjectName(const QString& name)       { writeGroupEntry( "General", "project_name", name ); }

  /** the mainsubdir, not "po"*/
  void setSubDir(const QString& dirname)         { writeGroupEntry( "General", "sub_dir", dirname ); }

  /** Store the authors name. */
  void setAuthor(const QString& name)            { writeGroupEntry( "General", "author", name ); }

  /** Store the authors eMail-address. */
  void setEmail(const QString& email)            { writeGroupEntry( "General", "email", email ); }

  /** Store the project version. */
  void setVersion(const QString& version)        { writeGroupEntry( "General", "version",version ); }

  /** Store the name of version control system */
  void setVCSystem(const QString& vcsystem);

    /** Store the classview tree. */
  void setClassViewTree( const QString& tree )   { writeGroupEntry( "General", "classview_tree", tree ); }

  /** at the moment only an english sgmlfile in docs/en/ */
  void setSGMLFile(const QString& name)          { writeGroupEntry( "General", "sgml_file", name ); }

  /** if yes the makefiles in the project are modified from KDevelop*/
  void setModifyMakefiles(bool enable=true);
  

  /** Store options for make( f.e. "-k" for "make -k"*/
  void setMakeOptions(const QString& options)    { writeGroupEntry( "General", "make_options", options ); }

  /** Store the commandline execution arguments for the project binary */
  void setExecuteArgs(const QString& args)       { writeGroupEntry( "General", "execute_args", args ); }

  /** Store the commandline execution arguments for debugging  */
  void setDebugArgs(const QString& args)         { writeGroupEntry( "General", "debug_args", args ); }

  /** Store the additional arguments for configure */
  void setConfigureArgs(const QString& args)     { writeGroupEntry( "General", "configure_args", args ); }
  
  void setBinPROGRAM(const QString& name)        { writeGroupEntry( "Config for BinMakefileAm", "bin_program", name ); }

  void setPathToBinPROGRAM(const QString& name)  { writeGroupEntry( "Config for BinMakefileAm", "path_to_bin_program", name ); }

  void setLibtoolDir(const QString& dir)      { writeGroupEntry( "Config for BinMakefileAm", "libtool_dir", dir); }

  void setDirWhereMakeWillBeCalled(const QString& name)  { writeGroupEntry( "General", "dir_where_make_will_be_called", name ); }

  /** Store all open groups in the LFV*/
  void setLFVOpenGroups(QStrList groups);

  void setShortInfo(QStrList short_info);

  /** Store the linker flags. */
  void setLDFLAGS(const QString& flags);
  void setLDADD(const QString& libstring);

  /** Store the C++ compiler flags. */
  void setCXXFLAGS(const QString& flags);
  void setAdditCXXFLAGS(const QString& flags);

  void setFilters(const QString& group,QStrList& filters);

  void setActiveTarget(const QString& target);

public: // Methods to fetch project options

  /** Fetch the version of the kdevprj file format*/
  QString getKDevPrjVersion() { return readGroupEntry( "General", "kdevprj_version" ); }

  /** Fetch the type: {normal_cpp,normal_kde,mini_kde} */
  /**     (normalogl_kde added by Robert Wheat, 01-22-2000, OpenGL(tm) support */
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

  bool getModifyMakefiles();

  /** Fetch the options for make( i.e "-k" for "make -k". */
  QString getMakeOptions()    { return readGroupEntry( "General", "make_options" ); }

  /** Fetch the commandline execution arguments for the project binary. */
  QString getExecuteArgs()    { return readGroupEntry( "General", "execute_args" ); }

  /** Fetch the commandline execution arguments for debugging. */
  QString getDebugArgs()      { return readGroupEntry( "General", "debug_args" ); }

  /** Fetch the commandline additional arguments for configure */
  QString getConfigureArgs()  { return readGroupEntry( "General", "configure_args" ); }

  QString getBinPROGRAM()     { return readGroupEntry( "Config for BinMakefileAm", "bin_program" ); }

  QString pathToBinPROGRAM()  { return readGroupEntry( "Config for BinMakefileAm", "path_to_bin_program" ); }

  QString getDirWhereMakeWillBeCalled(QString defaultStr);

  QString getExecutableDir();

  QString getExecutable();

  QString getLibtoolDir();

  QString getLibtool();

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

  void getFilters(const QString& group,QStrList& filters);

  TFileInfo getFileInfo(const QString& filename);

  TMakefileAmInfo getMakefileAmInfo(const QString& rel_name);

  TDialogFileInfo getDialogFileInfo(const QString& rel_filename);

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

  /** return true if a Makefile.am was changed
      the user of this method has to take care that the flag
      is resetted after query to get a new situation.

      to do so use:
        clearMakefileAmChanged();
  */
  bool getMakefileAmChanged();
  /**the new projectmanagment*/
  void getAllFiles(QStrList& list);
  void getAllTopLevelDialogs(QStrList& list);

  /** Get all sources for this makefile */
  void getSources(const QString& rel_name_makefileam,QStrList& sources);

  /** Get all po files for this makefile */
  void getPOFiles(const QString& rel_name_makefileam,QStrList& po_files);

  bool isValid() const    { return valid; }

  bool isKDEProject();
  bool isKDE2Project();
  bool isQtProject();
  bool isQt2Project();
  bool isCustomProject();
  /** check if a subdir is in the project (registered). */
//  bool isDirInProject(const QString& rel_name);



  /** Method that returns the filetype for a given file by looking at
   * it's extension.
   * @param aFile The filename to check.
   */
  static ProjectFileType getType( const QString& aFile );

  /** Return the type matching a string.
   * @param aStr The string representation of a type.
   */
  ProjectFileType getTypeFromString( const QString& aStr );

  /** Return a string matching a type.
   * @param aType The type to get the string for.
   */
  QString getTypeString( ProjectFileType aType );

public: // Public methods

  /** if ace_group == empty insert prepend at the grouplist*/
  void addLFVGroup(const QString& name, const QString& ace_group);
  void removeLFVGroup(const QString& name);
  void writeFileInfo(TFileInfo info);
  void writeDialogFileInfo(TDialogFileInfo info);
  //  void writeMakefileAmInfo(TMakefileAmInfo info);
  /** return true if a new subdir was added to the project*/
  bool addFileToProject(QString rel_name,TFileInfo info);
  bool addDialogFileToProject(const QString& rel_name,TDialogFileInfo info);
  void removeFileFromProject(const QString& rel_name);
  void setKDevelopWriteArea(const QString& makefile);
  void addMakefileAmToProject(const QString& rel_name,TMakefileAmInfo info);

  void updateConfigureIn();
  void updateMakefilesAm();
  void changeLibraryType(const QString &makefile, const QString &type);
  void createLibraryMakefileAm(const QString &makefile, const QString &type);
  //  void createBinMakefileAm();
  void updateMakefileAm(const QString& makefile);

  /** read the projectfile */
  bool readProject();
  /** create a new project file ready for entries to be added */
  bool createEmptyProject();
  /** write the projectfile to the disk*/
  void writeProject();

  void  writeWorkspace(TWorkspace ws);
  TWorkspace getWorkspace(int id);
  void setCurrentWorkspaceNumber(int id);
  int getCurrentWorkspaceNumber();

/** method to fill up a string template with actual project info
  *@author Walter Tasin
  */
  QString& setInfosInString(QString& strtemplate, bool basics=true);
  void clearMakefileAmChanged();
  /** returns the Qt translation files (*.ts) in the project
 */
  void getTSFiles(const QString& makefileam, QStrList& ts_files);

  QString getRunFromDir();

  /**
  * checks to see if the named file is a shell script
  * mainly used to see if we need to run the libtool wrapper
  */
  bool isAScript(const QString &filename);

protected:

  /** Write an entry to the project file.
   * @param group Name of the group.
   * @param tag   The value-tag e.g version.
   * @param entry The string to store. */
  void writeGroupEntry( const QString& group, const QString& tag, const QString& entry );

  /** Read an entry from the project file and return it.
   * @param group Name of the group.
   * @param tag   The value-tag e.g version. */
  QString readGroupEntry( const QString& group, const QString& tag, const QString& defaultValue = QString::null);

  void setSourcesHeaders();
  void getAllStaticLibraries(QStrList& libs);
  QString getDir(const QString& rel_name);
  QString getName(const QString& rel_name);

private: // Protected attributes

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
  
  KSimpleConfig* config;

  /** true if the project was read*/
  bool valid;
};
#endif

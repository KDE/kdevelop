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

#include "ckdevelop.h"
#include "cproject.h"
#include "resource.h"

#include "vc/versioncontrol.h"
#include "ctoolclass.h"
#include "clibpropdlgimpl.h"
#include "cprjconfchange.h"

#include <kprocess.h>
#include <kconfigbase.h>
#include <kapp.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kcombobox.h>

#include <qdir.h>
#include <qregexp.h>
#include <qmessagebox.h>

#define PROJECT_VERSION_STR "KDevelop Project File Version 0.3 #DO NOT EDIT#"

CProject::CProject(const QString& file) :
  prjfile (file),
  dir(QString::null),
  vc(0),
  config(0),
  valid(false),
  topMakefile(QString::null),
  cvsMakefile(QString::null),
  m_ctagsDataBase()
{
  ptStringMap = new QString[(unsigned)PT_END_POS];
  ptStringMap[ CORBA_SOURCE ] = "CORBA_SOURCE";
  ptStringMap[ CPP_SOURCE ] = "SOURCE";
  ptStringMap[ CPP_HEADER ] = "HEADER";
  ptStringMap[ FTN_SOURCE ] = "SOURCE";
  ptStringMap[ SCRIPT ] = "SCRIPT";
  ptStringMap[ DATA ] = "DATA";
  ptStringMap[ QT_TS ] = "QT_TS";
  ptStringMap[ PO ] = "PO";
  ptStringMap[ KDEV_DIALOG ] = "KDEV_DIALOG";
  ptStringMap[ LEXICAL ] = "SOURCE";
}

CProject::~CProject(){
  delete vc;
  delete []ptStringMap;
  delete config;
}


bool CProject::prepareToReadProject()
{
  // if we re-read this file then get rid of these
  delete vc;
  delete config;

  vc      = 0;
  config  = 0;
  valid   = false;

  // Make sure we can write to the file, as well as read the file
  QFileInfo fileinfo(prjfile);
  if (!(fileinfo.isWritable() && fileinfo.isReadable()))
    return false;

  // organise the prjfile file and check that it _is_ a kdevelop prjfile.
  config = new KSimpleConfig(prjfile);
  config->setGroup( "General" );
  if (!config->hasKey("kdevprj_version"))    // still too simple a test!
    return false;

  dir = fileinfo.dirPath() + "/";
  valid = true;
	return true;
}

void CProject::readProject()
{
  setSourcesHeaders();
  vc = VersionControl::getVersionControl(getVCSystem());
}

bool CProject::createEmptyProject()
{
  // This shouldn't be happening but....
  ASSERT(vc == 0);
  ASSERT(config == 0);
  delete vc;
  delete config;

  vc      = 0;
  config  = 0;
  valid   = false;

  // Make sure we can write to the file, as well as read the file
  QFile file(prjfile);
  if (file.exists() || !file.open(IO_ReadWrite))
    return false;
  file.close();

  config = new KSimpleConfig(prjfile);

  QFileInfo fileinfo(prjfile);
  dir = fileinfo.dirPath() + "/";

  valid = true;
  return true;
}

void CProject::writeProject(){
  if (config) {
    config->sync();
  }
}

/*********************************************************************
 *                                                                   *
 *                 METHODS TO STORE CONFIG VALUES                    *
 *                                                                   *
 ********************************************************************/

void CProject::clearMakefileAmChanged()
{
    config->setGroup("General");
    config->writeEntry("AMChanged", false );
    // this is an immediate flag ... it should be written immediately
    //  to the project file
    config->sync();
}


void CProject::setVCSystem(const QString& vcsystem)
{
    writeGroupEntry( "General", "version_control", vcsystem );
    if (vc)
        delete vc;
    vc = VersionControl::getVersionControl(vcsystem);
}


void CProject::setLFVOpenGroups(QStringList groups){
  config->setGroup("General");
  config->writeEntry( "lfv_open_groups", groups );
}

void CProject::setShortInfo(QStrList short_info){
  config->setGroup("General");
  config->writeEntry( "short_info", short_info );
}

void CProject::setLDFLAGS(const QString& flags){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "ldflags", flags );
  config->setDollarExpansion(true);
}

void CProject::setLDADD(const QString& libstring){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "ldadd", libstring );
  config->setDollarExpansion(true);
}
/** No descriptions */
void CProject::setCFLAGS(const QString& flags){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "cflags",flags );
  config->setDollarExpansion(true);
}
/** No descriptions */
void CProject::setCPPFLAGS(const QString& flags){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "cppflags",flags );
  config->setDollarExpansion(true);
}

void CProject::setCXXFLAGS(const QString& flags){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "cxxflags",flags );
  config->setDollarExpansion(true);
}

void CProject::setModifyMakefiles(bool enable){
    config->setGroup("General");
    config->writeEntry("modifyMakefiles",enable);
    config->sync();
}
void CProject::setShowNonProjectFiles(bool enable){
    config->setGroup("General");
    config->writeEntry("showNonProjectFiles",enable);
    config->sync();
}
void CProject::setAdditCXXFLAGS(const QString& flags){
  config->setDollarExpansion(false);
  writeGroupEntry( "Config for BinMakefileAm", "addcxxflags", flags );
  config->setDollarExpansion(true);
}

void CProject::setFilters(const QString& group,QStringList& filters){
  config->setGroup("LFV Groups");
  config->writeEntry(group,filters);
}

void CProject::writeFileInfo(TFileInfo info){
  config->setGroup(info.rel_name);
  config->writeEntry("type", getTypeString( info.type ) );
  config->writeEntry("dist",info.dist);
  config->writeEntry("install",info.install);
  // save the $ because kconfig removes one
  info.install_location.replace(QRegExp("[$]"),"$$");
  config->writeEntry("install_location",info.install_location);
}

void CProject::writeDialogFileInfo(TDialogFileInfo info){
  config->setGroup(info.rel_name);
  config->writeEntry("type", "KDEV_DIALOG" );
  config->writeEntry("dist",info.dist);
  config->writeEntry("install",info.install);
  // save the $ because kconfig removes one
  info.install_location.replace(QRegExp("[$]"),"$$");
  config->writeEntry("install_location",info.install_location);

  config->writeEntry("baseclass",info.baseclass);
  config->writeEntry("widget_files",info.widget_files);
  config->writeEntry("is_toplevel_dialog",info.is_toplevel_dialog);
  config->writeEntry("header_file",info.header_file);
  config->writeEntry("cpp_file",info.header_file);
  config->writeEntry("data_file",info.data_file);
  config->writeEntry("classname",info.classname);

}

void CProject::writeMakefileAmInfo(const TMakefileAmInfo* info)
{
  config->setGroup(info->rel_name);
  config->writeEntry("type", info->type);
  config->writeEntry("sub_dirs", info->sub_dirs);
  config->writeEntry("sharedlib_LDFLAGS",info->sharedlibLDFLAGS);
  config->writeEntry("sharedlib_rootname", info->sharedlibRootName);
}

/*********************************************************************
 *                                                                   *
 *                 METHODS TO FETCH CONFIG VALUES                    *
 *                                                                   *
 ********************************************************************/

void CProject::getLFVOpenGroups(QStringList& groups){
  config->setGroup("General");
  groups = config->readListEntry("lfv_open_groups");
}

void CProject::getLFVGroups(QStringList& groups){
  groups.clear();
  config->setGroup("LFV Groups");
  groups = config->readListEntry("groups");
}

QStrList CProject::getShortInfo(){
  QStrList list;
  config->setGroup("General");
  config->readListEntry("short_info",list);
  return list;
}

QString CProject::getLDFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "ldflags" );
  config->setDollarExpansion(true);
  return str;
}

QString CProject::getLDADD(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "ldadd" );
  config->setDollarExpansion(true);
  return str;
}
bool CProject::getModifyMakefiles(){
    config->setGroup("General");
    return config->readBoolEntry("modifyMakefiles",true);
}
/** whether to show non-project files in the file-view */
bool CProject::getShowNonProjectFiles()
{
    config->setGroup("General");
    return config->readBoolEntry("showNonProjectFiles",true);
}
/** returns the preprocessor flags */
QString CProject::getCPPFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "cppflags" );
  config->setDollarExpansion(true);
  return str;
}
/** returns the CFLAGS string */
QString CProject::getCFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "cflags" );
  config->setDollarExpansion(true);
  return str;
}

QString CProject::getCXXFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "cxxflags" );
  config->setDollarExpansion(true);
  return str;
}

QString CProject::getAdditCXXFLAGS(){
  QString str;
  config->setDollarExpansion(false);
  str = readGroupEntry( "Config for BinMakefileAm", "addcxxflags" );
  config->setDollarExpansion(true);
  return str;
}

void CProject::getFilters(const QString& group,QStringList& filters){
  filters.clear();
  config->setGroup("LFV Groups");
  filters = config->readListEntry(group);
}

TFileInfo CProject::getFileInfo(const QString& rel_filename){
  TFileInfo info;
  config->setGroup(rel_filename);
  info.rel_name = rel_filename;
  info.type = getTypeFromString( config->readEntry("type") );
  info.dist = config->readBoolEntry("dist");
  info.install = config->readBoolEntry("install");
  config->setDollarExpansion(false);
  info.install_location = config->readEntry("install_location");
  info.install_location.replace(QRegExp("[$][$]"), "$");
  config->setDollarExpansion(true);

  return info;
}

TMakefileAmInfo CProject::getMakefileAmInfo(const QString& rel_name){
  TMakefileAmInfo info;

  config->setGroup(rel_name);
  info.rel_name = rel_name;
  info.type = config->readEntry("type");
  config->readListEntry("sub_dirs",info.sub_dirs);
  info.sharedlibLDFLAGS = config->readEntry("sharedlib_LDFLAGS");
  info.sharedlibRootName = config->readEntry("sharedlib_rootname");

  return info;
}

QString CProject::getDirWhereMakeWillBeCalled(QString defaultStr)
{
  QString dir = readGroupEntry( "General", "dir_where_make_will_be_called");
  if (dir.isEmpty())
    return defaultStr;
  return dir;
}



/*********************************************************************
 *                                                                   *
 *                         PUBLIC QUERIES                            *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------------- CProject::getType()
 * getType()
 *   Return the type of file based on the extension.
 *
 * Parameters:
 *   aFile           The file to check.
 *
 * Returns:
 *   ProjectFileType The filetype.
 *   DATA            If unknown.
 *-----------------------------------------------------------------*/
ProjectFileType CProject::getType( const QString& aFile )
{
  ProjectFileType retVal=DATA;

  QString ext(aFile);
  int pos = ext.findRev('.');

  if (pos == -1 )
    return retVal; // not found, so DATA

  ext = ext.right(ext.length()-pos);

  //ext = rindex( aFile, '.' );
  if( !ext.isEmpty() ) {
    // Convert to lowercase.
    ext = ext.lower();

    // Check for a known extension.
    if( ext == ".cpp" || ext == ".c" || ext == ".cc" ||
        ext == ".ec" || ext == ".ecpp" || ext == ".C" ||
        ext == ".cxx" || ext == ".ui" || ext == ".inl" || ext == ".m" || ext == ".mm" || ext == ".M" )
      retVal = CPP_SOURCE;
    // .ui = Qt2 designer files to be added to the SOURCES line for compiling Ralf N. 02.09.00
    else if( ext == ".h" || ext == ".hxx" || ext == ".hpp" ||
             ext == ".H" || ext == ".hh" || ext == ".tlh" )
      retVal = CPP_HEADER;
    // Fortran support (rokrau 05/22/01)
    else if ( (ext==".F") || (ext==".f") ||
              (ext==".F77") || (ext==".f77") ||
              (ext==".FTN") || (ext==".ftn") )
      retVal = FTN_SOURCE;
    else if( ext == ".l++" || ext == ".lxx" || ext == ".ll" || ext == ".l")
      retVal = LEXICAL;
    else if( ext == ".idl" )
      retVal = CORBA_SOURCE;
    else if( ext == ".kdevdlg" )
      retVal = KDEV_DIALOG;
    else if( ext == ".po" )
      retVal = PO;
    else if( ext == ".ts" )
      retVal = QT_TS;
  }
  return retVal;
}

/*------------------------------------- CProject::getTypeFromString()
 * getTypeFromString()
 *   Return the type matching a string
 *
 * Parameters:
 *   aStr            String representation of the type.
 *
 * Returns:
 *   ProjectFileType The filetype.
 *   DATA            If unknown.
 *-----------------------------------------------------------------*/
ProjectFileType CProject::getTypeFromString( const QString& aStr )
{
  int i;

  for(i=0; i<PT_END_POS && ptStringMap[ i ] != aStr; i++ )
    ;

  return ( i == PT_END_POS ? DATA : (ProjectFileType)i );
}

/*------------------------------------------ CProject::getTypeString()
 * getTypeString()
 *   Return the string matching a type.
 *
 * Parameters:
 *   aType            The type to get the string for.
 *
 * Returns:
 *   QString          The type as a string.
 *-----------------------------------------------------------------*/
QString CProject::getTypeString( ProjectFileType aType )
{
  return ptStringMap[ aType ];
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

//bool CProject::isDirInProject(QString rel_name){
//
////  KDEBUG(KDEBUG_INFO,CPROJECT,"isDirInProject() Don't use this function,it's not implemented!");
//  return true;
//  int pos = rel_name.findRev('/');
//  QString dir_name;
//
//
//  if(pos == -1){ // not found
//    dir_name = rel_name.copy();
//    rel_name = "Makefile.am";
//  }
//  else{
//    dir_name = rel_name.right(rel_name.length()-pos-1);
//    rel_name.truncate(pos+1);
//    rel_name.append("Makefile.am");
//  }
//
//  TMakefileAmInfo info = getMakefileAmInfo(rel_name);
//  QString str;
//
//  for(str=info.sub_dirs.first();str!=0;str=info.sub_dirs.next()){
//    kdDebug() << endl << str;
//  }
//}

bool CProject::addDialogFileToProject(const QString& /*rel_name*/,TDialogFileInfo info){
  config->setGroup(info.rel_name);
  config->writeEntry("baseclass",info.baseclass);
  config->writeEntry("widget_files",info.widget_files);
  config->writeEntry("is_toplevel_dialog",info.is_toplevel_dialog);
  config->writeEntry("header_file",info.header_file);
  config->writeEntry("cpp_file",info.header_file); // should this not be source_file?
  config->writeEntry("data_file",info.data_file);
  config->writeEntry("classname",info.classname);

  TFileInfo file_info;
  file_info.rel_name = info.rel_name;
  file_info.type = KDEV_DIALOG;
  file_info.dist = info.dist;
  file_info.install = info.install;

  return addFileToProject(file_info.rel_name,file_info);
}

bool CProject::addFileToProject(QString rel_name,TFileInfo info)
{

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

  //add the file into the filesentry in the Makefile.am group
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

 // kdDebug() << endl << "*check:*" << makefile_name;

  while((slash_pos = makefile_name.findRev('/')) != -1)
  { // if found
    slash2_pos = makefile_name.findRev('/',slash_pos-1);
    if (slash2_pos != -1 && makefile_name != "/")
    {
      makefile_name.remove(slash2_pos,slash_pos-slash2_pos);
      check_makefile_list.append(makefile_name);
      kdDebug() << "append to check_makefile_list: " << makefile_name << endl;
    }
    else
      makefile_name = "";

    if (makefile_name == "/Makefile.am")
      break;
  }

  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);

  TMakefileAmInfo makefileaminfo;

  for(makefile_name=check_makefile_list.first();
      makefile_name!=0;
      makefile_name=check_makefile_list.next())
  {
    // check if current makefile exists and all makefile above,if not add it
    if(makefile_list.find(makefile_name) == -1)
    {
      makefileaminfo.rel_name = makefile_name;
      if (makefile_name_org == makefile_name)
      { // the first makefileam
        if(info.type == CPP_SOURCE )
        {
          // set the defaults
          makefileaminfo.type = "static_library";
          makefileaminfo.sharedlibLDFLAGS = "-version-info 0:0:0";
          QDir dir(getDir(makefile_name));
          makefileaminfo.sharedlibRootName = dir.dirName();

          // allow user to adjust defaults if they want
          TMakefileAmInfo tmpInfo = makefileaminfo;
          CLibPropDlgImpl dlg(&tmpInfo);
          if( !isCustomProject()) {
            if (dlg.exec()) {
              makefileaminfo = tmpInfo;
            }
          }

          // update project file THEN create makefile
          writeMakefileAmInfo(&makefileaminfo);
          createLibraryMakefileAm(&makefileaminfo);
        }
        else
          makefileaminfo.type = "normal";

        addMakefileAmToProject(makefile_name,makefileaminfo);
      }
      else
      {
        makefileaminfo.type = "normal";
        addMakefileAmToProject(makefile_name,makefileaminfo);
      }
    }
  }
  //++++++++++++++++add Makefile to the project if needed (end)


  makefile_name = check_makefile_list.first(); // get the complete makefilename


  // change the makefile type if needed
  if(info.type == CPP_SOURCE)
  { // a static library is needed?
    config->setGroup(makefile_name);

    if(config->readEntry("type", "") != "prog_main" &&
        config->readEntry("type", "") != "shared_library")
    {
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
//      KDEBUG1(KDEBUG_INFO,DIALOG,"SUBDIR %s",subdir.data());
      makefile_name.remove(slash2_pos,slash_pos-slash2_pos);
      config->setGroup(makefile_name);
      sub_dirs.clear();
      config->readListEntry("sub_dirs",sub_dirs);

      if (sub_dirs.find(subdir) == -1)
      {
        new_subdir = true;
        sub_dirs.append(subdir);
        config->writeEntry("sub_dirs",sub_dirs);
      }
    }
    else
    {
      // the subdirs of the topdir are special
      subdir = makefile_name.left(slash_pos);
      config->setGroup("Makefile.am");
      sub_dirs.clear();
      config->readListEntry("sub_dirs",sub_dirs);

      if(sub_dirs.find(subdir) == -1)
      {
        new_subdir = true;
        sub_dirs.append(subdir);
        config->writeEntry("sub_dirs",sub_dirs);
      }
      makefile_name = "";
    }
  }

  if(new_subdir)
    updateConfigureIn();

  //  createMakefilesAm(); // do some magic generation

  // writethe fileinfo
  writeFileInfo( info );

  config->sync();
  setSourcesHeaders();
  // For Qt 2 projects:
  // update the projectdir/Makefile.am to add sources to the SOURCES line
  if(isQt2Project())
    updateMakefilesAm();
  ///////////
  return new_subdir;
}

void CProject::removeFileFromProject(const QString& rel_name){
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
  config->setGroup("General");

  bool update = getModifyMakefiles();

  if ( getProjectType() == "normal_empty" || update == false)
    return;

  config->writeEntry("AMChanged", true );
  // this is an immediate flag ... it should be written immediately
  //  to the project file
  config->sync();

  QString makefile;
  QStrList makefile_list;

  config->readListEntry("makefiles",makefile_list);
  for(makefile = makefile_list.first();makefile !=0;makefile =makefile_list.next()){ // every Makefile
    config->setGroup(makefile);
    updateMakefileAm(makefile);
  }

}

void CProject::updateMakefileAm(const QString& makefile)
{
  setKDevelopWriteArea(makefile);
  QString abs_filename = getProjectDir() + makefile;
  QFile file(abs_filename);

  /* update of MakefileAm, only if the filename contains Makefile.am at the end of the string */
  if (makefile.right(11)!="Makefile.am")
    return;

  QStrList list;
  QStrList files;
  QStrList subdirs;
  QString str;
  QString str2;
  QString dist_str;
  QTextStream stream(&file);
  bool found=false, customfile=false, insideKDevControlled=false, 
       foundINCLUDES=false;
  
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

  if(file.open(IO_ReadOnly))
  { // read the makefileam
    while(!stream.eof())
      list.append(stream.readLine());
  }
  file.close();

  /* Now check the content of Makefile.am
     - is it a custom file?
     - are INCLUDES present outside the kdevelop specific part?
  */  
  for(str = list.first(); str != 0; str = list.next())
  {
    if (!customfile && str.find(QRegExp("^\\s*#+\\s*kdevelop-pragma:\\s*custom",false))>=0)
      customfile=true;
    if (str == "####### kdevelop will overwrite this part!!! (begin)##########")
      insideKDevControlled=true;
    if (str == "####### kdevelop will overwrite this part!!! (end)############")    
      insideKDevControlled=false;
      
    /* To maintain compatibility with older project
       we have to search INCLUDES in the non-kdevelop controlled part
       of Makefile.am
     
       If these flags are found, so don't add these to the kdevelop-controlled part
    */
    if (!insideKDevControlled && str.find(QRegExp("^\\s*INCLUDES\\s*="))>=0)
       foundINCLUDES=true;    
  }

  if(file.open(IO_WriteOnly))
  {
    for(str = list.first();str != 0;str = list.next())
    { // every line
      if (!customfile && str == "####### kdevelop will overwrite this part!!! (begin)##########")
      {
        stream << str << "\n";

        //***************************generate needed things for the main makefile*********
        config->setGroup(makefile);
        if (config->readEntry("type") == "prog_main")
        { // the main makefile
          stream << "bin_PROGRAMS = " << getBinPROGRAM() << "\n";
          getSources(makefile,source_files);
          for(str= source_files.first();str !=0;str = source_files.next())
            sources =  str + " " + sources ;

          //    stream << "CXXFLAGS = " << getCXXFLAGS()+" "+getAdditCXXFLAGS() << "\n";
          //stream << "LDFLAGS = " << getLDFLAGS()  << "\n";
          if (!foundINCLUDES)
            stream << "\nINCLUDES = $(all_includes)\n";
          else
            stream << "\n## INCLUDES were found outside kdevelop specific part\n";
          
          stream << "\n" << getBinPROGRAM()  <<  "_SOURCES = " << sources << "\n";
          /********************* QT 2 INTERNATIONALIZATION **************/
          if(isQt2Project())
          {
            QString all_srces;
            QString uifiles;
            QStrList src_list=getSources();
            for(str= src_list.first();str !=0;str = src_list.next())
            {
              str.replace( QRegExp(getProjectDir()+getSubDir()), "" );
              if(str.right(3)==".ui")
                uifiles = str + " " + uifiles;
              else
                all_srces =  str + " " + all_srces ;
            }

            // add a separate SOURCES and INTERFACES line for lupdate to get translations
            stream << "SOURCES = " << all_srces  << "\n";
            stream << "INTERFACES = " << uifiles << "\n";
            // add a TRANSLATIONS line for lupdate/lrelease containing all .ts files
            QStrList ts_files;
            getTSFiles(makefile,ts_files);

            QString tsstr, tsfiles;
            for(tsstr= ts_files.first();tsstr !=0;tsstr = ts_files.next())
              tsfiles =  tsstr + " " + tsfiles ;

            stream << "TRANSLATIONS = " << tsfiles << "\n";
            // am_edit used only for qt apps requires this switch in Makefile.am큦 to use tr instead of i18n and other specific stuff
            stream << "KDE_OPTIONS = qtonly\n";
          }

          /********************* QT 2 INTERNATIONALIZATION END **************/

          if(static_libs.isEmpty())
            stream << getBinPROGRAM()  <<  "_LDADD   = " << getLDADD();
          else
          { // we must link some libs
            stream << getBinPROGRAM()  <<  "_LDADD   = ";
            for(libname = static_libs.first();libname != 0;libname = static_libs.next())
              stream << libname.replace(QRegExp("^"+getSubDir()),"./") << " "; // remove the subdirname

            stream << getLDADD();
          }

          if(getProjectType() != "normal_cpp" && getProjectType() != "normal_c" && getProjectType() != "normal_gnome" && getProjectType() != "normal_objc")
            stream << " $(LIBSOCKET)" << "\n";
          else
            stream << "\n";

        }
        //***************************generate needed things for static_library*********
        config->setGroup(makefile);
        if(config->readEntry("type") == "static_library")
        {
          getSources(makefile,source_files);
          for(str= source_files.first();str !=0;str = source_files.next())
            sources =  str + " " + sources;

          QDir dir(getDir(makefile));
          QString type=getProjectType();
          
          stream << "noinst_LIBRARIES = lib" << dir.dirName() << ".a\n\n";
          
          if (!foundINCLUDES)
            stream << "INCLUDES = $(all_includes)\n";
          else
            stream << "## INCLUDES were found outside kdevelop specific part\n";
          
          if (QFileInfo(getProjectDir() + "am_edit").exists() ||QFileInfo(getProjectDir() + "admin/am_edit").exists())
            stream << "\nlib" << canonicalizeDirName(dir.dirName()) << "_a_METASOURCES = AUTO\n";
           else
             if (QFileInfo(getProjectDir() + "automoc").exists())
               stream << "\nlib" << canonicalizeDirName(dir.dirName()) << "_a_METASOURCES = USE_AUTOMOC\n";


          stream << "\nlib" << canonicalizeDirName(dir.dirName()) << "_a_SOURCES = " << sources << "\n";
          if(isQt2Project())
            // am_edit used only for qt apps requires this switch in Makefile.am큦 to use tr instead of i18n and other specific stuff
            stream << "KDE_OPTIONS = qtonly\n";
        }

        //***************************generate needed things for shared_library*********
        config->setGroup(makefile);
        if(config->readEntry("type") == "shared_library" )
        {

          getSources(makefile,source_files);
          for(str= source_files.first();str !=0;str = source_files.next())
            sources =  str + " " + sources ;

          config->setGroup(makefile);
          QString libRootName = config->readEntry("sharedlib_rootname");
          if(libRootName.isEmpty())
          {
            QDir dir(getDir(makefile));
            libRootName = dir.dirName();
          }

          QString type=getProjectType();
          config->setGroup(makefile);
          QString sharedlibLDFLAGS = config->readEntry("sharedlib_LDFLAGS");

          bool sharedLibSubDir=!sharedlibLDFLAGS.isEmpty();

          if (!sharedLibSubDir)
          {
            if( (type == "kio_slave") )
              stream << "kde_module_LTLIBRARIES = kio_" << libRootName << ".la\n\n";
            else if( (type == "kc_module") )
              stream << "kde_module_LTLIBRARIES = libkcm_" << libRootName << ".la\n\n";
            else
              stream << "lib_LTLIBRARIES = lib" << libRootName << ".la\n\n";
          }
          else
            stream << "lib_LTLIBRARIES = lib" << libRootName << ".la\n\n";

          if (!foundINCLUDES)
            stream << "\nINCLUDES = $(all_includes)\n";
          else
            stream << "\n## INCLUDES were found outside kdevelop specific part\n";

          stream << "\n\nLDFLAGS = " << getLDFLAGS() << "\n\n";

          if (sharedLibSubDir)
            stream << "\nlib" << canonicalizeDirName(libRootName) << "_la_LDFLAGS = " << sharedlibLDFLAGS << "\n\n";

          if (QFileInfo(getProjectDir() + "am_edit").exists() ||QFileInfo(getProjectDir() + "admin/am_edit").exists())
          {
            if( (type == "kio_slave") && !sharedLibSubDir)
              stream << "kio_" << canonicalizeDirName(libRootName) << "_la_METASOURCES=AUTO\n\n";
            else if( (type == "kc_module") && !sharedLibSubDir)
              stream << "libkcm_" << canonicalizeDirName(libRootName) << "_la_METASOURCES=AUTO\n\n";
            else
              stream << "lib" << canonicalizeDirName(libRootName) << "_la_METASOURCES = AUTO\n\n";
          }
          else
          {
            if (QFileInfo(getProjectDir() + "automoc").exists())
            {
              if( (type == "kio_slave") && !sharedLibSubDir)
                stream << "kio_" << canonicalizeDirName(libRootName) <<  "_la_METASOURCES = USE_AUTOMOC\n\n";
              else if( (type == "kc_module") && !sharedLibSubDir)
                stream << "libkcm_" << canonicalizeDirName(libRootName) <<  "_la_METASOURCES = USE_AUTOMOC\n\n";
              else
                stream << "lib" << canonicalizeDirName(libRootName) << "_la_METASOURCES = USE_AUTOMOC\n\n";
            }
          }

          if (!sharedLibSubDir)
          {
            if( type == "kpart_plugin")
            {
              stream << "\nLDADD = " << getLDADD() << "\n\n";
              stream << "lib" << canonicalizeDirName(libRootName) << "_la_SOURCES = " << sources << "\n";
              stream << "lib" << canonicalizeDirName(libRootName) << "_la_LIBADD = " << getLDADD() << "\n\n";
              stream << "lib" << canonicalizeDirName(libRootName) << "_la_LDFLAGS = $(all_libraries) -avoid-version -module -no-undefined\n\n";
            }
            else if( (type == "kio_slave"))
            {
              stream << "kio_" << canonicalizeDirName(libRootName) << "_la_SOURCES = " << sources << "\n";
              stream << "kio_" << canonicalizeDirName(libRootName) << "_la_LIBADD = " << getLDADD() << "\n\n";
              stream << "kio_" << canonicalizeDirName(libRootName) << "_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)  " << getLDFLAGS() << "\n\n";
            }
            else if( (type == "kc_module"))
            {
              stream << "libkcm_" << canonicalizeDirName(libRootName) << "_la_SOURCES = " << sources << "\n";
              stream << "libkcm_" << canonicalizeDirName(libRootName) << "_la_LIBADD = " << getLDADD() << "\n\n";
              stream << "libkcm_" << canonicalizeDirName(libRootName) << "_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)  " << getLDFLAGS() << "\n\n";
            }
            else
              stream << "lib" << canonicalizeDirName(libRootName) << "_la_SOURCES = " << sources << "\n";
          }
          else
            stream << "lib" << canonicalizeDirName(libRootName) << "_la_SOURCES = " << sources << "\n";

          if(isQt2Project())
            // am_edit used only for qt apps requires this switch in Makefile.am큦 to use tr instead of i18n and other specific stuff
            stream << "KDE_OPTIONS = qtonly\n";
        }

        //***************************generate needed things for a po makefile*********
        config->setGroup(makefile);
        if (config->readEntry("type") == "po" && (!isKDE2Project()))
        { // a po makefile - KDE2 Projects have PO_FILES=AUTO in it.
          getPOFiles(makefile,po_files);
          for(str= po_files.first();str !=0;str = po_files.next())
            pos =  str + " " + pos ;

          stream <<  "POFILES = " << pos << "\n";
        }

        // ********generate the dist-hook, to fix a automoc problem, hope "make dist" works now******
        //  QString type=getProjectType();
        if(!(isKDE2Project() || isQt2Project()) )
        {  // KDE2 and Qt2 use KDE큦 admin dir which contains am_edit
          if (makefile==QString("Makefile.am"))
          {
            if (QFileInfo(getProjectDir() + "am_edit").exists())
              stream << "dist-hook:\n\t-perl am_edit\n";
            else
              if (QFileInfo(getProjectDir() + "automoc").exists())
                 stream << "dist-hook:\n\t-perl automoc\n";
          }
        }

        //************SUBDIRS***************
        if(!subdirs.isEmpty())
        { // the SUBDIRS key
          stream << "\nSUBDIRS = ";
          for(str2 = subdirs.first();str2 !=0;str2 = subdirs.next()){
            stream << str2 << " ";
          }
        }
        stream << "\n";

        //************EXTRA_DIST************
        dist_str = "\nEXTRA_DIST = ";
        bool dist_write=false;
        for(str2 = files.first();str2 !=0;str2 = files.next())
        {
          config->setGroup(str2);
          if (config->readBoolEntry("dist"))
          {
            dist_str = dist_str + getName(str2) + " ";
            dist_write = true;
          }
        }
        if (dist_write)
          stream << dist_str << "\n";

        config->setDollarExpansion(false);
        //**************install-data-local****************
        bool install_data=false;
        QString install_location;
        QString install_data_str = "\ninstall-data-local:\n";
        for(str2 = files.first();str2 !=0;str2 = files.next())
        {
          config->setGroup(str2);
          install_location=config->readEntry("install_location");
          install_location.replace(QRegExp("[$][$]"), "$");
          if (config->readBoolEntry("install") && config->readEntry("type") != "SCRIPT")
          {
            install_data_str = install_data_str + "\t$(mkinstalldirs) "
              + getDir(install_location) + "\n";
            install_data_str = install_data_str + "\t$(INSTALL_DATA) $(srcdir)/" +
              getName(str2) + " " + install_location + "\n";

            install_data = true;
          }
        }
        if(install_data)
          stream << install_data_str;

        //**************install-exec-local****************
        bool install_exec=false;
        QString install_exec_str = "\ninstall-exec-local:\n";
        for(str2 = files.first();str2 !=0;str2 = files.next())
        {
          config->setGroup(str2);
          install_location=config->readEntry("install_location");
          install_location.replace(QRegExp("[$][$]"), "$");
          if (config->readBoolEntry("install") && config->readEntry("type") == "SCRIPT")
          {
            install_exec_str = install_exec_str + "\t$(mkinstalldirs) "
              + getDir(install_location) + "\n";
            install_exec_str = install_exec_str + "\t$(INSTALL_SCRIPT) " +
              getName(str2) + " " + install_location + "\n";
            install_exec = true;
          }
        }

        if(install_exec)
          stream << install_exec_str;

        //**************uninstall-local*******************
        bool uninstall_local=false;
        QString uninstall_local_str = "\nuninstall-local:\n";
        for(str2 = files.first();str2 !=0;str2 = files.next())
        {
          config->setGroup(str2);
          install_location=config->readEntry("install_location");
          install_location.replace(QRegExp("[$][$]"), "$");
          if (config->readBoolEntry("install"))
          {
            uninstall_local_str = uninstall_local_str + "\t-rm -f " +
              install_location +"\n";
            uninstall_local=true;
          }
        }
        if(uninstall_local)
          stream << uninstall_local_str;

        stream << "\n";

        config->setDollarExpansion(true);
        found = true;
      }

      if(found == false)
        stream << str << "\n";

      if(!customfile && str =="####### kdevelop will overwrite this part!!! (end)############")
      {
        stream << str << "\n";
        found = false;
      }
    } // end for
  }// end writeonly
  else
  {
    kdDebug() << "ERROR: Cannot write to " << abs_filename << endl;
  }

  file.close();
}

void CProject::refreshMakefileAm(TMakefileAmInfo* info)
{
  writeMakefileAmInfo(info);
  updateMakefileAm(info->rel_name);
}

QString CProject::getDir(const QString& rel_name){
  int pos = rel_name.findRev('/');
  return rel_name.left(pos+1);
}

QString CProject::getName(const QString& rel_name){
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
    if(info.type == KDEV_DIALOG){
      list.append(info.rel_name);
    }
  }
}

TDialogFileInfo CProject::getDialogFileInfo(const QString& rel_filename)
{
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

bool CProject::getMakefileAmChanged()
{
  config->setGroup("General");
  return config->readBoolEntry("AMChanged", true);
}

void CProject::getSources(const QString& rel_name_makefileam,QStrList& sources)
{
  sources.clear();
  QStrList files;
  QString file;
  TFileInfo info;
  config->setGroup(rel_name_makefileam);
  config->readListEntry("files",files);

  for(file = files.first();file != 0;file = files.next()){
    info = getFileInfo(file);
    if((info.type==CPP_SOURCE)||(info.type==FTN_SOURCE)){
      sources.append(getName(file));
    }
  }
}

void CProject::getPOFiles(const QString& rel_name_makefileam, QStrList& po_files){
  po_files.clear();
  QStrList files;
  char *file;
  TFileInfo info;
  config->setGroup(rel_name_makefileam);
  config->readListEntry("files",files);

  for(file = files.first();file != 0;file = files.next())
  {
    if( getType( file ) == PO )
    {
      info = getFileInfo(file);
      if(info.type == PO){
        po_files.append(getName(file));
      }
    }
  }
}

/** returns the Qt translation files (*.ts) in the project
 */
void CProject::getTSFiles(const QString& makefileam, QStrList& ts_files){

  ts_files.clear();
  QStrList files;
  char *file;
  TFileInfo info;
  config->setGroup(makefileam);
  config->readListEntry("files",files);

  for(file = files.first();file != 0;file = files.next())
  {
    if( getType( file ) == QT_TS )
    {
      info = getFileInfo(file);
      if(info.type == QT_TS){
        ts_files.append(getName(file));
      }
    }
  }
}

void CProject::setSourcesHeaders(){
  // clear the lists
  header_files.clear();
  cpp_files.clear();
  TFileInfo info;
  QStrList files;
  char *file;

  getAllFiles(files);

  for(file = files.first();file != 0;file = files.next()){
    info = getFileInfo(file);
    if((info.type==CPP_SOURCE)||(info.type==FTN_SOURCE)){
      cpp_files.append(getProjectDir()+file);
    }
    if(info.type == CPP_HEADER){
      header_files.append(getProjectDir()+file);
    }
  }
    // switch( getType( file ) )
//     {
//       case CPP_SOURCE:
    //   cpp_files.append(getProjectDir()+file);
//         break;
//       case CPP_HEADER:
//         header_files.append(getProjectDir()+file);
//         break;
//       default:
//         break;
//     }
}

void CProject::setKDevelopWriteArea(const QString& makefile){
  QString abs_filename = getProjectDir() + makefile;
  QFile file(abs_filename);
  QStrList list;

  /* addition KDevelop write-area, only if the filename contains Makefile.am
     at the end of the string */
  if (makefile.right(11)!="Makefile.am")
    return;

  bool found = false, customfile=false;
  QTextStream stream(&file);
  QString str;

  if(file.open(IO_ReadOnly)){ // read the makefileam
    while(!stream.eof()){
      list.append(stream.readLine());
    }
  }
  file.close();

  for(str = list.first();str != 0;str = list.next()){
    if (str.find(QRegExp("^\\s*#+\\s*kdevelop-pragma:\\s*custom",false))>=0)
      customfile=true;
    if (str == "####### kdevelop will overwrite this part!!! (begin)##########"){
      found = true;
    }
  }
  if(!found && !customfile){
    // create the writeable area
    if (file.open(IO_WriteOnly))
    {
      stream << "####### kdevelop will overwrite this part!!! (begin)##########\n";
      stream << "####### kdevelop will overwrite this part!!! (end)############\n";
      for(str = list.first();str != 0;str = list.next())
      {
        stream << str << "\n";
      }
    }
    else
    {
      kdDebug() << "ERROR: Cannot write to " << abs_filename << endl;
    }
    file.close();
  }
}

void CProject::addLFVGroup(const QString& name, const QString& ace_group)
{
  QStringList groups;
  config->setGroup("LFV Groups");
  groups = config->readListEntry("groups");
  if(ace_group.isEmpty()){
    groups.insert(groups.begin(),name);
    config->writeEntry("groups",groups);
    return;
  }
  QStringList::Iterator pos = groups.find(ace_group);
  pos++;
  if (pos == groups.end()) {
    groups.append(name);
  }
  else {
    groups.insert(pos,name);
  }
  config->writeEntry("groups",groups);
}

void CProject::removeLFVGroup(const QString& name)
{
  QStringList groups;
  config->setGroup("LFV Groups");
  groups = config->readListEntry("groups");
  groups.remove(name);
  config->deleteEntry(name,false);
  config->writeEntry("groups",groups);
}

void CProject::addMakefileAmToProject(const QString& rel_name,TMakefileAmInfo info){

//  cerr << "Create " << rel_name << endl;
//  for ( QString subdir=info.sub_dirs.first(); subdir != 0; subdir=info.sub_dirs.next() )
//    cerr << subdir << endl;

  config->setGroup(rel_name);
  config->writeEntry("type",info.type);
  config->writeEntry("sub_dirs",info.sub_dirs);

  QStrList makefile_list;
  config->setGroup("General");
  config->readListEntry("makefiles",makefile_list);
  makefile_list.append(rel_name);
  config->writeEntry("makefiles",makefile_list);
}

void CProject::updateConfigureIn(){
  if( isCustomProject()) return; // do nothing

  QStringList makefile_list;
  QString fullLine, makefile;

  CPrjConfChange configureIn(getProjectDir());

  config->setGroup("General");
  makefile_list=config->readListEntry("makefiles");
  for(QStringList::Iterator it = makefile_list.begin();
      it != makefile_list.end(); ++it)
  {
      makefile=*it;
      fullLine+=makefile.remove(makefile.length()-3,3) + " ";
  }

  configureIn.setLine("^[\\s]*AC_OUTPUT[\\s]*\\(", "AC_OUTPUT("+fullLine+")");

  configureIn.setLine("^[\\s]*KDE_DO_IT_ALL[\\s]*\\(",
     "KDE_DO_IT_ALL("+getProjectName().lower()+","+getVersion()+")");
  configureIn.setLine("^[\\s]*AM_INIT_AUTOMAKE[\\s]*\\(",
     "AM_INIT_AUTOMAKE("+getProjectName().lower()+","+getVersion()+")");

  configureIn.writeConfFile();

}
void  CProject::writeWorkspace(TWorkspace ws){
  if(getVersionControl() == 0){ // I love hacks :-), this is for better multiusersupport -Sandy
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
    config->sync();
  }
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

  return ws;
}

void CProject::getAllStaticLibraries(QStrList& libs){
  QStrList makefiles;
  QString makefile;

  libs.clear();
  config->setGroup("General");

  config->readListEntry("makefiles",makefiles);

  for(makefile=makefiles.first();makefile != 0;makefile=makefiles.next())
  {
    config->setGroup(makefile);
    if(config->readEntry("type") == "static_library")
    {
      QDir dir(getDir(makefile));
      libs.append(getDir(makefile) + "lib" + dir.dirName() + ".a");
    }

    if(config->readEntry("type") == "shared_library")
    {
      QString libRootName = config->readEntry("sharedlib_rootname");
      if(libRootName.isEmpty())
      {
        QDir dir(getDir(makefile));
        libRootName = dir.dirName();
      }

      libs.append(getDir(makefile) + "lib" + libRootName + ".la");
    }
  }
}

//void CProject::changeLibraryType(const QString &makefile, const QString &type)
//{
//  writeGroupEntry(makefile, "type", type+"_library");
//}

void CProject::createLibraryMakefileAm(const TMakefileAmInfo* info)
//const QString &makefile, const QString &type)
{
  QString abs_filename = getProjectDir() + info->rel_name;

//  QString lib_template = locate("data", "kdevelop/templates/MAM_"+type+"_template");
  QFile fileDest(abs_filename);
//  QFile file(lib_template);
//  QStrList list;
//  QString str;

  QTextStream streamDest(&fileDest);
//  QTextStream stream(&file);
//  if(file.open(IO_ReadOnly))
//  { // read the template
//    while(!stream.eof())
//    {
//      list.append(stream.readLine());
//    }
//  }
//  file.close();

  // save the old file
  if (QFileInfo(abs_filename).exists())
    QDir().rename(abs_filename, abs_filename+".old");

//  QDir dir(getDir(makefile));
  if(fileDest.open(IO_WriteOnly))
  { // write the Makefile.am
//    for(str = list.first();str != 0;str = list.next())
//    {
//      str.replace(QRegExp("\\|LIBNAME\\|"), dir.dirName());
//      str.replace(QRegExp("\\|LIBADD\\|"), getLDADD());
//      streamDest << str << "\n";
//    }
    fileDest.close();
  }
}

/*-------------------------------------- CProject::setInfosInString()
 * setInfosInString()
 *    changes the string to the real values of the project         *
 * Parameters:
 *    strtemplate   template string to change
 *    basics        if true change only basic informations
 *                  (optimizing parameter)
 *
 * Returns:
 *   reference to the stringlist
 *-----------------------------------------------------------------*/
QString& CProject::setInfosInString(QString& strtemplate, bool basics)
{
   QString date=QDate::currentDate().toString();
   QString year;
   year.setNum(QDate::currentDate().year());
   QString projectName=getProjectName();

   strtemplate.replace(QRegExp("\\|PRJNAME\\|"), projectName);
   strtemplate.replace(QRegExp("\\|NAME\\|"), projectName);
   strtemplate.replace(QRegExp("\\|NAMELITTLE\\|"), projectName.lower());
   strtemplate.replace(QRegExp("\\|NAMEBIG\\|"), projectName.upper());

   strtemplate.replace(QRegExp("\\|PRJDIR\\|"), getProjectDir());
   strtemplate.replace(QRegExp("\\|PRJFILE\\|"), getProjectFile());
   strtemplate.replace(QRegExp("\\|SUBDIR\\|"), getSubDir());
   strtemplate.replace(QRegExp("\\|AUTHOR\\|"), getAuthor());
   strtemplate.replace(QRegExp("\\|EMAIL\\|"), getEmail());
   strtemplate.replace(QRegExp("\\|VERSION\\|"), getVersion());
   strtemplate.replace(QRegExp("\\|BINPROGRAM\\|"), getBinPROGRAM());
   strtemplate.replace(QRegExp("\\|DATE\\|"), date);
   strtemplate.replace(QRegExp("\\|YEAR\\|"), year);

   if (!basics)
   {
     strtemplate.replace(QRegExp("\\|KDEVPRJVER\\|"), getKDevPrjVersion());
     strtemplate.replace(QRegExp("\\|PRJTYPE\\|"), getProjectType());
     strtemplate.replace(QRegExp("\\|CLASSVIEWTREE\\|"), getClassViewTree());
     strtemplate.replace(QRegExp("\\|SGMLFILE\\|"), getSGMLFile());
     strtemplate.replace(QRegExp("\\|EXEARGS\\|"), getExecuteArgs());
     strtemplate.replace(QRegExp("\\|MAKEOPTIONS\\|"), getMakeOptions());
     strtemplate.replace(QRegExp("\\|LDFLAGS\\|"), getLDFLAGS());
     strtemplate.replace(QRegExp("\\|LDADD\\|"), getLDADD());
     strtemplate.replace(QRegExp("\\|CXXFLAGS\\|"), getCXXFLAGS());
     strtemplate.replace(QRegExp("\\|ADDITCXX\\|"), getAdditCXXFLAGS());
   }

   return strtemplate;
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CProject::writeGroupEntry()
 * writeGroupEntry()
 *   Write an entry to the project file.
 *
 * Parameters:
 *   group          Name of the group.
 *   tag            The value-tag e.g version.
 *   entry          The string to store.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CProject::writeGroupEntry( const QString& group, const QString& tag, const QString& entry )
{
  config->setGroup( group );
  config->writeEntry( tag, entry );
}

/*-------------------------------------- CProject::readGroupEntry()
 * writeGroupEntry()
 *   Read an entry from the project file and return it.
 *
 * Parameters:
 *   group          Name of the group.
 *   tag            The value-tag e.g version.
 *
 * Returns:
 *   QString        The value.
 *-----------------------------------------------------------------*/
QString CProject::readGroupEntry( const QString& group, const QString& tag, const QString& defaultValue )
{
//  KConfigGroupSaver (&config, group);
  config->setGroup(group);
  QString value = config->readEntry( tag, defaultValue);
  return value;
}

bool CProject::isKDEProject()
{
  if (getProjectType() == "normal_kde" || getProjectType() == "mini_kde" || getProjectType() == "normalogl_kde")
      return true;
  return false;
}
bool CProject::isKDE2Project()
{
  if (getProjectType()=="normal_kde2" || getProjectType()=="mini_kde2" || getProjectType()=="mdi_kde2" ||
      getProjectType()=="kicker_app" || getProjectType()=="kio_slave" || getProjectType()=="kc_module" ||
      getProjectType()=="kpart_plugin" || getProjectType()=="ktheme" ||
  	  getProjectType() == "normal_kdeobjc" || getProjectType() == "mini_kdeobjc")
     return true;
  return false;
}
bool CProject::isQtProject(){
  if (getProjectType() == "normal_qt" )
    return true;
  return false;
}
bool CProject::isQt2Project(){
  if (getProjectType()=="normal_qt2" || getProjectType()=="mdi_qt2"|| getProjectType()== "mdi_qextmdi" ||
  		getProjectType()== "normal_qtobjc" )
    return true;
  return false;
}


bool CProject::isCustomProject(){
  if(getProjectType() == "normal_empty") return true;
  return false;
}

bool CProject::isObjcProject(){
  return (	getProjectType() == "normal_objc" || getProjectType() == "normal_qtobjc"
  			|| getProjectType() == "normal_kdeobjc" || getProjectType() == "mini_kdeobjc" );
}

// **************************************************************************

QString CProject::getExecutableDir()
{
  // If we arnt a custom project then we are already in the correct
  // directory - HACK alert!!!
  if (!isCustomProject())
    return "./";

  QString underDir=pathToBinPROGRAM();

  // Setup the default path - make it relative to project dir
  if (underDir.isEmpty())
  {
    underDir = getProjectDir();
    if (underDir[0] == '/')
      underDir = CToolClass::getRelativePath(getProjectDir(), underDir);
  }

  if (underDir.isEmpty())
    underDir = "./";

  if (underDir.right(1) != "/")
    underDir+="/";

  return underDir;
}

// **************************************************************************

QString CProject::getExecutable()
{
  QString exe = getExecutableDir()+getBinPROGRAM();
  return exe;
}

// **************************************************************************

QString CProject::getLibtoolDir()
{
  QString dir = readGroupEntry( "Config for BinMakefileAm", "libtool_dir", "");
  if (!dir.isEmpty() && dir.right(1) != "/")
    dir+="/";

  return dir;
}

// **************************************************************************

QString CProject::getLibtool()
{
  QString dir = getLibtoolDir();
  QString libtool;

  if (dir.isEmpty())
  {
    // Now try the projects root dir and see if one exists there
    dir = getProjectDir();
    if (!dir.isEmpty())
    {
      if (dir.right(1) != "/")
        dir+="/";

      libtool = dir+"libtool";
    }
  }
  else
    libtool=dir+"libtool";

  if (!libtool.isEmpty())
    if (QFile::exists(libtool))
      return libtool;

  return QString::null;
}

// **************************************************************************

// The directory that the program will be run from.
QString CProject::getRunFromDir()
{
  if (isCustomProject())
    return getProjectDir();

  QString underDir=pathToBinPROGRAM();
  if (underDir.isEmpty())
    underDir = getProjectDir() + getSubDir();

  if (underDir[0] != '/')
    underDir = getProjectDir() + underDir;

  return underDir;
}

/*---------------------------------------- isAScript()
 * bool CProject::isAScript()
 *
 *  checks the file if it is a script
 *
 * Returns:
 *       returns true if it is a script file
 *-----------------------------------------------------------------*/
bool CProject::isAScript(const QString &filename)
{
  bool bIsWrapper=false;
  int ch;
  QFile executable(filename);

  if (executable.open(IO_ReadOnly))
  {
   while ((ch=executable.getch())==' ' || ch=='\n' || ch=='\t');

   if (ch == '#')
    if (executable.getch() == '!')
       bIsWrapper=true;
   executable.close();
  }

  return bIsWrapper;
}

/*
 * void CProject::findMakefile(const MakefileType type, const QString& name)
 *
 * Purpose: A private (could well be just protected) helper function that
 * tries to find an existing makefile.
 * The makefile is looked for either in the user specified makefile directory
 * and, if the former is not specified, in the default project directory.
 * The parameter type specifies what kind of makefile we are looking for.
 * This function can easily be extended for "custom" projects.
 * The parameter name is the name of a makefile that we look for in the
 * directory. If name is given all the function does is to check whether
 * it really exists.
 * If name is null, then we try to guess the makefile from a list.
 * This should be very obvious, once it's found the function returns.
 *
 * Author: rokrau
 *
 * In:
 * MakefileType type,        the type of makefile we are looking for
 *                           from the enum in CProject
 * QString& name,            the name of the makefile we are looking for.
 *                           Can be QString::null
 * Out: N/A
 * Return: QString makefile, the absolute name of the makefile if found,
 *                           QString::null otherwise.
 */
QString CProject::findMakefile(const CMakefile::Type type, const QString& name)
{
	// these are the guesses for toplevel makefile names
	QStringList guess_top;
	guess_top.append("makefile");
	guess_top.append("Makefile");
	// these are the guesses for cvs makefile names
	QStringList guess_cvs;
	guess_cvs.append("Makefile.cvs");
	guess_cvs.append("Makefile.dist");
	guess_cvs.append("makefile.cvs");
	guess_cvs.append("makefile.dist");
	guess_cvs.append("autogen.sh"); // hmmm...
	// the string we are looking for
	QString makefile=QString::null;
	/*
	 * first, we find the correct directory where the makefile
	 * will be called
	 * then reset to the project's root directory
	 */
	QDir::setCurrent(dir);
	// get the path where make will run
	QString makefileDir = getDirWhereMakeWillBeCalled(dir);
	// avoid a "." subdir
	if (makefileDir == "./") {
		makefileDir = dir;
	}
	// in case of a relative path complete the path to an absolute one
	if (makefileDir[0] != '/') {
		makefileDir = dir + makefileDir;
	}
	// if we know the makefile name we can short circuit the whole affair
	if (!name.isNull()&&!name.isEmpty()) {
		// since we dont trust users we need to check
		if (QFileInfo(makefileDir + name).exists()) {
			makefile = makefileDir + name;
			return makefile;
		}
		else {
			QMessageBox::warning(0,i18n("Makefile not found"),
			    i18n("the makefile you specified does not exist or could not be accessed",
			         "You may want to check your settings in Project->Options->MakeOptions!"),
			    QMessageBox::Ok,
			    QMessageBox::NoButton,
			    QMessageBox::NoButton);
			return makefile; // still QString::null
		}
	}
	// if we dont know anything, we can try to find a valid makefile by guessing
	QStringList* list = &guess_top;
	if (type==CMakefile::cvs) list = &guess_cvs;
	for ( QStringList::Iterator it = list->begin(); it != list->end(); ++it ) {
		QString guess = makefileDir + (*it);
		if (QFileInfo(guess).exists()) {
			makefile = guess;
			return makefile ; // done when first match found
		}
	}
	/*
	 * finally, if we are still around, something is wrong and we let the
	 * user know about it
	 */
	QString makefileGenerator;
	if (getProjectType() != "normal_empty") {
		makefileGenerator = i18n(" by Build->Configure!");
	}
	else {
		makefileGenerator = i18n(". Possibly by tmake?");
	}
	// if the project is still empty, there is no need to complain
	if (!isEmpty())
	{
		/* If a toplevel Makefile wasnt found we should let the user know
		 * that he needs to create it.
		 * If the user opens a distclean project he will also get this warning
		 * which is sort of annoying, but I dont know of a way to prevent this.
		 */
		// I have to rethink this a bit where to get the info about either
		// the session management or the mainwindow for the toolbar - anyway, I need to
		// know whether we're using i18n("(Default)") or one of the configs... R.N.
#warning fixme: toplevel makefile detection when using VPATH RalfN
//	  if(type==CMakefile::toplevel){
//			QMessageBox::warning(0,i18n("Makefile not found"),
//			i18n("There is no makefile to build your application.\n"
//			     "Possibly you forgot to create the makefiles.\n"
//			     "In this case please run Build->Configure.\n"),
//			QMessageBox::Ok,
//			QMessageBox::NoButton,
//			QMessageBox::NoButton);
//		}
//		else
		if ((type==CMakefile::cvs) && !isCustomProject())
		{
			QMessageBox::warning(0,i18n("Makefile not found"),
			i18n("There is no makefile to generate the configure script.\n"
			     "Possibly you did not get a complete checkout of the project.\n"
			     "Or you need to rename the file to Makefile.cvs.\n"),
			QMessageBox::Ok,
			QMessageBox::NoButton,
			QMessageBox::NoButton);
		}
	}
	return makefile; // still QString::null
}
/*
 * void CProject::setTopMakefile(const QString& name)
 *
 * Purpose: Find and set the toplevel makefile, that is, the makefile
 *          that is called to build the application
 *
 * Author:  rokrau
 *
 * In:      QString& name,
 *          the name of the makefile if specified by the user.
 *          Can be QString::null
 * Out:     N/A
 * Return:  void
 */
void CProject::setTopMakefile(const QString& name)
{
	topMakefile=findMakefile(CMakefile::toplevel, name);
}
/*
 * void CProject::setCvsMakefile(const QString& name)
 *
 * Purpose: Find and set the cvs makefile, that is, the makefile
 *          that is called to build the configure script
 *
 * Author:  rokrau
 *
 * In:      QString& name,
 *          the name of the makefile if specified by the user.
 *          Can be QString::null
 * Out:     N/A
 * Return:  void
 */
void CProject::setCvsMakefile(const QString& name)
{
	cvsMakefile=findMakefile(CMakefile::cvs, name);
}
/** check if the Project is empty. */
bool CProject::isEmpty() {
	QStrList all;
	getAllFiles(all);
	return all.isEmpty();
}

/**
 * CProject* currentProject();
 *
 * Purpose: Return a pointer to the current CProject object
 *          This is at least a little bit nicer than accessing
 *          the global CKDevelop object directly.
 * Author:  rokrau
 * In:      N/A
 * Out:     N/A
 * Return:  a pointer to the current CProject
 **/
CProject* currentProject()
{
  CKDevelop* gkdevelop = dynamic_cast<CKDevelop*>(qApp->mainWidget());
  return gkdevelop?gkdevelop->getProject():0L;
}

QString CProject::canonicalizeDirName(const QString& dir_name)
{
  QString canonical_dir_name = dir_name;

  // replaces all character, except for [A-Za-z0-9_] class, with underscore
  canonical_dir_name.replace(QRegExp("[^A-Za-z0-9_]"), "_");

  return canonical_dir_name;
}



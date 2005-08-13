/***************************************************************************
                          perlparser.cpp  -  description
                             -------------------
    begin                : Sun Nov 2 2003
    copyright            : (C) 2003 by luc
    email                : willems.luc(at)pandora.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "perlparser.h"

#include <kdebug.h>
#include <qfile.h>
#include <qregexp.h>
#include <qfileinfo.h>

perlparser::perlparser(KDevCore* core,CodeModel* model, QString interpreter) {
  m_core = core;
  m_model = model;
  m_interpreter=interpreter;
  //get INC paths for current installed perl
  getPerlINC();

}

perlparser::~perlparser(){
}

const QStringList perlparser::UseFiles()
{
  return m_usefiles;
}

void perlparser::initialParse() {
  m_usefiles.clear();
}

void perlparser::parse(const QString &fileName){
  QFile f(fileName);
  if (!f.open(IO_ReadOnly))
      return;
  QTextStream stream(&f);
  QStringList list;
  QString rawline;
  while (!stream.eof()) {
   rawline = stream.readLine();
   list.append(rawline.stripWhiteSpace().local8Bit());
 }
 f.close();
 kdDebug(9016) << "parsing " << fileName << endl;

 m_file = m_model->create<FileModel>();
 m_file->setName(fileName );
 this->parseLines(&list,fileName);
 m_model->addFile( m_file );
}

void perlparser::parseLines(QStringList* lines,const QString &fileName)
{
  QRegExp  packagere("^[ \t]*package[ \t]+([+A-Za-z0-9_:]*).*\\;");
  QRegExp     basere("^[ \t]*use[ \t]+base[ \t]*\\(\'*\"*([A-Za-z0-9_:]*)");
  QRegExp      libre("^[ \t]*use[ \t]+lib[ \t]*\\(\'*\"*([A-Za-z0-9_:]*)");
  QRegExp      usere("^[ \t]*use[ \t]+([+A-Za-z0-9_:]*).*\\;");
  QRegExp      isare("^[ \t]*@ISA[ \t=qw\\(\'\"]*([A-Za-z0-9_: ]*)");
  QRegExp   globalre("^[ \t]*our[ \t]+\\(*([ \t,$%@*+A-Za-z0-9_]*)\\)*.*");
  QRegExp   myre("^[ \t]*my[ \t]+\\(*([ \t,$%@*+A-Za-z0-9_]*)\\)*.*");
  QRegExp      subre("^[ \t]*sub[ \t]+([A-Za-z0-9_]+)([A-Za-z0-9_]|([ \t]*[{])?)$");
  QRegExp    blessre("bless[ \t]*[\\( ]*([,$%@*+A-Za-z0-9_]*).*;");
  QRegExp     namere("^[ \t]*([$%@*])([A-Za-z0-9_]*).*$");
  QRegExp  privatere("^_([A-Za-z0-9_]*)");
  QRegExp  startpod("^=[a-z0-9]+ [a-z0-9]*");
  QRegExp  cutpod("^=cut");

  QString line;

  //clear all "last" know things
  m_lastsub="";
  m_lastattr="";
  m_inpackage = false;
  m_inscript = false;
  m_inclass=false;

  m_lastscript=0;
  m_lastpackage=0;
  m_lastclass=0;

  int lineNo = -1;
  
  bool inpod = false;
  bool endpod = false;

  //check if we are parsing a script or module
  QFileInfo fi(fileName);
  bool inscript =(fi.extension() == "pl");
  kdDebug(9016) << "inscript : " << inscript << "," << fi.extension() << endl;

  if (inscript) {
    addScript(fileName,lineNo,fi.fileName());
  }

  for ( QStringList::Iterator it = lines->begin(); it != lines->end(); ++it ) {
    ++lineNo;
    line = (*it).local8Bit();
    //empty line ?
    if (line.isEmpty()) { continue;}
    //some POD checking , quick and dirty but it seams to work
    if(inpod && endpod) { inpod=false; endpod=false;}
    //are we in pod documentation ?
    if (startpod.search(line)>=0) {inpod=true; continue;}
    //are we in pod documentation ?
    if (inpod) { endpod=( cutpod.search(line)>=0 ); continue; }


    //sub matching
    if (subre.search(line)>=0){
          QString subname=subre.cap(1);
          kdDebug(9016) << "subre match [" << subname << "]" << endl;
          bool prive = privatere.search(subname) >= 0;
          kdDebug(9016) << "prive match [" << prive << "]" << endl;
          if (m_inscript)      { addScriptSub(fileName,lineNo,subname,prive);}
          else {
             if (m_inclass)    { addClassMethod(fileName,lineNo,subname,prive);}
             else              { addPackageSub(fileName,lineNo,subname,prive);}
          }
          continue;
    } //sub

    //our matching
    if (globalre.search(line)>=0) {
        //splitup multible ours
        QString varlist=globalre.cap(1);
        kdDebug(9016) << "globalre match [" << varlist <<"]" << endl;
        QStringList vars=QStringList::split(",",varlist);
        for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
            if (namere.search(*it)>=0) {
              QString var = namere.cap(2);
              kdDebug(9016) << "namere match [" << var << "]" << endl;
              if (m_lastpackage)  { addAttributetoPackage(fileName,lineNo,var); }
              else                { addAttributetoScript(fileName,lineNo,var); }
            }
        }
        continue;
    } //globalre

        
    //bless matching
    if ((blessre.search(line)>=0) && (!m_inscript)) {
         kdDebug(9016) << "blessre match []" << endl;
         addClass(fileName,lineNo);
         addConstructor(fileName,lineNo,m_lastsub);
         continue;
    } //bless

    //base matching
    if ((basere.search(line)>=0) && (!m_inscript)) {
         QString parent = basere.cap(1);
         //create child & parent classes
         kdDebug(9016) << "basere match [" << parent << "]" << endl;
         addClass(fileName,lineNo);
         addParentClass(parent);
         continue;
    } else {
      if (libre.search(line)>=0) {
         QString path = libre.cap(1);
         //add lib to INC path list
         kdDebug(9016) << "libre match [" << path << "]" << endl;
         m_INClist.append(path);
         continue;
      } else {
         if (usere.search(line)>=0) {
           //add lib to use list for later parsing
           QString lib = usere.cap(1);
           kdDebug(9016) << "usere match [" << lib << "]" << endl;
           addUseLib(lib);
           continue;
         }                              \
     }
    } //base

    if ((isare.search(line)>=0) && (!m_inscript)) {
         QString parent = isare.cap(1);
         //create child & parent classes
         kdDebug(9016) << "isare match [" << parent << "]" << endl;
         addClass(fileName,lineNo);
         addParentClass(parent);
         continue;
    } //isa

    if ((packagere.search(line)>=0) && (!m_inscript)) {
         QString package=packagere.cap(1);
         kdDebug(9016) << "packagere match [" << package << "]" << endl;
         addPackage(fileName,lineNo,package);
         continue;
    }//package

  } // for lines loop
}

void perlparser::addPackage(const QString& fileName ,int lineNr , const QString& name)
{
 kdDebug(9016) << "AddPackage [" << name << "]" << endl;
 NamespaceDom package = m_model->create<NamespaceModel>();

 package->setName(name);
 package->setFileName(fileName );
 package->setStartPosition(lineNr, 0 );
 package->setScope(name);

 if (!m_file->hasNamespace(name)) {
     m_file->addNamespace(package);
     m_lastpackage=package;
 } else {
   kdDebug(9016) << "addPackage [" << name << " exist]" << endl;
 }

 //clear all "last" know things
 m_lastpackagename=name;
 m_lastsub="";
 m_lastattr="";
 m_inpackage=true;
 m_inscript = false;
 m_inclass=false;

 m_lastclass=0;
 m_lastscript=0;

}

void perlparser::addScript(const QString& fileName ,int lineNr ,const QString& name)
{
 kdDebug(9016) << "addScript [" << name << "]" << endl;

 //map name of script under /scripts
 //m_file->setName("/Scripts/"+name);

 kdDebug(9016) << "addScript [" << name << "]" << endl;
 NamespaceDom script = m_model->create<NamespaceModel>();

 script->setName(name);
 script->setFileName(fileName );
 script->setStartPosition(lineNr, 0 );
 script->setScope(name);

 if (!m_file->hasNamespace(name)) {
      m_file->addNamespace(script);
      m_lastscript=script;
 } else {
   kdDebug(9016) << "addScript [" << name << " exist]" << endl;
 }
 
 //clear all "last" know things
 m_lastsub="";
 m_lastattr="";
 m_inpackage = false;
 m_inscript = true;
 m_inclass=false;

 m_lastscriptname=name;
 m_lastpackage=0;
 m_lastclass=0;

}

void perlparser::addAttributetoPackage(const QString& fileName ,int lineNr ,const QString& name)
{
 kdDebug(9016) << "addAttributetoPackage [" << name << "]" << endl;
 VariableDom var = m_model->create<VariableModel>();
 var->setName(name);
 var->setFileName( fileName );
 var->setStartPosition( lineNr, 0 );
 if (m_lastpackage) {
    if (!m_lastpackage->hasVariable(var->name()))
         m_lastpackage->addVariable(var);
 } else {
    kdDebug(9016) << "addAttributetoPackge[ no m_file]" << endl;
 }

 m_lastattr=name;
}

void perlparser::addAttributetoScript(const QString& fileName ,int lineNr ,const QString& name)
{
  kdDebug(9016) << "addAttributetoScript [" << name << "]" << endl;
  VariableDom var = m_model->create<VariableModel>();
  var->setName(name);
  var->setFileName( fileName );
  var->setStartPosition( lineNr, 0 );
  if (m_lastscript) {
    if (!m_lastscript->hasVariable(var->name()))
       m_lastscript->addVariable(var);
  } else {
    kdDebug(9016) << "addAttributeScript[ no m_file]" << endl;
  }
}

void perlparser::addClass(const QString& fileName ,int lineNr)
{
  kdDebug(9016) << "addClass [ " << m_lastpackagename << " ]" << endl;
  if (m_lastpackage->hasClass(m_lastpackagename)) {
    kdDebug(9016) << "Class already defined" << endl;
  } else {
    kdDebug(9016) << "new Class" << endl;
    ClassDom lastClass = m_model->create<ClassModel>();
    lastClass->setName(m_lastpackagename);
   	lastClass->setFileName(fileName);
	  lastClass->setStartPosition(lineNr, 0);
    m_lastpackage->addClass(lastClass);
    m_lastclass=lastClass;
    m_inclass=true;
  }
}

void perlparser::addConstructor(const QString& fileName ,int lineNr ,const QString& name)
{
  kdDebug(9016) << "set Constructor["<< name << "]" << endl;

  FunctionDom method;

  if (m_lastpackage->hasFunction(name)) {
      //remove last sub frompackage scope
      method = m_lastpackage->functionByName(name)[0];
      method->getStartPosition(&lineNr,0);
      m_lastpackage->removeFunction(method);
  }
  method = m_lastclass->functionByName(name)[0];
  if (!method) { 
     kdDebug(9016) << "add new Constructor["<< name << ", " << lineNr << "]" << endl;
     method = m_model->create<FunctionModel>();
     method->setName(name);
     method->setFileName( fileName );
     method->setStartPosition( lineNr, 0 );
     m_lastclass->addFunction(method);
  }
  method->setStatic(true);
  //update class position
  m_lastclass->setStartPosition(lineNr,0);
}

void perlparser::addGlobalSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
  kdDebug(9016) << "addGlobalSub[ " << name << "]" << endl;

  FunctionDom method = m_model->create<FunctionModel>();
  method->setName(name);
  method->setFileName( fileName );
  method->setStartPosition( lineNr, 0 );
  method->setStatic(true);
  if (privatesub)
     method->setAccess(CodeModelItem::Private);
  if (m_lastpackage) {
    if (!m_lastpackage->hasFunction(method->name()))
         m_lastpackage->addFunction(method);
  } else {
    kdDebug(9016) << "addGlobalsub[ no m_lastpackage]" << endl;
  }

 //also add seperate to namespace
 addPackageSub(fileName,lineNr,name,privatesub);
 m_lastsub=name;
}

void perlparser::addScriptSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)

{
  kdDebug(9016) << "addScriptSub[ " << name << "]" << endl;
  FunctionDom method = m_model->create<FunctionModel>();
  method->setName(name);
  method->setFileName( fileName );
  method->setStartPosition( lineNr, 0 );
  if (privatesub)
     method->setAccess(CodeModelItem::Private);
  if(m_lastscript) {
    m_lastscript->addFunction(method);
  } else {
  }

  m_lastsub=name;
}

void perlparser::addClassMethod(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
  kdDebug(9016) << "addClassMethod[ " << name << "]" << endl;
  FunctionDom method = m_model->create<FunctionModel>();
  method->setName(name);
  method->setFileName( fileName );
  method->setStartPosition( lineNr, 0 );
  method->setVirtual(true);
  if (privatesub)
     method->setAccess(CodeModelItem::Private);
  if (m_lastclass) {
    if (!m_lastclass->hasFunction(method->name()))
       m_lastclass->addFunction(method);
  } else {
    kdDebug(9016) << "addClassmethod[ no m_lastclass]" << endl;
  }

// addPackageSub(fileName,lineNr,name,privatesub);
 m_lastsub=name;
}

void perlparser::addPackageSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
  kdDebug(9016) << "addPackageSub[ " << name << "]" << endl;
  FunctionDom method = m_model->create<FunctionModel>();
  method->setName(name);
  method->setFileName( fileName );
  method->setStartPosition( lineNr, 0 );
  if (privatesub)
     method->setAccess(CodeModelItem::Private);
  if (m_lastpackage) {
    if (!m_lastpackage->hasFunction(method->name()))
       m_lastpackage->addFunction(method);
 } else {
   kdDebug(9016) << "addPackageSub[ no m_file]" << endl;
 }
 m_lastsub=name;
}

void perlparser::addParentClass(const QString& parent)
{
 kdDebug(9016) << "addParentClass[ " << parent << "]" << endl;
 if (m_lastclass) {
        m_lastclass->addBaseClass(parent);
 } else {
    kdDebug(9016) << "addParentClass[ no m_lastclass]" << endl;
 }
}

void perlparser::addUseLib(const QString& lib)
{
 if (!m_model->hasFile(lib)) {
   if (m_usefiles.findIndex(lib) == -1) {
      //only add if not already parsed or in the list
      kdDebug(9016) << "add lib for later parsing [" << lib << "]" << endl;
      m_usefiles.append(lib);
   }
 }
}

void perlparser::getPerlINC() {


 m_INClist.clear();

 QString cmd = "/usr/bin/perl -e\" print join('|',@INC);\"";
 QString result;

 FILE *fd = popen(cmd.local8Bit().data(), "r");
 char buffer[4090];
 QByteArray array;

 while (!feof(fd)) {
        int n = fread(buffer, 1, 2048, fd);
        if (n == -1) {
            pclose(fd);
            return;
        }
        array.setRawData(buffer, n);
        result=QString::QString(array);
        array.resetRawData(buffer, n);
 }
 pclose(fd);
 //get INC list so we can use it to parse "use" modules
 m_INClist = QStringList::split(QString("|"),result);
 kdDebug(9016) << "INC " << m_INClist.size() << " "<< result << endl;
}

QString perlparser::findLib( const QString& lib)
{
  QString result;

  QString file=lib;
  file.replace( QRegExp("::"), QString("/"));

  //find the correct path by using the INC list
  QStringList::Iterator inc = m_INClist.begin();
  while((inc != m_INClist.end()) && (result.isEmpty()) ) {
     QFileInfo fi((*inc) + "/" + file + ".pm");
     if ( fi.exists() ) {
        result = (*inc) + "/" + file + ".pm";
     }
     ++inc;
  }
  return result;
}


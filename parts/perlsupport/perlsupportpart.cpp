/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "perlsupportpart.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kregexp.h>
#include <qregexp.h>

#include <qprogressbar.h>
#include <kstatusbar.h>
#include "kdevmainwindow.h"

#include <kprocess.h>
#include <stdlib.h>
#include <unistd.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevappfrontend.h"
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "parsedscript.h"
#include "domutil.h"
#include "programmingbycontract.h"


typedef KGenericFactory<PerlSupportPart> PerlSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevperlsupport, PerlSupportFactory( "kdevperlsupport" ) );

PerlSupportPart::PerlSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport("PerlSupport", "perl", parent, name ? name : "PerlSupportPart")
{
    setInstance(PerlSupportFactory::instance());

    setXMLFile("kdevperlsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

    KAction *action;

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setStatusText( i18n("Runs the Perl program") );

    action = new KAction( i18n("Execute String..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setStatusText( i18n("Executes a string as Perl code") );

    action = new KAction( i18n("Start Perl Interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setStatusText( i18n("Starts the Perl interpreter without a program") );

    action = new KAction( i18n("Find Perl function documentation..."), 0,
                          this, SLOT(slotPerldocFunction()),
                          actionCollection(), "help_perldocfunction" );
    action->setStatusText( i18n("Show the documentation page of a Perl function") );

    action = new KAction( i18n("Find Perl FAQ entry..."), 0,
                          this, SLOT(slotPerldocFAQ()),
                          actionCollection(), "help_perldocfaq" );
    action->setStatusText( i18n("Show the FAQ entry for a keyword") );
    //get INC paths for current installed perl
    getPerlINC();
}


PerlSupportPart::~PerlSupportPart()
{}


void PerlSupportPart::projectOpened()
{
    kdDebug(9007) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PerlSupportPart::projectClosed()
{
}


void PerlSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString extension = fi.extension();
    if (extension == "pl" || extension == "pm") {
        kdDebug(9016) << "maybe " << fileName << endl;
        classStore()->removeWithReferences(fileName);
        parse(fileName);
    }
}

QString PerlSupportPart::findLib( const QString& lib)
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

void PerlSupportPart::parseUseFiles()
{
 QString filename;
 kdDebug(9016) << "parse addional libs" << endl;
 //parse addional use files
 for (QStringList::Iterator it = m_usefiles.begin(); it != m_usefiles.end() ;++it) {
      if (!classStore()->getScopeByName(*it)) {
         filename = findLib(*it);
         //if something found , parse it
         if (!filename.isEmpty()) {
           kdDebug(9016) << "found " << filename << endl;
           classStore()->removeWithReferences(filename);
           parse(filename);
         }
      }
 }
}

void PerlSupportPart::initialParse()
{
    kdDebug(9016) << "initialParse()" << endl;

    if (project()) {
        //copy from cpp support : give user some feedback
        mainWindow()->statusBar()->message( i18n("Updating...") );
        kapp->processEvents( );

        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allFiles();
        m_usefiles.clear();

        //progress bar
        QProgressBar* bar = new QProgressBar( files.count( ), mainWindow( )->statusBar( ) );
        bar->setMinimumWidth( 120 );
        bar->setCenterIndicator( true );
        mainWindow( )->statusBar( )->addWidget( bar );
        bar->show( );
        int n = 0;
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9016) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
            //update progress bar
            bar->setProgress( n++ );
            if( (n%5) == 0 )
              kapp->processEvents();
        }

        if (m_usefiles.size() > 0) {
            parseUseFiles();
        }

        emit updatedSourceInfo();
        //remove progressbar
        mainWindow( )->statusBar( )->removeWidget( bar );
        delete bar;
        kapp->restoreOverrideCursor();
        mainWindow()->statusBar()->message( i18n("Done") );
      
    } else {
        kdDebug(9016) << "No project" << endl;
    }
}


void PerlSupportPart::addedFilesToProject(const QStringList &fileList)
{
	kdDebug(9016) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		maybeParse(project()->projectDirectory() + "/" + ( *it ) );
	}

	emit updatedSourceInfo();
}


void PerlSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    kdDebug(9016) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		classStore()->removeWithReferences(project()->projectDirectory() + "/" + ( *it ) );
	}

    emit updatedSourceInfo();
}


void PerlSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9016) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        m_usefiles.clear();
        maybeParse(fileName);
        if (m_usefiles.size() > 0)
            parseUseFiles();
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PerlSupportPart::features()
{
    return KDevLanguageSupport::Features(Classes | Functions | Variables | Namespaces | Scripts );
//    return Functions;
}


QStringList PerlSupportPart::fileFilters()
{
    QStringList l;
    l << "*.pl" << "*.pm";
    return l;
}

void PerlSupportPart::parse(const QString &fileName){
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
 this->parseLines(&list,fileName);
}

void PerlSupportPart::parseLines(QStringList* lines,const QString &fileName)
{
  KRegExp  packagere("^[ \t]*package[ \t]+([+A-Za-z0-9_:]*).*\\;");
  KRegExp     basere("^[ \t]*use[ \t]+base[ \t]*\\(\'*\"*([A-Za-z0-9_:]*)");
  KRegExp      libre("^[ \t]*use[ \t]+lib[ \t]*\\(\'*\"*([A-Za-z0-9_:]*)");
  KRegExp      usere("^[ \t]*use[ \t]+([+A-Za-z0-9_:]*).*\\;");
  KRegExp      isare("^[ \t]*@ISA[ \t=qw\\(\'\"]*([A-Za-z0-9_: ]*)");
  KRegExp   globalre("^[ \t]*our[ \t]+\\(*([ \t,$%@*+A-Za-z0-9_]*)\\)*.*");
  KRegExp      subre("^[ \t]*sub[ \t]+([A-Za-z0-9_]*).*$");
  KRegExp    blessre("bless[ \t]*[\\( ]*([,$%@*+A-Za-z0-9_]*).*;");
  KRegExp     namere("^[ \t]*([$%@*])([A-Za-z0-9_]*).*$");
  KRegExp  privatere("^_([A-Za-z0-9_]*)");
  KRegExp  startpod("^=[a-z0-9]+ [a-z0-9]*");
  KRegExp  cutpod("^=cut");

  QString line;

  //clear all "last" know things
  m_lastpackage="";
  m_lastscript="";
  m_lastclass="";
  m_lastsub="";
  m_lastattr="";
  m_inpackage = false;
  m_inscript = false;
  m_inclass=false;
  
  int lineNo = -1;
  bool inpod = false;
  bool endpod = false;

  //check if we are parsing a script or module
  QFileInfo fi(fileName);
  bool inscript =(fi.extension() == "pl");
  kdDebug(9016) << "inscript : " << inscript << "," << fi.extension() << endl;

  if (inscript) { addScript(fileName,lineNo,fileName);}
  
  for ( QStringList::Iterator it = lines->begin(); it != lines->end(); ++it ) {
    ++lineNo;
    line = (*it).local8Bit();
    //empty line ?
    if (line.isEmpty()) { continue;}
    //some POD checking , quick and dirty but it seams to work
    if(inpod && endpod) { inpod=false; endpod=false;}
    //are we in pod documentation ?
    if (startpod.match(line)) {inpod=true; continue;}
    //are we in pod documentation ?
    if (inpod) { endpod= cutpod.match(line); continue; }

    //sub matching
    if (subre.match(line)) {
          QString subname=subre.group(1);
          kdDebug(9016) << "subre match [" << subname << "]" << endl;
          bool prive = privatere.match(subname);
          kdDebug(9016) << "prive match [" << prive << "]" << endl;
          if (m_inscript)      { addScriptSub(fileName,lineNo,subname,prive);}
          else {
             if (m_inclass)    { addClassMethod(fileName,lineNo,subname,prive);}
             else              { addGlobalSub(fileName,lineNo,subname,prive);}
          }
          continue;
    } //sub
      
    //our matching
    if (globalre.match(line)) {
        //splitup multible ours
        QString varlist=globalre.group(1);
        kdDebug(9016) << "globalre match [" << varlist <<"]" << endl;
        QStringList vars=QStringList::split(",",varlist);
        for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
            if (namere.match(*it)) {
              QString var = namere.group(2);
              kdDebug(9016) << "namere match [" << var << "]" << endl;
              if (m_lastpackage) { addAttributetoPackage(fileName,lineNo,var); }
              else               { addAttributetoScript(fileName,lineNo,var); }
            }
        }
        continue;
    } //globalre
      
    //bless matching
    if (blessre.match(line)) {
         kdDebug(9016) << "blessre match [" << m_lastpackage << "]" << endl;
         addClass(fileName,lineNo,m_lastpackage);
         addConstructor(m_lastclass,m_lastsub);
         continue;
    } //bless
      
    //base matching
    if (basere.match(line)) {
         QString parent = basere.group(1);
         //create child & parent classes
         kdDebug(9016) << "basere match [" << parent << "]" << endl;
         addClass(fileName,lineNo,m_lastpackage);
         addParentClass(parent,m_lastpackage);
         continue;
    } else {
      if (libre.match(line)) {
         QString path = libre.group(1);
         //add lib to INC path list
         kdDebug(9016) << "libre match [" << path << "]" << endl;
         m_INClist.append(path);
         continue;
      } else {
         if (usere.match(line)) {
           //add lib to use list for later parsing
           QString lib = usere.group(1);
           kdDebug(9016) << "usere match [" << lib << "]" << endl;
           m_usefiles.append(lib);
           continue;
         }
     }
    } //base
      
    if (isare.match(line)) {
         QString parent = isare.group(1);
         //create child & parent classes
         kdDebug(9016) << "isare match [" << parent << "]" << endl;
         addClass(fileName,lineNo,m_lastpackage);
         addParentClass(parent,m_lastpackage);
         continue;
    } //isa
      
    if (packagere.match(line)) {
         QString package=packagere.group(1);
         kdDebug(9016) << "packagere match [" << package << "]" << endl;
         addPackage(fileName,lineNo,package);
         continue;
    }//package

  } // for lines loop
}


QString PerlSupportPart::interpreter()
{
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevperlsupport/run/interpreter");
    if (prog.isEmpty())
        prog = "perl";

    return prog;
}


void PerlSupportPart::startApplication(const QString &program)
{
    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevperlsupport/run/terminal");
    appFrontend()->startAppCommand(QString::QString(), program, inTerminal);
}


void PerlSupportPart::slotExecute()
{
    QString program =  project()->mainProgram();
    QString cmd = interpreter() + " " + program;
    startApplication(cmd);
}


void PerlSupportPart::slotStartInterpreter()
{
    startApplication(interpreter());
}


void PerlSupportPart::slotExecuteString()
{
    bool ok;
    QString cmd = KLineEditDlg::getText(i18n("String to execute"), QString::null, &ok, 0);
    if (ok) {
        cmd.prepend("'");
        cmd.append("'");
        startApplication(cmd);
    }
}


void PerlSupportPart::slotPerldocFunction()
{
    bool ok;
    QString key = KLineEditDlg::getText(i18n("Show Perl documentation for function:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "perldoc:functions/";
        url += key;
        partController()->showDocument(KURL(url));
    }
}


void PerlSupportPart::slotPerldocFAQ()
{
    bool ok;
    QString key = KLineEditDlg::getText(i18n("Show FAQ entry for keyword:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "perldoc:faq/";
        url += key;
        partController()->showDocument(KURL(url));
    }
}

void PerlSupportPart::addPackage(const QString& fileName ,int lineNr , const QString& name)
{
 ParsedScopeContainer *p = 0;
 ParsedScopeContainer *s = 0;
 
 REQUIRE( "Valid package name",( ! name.isEmpty() ));

 //check if already exist
 p=classStore()->globalScope()->getScopeByName(name);
 if (!p) {
     p = new ParsedScopeContainer;
     p->setName(name);
     p->setDefinedInFile(fileName);
     p->setDeclaredInFile(fileName);
     p->setDefinedOnLine(lineNr);
     classStore()->globalScope()->addScope(p);

     //add to classstore 
     s = classStore()->getScopeByName(name);
     if (!s) {
        classStore()->addScope(p);
     }
 }

 //clear all "last" know things
 m_lastpackage=name;
 m_lastscript="";
 m_lastclass="";
 m_lastsub="";
 m_lastattr="";
 m_inpackage=true;
 m_inscript = false;
 m_inclass=false;
}

void PerlSupportPart::addScript(const QString& fileName ,int lineNr ,const QString& name)
{
 ParsedScript *p = 0;

 REQUIRE( "Valid script name",name.length() > 0 );

 //check if already exist
 p=classStore()->getScriptByName(name);
 if (!p) {
     p = new ParsedScript;
     p->setName(name);
     p->setDefinedInFile(fileName);
     p->setDeclaredInFile(fileName);
     p->setDefinedOnLine(lineNr);
     classStore()->addScript(p);
 }

 //clear all "last" know things
 m_lastpackage="";
 m_lastscript=name;
 m_lastclass="";
 m_lastsub="";
 m_lastattr="";
 m_inpackage = false;
 m_inscript = true;
 m_inclass=false;
}

void PerlSupportPart::addAttributetoPackage(const QString& fileName ,int lineNr ,const QString& name)
{
 ParsedScopeContainer *p = 0;
 ParsedAttribute *attr=0;
 
 REQUIRE( "Valid attribute name",name.length() > 0 );
 REQUIRE( "Valid package name",m_lastpackage.length() > 0 );

 //get package in namespace scope
 p=classStore()->getScopeByName(m_lastpackage);
 if (p) {
     attr=p->getAttributeByName(name);
     if (!attr) {
      attr = new ParsedAttribute;
      attr->setName(name);
      attr->setDefinedInFile(fileName);
      attr->setDeclaredInFile(fileName);
      attr->setDefinedOnLine(lineNr);
      p->addAttribute(attr);
     }
 }

 m_lastattr=name;
}

void PerlSupportPart::addAttributetoScript(const QString& fileName ,int lineNr ,const QString& name)
{
 ParsedScript *p = 0;
 ParsedAttribute *attr=0;

 REQUIRE( "Valid attribute name",name.length() > 0 );
 REQUIRE( "Valid script name",m_lastscript.length() > 0 );

 //get package in global scope
 p=classStore()->getScriptByName(m_lastscript);
 if (p) {
     attr=p->getAttributeByName(name);
     if (!attr) {
       attr = new ParsedAttribute;
       attr->setName(name);
       attr->setDefinedInFile(fileName);
       attr->setDeclaredInFile(fileName);
       attr->setDefinedOnLine(lineNr);
       p->addAttribute(attr);
       m_lastattr=name;
     }
 }
}

void PerlSupportPart::addClass(const QString& fileName ,int lineNr ,const QString& name)
{
 ParsedClass *p = 0;
 ParsedClass *c = 0;
 ParsedScopeContainer *s = 0;

 REQUIRE( "Valid class name",name.length() > 0 );

 //get class in global scope
 c = classStore()->getClassByName(name);
 if (!c) {
     p = new ParsedClass;
     p->setName(name);
     p->setDefinedInFile(fileName);
     p->setDeclaredInFile(fileName);
     p->setDefinedOnLine(lineNr);
     classStore()->addClass(p);

     //store class in package name scope
     s = classStore()->getScopeByName(m_lastpackage);
     if (s) {
       c = s->getClassByName(name);
       if (!c) {
         s->addClass(p);
       }
     }
 }
 
 m_inclass=true;
 m_lastclass=name;
}

void PerlSupportPart::addConstructor(const QString& classname,const QString& methodname)
{
 QValueList<ParsedMethod*> list;
 ParsedClass  *c = 0;
 ParsedMethod *p = 0;
 ParsedMethod *n = 0;

 REQUIRE( "Valid class name",classname.length() > 0 );
 REQUIRE( "Valid method name",methodname.length() > 0 );

 kdDebug(9016) << "method [" <<  m_lastpackage << "],[" <<  methodname <<"]" << endl;

 //remove global method name
 list = classStore()->globalScope()->getMethodByName(m_lastpackage + "::" +methodname);
 if (list.size() > 0) {
     //we can't overload so only first instance will be used
     p = list.first();
     //add constructor to class
     c = classStore()->getClassByName(classname);
     if (c) {
        //because remove clears the p pointer , create a new ParsedMethod
        n = new ParsedMethod;
        n->setName(methodname);
        n->setDefinedInFile(p->definedInFile());
        n->setDeclaredInFile(p->declaredInFile());
        n->setDefinedOnLine(p->definedOnLine());
        n->setAccess(p->access());
        n->setIsConstructor(true);
        c->addMethod(n);
        //remove global function
        classStore()->globalScope()->removeMethod(p);
     }
     list.clear();
 }
}

void PerlSupportPart::addGlobalSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
 QValueList<ParsedMethod*> list;

 REQUIRE( "Valid sub name",name.length() > 0 );
 REQUIRE( "Valid package name",m_lastpackage.length() > 0 );

 //get class in global scope
 list=classStore()->globalScope()->getMethodByName( m_lastpackage+"::"+name);
 if (list.size() == 0) {
     ParsedMethod *method = new ParsedMethod;
     method->setName(m_lastpackage + "::" + name);
     method->setDefinedInFile(fileName);
     method->setDeclaredInFile(fileName);
     method->setDefinedOnLine(lineNr);
     if (privatesub) { method->setAccess(PIE_PRIVATE);}
     classStore()->globalScope()->addMethod(method);
 }
 list.clear();
 //also add seperate to namespace
 addPackageSub(fileName,lineNr,name,privatesub);
 m_lastsub=name;
}

void PerlSupportPart::addScriptSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{

 QValueList<ParsedMethod*> list;
 ParsedScript *s = 0;

 REQUIRE( "Valid sub name",name.length() > 0 );
 REQUIRE( "Valid script name",m_lastscript.length() > 0 );

 //get class in global scope
 s=classStore()->getScriptByName(m_lastscript);
 if (s) {
     list = s->getMethodByName(name);
     if (list.size() == 0) {
       ParsedMethod *method = new ParsedMethod;
       method->setName(name);
       method->setDefinedInFile(fileName);
       method->setDeclaredInFile(fileName);
       method->setDefinedOnLine(lineNr);
       if (privatesub) { method->setAccess(PIE_PRIVATE);}
       s->addMethod(method);
     }
     list.clear();
 }
 m_lastsub=name;
}

void PerlSupportPart::addClassMethod(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
 QValueList<ParsedMethod*> list;
 ParsedClass  *c = 0;

 REQUIRE( "Valid sub name",name.length() > 0 );
 REQUIRE( "Valid class name",m_lastclass.length() > 0 );

 //get class in global scope
 c=classStore()->getClassByName(m_lastclass);
 if (c) {
     list = c->getMethodByName(name);
     if (list.size()==0) {
       ParsedMethod *method = new ParsedMethod;
       method->setName(name);
       method->setDefinedInFile(fileName);
       method->setDeclaredInFile(fileName);
       method->setDefinedOnLine(lineNr);
       if (privatesub) { method->setAccess(PIE_PRIVATE);}
       c->addMethod(method);
     }
     list.clear();
 }
 //also add seperate to namespace
 addPackageSub(fileName,lineNr,name,privatesub);
 m_lastsub=name;
}

void PerlSupportPart::addPackageSub(const QString& fileName ,int lineNr ,const QString& name ,bool privatesub)
{
 ParsedMethod *p = 0;
 ParsedScopeContainer *s = 0;
 
 REQUIRE( "Valid sub name",name.length() > 0 );
 REQUIRE( "Valid package name",m_lastpackage.length() > 0 );

 //get class in global scope
 s=classStore()->getScopeByName(m_lastpackage);
 if (s) {
     p = s->getMethodByNameAndArg(m_lastpackage + "::" + name);
     if (!p) {
       ParsedMethod *method = new ParsedMethod;
       method->setName(m_lastpackage + "::" + name);
       method->setDefinedInFile(fileName);
       method->setDeclaredInFile(fileName);
       method->setDefinedOnLine(lineNr);
       if (privatesub) { method->setAccess(PIE_PRIVATE);}
       s->addMethod(method);
     }
 }
}

void PerlSupportPart::addParentClass(const QString& parent ,const QString& child)
{
 ParsedParent *p = 0;
 ParsedClass *c = 0;

 REQUIRE( "Valid parent name",parent.length() > 0 );
 REQUIRE( "Valid child name",child.length() > 0 );

 //get class in global scope
 c=classStore()->getClassByName(child);
 if(c) {
    p = new ParsedParent;
    p->setName(parent);
    p->setAccess(PIE_PUBLIC);
    c->addParent(p);
 }
}

void PerlSupportPart::addUseLib(const QString& lib)
{

 REQUIRE( "Valid lib name",lib.length() > 0 );

 if (!classStore()->getScopeByName(lib)) {
    if (m_usefiles.findIndex(lib) == -1) {
      //only add if not already parsed or in the list
      kdDebug(9016) << "add lib for later parsing [" << lib << "]" << endl;
      m_usefiles.append(lib);
    }
 }
}

void PerlSupportPart::getPerlINC() {


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

#include "perlsupportpart.moc"

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

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevappfrontend.h"
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"
#include "domutil.h"


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

    core()->insertNewAction( action );
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


void PerlSupportPart::initialParse()
{
    kdDebug(9016) << "initialParse()" << endl;

    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9016) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
        }

        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
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
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PerlSupportPart::features()
{
    return KDevLanguageSupport::Features(Classes | Functions | Variables );
//    return Functions;
}


QStringList PerlSupportPart::fileFilters()
{
    QStringList l;
    l << "*.pl" << "*.pm";
    return l;
}

void PerlSupportPart::parse(const QString& fileName){
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

void PerlSupportPart::parseLines(QStringList* lines,const QString& fileName)
{
  KRegExp  packagere("^[ \t]*package[ \t]+([+A-Za-z0-9_:]*).*\\;");
  KRegExp     basere("^[ \t]*use[ \t]+base[ \t]*\\(\'*\"*([A-Za-z0-9_:]*)");
  KRegExp      isare("^[ \t]*@ISA[ \t=qw\\(\'\"]*([A-Za-z0-9_: ]*)");
  KRegExp   globalre("^[ \t]*our[ \t]+\\(*([ \t,$%@*+A-Za-z0-9_]*)\\)*.*");
  KRegExp      subre("^[ \t]*sub[ \t]+([A-Za-z0-9_]*).*$");
  KRegExp    blessre("bless[ \t]*[\\( ]*([,$%@*+A-Za-z0-9_]*).*;");
  KRegExp     namere("^[ \t]*([$%@*])([A-Za-z0-9_]*).*$");
  KRegExp  privatere("^_([A-Za-z0-9_]*)");
  KRegExp  startpod("^=[a-z0-9]+ [a-z0-9]*");
  KRegExp  cutpod("^=cut");

  ParsedClass  *lastClass = 0;
  ParsedMethod *lastsub = 0;

  QString line;
  QString package;
  QString parent;
  QString subname;
  QString var;
  
  int lineNo = -1;
  int packagelineNo =0;
  bool inClass = false;
  bool inpod = false;
  bool endpod = false;
  
  for ( QStringList::Iterator it = lines->begin(); it != lines->end(); ++it ) {
    ++lineNo;
    line = (*it).local8Bit();

    //empty line ?
    if (line.isNull()) {
       continue;
    }

    //some POD checking , quick and dirty but it seams to work
    if(inpod && endpod) {
         inpod=false;
         endpod=false;
//         kdDebug(9016) << "END pod : [" << lineNo << "," << inpod << "] " << line << endl;
    }

    //are we in pod documentation ?
    if (startpod.match(line)) {
         inpod=true;
//         kdDebug(9016) << "pod : [" << lineNo << "," << inpod << "," << endpod << "]" << line << endl;
         continue;
    }
    //are we in pod documentation ?
    if (inpod) {
         endpod= cutpod.match(line);
//         kdDebug(9016) << "skipping : [" << line << "]" << endl;
         continue;
    }

      if (subre.match(line) && package) {
          subname=subre.group(1);
          if (subname.isEmpty()) {
             kdDebug(9016) << "WARNING : subre empty" << endl;
             continue;
          }
//          kdDebug(9016) << "subre match [" << subname << "]" << endl;
          lastsub = new ParsedMethod;
          if (lastClass) { lastsub->setName(subname); }
          else           { lastsub->setName(package + "::" + subname); }   
          if (privatere.match(subname)) { lastsub->setAccess(PIE_PRIVATE); }
          else                          { lastsub->setAccess(PIE_PUBLIC); }
          if (inClass) {
             ParsedMethod *old = lastClass->getMethod(lastsub);
             if (!old) {
//                kdDebug(9016) << "class method add" << endl;
                lastClass->addMethod(lastsub);
             } else {
                kdDebug(9016) << "WARNING: class method[" << lastsub->name() <<"] already exist" << endl;
                delete lastsub;
                lastsub=old;
             }
          } else {
             ParsedMethod *old = classStore()->globalScope()->getMethod(lastsub);
             if (!old) {
//                kdDebug(9016) << "global sub add" << endl;
                classStore()->globalScope()->addMethod(lastsub);
             } else {
                kdDebug(9016) << "WARNING: global sub[" << lastsub->name() <<"] already exit" << endl;
                delete lastsub;
                lastsub=old;
             }
          }
          lastsub->setDefinedInFile(fileName);
          lastsub->setDefinedOnLine(lineNo);
          continue;
      } //sub
      
      if (globalre.match(line) && package) {
//        kdDebug(9016) << "globare match" << endl;
        //splitup mulible ours
        QStringList vars=QStringList::split(",",globalre.group(1));
        for ( QStringList::Iterator it = vars.begin(); it != vars.end(); ++it ) {
            if (namere.match(*it)) {
              var=package + "::" + namere.group(2);
              if (var.isEmpty()) {
                kdDebug(9016) << "WARNING: global var empty" << endl;
                continue;
	            }
              if (!classStore()->globalScope()->getAttributeByName(var)) {
                ParsedAttribute *attr = new ParsedAttribute;
                attr->setName(var);
                attr->setDefinedInFile(fileName);
                attr->setDefinedOnLine(lineNo);
//                kdDebug(9016) << "global var [" << var << "] added" << var << endl;
                classStore()->globalScope()->addAttribute(attr);
              }
            }
          } //for
        continue;
      } //globalre
      
      if (blessre.match(line) && package) {
//         kdDebug(9016) << "blessre match" << endl;
         inClass=true;
         //create class if not already there because of base match
         if (!classStore()->hasClass(package) && (!lastClass)) {
           lastClass = new ParsedClass;
           lastClass->setName(package);
           lastClass->setDefinedInFile(fileName);
           lastClass->setDeclaredInFile(fileName);
           lastClass->setDefinedOnLine(lineNo);
           lastClass->setDeclaredOnLine(packagelineNo);
           
//           kdDebug(9016) << "adding new BLESSED class" << endl;
           classStore()->globalScope()->addClass(lastClass);
           classStore()->addClass(lastClass);
         } else {
           kdDebug(9016) << "WARNING: Class ["<< package << "] already exist" << endl;
           lastClass = classStore()->getClassByName(package);
         }
         //mark last sub as constructor
       if (lastsub && lastClass) {
//              kdDebug(9016) << "adding new Constructor " << lastsub->name()<< endl;
              lastsub->setIsConstructor(true);
              lastsub->setName(subname);
              lastClass->addMethod(lastsub); 
         }
       continue;
      } //bless
      
      if (basere.match(line) && package) {
         parent = basere.group(1);
	       if (parent.isEmpty()) {
             kdDebug(9016) << "WARNING : empty basere match"  << basere.group(1) << endl;
	       continue;
	       }
//         kdDebug(9016) << "basere match ["  << basere.group(1) << "]" << endl;
         inClass=true;
         //create class if not already there 
         if (!classStore()->hasClass(package)) {
          //create class
          lastClass = new ParsedClass;
          lastClass->setName(package);
          lastClass->setDefinedInFile(fileName);
          lastClass->setDeclaredInFile(fileName);
          lastClass->setDefinedOnLine(lineNo);
          lastClass->setDeclaredOnLine(packagelineNo);
//          kdDebug(9016) << "adding new BASE class" << endl;
          classStore()->globalScope()->addClass(lastClass);
          classStore()->addClass(lastClass);
         } else {
           kdDebug(9016) << "WARNING : Class ["<< package << "] already exist" << endl;
           lastClass = classStore()->getClassByName(package);
         } 
         //add parent if found
         if (lastClass) {
           ParsedParent *parentClass = new ParsedParent;
           parentClass->setName(parent);
           parentClass->setAccess(PIE_PUBLIC);
           lastClass->addParent(parentClass);
         }
         continue;
      } //base
      
      if (isare.match(line) && package) {
         parent = isare.group(1);
         if (parent.isEmpty()) {
             kdDebug(9016) << "empty isa re match"  << basere.group(1) << endl;
             continue;
         }
//       kdDebug(9016) << "ISAre match ["  << basere.group(1) << "]" << endl;
         inClass=true;
          //create class if not already there
         if (!classStore()->hasClass(package)) {
          //create class
          lastClass = new ParsedClass;
          lastClass->setName(package);
          lastClass->setDefinedInFile(fileName);
          lastClass->setDefinedOnLine(lineNo);
         } else {
           kdDebug(9016) << "WARNING : Class ["<< package << "] already exist" << endl;
           lastClass = classStore()->getClassByName(package);
         }
         //add parent if found
         if (lastClass) {
           ParsedParent *parentClass = new ParsedParent;
           parentClass->setName(parent);
           parentClass->setAccess(PIE_PUBLIC);
           lastClass->addParent(parentClass);
//           kdDebug(9016) << "adding new ISA class" << endl;
           classStore()->globalScope()->addClass(lastClass);
           classStore()->addClass(lastClass);
         }
         continue;
      } //isa
      
      if (packagere.match(line)) {
         package=packagere.group(1);
         inClass=false;
         lastClass=0;
         lastsub=0;
         packagelineNo=lineNo;
//         kdDebug(9016) << "packagere match [" << package << "]" << endl;
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
    QString program = project()->projectDirectory() + "/" + project()->mainProgram();
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

#include "perlsupportpart.moc"

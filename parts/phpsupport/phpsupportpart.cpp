/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann,Sandy Meier                      *
 *   bernd@kdevelop.org,smeier@kdevelop.org                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "phpsupportpart.h"
#include "phpsupportfactory.h"
#include "parsedclass.h"
#include "parsedmethod.h"


PHPSupportPart::PHPSupportPart(KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
    setInstance(PHPSupportFactory::instance());
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

}


PHPSupportPart::~PHPSupportPart()
{}


void PHPSupportPart::projectOpened()
{
    kdDebug(9007) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PHPSupportPart::projectClosed()
{
}


void PHPSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    if (fi.extension() == "inc" || fi.extension() == "php" 
	|| fi.extension() == "html"
	|| fi.extension() == "php3") {
        classStore()->removeWithReferences(fileName);
        parse(fileName);
    }
}


void PHPSupportPart::initialParse()
{
    kdDebug(9016) << "initialParse()" << endl;
    
    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
	  //kdDebug(9016) << "maybe parse " << (*it) << endl;
	  maybeParse(*it);
        }
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9016) << "No project" << endl;
    }
}


void PHPSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9016) << "addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void PHPSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9016) << "removedFileFromProject()" << endl;
    classStore()->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void PHPSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9016) << "savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PHPSupportPart::features()
{
        return Features(Classes | Functions);
}


void PHPSupportPart::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
    KRegExp methodre("^[ \t]*function[ \t]*([0-9A-Za-z_]*)[ \t]*\\(([0-9A-Za-z_\\$\\,]*)\\).*$");
    KRegExp varre("^[ \t]*var[ \t]*([0-9A-Za-z_\\$]+)[ \t;=].*$");

    ParsedClass *lastClass = 0;
    QString rawline;
    QCString line;
    int lineNo = 0;
    int bracketOpen = 0;
    int bracketClose = 0;
    bool inClass = false;
    
    while (!stream.eof()) {
        rawline = stream.readLine();
        line = rawline.stripWhiteSpace().latin1();
	bracketOpen += line.contains("{");
	bracketClose += line.contains("}");

	//        kdDebug(9014) << "try match line: " << line << endl;
        if (classre.match(line)) {
	  //kdDebug(9014) << "regex match line: " << line << endl;
	  inClass= true;
	  bracketOpen = line.contains("{");
	  bracketClose = line.contains("}");
	  lastClass = new ParsedClass;
	  lastClass->setName(classre.group(1));
	  lastClass->setDefinedInFile(fileName);
	  lastClass->setDefinedOnLine(lineNo);
	  
	  QString parentStr = classre.group(3);
	  if(parentStr !=""){
	    ParsedParent *parent = new ParsedParent;
	    parent->setName(parentStr);
	    parent->setAccess(PIE_PUBLIC);
	    lastClass->addParent(parent);
	  }
	  
	  if (classStore()->hasClass(lastClass->name)) {
	    ParsedClass *old = classStore()->getClassByName(lastClass->name);
	    old->setDeclaredOnLine(lastClass->declaredOnLine);
	    old->setDeclaredInFile(lastClass->declaredInFile);
	    delete lastClass;
	    lastClass=0;
	  } else {
	    classStore()->addClass(lastClass);
	  }
	  
	  if(bracketOpen == bracketClose && bracketOpen !=0 && bracketClose !=0){
	    inClass = false; // ok we are out ouf class
	  }

	     
        } else if (methodre.match(line)) {
	  //	  kdDebug(9014) << "regex match line ( method ): " << line << endl;
	  ParsedMethod *method = new ParsedMethod;
	  method->setName(methodre.group(1));
	  ParsedArgument* anArg = new ParsedArgument();
	  QString arguments = methodre.group(2);
	  anArg->setType(arguments.stripWhiteSpace().latin1());
	  method->addArgument( anArg );

	  method->setDefinedInFile(fileName);
	  method->setDefinedOnLine(lineNo);
            
	  if (lastClass && inClass) {
	    //	    kdDebug(9014) << "in Class: " << line << endl;
	    ParsedMethod *old = lastClass->getMethod(method);
	    if (!old)
	      lastClass->addMethod(method);
	  } else {
	    ParsedMethod *old = classStore()->globalContainer.getMethod(method);
	    if (!old)
	      classStore()->globalContainer.addMethod(method);
	  }
        }
	else if (varre.match(line)) {
	  kdDebug(9014) << "###########regex match line ( var ): " << varre.group(1) << endl;
	  if (lastClass && inClass) {
	    ParsedAttribute* anAttr = new ParsedAttribute();
	    anAttr->setName(varre.group(1));
	    anAttr->setDefinedInFile(fileName);
	    anAttr->setDefinedOnLine(lineNo);
	    lastClass->addAttribute( anAttr );
	    
	  }
	  
	}
        ++lineNo;
    }
    f.close();
}

#include "phpsupportpart.moc"

/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
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
#include <qvbox.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kregexp.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "classstore.h"
#include "domutil.h"

#include "pythonsupportpart.h"
#include "pythonconfigwidget.h"
#include "parsedclass.h"
#include "parsedmethod.h"

typedef KGenericFactory<PythonSupportPart> PythonSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpythonsupport, PythonSupportFactory( "kdevpythonsupport" ) );

PythonSupportPart::PythonSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(parent, name)
{
    setInstance(PythonSupportFactory::instance());

    setXMLFile("kdevpythonsupport.rc");
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    KAction *action;

    action = new KAction( i18n("Execute program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setStatusText( i18n("Runs the Python program") );
    
    action = new KAction( i18n("Execute string..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setStatusText( i18n("Executes a string as Python code") );

    action = new KAction( i18n("Start Python interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setStatusText( i18n("Starts the Python interpreter without a program") );

    action = new KAction( i18n("Python documentation..."), 0,
                          this, SLOT(slotPydoc()),
                          actionCollection(), "help_pydoc" );
    action->setStatusText( i18n("Show a Python documentation page") );
}


PythonSupportPart::~PythonSupportPart()
{}


void PythonSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Python"));
    PythonConfigWidget *w = new PythonConfigWidget(*projectDom(), vbox, "python config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void PythonSupportPart::projectOpened()
{
    kdDebug(9014) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PythonSupportPart::projectClosed()
{
}


void PythonSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    if (fi.extension() == "py") {
        classStore()->removeWithReferences(fileName);
        parse(fileName);
    }
}


void PythonSupportPart::initialParse()
{
    kdDebug(9014) << "initialParse()" << endl;
    
    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9014) << "maybe parse " << (*it) << endl;
            maybeParse(*it);
        }
        
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9014) << "No project" << endl;
    }
}


void PythonSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9014) << "addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void PythonSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9014) << "removedFileFromProject()" << endl;
    classStore()->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void PythonSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9014) << "savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PythonSupportPart::features()
{
    return Features(Classes | Functions);
}


void PythonSupportPart::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    KRegExp classre("^[ \t]*class[ \t]+([A-Za-z0-9_]+)[ \t]*(\\(([A-Za-z0-9_, \t]+)\\))?.*$");
    KRegExp methodre("^[ \t]*def[ \t]+([A-Za-z0-9_]+).*$");

    ParsedClass *lastClass = 0;
    QString rawline;
    QCString line;
    int lineNo = 0;
    while (!stream.atEnd()) {
        rawline = stream.readLine();
        line = rawline.stripWhiteSpace().latin1();
        if (classre.match(line)) {
            
            lastClass = new ParsedClass;
            lastClass->setName(classre.group(1));
            lastClass->setDefinedInFile(fileName);
            lastClass->setDefinedOnLine(lineNo);

            QStringList parentList = QStringList::split(",", classre.group(3));
            QStringList::ConstIterator it;
            for (it = parentList.begin(); it != parentList.end(); ++it) {
                ParsedParent *parent = new ParsedParent;
                parent->setName((*it).stripWhiteSpace());
                parent->setAccess(PIE_PUBLIC);
                kdDebug(9014) << "Add parent" << parent->name() << endl;
                lastClass->addParent(parent);
            }
             
           if (classStore()->hasClass(lastClass->name())) {
                ParsedClass *old = classStore()->getClassByName(lastClass->name());
                old->setDeclaredOnLine(lastClass->declaredOnLine());
                old->setDeclaredInFile(lastClass->declaredInFile());
                delete lastClass;
                lastClass = old;
            } else {
                kdDebug(9014) << "Add class " << lastClass->name() << endl;
                classStore()->addClass(lastClass);
            }
           
        } else if (methodre.match(line)) {
            
            ParsedMethod *method = new ParsedMethod;
            method->setName(methodre.group(1));
            method->setDefinedInFile(fileName);
            method->setDefinedOnLine(lineNo);
            
            if (lastClass && rawline.left(3) != "def") {
                ParsedMethod *old = lastClass->getMethod(method);
                kdDebug(9014) << "Add class method " << method->name() << endl;
                if (!old)
                    lastClass->addMethod(method);
            } else {
                ParsedMethod *old = classStore()->globalContainer.getMethod(method);
                kdDebug(9014) << "Add global method " << method->name() << endl;
                if (!old)
                    classStore()->globalContainer.addMethod(method);
                lastClass = 0;
            }
            
        }
        ++lineNo;
    }
    
    f.close();
}


QString PythonSupportPart::interpreter()
{
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevpythonsupport/run/interpreter");
    if (prog.isEmpty())
        prog = "python";

    return prog;
}


void PythonSupportPart::startApplication(const QString &program)
{
    QString cmd;
    if (DomUtil::readBoolEntry(*projectDom(), "/kdevpythonsupport/run/terminal")) {
        cmd = "konsole -e /bin/sh -c '";
        cmd += program;
        cmd += "; echo \"\n";
        cmd += i18n("Press Enter to continue!");
        cmd += "\";read'";
    } else {
        cmd = program;
    }

    appFrontend()->startAppCommand(cmd);
}


void PythonSupportPart::slotExecute()
{
    QString program = project()->projectDirectory() + "/" + project()->mainProgram();
    QString cmd = interpreter() + " " + program;
    startApplication(cmd);
}


void PythonSupportPart::slotStartInterpreter()
{
    startApplication(interpreter());
}


void PythonSupportPart::slotExecuteString()
{
    bool ok;
    QString cmd = KLineEditDlg::getText(i18n("String to execute"), QString::null, &ok, 0);
    if (ok) {
        cmd.prepend("'");
        cmd.append("'");
        startApplication(cmd);
    }
}


void PythonSupportPart::slotPydoc()
{
    bool ok;
    QString key = KLineEditDlg::getText(i18n("Show Python documentation on keyword:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "pydoc:";
        url += key;
        partController()->showDocument(KURL(url));
    }
}

#include "pythonsupportpart.moc"

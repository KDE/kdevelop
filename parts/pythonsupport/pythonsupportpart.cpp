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

#include "pythonsupportpart.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kregexp.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "classstore.h"
#include "domutil.h"

#include "pythonconfigwidget.h"
#include "parsedclass.h"
#include "parsedmethod.h"


typedef KGenericFactory<PythonSupportPart> PythonSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpythonsupport, PythonSupportFactory( "kdevpythonsupport" ) );

PythonSupportPart::PythonSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport("PythonSupport", "python", parent, name ? name : "PythonSupportPart")
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

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setStatusText( i18n("Runs the Python program") );
    core()->insertNewAction( action );

    action = new KAction( i18n("Execute String..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setStatusText( i18n("Executes a string as Python code") );
    core()->insertNewAction( action );


    action = new KAction( i18n("Start Python Interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setStatusText( i18n("Starts the Python interpreter without a program") );
    core()->insertNewAction( action );


    action = new KAction( i18n("Python Documentation..."), 0,
                          this, SLOT(slotPydoc()),
                          actionCollection(), "help_pydoc" );
    action->setStatusText( i18n("Show a Python documentation page") );
    core()->insertNewAction( action );
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

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

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
        QStringList files = project()->allFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
        }

        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9014) << "No project" << endl;
    }
}


void PythonSupportPart::addedFilesToProject(const QStringList &fileList)
{
    kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		maybeParse(project()->projectDirectory() + "/" + ( *it ) );
	}

    emit updatedSourceInfo();
}


void PythonSupportPart::removedFilesFromProject(const QStringList &fileList)
{
	kdDebug(9014) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		classStore()->removeWithReferences(project()->projectDirectory() + "/" + ( *it ) );
	}

	emit updatedSourceInfo();
}


void PythonSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9014) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PythonSupportPart::features()
{
    return Features(Classes | Functions);
}


QStringList PythonSupportPart::fileFilters()
{
    QStringList l;
    l << "*.py";
    return l;
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
        line = rawline.stripWhiteSpace().local8Bit();
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
                classStore()->globalScope()->addClass(lastClass);
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
                if( old ){
                    delete( method );
                    method = old;
                } else {
                    lastClass->addMethod(method);
                }
            } else {
                ParsedMethod *old = classStore()->globalScope()->getMethod(method);
                kdDebug(9014) << "Add global method " << method->name() << endl;
                if( old ){
                    delete( method );
                    method = old;
                } else {
                    classStore()->globalScope()->addMethod(method);
                }
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
    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevpythonsupport/run/terminal");
    appFrontend()->startAppCommand(program, inTerminal);
}


void PythonSupportPart::slotExecute()
{
    QString program = project()->mainProgram();
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

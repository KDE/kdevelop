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

#include "customprojectpart.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qvaluestack.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "runoptionswidget.h"
#include "makeoptionswidget.h"
#include "custombuildoptionswidget.h"
#include "config.h"


typedef KGenericFactory<CustomProjectPart> CustomProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcustomproject, CustomProjectFactory( "kdevcustomproject" ) );

CustomProjectPart::CustomProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(CustomProjectFactory::instance());

    setXMLFile("kdevcustomproject.rc");

    KAction *action;

    action = new KAction( i18n("&Build Project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    
    action = new KAction( i18n("&Clean Project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    
    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );

    KActionMenu *menu = new KActionMenu( i18n("Build &Target"),
                                         actionCollection(), "build_target" );
    m_targetMenu = menu->popupMenu();

    connect( m_targetMenu, SIGNAL(aboutToShow()),
             this, SLOT(updateTargetMenu()) );
    connect( m_targetMenu, SIGNAL(activated(int)),
             this, SLOT(targetMenuActivated(int)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


CustomProjectPart::~CustomProjectPart()
{}


void CustomProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Run Options"));
    RunOptionsWidget *w1 = new RunOptionsWidget(*projectDom(), "/kdevcustomproject", vbox);
    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
    vbox = dlg->addVBoxPage(i18n("Build Options"));
    QTabWidget *buildtab = new QTabWidget(vbox);
    
    CustomBuildOptionsWidget *w2 = new CustomBuildOptionsWidget(*projectDom(), buildtab);
    connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
    buildtab->addTab(w2, i18n("Build"));

    MakeOptionsWidget *w3 = new MakeOptionsWidget(*projectDom(), "/kdevcustomproject", buildtab);
    connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
    buildtab->addTab(w3, i18n("Make"));

    w2->setMakeOptionsWidget(buildtab, w3);
}


void CustomProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    QFile f(dirName + "/" + projectName + ".filelist");
    if (f.open(IO_ReadOnly)) {
        QTextStream stream(&f);
        while (!stream.atEnd()) {
            QString s = stream.readLine();
            if (!s.startsWith("#"))
                m_sourceFiles << s;
        }
    } else {
        int r = KMessageBox::questionYesNo(topLevel()->main(),
                                           i18n("This project does not contain any files yet.\n"
                                                "Populate it with all C/C++/Java files below "
                                                "the project directory?"));
        if (r == KMessageBox::Yes)
            populateProject();
    }
}


void CustomProjectPart::populateProject()
{
    QApplication::setOverrideCursor(Qt::waitCursor);
    
    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);
    
    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9025) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9025) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                kdDebug(9025) << "Adding: " << path << endl;
                m_sourceFiles.append(path.mid(prefixlen));
            }
        }
    } while (!s.isEmpty());

    QApplication::restoreOverrideCursor();
}


void CustomProjectPart::closeProject()
{
    QFile f(m_projectDirectory + "/" + m_projectName + ".filelist");
    if (!f.open(IO_WriteOnly))
        return;

    QTextStream stream(&f);
    stream << "# KDevelop Custom Project File List" << endl;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        stream << (*it) << endl;
    f.close();
}


QString CustomProjectPart::projectDirectory()
{
    return m_projectDirectory;
}


QString CustomProjectPart::projectName()
{
    return m_projectName;
}


QString CustomProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram");
}


QString CustomProjectPart::activeDirectory()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevcustomproject/general/activedir");
}


QStringList CustomProjectPart::allSourceFiles()
{
    QStringList res;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        res += (m_projectDirectory + "/" + (*it));

    return res;
}


void CustomProjectPart::addFile(const QString &fileName)
{
    m_sourceFiles.append(fileName);
    kdDebug(9025) << "Emitting addFileToProject" << endl;
    emit addedFileToProject(fileName);
}


void CustomProjectPart::removeFile(const QString &fileName)
{
    m_sourceFiles.remove(fileName);
    kdDebug(9025) << "Emitting removeFileFromProject" << endl;
    emit removedFileFromProject(fileName);
}


QString CustomProjectPart::buildDirectory()
{
    QString dir = DomUtil::readEntry(*projectDom(), "/kdevcustomproject/build/builddir");
    return dir.isEmpty()? projectDirectory() : dir;
}


void CustomProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant";

    if (!ant) {
        QFileInfo fi(dir + "/Makefile");
        if (!fi.exists()) {
            KMessageBox::information(topLevel()->main(),
                                     i18n("There is no Makefile in this directory."));
            return;
        }
    }

    QString cmdline;
    if (ant) {
        cmdline = "ant";
    } else {
        cmdline = DomUtil::readEntry(dom, "/kdevcustomproject/make/makebin");
        if (cmdline.isEmpty())
            cmdline = MAKE_COMMAND;
        if (!DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/abortonerror"))
            cmdline += " -k";
        int jobs = DomUtil::readIntEntry(dom, "/kdevcustomproject/make/numberofjobs");
        if (jobs != 0) {
            cmdline += " -j";
            cmdline += QString::number(jobs);
        }
        if (DomUtil::readBoolEntry(dom, "/kdevcustomproject/make/dontact"))
            cmdline += " -n";
    }

    cmdline += " ";
    cmdline += target;
    
    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void CustomProjectPart::slotBuild()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1(""));
}


void CustomProjectPart::slotClean()
{
    startMakeCommand(buildDirectory(), QString::fromLatin1("clean"));
}


void CustomProjectPart::slotExecute()
{
    QString program = projectDirectory() + "/" + project()->mainProgram();
    
    if (DomUtil::readBoolEntry(*projectDom(), "/kdevcustomproject/run/terminal")) {
        QString terminal = "konsole -e /bin/sh -c '";
        terminal += program;
        terminal += "; echo \"\n";
        terminal += i18n("Press Enter to continue!");
        terminal += "\";read'";
        program = terminal;
    }
    
    DomUtil::PairList envvars = 
        DomUtil::readPairListEntry(*projectDom(), "/kdevcustomproject/envvars", "envvar", "name", "value");

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        environstr += (*it).first;
        environstr += "=";
        environstr += (*it).second;
        environstr += " ";
    }
    program.prepend(environstr);

    appFrontend()->startAppCommand(program);
}


void CustomProjectPart::updateTargetMenu()
{
    m_targets.clear();
    m_targetMenu->clear();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry(dom, "/kdevcustomproject/build/buildtool") == "ant";

    if (ant) {
        QFile f(buildDirectory() + "/build.xml");
        if (!f.open(IO_ReadOnly)) {
            kdDebug(9025) << "No build file" << endl;
            return;
        }
        QDomDocument dom;
        if (!dom.setContent(&f)) {
            kdDebug(9025) << "Build script not valid xml" << endl;
            f.close();
            return;
        }
        f.close();

        QDomNode node = dom.documentElement().firstChild();
        while (!node.isNull()) {
            if (node.toElement().tagName() == "target")
                m_targets.append(node.toElement().attribute("name"));
            node = node.nextSibling();
        }
    } else {
        QFile f(buildDirectory() + "/Makefile");
        if (!f.open(IO_ReadOnly)) {
            kdDebug(9025) << "No Makefile" << endl;
            return;
        }
        QTextStream stream(&f);
        QRegExp re(".PHONY\\s*:(.*)");
        while (!stream.atEnd()) {
            QString str = stream.readLine();
            // Read all continuation lines
            while (str.right(1) == "\\" && !stream.atEnd()) {
                str.remove(str.length()-1, 1);
                str += stream.readLine();
            }
            if (re.search(str) == 0)
                m_targets += QStringList::split(" ", re.cap(1).simplifyWhiteSpace());
        }
        f.close();
    }

    int id = 0;
    QStringList::ConstIterator it;
    for (it = m_targets.begin(); it != m_targets.end(); ++it)
        m_targetMenu->insertItem(*it, id++);
}


void CustomProjectPart::targetMenuActivated(int id)
{
    QString target = m_targets[id];
    startMakeCommand(buildDirectory(), target);
}

#include "customprojectpart.moc"

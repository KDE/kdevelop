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

#include "trollprojectpart.h"

#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "trollprojectwidget.h"
#include "config.h"


typedef KGenericFactory<TrollProjectPart> TrollProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevtrollproject, TrollProjectFactory( "kdevtrollproject" ) );

TrollProjectPart::TrollProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(TrollProjectFactory::instance());

    setXMLFile("kdevtrollproject.rc");

    m_widget = new TrollProjectWidget(this);
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Project"));
    QWhatsThis::add(m_widget, i18n("Project Tree\n\n"
                                   "The project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   ".pro file. The 'details' view in the lower half shows the "
                                   "targets for the active subproject selected in the overview."));

    topLevel()->embedSelectView(m_widget, i18n("Project"));

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

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


TrollProjectPart::~TrollProjectPart()
{
    if (m_widget)
        topLevel()->removeView(m_widget);
    delete m_widget;
}


void TrollProjectPart::projectConfigWidget(KDialogBase */*dlg*/)
{
}


void TrollProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    m_widget->openProject(dirName);
    m_projectName = projectName;
}


void TrollProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString TrollProjectPart::projectDirectory()
{
  return m_widget->subprojectDirectory();
}


QString TrollProjectPart::projectName()
{
    return m_projectName;
}


QString TrollProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevtrollproject/run/mainprogram");
}


QString TrollProjectPart::activeDirectory()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevtrollproject/general/activedir");
}


QStringList TrollProjectPart::allFiles()
{
    return m_widget->allFiles();
}


void TrollProjectPart::addFile(const QString &fileName)
{
  m_widget->addFile(fileName);
}


void TrollProjectPart::removeFile(const QString &fileName)
{
    // FIXME
}


void TrollProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QFileInfo fi(dir + "/Makefile");
    if (!fi.exists()) {
        int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory. Run configure first?"));
        if (r == KMessageBox::No)
            return;
        startQMakeCommand(dir);
    }
    QDomDocument &dom = *projectDom();

    if (target=="clean")
    {
      QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
      if (cmdline.isEmpty())
          cmdline = MAKE_COMMAND;
      cmdline += " clean";
      QString dircmd = "cd ";
      dircmd += dir;
      dircmd += " && ";
      makeFrontend()->queueCommand(dir, dircmd + cmdline);
    }

    QString cmdline = DomUtil::readEntry(dom, "/kdevtrollproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, "/kdevtrollproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevtrollproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;

    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void TrollProjectPart::startQMakeCommand(const QString &dir)
{
    QFileInfo fi(dir);

    QString cmdline = "qmake ";
    cmdline += fi.baseName() + ".pro";

    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void TrollProjectPart::slotBuild()
{
    startMakeCommand(m_widget->projectDirectory(), QString::fromLatin1(""));
}


void TrollProjectPart::slotClean()
{
    startMakeCommand(m_widget->projectDirectory(), QString::fromLatin1("clean"));
}


void TrollProjectPart::slotExecute()
{
    // temporary hack
    QString program = m_widget->projectDirectory() + "/" + m_projectName;

    QDomElement docEl = projectDom()->documentElement();
    QDomElement trollprojectEl = docEl.namedItem("kdevtrollproject").toElement();
    QDomElement envvarsEl = trollprojectEl.namedItem("envvars").toElement();

    QString environstr;
    QDomElement childEl = envvarsEl.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "envvar") {
            environstr += childEl.attribute("name");
            environstr += "=";
            environstr += childEl.attribute("value");
            environstr += " ";
        }
        childEl = childEl.nextSibling().toElement();
    }
    program.prepend(environstr);

    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevtrollproject/run/terminal");
    appFrontend()->startAppCommand(program, inTerminal);
}

void TrollProjectPart::execute(const QString &command)
{
    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevtrollproject/run/terminal");
    appFrontend()->startAppCommand(command, inTerminal);
}


#include "trollprojectpart.moc"

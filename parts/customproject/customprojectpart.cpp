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
#include <qvbox.h>
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
#include "makeoptionswidget.h"
#include "runoptionswidget.h"
#include "customprojectwidget.h"
#include "customprojectpart.h"
#include "config.h"

typedef KGenericFactory<CustomProjectPart> CustomProjectFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcustomproject, CustomProjectFactory( "kdevcustomproject" ) );

CustomProjectPart::CustomProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevProject(parent, name)
{
    setInstance(CustomProjectFactory::instance());

    setXMLFile("kdevcustomproject.rc");

    m_widget = new CustomProjectWidget(this);
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Project"));
    
    topLevel()->embedSelectView(m_widget, i18n("Project"));

    connect( m_widget, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );

    KAction *action;

    action = new KAction( i18n("&Build project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    
    action = new KAction( i18n("&Clean project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    
    action = new KAction( i18n("Execute program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_execute" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


CustomProjectPart::~CustomProjectPart()
{
    topLevel()->removeView(m_widget);
    delete m_widget;
}


void CustomProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Run Options"));
    RunOptionsWidget *w3 = new RunOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
    vbox = dlg->addVBoxPage(i18n("Make Options"));
    MakeOptionsWidget *w4 = new MakeOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w4, SLOT(accept()) );
}


void CustomProjectPart::openProject(const QString &dirName)
{
    m_widget->openProject(dirName);
}


void CustomProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString CustomProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram");
}


QString CustomProjectPart::projectDirectory()
{
    return m_widget->projectDirectory();
}


QStringList CustomProjectPart::allSourceFiles()
{
    return m_widget->allSourceFiles();
}


void CustomProjectPart::slotItemExecuted(QListViewItem *item)
{
    CustomProjectItem *spitem = static_cast<CustomProjectItem*>(item);
    if (spitem->type() == CustomProjectItem::File)
        partController()->editDocument(KURL(spitem->path()));
}


void CustomProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QFileInfo fi(dir + "/Makefile");
    if (!fi.exists()) {
        KMessageBox::information(m_widget, i18n("There is no Makefile in this directory."));
        return;
    }
    QDomDocument &dom = *projectDom();

    QString cmdline = DomUtil::readEntry(dom, "/kdevcustomproject/make/makebin");
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

    cmdline += " ";
    cmdline += target;
    
    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void CustomProjectPart::slotBuild()
{
    startMakeCommand(m_widget->projectDirectory(), QString::fromLatin1(""));
}


void CustomProjectPart::slotClean()
{
    startMakeCommand(m_widget->projectDirectory(), QString::fromLatin1("clean"));
}


void CustomProjectPart::slotExecute()
{
    QString program = m_widget->projectDirectory() + "/" + project()->mainProgram();
    
    if (DomUtil::readBoolEntry(*projectDom(), "/kdevcustomproject/run/terminal")) {
        QString terminal = "konsole -e /bin/sh -c '";
        terminal += program;
        terminal += "; echo \"\n";
        terminal += i18n("Press Enter to continue!");
        terminal += "\";read'";
        program = terminal;
    }
    
    QDomElement docEl = projectDom()->documentElement();
    QDomElement customprojectEl = docEl.namedItem("kdevcustomproject").toElement();
    QDomElement envvarsEl = customprojectEl.namedItem("envvars").toElement();
    
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

    appFrontend()->startAppCommand(program);
}

#include "customprojectpart.moc"

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
#include <qwhatsthis.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kaction.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "compileroptionswidget.h"
#include "makeoptionswidget.h"
#include "runoptionswidget.h"
#include "configureoptionswidget.h"
#include "addtranslationdlg.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"
#include "config.h"

K_EXPORT_COMPONENT_FACTORY( libkdevautoproject, AutoProjectFactory( "kdevautoproject" ) );

AutoProjectPart::AutoProjectPart(QObject *parent, const char *name, const QStringList &args)
    : KDevProject(parent, name)
{
    setInstance(AutoProjectFactory::instance());

    setXMLFile("kdevautoproject.rc");

    bool kde = ( args[ 0 ] == "kde" );

    m_widget = new AutoProjectWidget(this, kde);
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Project"));
    QWhatsThis::add(m_widget, i18n("Project Tree\n\n"
                                   "The project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   "Makefile.am. The 'details' view in the lower half shows the "
                                   "targets for the active subproject selected in the overview."));
    
    topLevel()->embedSelectView(m_widget, i18n("Project"));

    KAction *action;

    action = new KAction( i18n("Add translation..."), 0,
                          this, SLOT(slotAddTranslation()),
                          actionCollection(), "project_addtranslation" );
    if (!kde)
        action->setEnabled(false);
    
    action = new KAction( i18n("&Build project"), "make_kdevelop", Key_F8,
                          this, SLOT(slotBuild()),
                          actionCollection(), "build_build" );
    
    action = new KAction( i18n("Run configure"), 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "build_configure" );

    action = new KAction( i18n("Run automake && friends"), 0,
                          this, SLOT(slotMakefilecvs()),
                          actionCollection(), "build_makefilecvs" );

    action = new KAction( i18n("Install"), 0,
                          this, SLOT(slotInstall()),
                          actionCollection(), "build_install" );
    
    action = new KAction( i18n("&Clean project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "build_clean" );
    
    action = new KAction( i18n("&Distclean"), 0,
                          this, SLOT(slotDistClean()),
                          actionCollection(), "build_distclean" );

    action = new KAction( i18n("Make messages && merge"), 0,
                          this, SLOT(slotMakeMessages()),
                          actionCollection(), "build_messages" );
    if (!kde)
        action->setEnabled(false);

    QDomDocument &dom = *projectDom();
    if(DomUtil::readEntry(dom, "/kdevautoproject/run/default_implementation") != "no"){
      //ok we handle the execute in this kpart
      action = new KAction( i18n("Execute program"), "exec", 0,
			    this, SLOT(slotExecute()),
			    actionCollection(), "build_execute" );
    }

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    

}


AutoProjectPart::~AutoProjectPart()
{
    topLevel()->removeView(m_widget);
    delete m_widget;
}


void AutoProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Compiler Options"));
    CompilerOptionsWidget *w1 = new CompilerOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
    vbox = dlg->addVBoxPage(i18n("Configure Options"));
    ConfigureOptionsWidget *w2 = new ConfigureOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w2, SLOT(accept()) );
    QDomDocument &dom = *projectDom();
    if(DomUtil::readEntry(dom, "/kdevautoproject/run/default_implementation") != "no"){
      //ok we handle the execute in this kpart
      vbox = dlg->addVBoxPage(i18n("Run Options"));
      RunOptionsWidget *w3 = new RunOptionsWidget(this, vbox);
      connect( dlg, SIGNAL(okClicked()), w3, SLOT(accept()) );
    }
    vbox = dlg->addVBoxPage(i18n("Make Options"));
    MakeOptionsWidget *w4 = new MakeOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w4, SLOT(accept()) );
}


void AutoProjectPart::openProject(const QString &dirName)
{
    m_widget->openProject(dirName);
}


void AutoProjectPart::closeProject()
{
    m_widget->closeProject();
}


QString AutoProjectPart::mainProgram()
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram");
}


QString AutoProjectPart::projectDirectory()
{
    return m_widget->projectDirectory();
}


QStringList AutoProjectPart::allSourceFiles()
{
    return m_widget->allSourceFiles();
}


void AutoProjectPart::startMakeCommand(const QString &dir, const QString &target)
{
    partController()->saveAllFiles();

    QFileInfo fi1(dir + "/Makefile");
    if (!fi1.exists()) {
        QFileInfo fi2(m_widget->buildDirectory() + "/configure");
        if (!fi2.exists()) {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory\n"
                                                              "and no configure script for this project.\n"
                                                              "Run automake & friends and configure first?"));
            if (r == KMessageBox::No)
                return;
            slotMakefilecvs();
            slotConfigure();
        } else {
            int r = KMessageBox::questionYesNo(m_widget, i18n("There is no Makefile in this directory. Run configure first?"));
            if (r == KMessageBox::No)
                return;
            slotConfigure();
        }
    }
    QDomDocument &dom = *projectDom();

    QString cmdline = DomUtil::readEntry(dom, "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, "/kdevautoproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, "/kdevautoproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, "/kdevautoproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;
    
    QString dircmd = "cd ";
    dircmd += dir;
    dircmd += " && ";

    makeFrontend()->queueCommand(dir, dircmd + cmdline);
}


void AutoProjectPart::slotBuild()
{
    startMakeCommand(m_widget->buildDirectory(), QString::fromLatin1(""));
}


void AutoProjectPart::slotConfigure()
{
    QDomDocument &dom = *projectDom();
    
    QString cmdline = projectDirectory();
    cmdline += "/configure";
    QString cc = DomUtil::readEntry(dom, "/kdevautoproject/compiler/ccompilerbinary");
    if (!cc.isEmpty())
        cmdline.prepend(QString("CC=%1 ").arg(cc));
    QString cflags = DomUtil::readEntry(dom, "/kdevautoproject/compiler/cflags");
    if (!cflags.isEmpty())
        cmdline.prepend(QString("CFLAGS=%1 ").arg(cflags));
    QString cxx = DomUtil::readEntry(dom, "/kdevautoproject/compiler/cxxcompilerbinary");
    if (!cxx.isEmpty())
        cmdline.prepend(QString("CXX=%1 ").arg(cxx));
    QString cxxflags = DomUtil::readEntry(dom, "/kdevautoproject/compiler/cxxflags");
    if (!cxxflags.isEmpty())
        cmdline.prepend(QString("CXXFLAGS=%1 ").arg(cxxflags));
    QString f77 = DomUtil::readEntry(dom, "/kdevautoproject/compiler/f77compilerbinary");
    if (!f77.isEmpty())
        cmdline.prepend(QString("F77=%1 ").arg(f77));
    QString fflags = DomUtil::readEntry(dom, "/kdevautoproject/compiler/f77flags");
    if (!fflags.isEmpty())
        cmdline.prepend(QString("FFLAGS=%1 ").arg(fflags));

    QString configargs = DomUtil::readEntry(dom, "/kdevautoproject/configure/configargs");
    if (!configargs.isEmpty()) {
	cmdline += " ";
        cmdline += configargs;
    }

    QString dircmd = "cd ";
    dircmd += m_widget->buildDirectory();
    dircmd += " && ";

    makeFrontend()->queueCommand(m_widget->buildDirectory(), dircmd + cmdline);
}


void AutoProjectPart::slotMakefilecvs()
{
    QString cmdline = DomUtil::readEntry(*projectDom(), "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    cmdline += " -f Makefile.cvs";
    
    QFileInfo fi(projectDirectory() + "/Makefile.cvs");
    if (!fi.exists()) {
        fi.setFile(projectDirectory() + "/autogen.sh");
        if (!fi.exists()) {
            KMessageBox::sorry(m_widget, i18n("There is neither a Makefile.cvs file nor an "
                                              "autogen.sh script in the project directory"));
            return;
            cmdline = "autogen.sh";
        }
    }

    QString dircmd = "cd ";
    dircmd += projectDirectory();
    dircmd += " && ";

    makeFrontend()->queueCommand(projectDirectory(), dircmd + cmdline);
}


void AutoProjectPart::slotInstall()
{
    startMakeCommand(m_widget->buildDirectory(), QString::fromLatin1("install"));
}


void AutoProjectPart::slotClean()
{
    startMakeCommand(m_widget->buildDirectory(), QString::fromLatin1("clean"));
}


void AutoProjectPart::slotDistClean()
{
    startMakeCommand(m_widget->buildDirectory(), QString::fromLatin1("distclean"));
}


void AutoProjectPart::slotMakeMessages()
{
    startMakeCommand(m_widget->buildDirectory(), QString::fromLatin1("package-messages"));
}


void AutoProjectPart::slotExecute()
{
    QString program = m_widget->buildDirectory() + "/" + project()->mainProgram();
    
    if (DomUtil::readBoolEntry(*projectDom(), "/kdevautoproject/run/terminal")) {
        QString terminal = "konsole -e /bin/sh -c '";
        terminal += program;
        terminal += "; echo \"\n";
        terminal += i18n("Press Enter to continue!");
        terminal += "\";read'";
        program = terminal;
    }
    
    QDomElement docEl = projectDom()->documentElement();
    QDomElement autoprojectEl = docEl.namedItem("kdevautoproject").toElement();
    QDomElement envvarsEl = autoprojectEl.namedItem("envvars").toElement();
    
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


void AutoProjectPart::slotAddTranslation()
{
    AddTranslationDialog dlg(this, m_widget);
    dlg.exec();
}

#include "autoprojectpart.moc"

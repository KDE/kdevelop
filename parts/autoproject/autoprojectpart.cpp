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

#include <qwhatsthis.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "autoprojectfactory.h"
#include "autoprojectwidget.h"
#include "projectoptionswidget.h"
#include "autoprojectpart.h"


AutoProjectPart::AutoProjectPart(KDevApi *api, QObject *parent, const char *name)
    : KDevProject(api, parent, name)
{
    setInstance(AutoProjectFactory::instance());

    setXMLFile("kdevautoproject.rc");

    m_widget = new AutoProjectWidget(this);
    m_widget->setIcon(SmallIcon("make"));
    m_widget->setCaption(i18n("Project"));
    QWhatsThis::add(m_widget, i18n("Project Tree\n\n"
                                   "The project tree consists of two parts. The 'overview' "
                                   "in the upper half shows the subprojects, each one having a "
                                   "Makefile.am. The 'details' view in the lower half shows the "
                                   "targets for the active subproject selected in the overview."));
    
    core()->embedWidget(m_widget, KDevCore::SelectView, i18n("Project"));

    KAction *action;

    action = new KAction( i18n("&Build project"), Key_F9,
                          this, SLOT(slotBuild()),
                          actionCollection(), "project_build" );
    
    action = new KAction( i18n("&Clean project"), 0,
                          this, SLOT(slotClean()),
                          actionCollection(), "project_clean" );
    
    action = new KAction( i18n("Run configure"), 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "project_configure" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


AutoProjectPart::~AutoProjectPart()
{
    delete m_widget;
}


void AutoProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Compiler Options"));
    ProjectOptionsWidget *w = new ProjectOptionsWidget(this, vbox, "documentation tree config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
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
    QDomDocument &doc = *document();

    return DomUtil::readEntry(doc, "/kdevautoproject/general/mainprogram");
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
    // Do something smarter here...
    if (makeFrontend()->isRunning()) {
        KMessageBox::sorry(0, i18n("There is currently a job running."));
        return;
    }

    core()->saveAllFiles();

    QDomDocument &doc = *document();

    QString cmdline = DomUtil::readEntry(doc, "/kdevautoproject/make/makebin");
    if (cmdline.isEmpty())
        cmdline = "make";
    if (!DomUtil::readBoolEntry(doc, "/kdevautoproject/make/abortonerror"))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(doc, "/kdevautoproject/make/numberofjobs");
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(doc, "/kdevautoproject/make/dontact"))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;
    
    makeFrontend()->startMakeCommand(dir, cmdline);
}


void AutoProjectPart::slotBuild()
{
    startMakeCommand(projectDirectory(), QString::fromLatin1(""));
}


void AutoProjectPart::slotClean()
{
    startMakeCommand(projectDirectory(), QString::fromLatin1("clean"));
}


void AutoProjectPart::slotConfigure()
{
    // Do something smarter here...
    if (makeFrontend()->isRunning()) {
        KMessageBox::sorry(0, i18n("There is currently a job running."));
        return;
    }

    QDomDocument &doc = *document();
    
    QString cmdline = "./configure";
    QString cc = DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompilerbinary");
    if (!cc.isEmpty())
        cmdline.prepend(QString("CC=%1 ").arg(cc));
    QString cflags = DomUtil::readEntry(doc, "/kdevautoproject/compiler/cflags");
    if (!cflags.isEmpty())
        cmdline.prepend(QString("CFLAGS=%1 ").arg(cflags));
    QString cxx = DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxcompilerbinary");
    if (!cxx.isEmpty())
        cmdline.prepend(QString("CXX=%1 ").arg(cxx));
    QString cxxflags = DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxflags");
    if (!cxxflags.isEmpty())
        cmdline.prepend(QString("CXXFLAGS=%1 ").arg(cxxflags));
    QString f77 = DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77compilerbinary");
    if (!f77.isEmpty())
        cmdline.prepend(QString("F77=%1 ").arg(f77));
    QString fflags = DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77flags");
    if (!fflags.isEmpty())
        cmdline.prepend(QString("FFLAGS=%1 ").arg(fflags));

    QString configargs = DomUtil::readEntry(doc, "/kdevautoproject/configure/configargs");
    if (!configargs.isEmpty()) {
	cmdline += " ";
        cmdline += configargs;
    }

    makeFrontend()->startMakeCommand(projectDirectory(), cmdline);
}

#include "autoprojectpart.moc"

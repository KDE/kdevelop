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

#include <qdir.h>
#include <qwidget.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdevcore.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kaction.h>

#include "importdlg.h"
#include "appwizarddlg.h"
#include "appwizardpart.h"
#include "appwizardfactory.h"
#include <kdevmakefrontend.h>
#include <kdevpartcontroller.h>

AppWizardPart::AppWizardPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name)
{
    setInstance(AppWizardFactory::instance());
    setXMLFile("kdevappwizard.rc");

    KAction *action;
    
    action = new KAction( i18n("&New project..."), "window_new", 0,
                          this, SLOT(slotNewProject()),
                          actionCollection(), "project_new" );
    action->setStatusText( i18n("Generates a new project from a template") );
    action->setWhatsThis( i18n("New project\n\n"
                               "This starts KDevelop's application wizard. "
                               "It helps you to generate a skeleton for your "
                               "application from a set of templates.") );

    action = new KAction( i18n("&Import existing directory..."),"wizard", 0,
                          this, SLOT(slotImportProject()),
                          actionCollection(), "project_import" );
    action->setStatusText( i18n("Creates a project file for a given directory.") );
    
}


AppWizardPart::~AppWizardPart()
{
}


void AppWizardPart::slotNewProject()
{
    kdDebug(9010) << "new project" << endl;
    AppWizardDialog dlg(this, 0, "app wizard");
    connect(makeFrontend(),SIGNAL(commandFinished(QString)),this,SLOT(slotCommandFinished(QString)));
    if(dlg.exec() == QDialog::Accepted){
      m_creationCommand = dlg.getCommandLine();
      m_projectFileName = dlg.getProjectLocation() +"/" + dlg.getProjectName() + ".kdevelop";
      m_showFileAfterGeneration = dlg.getShowFileAfterGeneration();
    }else{
      disconnect(makeFrontend(),0,this,0);
    }
}


void AppWizardPart::slotImportProject()
{
    ImportDialog dlg(this, 0, "import dialog");
    dlg.exec();
}

void AppWizardPart::slotCommandFinished(QString command){
  if(m_creationCommand == command){
    // load the created project and maybe the first file (README...)
    core()->openProject(m_projectFileName);  // opens the project
    if(m_showFileAfterGeneration != ""){
      KURL u;
      u.setPath(m_showFileAfterGeneration);
      partController()->editDocument(u);
    }
    disconnect(makeFrontend(),0,this,0);
  }
}
#include "appwizardpart.moc"

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
#include <qmessagebox.h>

#include "importdlg.h"
#include "appwizarddlg.h"
#include "appwizardpart.h"
#include "appwizardfactory.h"
#include <kdevmakefrontend.h>
#include <kdevpartcontroller.h>
#include <kdevlanguagesupport.h>
#include <codemodel.h>

AppWizardPart::AppWizardPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("AppWizard", "appwizard", parent, name ? name : "AppWizardPart")
{
    setInstance(AppWizardFactory::instance());
    setXMLFile("kdevappwizard.rc");

    KAction *action;

    action = new KAction( i18n("&New Project..."), "window_new", 0,
                          this, SLOT(slotNewProject()),
                          actionCollection(), "project_new" );
    action->setToolTip( i18n("Generate a new project from a template") );
    action->setWhatsThis( i18n("<b>New project</b><p>"
                               "This starts KDevelop's application wizard. "
                               "It helps you to generate a skeleton for your "
                               "application from a set of templates.") );

    action = new KAction( i18n("&Import Existing Project..."),"wizard", 0,
                          this, SLOT(slotImportProject()),
                          actionCollection(), "project_import" );
    action->setToolTip( i18n("Import existing project") );
    action->setWhatsThis( i18n("<b>Import existing project</b><p>Creates a project file for a given directory.") );
}


AppWizardPart::~AppWizardPart()
{
}


void AppWizardPart::slotNewProject()
{
    kdDebug(9010) << "new project" << endl;
    AppWizardDialog dlg(this, 0, "app wizard");
    connect( makeFrontend(), SIGNAL(commandFinished(const QString&)),
             this, SLOT(slotCommandFinished(const QString&)) );
    dlg.templates_listview->setFocus();
    if (dlg.exec()) {
        m_creationCommand = dlg.getCommandLine();
        m_projectLocation = dlg.getProjectLocation() + "/";
        m_projectFileName = m_projectLocation + dlg.getProjectName().lower() + ".kdevelop";
        m_openFilesAfterGeneration = dlg.getFilesToOpenAfterGeneration();
    } else {
      disconnect(makeFrontend(), 0, this, 0);
    }
}


void AppWizardPart::slotImportProject()
{
    ImportDialog dlg(this, 0, "import dialog");
    dlg.exec();
}

void AppWizardPart::openSpecifiedFiles()
{
    for ( QStringList::Iterator it = m_openFilesAfterGeneration.begin();
          it != m_openFilesAfterGeneration.end(); ++it ) {
        if ( !(*it).isNull() ) {
            KURL url(m_projectLocation + *it);
            kdDebug(9010) << "Try to open: " << url.url() << endl;
            partController()->editDocument(url);
        }
    }
}

void AppWizardPart::slotCommandFinished(const QString &command)
{
    if (m_creationCommand == command){
        // load the created project and maybe the first file (README...)
        core()->openProject(m_projectFileName);  // opens the project

        openSpecifiedFiles();

        disconnect(makeFrontend(), 0, this, 0);
    }
}

#include "appwizardpart.moc"

/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDir>
#include <QWidget>
#include <Q3Wizard>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdevcore.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kmacroexpander.h>

#include "appwizardpart.h"
#include "appwizarddlg.h"
#include "appwizardfactory.h"
#include <kdevmakefrontend.h>
#include <kdevdocumentcontroller.h>
#include <kdevlanguagesupport.h>

AppWizardPart::AppWizardPart(QObject *parent, const char */*name*/, const QStringList &)
    : KDevPlugin(AppWizardFactory::info(), parent)
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

    /* Not ported yet
    action = new KAction( i18n("&Import Existing Project..."),"wizard", 0,
                          this, SLOT(slotImportProject()),
                          actionCollection(), "project_import" );
    action->setToolTip( i18n("Import existing project") );
    action->setWhatsThis( i18n("<b>Import existing project</b><p>Creates a project file for a given directory.") );
    */
}


AppWizardPart::~AppWizardPart()
{
}


void AppWizardPart::slotNewProject()
{
    kDebug(9010) << "new project" << endl;
    AppWizardDialog dlg(this, 0, "app wizard");
    dlg.templates_listview->setFocus();
    dlg.exec();
}


void AppWizardPart::slotImportProject()
{
    /* Not ported yet
    ImportDialog dlg(this, 0, "import dialog");
    dlg.exec();
    */
}

#include "appwizardpart.moc"

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

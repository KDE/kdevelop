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

#include "appwizarddlg.h"
#include "appwizardfactory.h"
#include "appwizardpart.h"


AppWizardPart::AppWizardPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
    setInstance(AppWizardFactory::instance());
    setXMLFile("kdevappwizard.rc");

#if 0
    KAction *action;
    
    action = new KAction( i18n("&New project..."), "wizard", 0,
                          this, SLOT(slotNewProject()),
                          actionCollection(), "project_new" );
    action->setStatusText( i18n("Generates a new project from a template") );
    action->setWhatsThis( i18n("New project\n\n"
                               "This starts KDevelop's application wizard. "
                               "It helps you to generate a skeleton for your "
                               "application from a set of templates.") );
#else
    (void) new KAction( i18n("&New project..."), "window_new",0,
                        this, SLOT(slotNewProject()),
                        actionCollection(), "project_new" );
#endif

    m_dialog = 0;
}


AppWizardPart::~AppWizardPart()
{
    delete m_dialog;
}


void AppWizardPart::slotNewProject()
{
    kdDebug(9010) << "new project" << endl;
    if (!m_dialog)
        m_dialog = new AppWizardDialog(this, 0, "app wizard");

    m_dialog->show();
}

#include "appwizardpart.moc"

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
#include "appwizardpart.h"


AppWizardPart::AppWizardPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
    setXMLFile("kdevappwizard.rc");

    (void) new KAction( i18n("&New project..."), 0,
                        this, SLOT(slotNewProject()),
                        actionCollection(), "project_new" );

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

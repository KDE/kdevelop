/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "globalcvs.h"

#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include "cvsform.h"
#include "kdevmakefrontend.h"

typedef KGenericFactory<GlobalCvs> GlobalCvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevglobalcvs, GlobalCvsFactory( "kdevglobalcvs" ) );

GlobalCvs::GlobalCvs(QObject *parent, const char *name, const QStringList &)
        : KDevGlobalVersionControl("CVS",parent, name) {

    setInstance(GlobalCvsFactory::instance());
    setXMLFile("kdevglobalcvs.rc");

/*    KAction * action = new KAction( i18n("Import Cvs Repository..."),"wizard", 0,
                                    this, SLOT(slotImportCvs()),
                                    actionCollection(), "cvs_import" );
    action->setStatusText( i18n("Imports an existing Cvs repository.") ); */
}


GlobalCvs::~GlobalCvs() {}

void GlobalCvs::slotImportCvs() {}

QWidget* GlobalCvs::newProjectWidget(QWidget *parent) {
    form = new CvsForm(parent);
    return form;
}

void GlobalCvs::createNewProject(const QString& dir) {
    if (!form)
        return;
    QString init("");
    if (form->init_check->isChecked())
    	init = "cvs -d " + form->root_edit->text() + " init && ";
    QString command = init + "cd "+ dir +" && cvs -d " + form->root_edit->text() + " import -m '" + form->message_edit->text() + "' "
                      + form->repository_edit->text() + " " + form->vendor_edit->text() + " " + form->release_edit->text();
    makeFrontend()->queueCommand(dir,command);
}

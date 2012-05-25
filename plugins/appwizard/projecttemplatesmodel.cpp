/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "projecttemplatesmodel.h"

#include <QFileInfo>

#ifdef Q_WS_WIN
#include <kzip.h>
#else
#include <ktar.h>
#endif //Q_WS_WIN
#include <kurl.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "appwizardplugin.h"
#include <KIcon>

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPlugin *parent)
    :KDevelop::TemplatesModel(parent)
{
    setDescriptionResourceType("apptemplate_descriptions");
    setTemplateResourceType("apptemplates");
}

void ProjectTemplatesModel::refresh()
{
    KDevelop::TemplatesModel::refresh();
    setHorizontalHeaderLabels(QStringList() << i18n("Project Templates"));    
}

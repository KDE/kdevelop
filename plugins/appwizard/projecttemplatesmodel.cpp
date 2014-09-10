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
#include <QDir>

#include <kdebug.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <kcomponentdata.h>

#include "appwizardplugin.h"

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPlugin *parent)
: KDevelop::TemplatesModel("kdevappwizard", parent)
{
    refresh();
}

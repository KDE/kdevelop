/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projecttemplatesmodel.h"
#include "appwizardplugin.h"

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPlugin *parent)
: KDevelop::TemplatesModel(QStringLiteral("kdevappwizard"), parent)
{
    refresh();
}

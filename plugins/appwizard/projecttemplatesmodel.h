/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _PROJECTTEMPLATESMODEL_H_
#define _PROJECTTEMPLATESMODEL_H_

#include "language/codegen/templatesmodel.h"

class AppWizardPlugin;

class ProjectTemplatesModel: public KDevelop::TemplatesModel {
    Q_OBJECT
public:
    explicit ProjectTemplatesModel(AppWizardPlugin *parent);
};

#endif


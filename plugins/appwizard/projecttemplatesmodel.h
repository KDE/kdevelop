/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _PROJECTTEMPLATESMODEL_H_
#define _PROJECTTEMPLATESMODEL_H_

#include "language/codegen/templatesmodel.h"

#include <QMap>
#include <QStandardItemModel>

class AppWizardPlugin;

class ProjectTemplatesModel: public KDevelop::TemplatesModel {
public:
    ProjectTemplatesModel(AppWizardPlugin *parent);
};

#endif


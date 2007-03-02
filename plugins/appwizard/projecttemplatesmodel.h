/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _PROJECTTEMPLATESMODEL_H_
#define _PROJECTTEMPLATESMODEL_H_

#include <QMap>
#include <QStandardItemModel>

class AppWizardPart;

class ProjectTemplatesModel: public QStandardItemModel {
public:
    ProjectTemplatesModel(AppWizardPart *parent);

    void refresh();

private:
    void extractTemplateDescriptions();
    QStandardItem *createItem(const QString &name, const QString &category);

    AppWizardPart *m_part;

    QMap<QString, QStandardItem*> m_templateItems;
};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

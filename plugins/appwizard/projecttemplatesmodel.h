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

#include <QMap>
#include <QStandardItemModel>

class AppWizardPlugin;

class ProjectTemplatesModel: public QStandardItemModel {
public:
    ProjectTemplatesModel(AppWizardPlugin *parent);

    void refresh();
    QModelIndexList templateIndexes(const QString& fileName);

    AppWizardPlugin* plugin() { return m_plugin; }

private:
    void extractTemplateDescriptions();
    bool templateExists(const QString&);
    QStandardItem *createItem(const QString &name, const QString &category);

    AppWizardPlugin *m_plugin;

    QMap<QString, QStandardItem*> m_templateItems;
};

#endif


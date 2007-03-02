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
#include "projecttemplatesmodel.h"

#include <QFileInfo>
#include <QStandardItem>

#include <ktar.h>
#include <kurl.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "appwizardpart.h"

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPart *parent)
    :QStandardItemModel(parent), m_part(parent)
{
    m_templateItems[""] = invisibleRootItem();
}

void ProjectTemplatesModel::refresh()
{
    clear();
    extractTemplateDescriptions();

    KStandardDirs *dirs = m_part->componentData().dirs();
    QStringList templateDescriptions = dirs->findAllResources("apptemplate_descriptions");
    foreach (QString templateDescription, templateDescriptions)
    {
        KConfig templateConfig(templateDescription);
        KConfigGroup general(&templateConfig, "General");
        QString name = general.readEntry("Name");
        QString category = general.readEntry("Category");

        QStandardItem *templateItem = createItem(name, category);
        templateItem->setData(templateDescription);
    }
}

QStandardItem *ProjectTemplatesModel::createItem(const QString &name, const QString &category)
{
    QStringList path = category.split("/");

    QStandardItem *parent = invisibleRootItem();
    QStringList currentPath;
    foreach (QString entry, path)
    {
        currentPath << entry;
        if (!m_templateItems.contains(currentPath.join("/")))
        {
            QStandardItem *item = new QStandardItem(entry);
            parent->appendRow(item);
            m_templateItems[currentPath.join("/")] = item;
            parent = item;
        }
        else
            parent = m_templateItems[currentPath.join("/")];
    }

    QStandardItem *templateItem = new QStandardItem(name);
    parent->appendRow(templateItem);
    return templateItem;
}

void ProjectTemplatesModel::extractTemplateDescriptions()
{
    KStandardDirs *dirs = m_part->componentData().dirs();
    QStringList templateArchives = dirs->findAllResources("apptemplates");

    QString localDescriptionsDir = dirs->saveLocation("apptemplate_descriptions");

    foreach (QString archName, templateArchives)
    {
        kDebug(9010) << "processing template " << archName << endl;
        KTar templateArchive(archName, "application/x-bzip2");
        if (templateArchive.open(QIODevice::ReadOnly))
        {
            QFileInfo templateInfo(archName);
            const KArchiveEntry *templateEntry =
                templateArchive.directory()->entry(templateInfo.baseName() + ".kdevtemplate");
            if (!templateEntry || !templateEntry->isFile())
            {
                kDebug(9010) << "template " << archName << " does not contain .kdevtemplate file" << endl;
                continue;
            }
            const KArchiveFile *templateFile = (KArchiveFile*)templateEntry;

            kDebug(9010) << "copy template description to " << localDescriptionsDir << endl;
            templateFile->copyTo(localDescriptionsDir);
        }
        else
            kDebug(9010) << "could not open template " << archName << endl;
    }
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

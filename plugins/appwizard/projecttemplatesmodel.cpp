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

#include "appwizardpart.h"
#include "projecttemplateitem.h"

ProjectTemplatesModel::ProjectTemplatesModel(AppWizardPart *parent)
    :QStandardItemModel(parent), m_part(parent)
{
}

void ProjectTemplatesModel::refresh()
{
    clear();
    m_templateItems.clear();
    m_templateItems[""] = invisibleRootItem();
    extractTemplateDescriptions();

    KStandardDirs *dirs = m_part->componentData().dirs();
    QStringList templateDescriptions = dirs->findAllResources("apptemplate_descriptions");
    foreach (QString templateDescription, templateDescriptions)
    {
        KConfig templateConfig(templateDescription);
        KConfigGroup general(&templateConfig, "General");
        QString name = general.readEntry("Name");
        QString category = general.readEntry("Category");

        ProjectTemplateItem *templateItem = createItem(name, category);
        templateItem->setData(templateDescription);
    }
    setHorizontalHeaderLabels(QStringList() << i18n("Project Templates"));
}

ProjectTemplateItem *ProjectTemplatesModel::createItem(const QString &name, const QString &category)
{
    QStringList path = category.split("/");

    QStandardItem *parent = invisibleRootItem();
    QStringList currentPath;
    foreach (QString entry, path)
    {
        currentPath << entry;
        if (!m_templateItems.contains(currentPath.join("/")))
        {
            ProjectTemplateItem *item = new ProjectTemplateItem(entry);
            parent->appendRow(item);
            m_templateItems[currentPath.join("/")] = item;
            parent = item;
        }
        else
            parent = m_templateItems[currentPath.join("/")];
    }

    ProjectTemplateItem *templateItem = new ProjectTemplateItem(name);
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
        kDebug(9010) << "processing template" << archName;
#ifdef Q_WS_WIN
	KZip templateArchive(archName);
#else
        KTar templateArchive(archName, "application/x-bzip");
#endif //Q_WS_WIN
        if (templateArchive.open(QIODevice::ReadOnly))
        {
            QFileInfo templateInfo(archName);
            const KArchiveEntry *templateEntry =
                templateArchive.directory()->entry(templateInfo.baseName() + ".kdevtemplate");
            if (!templateEntry || !templateEntry->isFile())
            {
                kDebug(9010) << "template" << archName << "does not contain .kdevtemplate file";
                continue;
            }
            const KArchiveFile *templateFile = (KArchiveFile*)templateEntry;

            kDebug(9010) << "copy template description to" << localDescriptionsDir;
            templateFile->copyTo(localDescriptionsDir);
        }
        else
            kDebug(9010) << "could not open template" << archName;
    }
}


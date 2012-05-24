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
    :QStandardItemModel(parent), m_plugin(parent)
{
}

bool ProjectTemplatesModel::templateExists( const QString& descname )
{
    QFileInfo fi(descname);
    foreach( const QString& templatename, m_plugin->componentData().dirs()->findAllResources("apptemplates") )
    {
        if( QFileInfo(templatename).baseName() == fi.baseName() ) {
            return true;
        }
    }
    return false;
}

void ProjectTemplatesModel::refresh()
{
    clear();
    m_templateItems.clear();
    m_templateItems[""] = invisibleRootItem();
    extractTemplateDescriptions();

    KStandardDirs *dirs = m_plugin->componentData().dirs();
    const QStringList templateDescriptions = dirs->findAllResources("apptemplate_descriptions");
    foreach (const QString &templateDescription, templateDescriptions)
    {
        if( templateExists( templateDescription ) ) {

            KConfig templateConfig(templateDescription);
            KConfigGroup general(&templateConfig, "General");
            QString name = general.readEntry("Name");
            QString category = general.readEntry("Category");
            QString icon = general.readEntry("Icon");
            QString comment = general.readEntry("Comment");
    
            QStandardItem *templateItem = createItem(name, category);
            templateItem->setData(templateDescription);
            templateItem->setData(icon, Qt::UserRole+2);
            templateItem->setData(comment, Qt::UserRole+3);
        } else {
            // Template file doesn't exist anymore, so remove the description
            // saves us the extra lookups for templateExists on the next run
            QFile(templateDescription).remove();
        }
    }
    setHorizontalHeaderLabels(QStringList() << i18n("Project Templates"));
}

QStandardItem *ProjectTemplatesModel::createItem(const QString& name, const QString& category)
{
    QStringList path = category.split('/');

    QStandardItem *parent = invisibleRootItem();
    QStringList currentPath;
    foreach (const QString& entry, path)
    {
        currentPath << entry;
        if (!m_templateItems.contains(currentPath.join("/")))
        {
            QStandardItem *item = new QStandardItem(entry);
            item->setEditable(false);
            parent->appendRow(item);
            m_templateItems[currentPath.join("/")] = item;
            parent = item;
        }
        else
            parent = m_templateItems[currentPath.join("/")];
    }

    QStandardItem *templateItem = new QStandardItem(name);
    templateItem->setEditable(false);
    parent->appendRow(templateItem);
    return templateItem;
}

void ProjectTemplatesModel::extractTemplateDescriptions()
{
    KStandardDirs *dirs = m_plugin->componentData().dirs();
    QStringList templateArchives = dirs->findAllResources("apptemplates");

    QString localDescriptionsDir = dirs->saveLocation("apptemplate_descriptions");

    foreach (const QString &archName, templateArchives)
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

QModelIndexList ProjectTemplatesModel::templateIndexes(const QString& fileName)
{
    QFileInfo info(fileName);
    QString description = m_plugin->componentData().dirs()->findResource("apptemplate_descriptions", info.baseName() + ".kdevtemplate");

    QModelIndexList indexes;

    if (!description.isEmpty())
    {
        KConfig templateConfig(description);
        KConfigGroup general(&templateConfig, "General");
        QStringList categories = general.readEntry("Category").split('/');

        QStringList levels;
        foreach (const QString& category, categories)
        {
            levels << category;
            indexes << m_templateItems[levels.join(QString('/'))]->index();
        }

        if (!indexes.isEmpty())
        {
            QString name = general.readEntry("Name");
            QStandardItem* categoryItem = m_templateItems[levels.join(QString('/'))];
            for (int i = 0; i < categoryItem->rowCount(); ++i)
            {
                QStandardItem* templateItem = categoryItem->child(i);
                if (templateItem->text() == name)
                {
                    indexes << templateItem->index();
                    break;
                }
            }
        }
    }

    return indexes;
}



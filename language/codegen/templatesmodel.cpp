/*  This file is part of KDevelop
    Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "templatesmodel.h"

#include <KTar>
#include <KZip>
#include <KStandardDirs>
#include <KComponentData>
#include <KDebug>
#include <KConfigGroup>
#include <KLocale>
#include <KMimeType>

#include <QFileInfo>
#include <QDir>

using namespace KDevelop;

class KDevelop::TemplatesModelPrivate
{
public:
    TemplatesModelPrivate(const KComponentData& componentData);

    QByteArray descriptionResourceType;
    QByteArray templateResourceType;
    QMap<QString, QStandardItem*> templateItems;
    KComponentData componentData;
};

TemplatesModelPrivate::TemplatesModelPrivate(const KComponentData& componentData)
: componentData(componentData)
{

}


TemplatesModel::TemplatesModel(const KComponentData& componentData, QObject* parent)
: QStandardItemModel(parent)
, d(new TemplatesModelPrivate(componentData))
{

}

TemplatesModel::~TemplatesModel()
{
    delete d;
}


bool TemplatesModel::templateExists( const QString& descriptionName )
{
    QFileInfo fi(descriptionName);
    foreach( const QString& templatename, d->componentData.dirs()->findAllResources(d->templateResourceType) )
    {
        if( QFileInfo(templatename).baseName() == fi.baseName() ) {
            return true;
        }
    }
    return false;
}

void TemplatesModel::refresh()
{
    clear();
    d->templateItems.clear();
    d->templateItems[""] = invisibleRootItem();
    extractTemplateDescriptions();

    KStandardDirs *dirs = d->componentData.dirs();
    const QStringList templateDescriptions = dirs->findAllResources(d->descriptionResourceType);
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
            templateItem->setData(templateDescription, DescriptionFileRole);
            templateItem->setData(icon, IconNameRole);
            templateItem->setData(comment, CommentRole);
        } else {
            // Template file doesn't exist anymore, so remove the description
            // saves us the extra lookups for templateExists on the next run
            QFile(templateDescription).remove();
        }
    }
}

QStandardItem *TemplatesModel::createItem(const QString& name, const QString& category)
{
    QStringList path = category.split('/');

    QStandardItem *parent = invisibleRootItem();
    QStringList currentPath;
    foreach (const QString& entry, path)
    {
        currentPath << entry;
        if (!d->templateItems.contains(currentPath.join("/"))) {
            QStandardItem *item = new QStandardItem(entry);
            item->setEditable(false);
            parent->appendRow(item);
            d->templateItems[currentPath.join("/")] = item;
            parent = item;
        } else {
            parent = d->templateItems[currentPath.join("/")];
        }
    }

    QStandardItem *templateItem = new QStandardItem(name);
    templateItem->setEditable(false);
    parent->appendRow(templateItem);
    return templateItem;
}

void TemplatesModel::extractTemplateDescriptions()
{
    KStandardDirs *dirs = d->componentData.dirs();
    QStringList templateArchives = dirs->findAllResources(d->templateResourceType);

    kDebug() << d->descriptionResourceType;
    QString localDescriptionsDir = dirs->saveLocation(d->descriptionResourceType);

    foreach (const QString &archName, templateArchives)
    {
        kDebug() << "processing template" << archName;

        KArchive* templateArchive;
        if (QFileInfo(archName).completeSuffix() == "zip")
        {
            templateArchive = new KZip(archName);
        }
        else
        {
            templateArchive = new KTar(archName);
        }

        if (templateArchive->open(QIODevice::ReadOnly))
        {
            /*
             * This class looks for template description files in the following order
             *
             * - "basename.kdevtemplate"
             * - "*.kdevtemplate"
             * - "basename.desktop"
             * - "*.desktop"
             *
             * This is done because application templates can contain .desktop files used by the application
             * so the kdevtemplate suffix must have priority.
             */
            QFileInfo templateInfo(archName);
            const KArchiveEntry *templateEntry =
                templateArchive->directory()->entry(templateInfo.baseName() + ".kdevtemplate");

            if (!templateEntry || !templateEntry->isFile())
            {
                /*
                 * First, if the .kdevtemplate file is not found by name,
                 * we check all the files in the archive for any .kdevtemplate file
                 *
                 * This is needed because kde-files.org renames downloaded files
                 */
                foreach (const QString& entryName, templateArchive->directory()->entries())
                {
                    if (entryName.endsWith(".kdevtemplate"))
                    {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                templateEntry = templateArchive->directory()->entry(templateInfo.baseName() + ".desktop");
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                foreach (const QString& entryName, templateArchive->directory()->entries())
                {
                    if (entryName.endsWith(".desktop"))
                    {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }
            if (!templateEntry || !templateEntry->isFile())
            {
                kDebug() << "template" << archName << "does not contain .kdevtemplate or .desktop file";
                continue;
            }
            const KArchiveFile *templateFile = (KArchiveFile*)templateEntry;

            kDebug() << "copy template description to" << localDescriptionsDir;
            templateFile->copyTo(localDescriptionsDir);

            /*
             * Rename the extracted description
             * so that its basename matches the basename of the template archive
             */
            QFileInfo descriptionInfo(localDescriptionsDir + templateEntry->name());
            QFile::rename(
                descriptionInfo.absoluteFilePath(),
                localDescriptionsDir + templateInfo.baseName() + '.' + descriptionInfo.suffix()
            );
        }
        else
            kDebug() << "could not open template" << archName;
    }
}

QModelIndexList TemplatesModel::templateIndexes(const QString& fileName)
{
    QFileInfo info(fileName);
    QString description = d->componentData.dirs()->findResource(d->descriptionResourceType, info.baseName() + ".kdevtemplate");
    if (description.isEmpty())
    {
        description = d->componentData.dirs()->findResource(d->descriptionResourceType, info.baseName() + ".desktop");
    }

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
            indexes << d->templateItems[levels.join(QString('/'))]->index();
        }

        if (!indexes.isEmpty())
        {
            QString name = general.readEntry("Name");
            QStandardItem* categoryItem = d->templateItems[levels.join(QString('/'))];
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

QByteArray TemplatesModel::descriptionResourceType() const
{
    return d->descriptionResourceType;
}

void TemplatesModel::setDescriptionResourceType (const QByteArray& type)
{
    d->descriptionResourceType = type;
}

QByteArray TemplatesModel::templateResourceType() const
{
    return d->templateResourceType;
}

void TemplatesModel::setTemplateResourceType (const QByteArray& type)
{
    d->templateResourceType = type;
}

KComponentData TemplatesModel::componentData()
{
    return d->componentData;
}

QString TemplatesModel::loadTemplateFile(const QString& fileName)
{
    QString saveLocation = componentData().dirs()->saveLocation("apptemplates");
    QFileInfo info(fileName);
    QString destination = saveLocation + info.baseName();

    KMimeType::Ptr mimeType = KMimeType::findByUrl(KUrl(fileName));
    kDebug() << "Loaded file" << fileName << "with type" << mimeType->name();

    if (mimeType->name() == "application/x-desktop")
    {
        kDebug() << "Loaded desktop file" << info.absoluteFilePath() << ", compressing";
#ifdef Q_WS_WIN
        destination += ".zip";
        KZip archive(destination);
#else
        destination += ".tar.bz2";
        KTar archive(destination, "application/x-bzip");
#endif //Q_WS_WIN

        archive.open(QIODevice::WriteOnly);

        QDir dir(info.absoluteDir());
        QDir::Filters filter = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
        foreach (const QFileInfo& entry, dir.entryInfoList(filter))
        {
            if (entry.isFile())
            {
                archive.addLocalFile(entry.absoluteFilePath(), entry.fileName());
            }
            else if (entry.isDir())
            {
                archive.addLocalDirectory(entry.absoluteFilePath(), entry.fileName());
            }
        }
        archive.close();
    }
    else
    {
        kDebug() << "Copying" << fileName << "to" << saveLocation;
        QFile::copy(fileName, destination);
    }

    refresh();

    return destination;
}

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

#include "templatepreviewicon.h"
#include <debug.h>
#include <interfaces/icore.h>

#include <KConfig>
#include <KTar>
#include <KZip>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QMimeType>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>

using namespace KDevelop;

class KDevelop::TemplatesModelPrivate
{
public:
    explicit TemplatesModelPrivate(const QString& typePrefix);

    QString typePrefix;

    QStringList searchPaths;

    QMap<QString, QStandardItem*> templateItems;

    /**
     * Extracts description files from all available template archives and saves them to a location
     * determined by descriptionResourceSuffix().
     **/
    void extractTemplateDescriptions();

    /**
     * Creates a model item for the template @p name in category @p category
     *
     * @param name the name of the new template
     * @param category the category of the new template
     * @param parent the parent item
     * @return the created item
     **/
    QStandardItem *createItem(const QString& name, const QString& category, QStandardItem* parent);

    enum ResourceType
    {
        Description,
        Template,
        Preview
    };
    QString resourceFilter(ResourceType type, const QString &suffix = QString())
    {
        QString filter = typePrefix;
        switch(type) {
            case Description:
                filter += QLatin1String("template_descriptions/");
                break;
            case Template:
                filter += QLatin1String("templates/");
                break;
            case Preview:
                filter += QLatin1String("template_previews/");
                break;
        }
        return filter + suffix;
    }
};

TemplatesModelPrivate::TemplatesModelPrivate(const QString& _typePrefix)
: typePrefix(_typePrefix)
{
    if (!typePrefix.endsWith('/')) {
        typePrefix.append('/');
    }
}

TemplatesModel::TemplatesModel(const QString& typePrefix, QObject* parent)
: QStandardItemModel(parent)
, d(new TemplatesModelPrivate(typePrefix))
{
}

TemplatesModel::~TemplatesModel() = default;

void TemplatesModel::refresh()
{
    clear();
    d->templateItems.clear();
    d->templateItems[QString()] = invisibleRootItem();
    d->extractTemplateDescriptions();

    QStringList templateArchives;
    foreach(const QString& archivePath, d->searchPaths) {
        const QStringList files = QDir(archivePath).entryList(QDir::Files);
        foreach(const QString& file, files) {
            templateArchives.append(archivePath + file);
        }
    }

    QStringList templateDescriptions;
    const QStringList templatePaths = {QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +'/'+ d->resourceFilter(TemplatesModelPrivate::Description)};
    foreach(const QString& templateDescription, templatePaths) {
        const QStringList files = QDir(templateDescription).entryList(QDir::Files);
        foreach(const QString& file, files) {
            templateDescriptions.append(templateDescription + file);
        }
    }

    foreach (const QString &templateDescription, templateDescriptions) {
        QFileInfo fi(templateDescription);
        bool archiveFound = false;
        foreach( const QString& templateArchive, templateArchives ) {
            if( QFileInfo(templateArchive).baseName() == fi.baseName() ) {
                archiveFound = true;

                KConfig templateConfig(templateDescription);
                KConfigGroup general(&templateConfig, "General");
                QString name = general.readEntry("Name");
                QString category = general.readEntry("Category");
                QString comment = general.readEntry("Comment");
                TemplatePreviewIcon icon(general.readEntry("Icon"), templateArchive, d->resourceFilter(TemplatesModelPrivate::Preview));

                QStandardItem *templateItem = d->createItem(name, category, invisibleRootItem());
                templateItem->setData(templateDescription, DescriptionFileRole);
                templateItem->setData(templateArchive, ArchiveFileRole);
                templateItem->setData(comment, CommentRole);
                templateItem->setData(QVariant::fromValue<TemplatePreviewIcon>(icon), PreviewIconRole);
            }
        }

        if (!archiveFound)
        {
            // Template file doesn't exist anymore, so remove the description
            // saves us the extra lookups for templateExists on the next run
            QFile(templateDescription).remove();
        }
    }
}

QStandardItem *TemplatesModelPrivate::createItem(const QString& name, const QString& category, QStandardItem* parent)
{
    QStringList path = category.split('/');

    QStringList currentPath;
    foreach (const QString& entry, path)
    {
        currentPath << entry;
        if (!templateItems.contains(currentPath.join(QLatin1Char('/')))) {
            QStandardItem *item = new QStandardItem(entry);
            item->setEditable(false);
            parent->appendRow(item);
            templateItems[currentPath.join(QLatin1Char('/'))] = item;
            parent = item;
        } else {
            parent = templateItems[currentPath.join(QLatin1Char('/'))];
        }
    }

    QStandardItem *templateItem = new QStandardItem(name);
    templateItem->setEditable(false);
    parent->appendRow(templateItem);
    return templateItem;
}

void TemplatesModelPrivate::extractTemplateDescriptions()
{
    QStringList templateArchives;
    searchPaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, resourceFilter(Template), QStandardPaths::LocateDirectory);
    searchPaths.removeDuplicates();
    foreach(const QString &archivePath, searchPaths) {
        const QStringList files = QDir(archivePath).entryList(QDir::Files);
        foreach(const QString& file, files) {
            if(file.endsWith(QLatin1String(".zip")) || file.endsWith(QLatin1String(".tar.bz2"))) {
                QString archfile = archivePath + file;
                templateArchives.append(archfile);
            }
        }
    }

    QString localDescriptionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +'/'+ resourceFilter(Description);

    QDir dir(localDescriptionsDir);
    if(!dir.exists())
        dir.mkpath(QStringLiteral("."));

    foreach (const QString &archName, templateArchives)
    {
        qCDebug(LANGUAGE) << "processing template" << archName;

        QScopedPointer<KArchive> templateArchive;
        if (QFileInfo(archName).completeSuffix() == QLatin1String("zip"))
        {
            templateArchive.reset(new KZip(archName));
        }
        else
        {
            templateArchive.reset(new KTar(archName));
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
            QString suffix = QStringLiteral(".kdevtemplate");
            const KArchiveEntry *templateEntry =
                templateArchive->directory()->entry(templateInfo.baseName() + suffix);

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
                    if (entryName.endsWith(suffix)) {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                suffix = QStringLiteral(".desktop");
                templateEntry = templateArchive->directory()->entry(templateInfo.baseName() + suffix);
            }

            if (!templateEntry || !templateEntry->isFile())
            {
                foreach (const QString& entryName, templateArchive->directory()->entries())
                {
                    if (entryName.endsWith(suffix)) {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }
            if (!templateEntry || !templateEntry->isFile())
            {
                qCDebug(LANGUAGE) << "template" << archName << "does not contain .kdevtemplate or .desktop file";
                continue;
            }
            const KArchiveFile *templateFile = static_cast<const KArchiveFile*>(templateEntry);

            qCDebug(LANGUAGE) << "copy template description to" << localDescriptionsDir;
            const QString descriptionFileName = templateInfo.baseName() + suffix;
            if (templateFile->name() == descriptionFileName) {
                templateFile->copyTo(localDescriptionsDir);
            } else {
                // Rename the extracted description
                // so that its basename matches the basename of the template archive
                // Use temporary dir to not overwrite other files with same name
                QTemporaryDir dir;
                templateFile->copyTo(dir.path());
                const QString destinationPath = localDescriptionsDir + descriptionFileName;
                QFile::remove(destinationPath);
                QFile::rename(dir.path() + QLatin1Char('/') + templateFile->name(), destinationPath);
            }
        }
        else
        {
            qCWarning(LANGUAGE) << "could not open template" << archName;
        }
    }
}

QModelIndexList TemplatesModel::templateIndexes(const QString& fileName) const
{
    QFileInfo info(fileName);
    QString description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, d->resourceFilter(TemplatesModelPrivate::Description, info.baseName() + ".kdevtemplate"));
    if (description.isEmpty())
    {
        description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, d->resourceFilter(TemplatesModelPrivate::Description, info.baseName() + ".desktop"));
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

QString TemplatesModel::typePrefix() const
{
    return d->typePrefix;
}

void TemplatesModel::addDataPath(const QString& path)
{
    QString realpath = path + d->resourceFilter(TemplatesModelPrivate::Template);
    d->searchPaths.append(realpath);
}

QString TemplatesModel::loadTemplateFile(const QString& fileName)
{
    QString saveLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +'/'+ d->resourceFilter(TemplatesModelPrivate::Template);

    QDir dir(saveLocation);
    if(!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QFileInfo info(fileName);
    QString destination = saveLocation + info.baseName();

    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(fileName);
    qCDebug(LANGUAGE) << "Loaded file" << fileName << "with type" << mimeType.name();

    if (mimeType.name() == QLatin1String("application/x-desktop"))
    {
        qCDebug(LANGUAGE) << "Loaded desktop file" << info.absoluteFilePath() << ", compressing";
#ifdef Q_WS_WIN
        destination += ".zip";
        KZip archive(destination);
#else
        destination += QLatin1String(".tar.bz2");
        KTar archive(destination, QStringLiteral("application/x-bzip"));
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
        qCDebug(LANGUAGE) << "Copying" << fileName << "to" << saveLocation;
        QFile::copy(fileName, saveLocation + info.fileName());
    }

    refresh();

    return destination;
}

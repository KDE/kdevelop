/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

namespace {
/**
 * Invoke a given callback for each path segment and the cumulative path that ends with it of a given path.
 *
 * @param path a path to iterate over
 * @param callback a function (object) that takes two arguments:
 *        QStringView @c segment, @c const QString& @c cumulativePath
 */
template<typename SegmentAndCumulativePathUser>
void forEachPathSegment(const QString& path, SegmentAndCumulativePathUser callback)
{
    constexpr QLatin1Char pathSeparator{'/'};
    QString cumulativePath;
    cumulativePath.reserve(path.size());

    for (const auto segment : QStringTokenizer(path, pathSeparator)) {
        if (!cumulativePath.isEmpty()) {
            cumulativePath += pathSeparator;
        }
        cumulativePath += segment;
        callback(segment, std::as_const(cumulativePath));
    }
    Q_ASSERT(cumulativePath == path);
}

[[nodiscard]] QStandardItem* createTemplateItem(const QString& name, QStandardItem* parent)
{
    auto* const templateItem = new QStandardItem(name);
    templateItem->setEditable(false);
    parent->appendRow(templateItem);
    return templateItem;
}

} // unnamed namespace

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
    QStandardItem* createItem(const QString& name, const QString& category, QStandardItem* parent);

    enum ResourceType
    {
        Description,
        Template,
        Preview
    };
    QString resourceFilter(ResourceType type, const QString& suffix = QString()) const
    {
        QString filter = typePrefix;
        switch (type) {
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
    if (!typePrefix.endsWith(QLatin1Char('/'))) {
        typePrefix.append(QLatin1Char('/'));
    }
}

TemplatesModel::TemplatesModel(const QString& typePrefix, QObject* parent)
    : QStandardItemModel(parent)
    , d_ptr(new TemplatesModelPrivate(typePrefix))
{
}

TemplatesModel::~TemplatesModel() = default;

void TemplatesModel::refresh()
{
    Q_D(TemplatesModel);

    clear();
    d->templateItems.clear();
    d->templateItems[QString()] = invisibleRootItem();
    d->extractTemplateDescriptions();

    QStringList templateArchives;
    for (const QString& archivePath : std::as_const(d->searchPaths)) {
        const QStringList files = QDir(archivePath).entryList(QDir::Files);
        for (const QString& file : files) {
            templateArchives.append(archivePath + file);
        }
    }

    QStringList templateDescriptions;
    const QStringList templatePaths =
    {QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->resourceFilter(
         TemplatesModelPrivate::Description)};
    for (const QString& templateDescription : templatePaths) {
        const QStringList files = QDir(templateDescription).entryList(QDir::Files);
        for (const QString& file : files) {
            templateDescriptions.append(templateDescription + file);
        }
    }

    for (const QString& templateDescription : std::as_const(templateDescriptions)) {
        QFileInfo fi(templateDescription);
        bool archiveFound = false;
        for (const QString& templateArchive : std::as_const(templateArchives)) {
            if (QFileInfo(templateArchive).baseName() == fi.baseName()) {
                archiveFound = true;

                KConfig templateConfig(templateDescription);
                KConfigGroup general(&templateConfig, QStringLiteral("General"));
                QString name = general.readEntry("Name");
                QString category = general.readEntry("Category");
                QString comment = general.readEntry("Comment");
                TemplatePreviewIcon icon(general.readEntry("Icon"), templateArchive, d->resourceFilter(
                        TemplatesModelPrivate::Preview));

                QStandardItem* templateItem = d->createItem(name, category, invisibleRootItem());
                templateItem->setData(templateDescription, DescriptionFileRole);
                templateItem->setData(templateArchive, ArchiveFileRole);
                templateItem->setData(comment, CommentRole);
                templateItem->setData(QVariant::fromValue<TemplatePreviewIcon>(icon), PreviewIconRole);
            }
        }

        if (!archiveFound) {
            // Template file doesn't exist anymore, so remove the description
            // saves us the extra lookups for templateExists on the next run
            QFile(templateDescription).remove();
        }
    }
}

QStandardItem* TemplatesModelPrivate::createItem(const QString& name, const QString& category, QStandardItem* parent)
{
    forEachPathSegment(category, [&parent, this](QStringView segment, const QString& cumulativePath) {
        auto it = templateItems.constFind(cumulativePath);
        if (it == templateItems.cend()) {
            // clazy:exclude-next-line=strict-iterators
            it = templateItems.insert(cumulativePath, createTemplateItem(segment.toString(), parent));
        }
        parent = *it;
    });

    return createTemplateItem(name, parent);
}

void TemplatesModelPrivate::extractTemplateDescriptions()
{
    QStringList templateArchives;
    searchPaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, resourceFilter(
                                                 Template), QStandardPaths::LocateDirectory);
    searchPaths.removeDuplicates();
    for (const QString& archivePath : std::as_const(searchPaths)) {
        const QStringList files = QDir(archivePath).entryList(QDir::Files);
        for (const QString& file : files) {
            if (file.endsWith(QLatin1String(".zip")) || file.endsWith(QLatin1String(".tar.bz2"))) {
                QString archfile = archivePath + file;
                templateArchives.append(archfile);
            }
        }
    }

    QString localDescriptionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char(
        '/') + resourceFilter(Description);

    QDir dir(localDescriptionsDir);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    for (const QString& archName : std::as_const(templateArchives)) {
        qCDebug(LANGUAGE) << "processing template" << archName;

        QScopedPointer<KArchive> templateArchive;
        if (QFileInfo(archName).completeSuffix() == QLatin1String("zip")) {
            templateArchive.reset(new KZip(archName));
        } else
        {
            templateArchive.reset(new KTar(archName));
        }

        if (templateArchive->open(QIODevice::ReadOnly)) {
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
            const KArchiveEntry* templateEntry =
                templateArchive->directory()->entry(templateInfo.baseName() + suffix);

            if (!templateEntry || !templateEntry->isFile()) {
                /*
                 * First, if the .kdevtemplate file is not found by name,
                 * we check all the files in the archive for any .kdevtemplate file
                 *
                 * This is needed because kde-files.org renames downloaded files
                 */
                const auto dirEntries = templateArchive->directory()->entries();
                for (const QString& entryName : dirEntries) {
                    if (entryName.endsWith(suffix)) {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }

            if (!templateEntry || !templateEntry->isFile()) {
                suffix = QStringLiteral(".desktop");
                templateEntry = templateArchive->directory()->entry(templateInfo.baseName() + suffix);
            }

            if (!templateEntry || !templateEntry->isFile()) {
                const auto dirEntries = templateArchive->directory()->entries();
                for (const QString& entryName : dirEntries) {
                    if (entryName.endsWith(suffix)) {
                        templateEntry = templateArchive->directory()->entry(entryName);
                        break;
                    }
                }
            }
            if (!templateEntry || !templateEntry->isFile()) {
                qCDebug(LANGUAGE) << "template" << archName << "does not contain .kdevtemplate or .desktop file";
                continue;
            }
            const auto* templateFile = static_cast<const KArchiveFile*>(templateEntry);

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
        } else
        {
            qCWarning(LANGUAGE) << "could not open template" << archName << ':' << templateArchive->errorString();
        }
    }
}

QModelIndexList TemplatesModel::templateIndexes(const QString& fileName) const
{
    Q_D(const TemplatesModel);

    QFileInfo info(fileName);
    QString description =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                               d->resourceFilter(TemplatesModelPrivate::Description,
                                                 info.baseName() + QLatin1String(".kdevtemplate")));
    if (description.isEmpty()) {
        description =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                   d->resourceFilter(TemplatesModelPrivate::Description,
                                                     info.baseName() + QLatin1String(".desktop")));
    }

    QModelIndexList indexes;

    if (!description.isEmpty()) {
        KConfig templateConfig(description);
        KConfigGroup general(&templateConfig, QStringLiteral("General"));
        const auto category = general.readEntry("Category");

        forEachPathSegment(category,
                           [&indexes, d]([[maybe_unused]] QStringView segment, const QString& cumulativePath) {
                               indexes << d->templateItems[cumulativePath]->index();
                           });

        if (!indexes.isEmpty()) {
            QString name = general.readEntry("Name");
            const auto* const categoryItem = d->templateItems[category];
            for (int i = 0; i < categoryItem->rowCount(); ++i) {
                QStandardItem* templateItem = categoryItem->child(i);
                if (templateItem->text() == name) {
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
    Q_D(const TemplatesModel);

    return d->typePrefix;
}

void TemplatesModel::addDataPath(const QString& path)
{
    Q_D(TemplatesModel);

    QString realpath = path + d->resourceFilter(TemplatesModelPrivate::Template);
    d->searchPaths.append(realpath);
}

QString TemplatesModel::loadTemplateFile(const QString& fileName)
{
    Q_D(TemplatesModel);

    QString saveLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') +
                           d->resourceFilter(TemplatesModelPrivate::Template);

    QDir dir(saveLocation);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QFileInfo info(fileName);
    QString destination = saveLocation + info.baseName();

    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(fileName);
    qCDebug(LANGUAGE) << "Loaded file" << fileName << "with type" << mimeType.name();

    if (mimeType.name() == QLatin1String("application/x-desktop")) {
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
        const auto dirEntryInfos = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& entry : dirEntryInfos) {
            if (entry.isFile()) {
                archive.addLocalFile(entry.absoluteFilePath(), entry.fileName());
            } else if (entry.isDir()) {
                archive.addLocalDirectory(entry.absoluteFilePath(), entry.fileName());
            }
        }

        archive.close();
    } else
    {
        qCDebug(LANGUAGE) << "Copying" << fileName << "to" << saveLocation;
        QFile::copy(fileName, saveLocation + info.fileName());
    }

    return destination;
}

#include "moc_templatesmodel.cpp"

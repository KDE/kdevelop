/*
    SPDX-FileCopyrightText: 2010-2012, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetselectionmanager.h"

// plugin
#include "checksetselectionlock.h"
#include "debug.h"
// KF
#include <KConfigGroup>
#include <KConfig>
#include <KDirWatch>
// Qt
#include <QFileInfo>
#include <QDir>
#include <QUuid>

namespace ClangTidy {

QStringList checkSetSelectionFileNameFilter() { return QStringList { QStringLiteral("*.kdevctcs"), QStringLiteral("*.kdevlock") }; }
inline QLatin1String checkSetSelectionFileSuffix() { return QLatin1String(".kdevctcs"); }
inline QLatin1String checkSetSelectionDirSubPath() { return QLatin1String("/kdevclangtidy/checksetselections"); }
inline QLatin1String defaultCheckSetSelectionFileSubPath() { return QLatin1String("/kdevclangtidy/defaultchecksetselection"); }


QVector<QString> lockedCheckSetSelectionIds(const CheckSetSelectionFileInfoLookup& checkSetSelectionFileInfoLookup)
{
    QVector<QString> result;

    for (auto it = checkSetSelectionFileInfoLookup.constBegin(), end = checkSetSelectionFileInfoLookup.constEnd();
         it != end; ++it) {
        if (it.value().isLocked()) {
            result.append(it.key());
        }
    }

    return result;
}

void updateLockStatus(CheckSetSelectionFileInfoLookup& checkSetSelectionFileInfoLookup,
                      const QVector<QString>& lockedCheckSetSelectionIds,
                      const QVector<QString>& unlockedCheckSetSelectionIds)
{
    if (lockedCheckSetSelectionIds.isEmpty() && unlockedCheckSetSelectionIds.isEmpty()) {
        return;
    }

    for (auto it = checkSetSelectionFileInfoLookup.begin(), end = checkSetSelectionFileInfoLookup.end();
         it != end; ++it) {
        bool isLocked;

        if (lockedCheckSetSelectionIds.contains(it.key())) {
            isLocked = true;
        } else if (unlockedCheckSetSelectionIds.contains(it.key())) {
            isLocked = false;
        } else {
            continue;
        }

        it.value().setLocked(isLocked);
    }
}

QString defaultCheckSetSelectionFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + defaultCheckSetSelectionFileSubPath();
}

QString checkSetSelectionFilePath(const QString& checkSetSelectionId)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + checkSetSelectionDirSubPath() + QLatin1Char('/') + checkSetSelectionId + checkSetSelectionFileSuffix();
}

QString checkSetSelectionFileName(const QString& checkSetSelectionId)
{
    return checkSetSelectionId + checkSetSelectionFileSuffix();
}

// TODO: add global lock
// TODO: make calls async
// TODO: only load checkset setlections on demand
CheckSetSelectionManager::CheckSetSelectionManager()
    : m_checkSetSelectionFileWatcher(new KDirWatch(this))
{
    connect(m_checkSetSelectionFileWatcher, &KDirWatch::dirty,
            this, &CheckSetSelectionManager::onCheckSetSelectionsFolderChanged);

    // get all folder where checkSetSelections could be stored
    const QStringList dataFolderPaths =
        QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const QString& dataFolderPath : dataFolderPaths) {
        const QString checkSetSelectionFolderPath = dataFolderPath + checkSetSelectionDirSubPath();
        // watch folder for changes
        m_checkSetSelectionFileWatcher->addDir(checkSetSelectionFolderPath, KDirWatch::WatchDirOnly);

        // read current files
        onCheckSetSelectionsFolderChanged(checkSetSelectionFolderPath);
    }

    // default checkset selection
    // While there is no proper config syncing offer in the used frameworks, use a
    // single file with the id as content as workaround and watch for it changing
    auto* defaultCheckSetSelectionWatcher = new KDirWatch(this);
    connect(defaultCheckSetSelectionWatcher, &KDirWatch::created,
            this, &CheckSetSelectionManager::onDefaultCheckSetSelectionChanged);
    connect(defaultCheckSetSelectionWatcher, &KDirWatch::dirty,
            this, &CheckSetSelectionManager::onDefaultCheckSetSelectionChanged);
    const QString _defaultCheckSetSelectionFilePath = defaultCheckSetSelectionFilePath();

    defaultCheckSetSelectionWatcher->addFile(_defaultCheckSetSelectionFilePath);

    onDefaultCheckSetSelectionChanged(_defaultCheckSetSelectionFilePath);

    // report any problems with existing checkset selections?
}

CheckSetSelectionManager::~CheckSetSelectionManager() = default;

int CheckSetSelectionManager::checkSetSelectionsCount() const
{
    return m_checkSetSelections.count();
}

QVector<CheckSetSelection> CheckSetSelectionManager::checkSetSelections() const
{
    return m_checkSetSelections;
}

CheckSetSelection CheckSetSelectionManager::checkSetSelection(const QString& checkSetSelectionId) const
{
    CheckSetSelection result;

    for (const CheckSetSelection& checkSetSelection : m_checkSetSelections) {
        if (checkSetSelection.id() == checkSetSelectionId) {
            result = checkSetSelection;
            break;
        }
    }

    return result;
}

QString CheckSetSelectionManager::defaultCheckSetSelectionId() const
{
    return m_defaultCheckSetSelectionId;
}

CheckSetSelection CheckSetSelectionManager::defaultCheckSetSelection() const
{
    return checkSetSelection(m_defaultCheckSetSelectionId);
}

bool CheckSetSelectionManager::isCheckSetSelectionLocked(const QString& checkSetSelectionId) const
{
    bool result = false;

    // search in all folders for the info
    for (const CheckSetSelectionFileInfoLookup& checkSetSelectionFileInfoLookup : m_checkSetSelectionFileInfoLookupPerFolder) {
        CheckSetSelectionFileInfoLookup::ConstIterator it =
            checkSetSelectionFileInfoLookup.find(checkSetSelectionId);
        if (it != checkSetSelectionFileInfoLookup.constEnd()) {
            result = it->isLocked();
            break;
        }
    }

    return result;
}

void CheckSetSelectionManager::saveCheckSetSelections(QVector<CheckSetSelection>& checkSetSelections)
{
    // TODO: do not save if locked by someone else -> needs passing of our lock? or just registering our own and check?
    // create and set unique id
    std::for_each(checkSetSelections.begin(), checkSetSelections.end(), [this](CheckSetSelection& checkSetSelection) {
        const QString checkSetSelectionId = checkSetSelection.id();

        bool needsId = true;
        if (!checkSetSelectionId.isEmpty()) {
            // already existing?
            auto hasCheckSetSelectionId = [&checkSetSelectionId] (const CheckSetSelection& existingProfile) {
                return (checkSetSelectionId == existingProfile.id());
            };
            if (std::any_of(m_checkSetSelections.constBegin(), m_checkSetSelections.constEnd(), hasCheckSetSelectionId)) {
                needsId = false;
            }
        }

        // set new uuid for non-existing
        if (needsId) {
            checkSetSelection.setId(QUuid::createUuid().toString());
        }

        saveCheckSetSelection(checkSetSelection);
    });
}

void CheckSetSelectionManager::removeCheckSetSelections(const QVector<QString>& checkSetSelectionIds)
{
    for (const QString& checkSetSelectionId : checkSetSelectionIds) {
        removeCheckSetSelection(checkSetSelectionId);
    }
}

void CheckSetSelectionManager::setDefaultCheckSetSelection(const QString& checkSetSelectionId)
{
    QFile defaultCheckSetSelectionFile(defaultCheckSetSelectionFilePath());
    defaultCheckSetSelectionFile.open(QIODevice::WriteOnly);

    defaultCheckSetSelectionFile.write(checkSetSelectionId.toUtf8());
    defaultCheckSetSelectionFile.close();
}

CheckSetSelectionLock CheckSetSelectionManager::createLock(const QString& checkSetSelectionId)
{
    const QString checkSetSelectionFilePath = filePathOfCheckSetSelection(checkSetSelectionId);

    return CheckSetSelectionLock(checkSetSelectionFilePath, checkSetSelectionId);
}

CheckSetSelection CheckSetSelectionManager::loadCheckSetSelection(const QString& absoluteFilePath) const
{
    CheckSetSelection result;

    KConfig configFile(absoluteFilePath, KConfig::SimpleConfig);

    // check version
    KConfigGroup formatConfigGroup = configFile.group(QStringLiteral("KDEVCTCS"));
    const QString formatVersion = formatConfigGroup.readEntry("Version");
    if (!formatVersion.startsWith(QLatin1String("1."))) {
        return result;
    }

    result.setId(QFileInfo(absoluteFilePath).baseName());

    KConfigGroup generalConfigGroup = configFile.group(QStringLiteral("General"));
    result.setName(generalConfigGroup.readEntry("Name"));

    KConfigGroup layoutConfigGroup = configFile.group(QStringLiteral("Checks"));
    result.setSelection(layoutConfigGroup.readEntry("Selection", QString()));

    return result;
}

void CheckSetSelectionManager::saveCheckSetSelection(const CheckSetSelection& checkSetSelection) const
{
    const QString fileName = checkSetSelectionFilePath(checkSetSelection.id());
    KConfig configFile(fileName, KConfig::SimpleConfig);

    KConfigGroup formatConfigGroup = configFile.group(QStringLiteral("KDEVCTCS"));
    formatConfigGroup.writeEntry("Version", "1.0");

    KConfigGroup generalConfigGroup = configFile.group(QStringLiteral("General"));
    generalConfigGroup.writeEntry("Name", checkSetSelection.name());

    KConfigGroup layoutConfigGroup = configFile.group(QStringLiteral("Checks"));
    layoutConfigGroup.writeEntry("Selection", checkSetSelection.selectionAsString());
}

void CheckSetSelectionManager::removeCheckSetSelection(const QString& checkSetSelectionId)
{
    const QString filePath = filePathOfCheckSetSelection(checkSetSelectionId);
    if (!filePath.isEmpty()) {
        QFile::remove(filePath);
    }
}

QString CheckSetSelectionManager::filePathOfCheckSetSelection(const QString& checkSetSelectionId) const
{
    QString result;

    for (QHash<QString, CheckSetSelectionFileInfoLookup>::ConstIterator foldersIt =
             m_checkSetSelectionFileInfoLookupPerFolder.constBegin();
         foldersIt != m_checkSetSelectionFileInfoLookupPerFolder.constEnd() && result.isEmpty();
         ++foldersIt) {
        const CheckSetSelectionFileInfoLookup& fileInfoList = foldersIt.value();
        for (CheckSetSelectionFileInfoLookup::ConstIterator folderIt = fileInfoList.constBegin();
             folderIt != fileInfoList.constEnd();
             ++folderIt) {
            if (folderIt.key() == checkSetSelectionId) {
                result = foldersIt.key() + QLatin1Char('/') + checkSetSelectionFileName(checkSetSelectionId);
                break;
            }
        }
    }

    return result;
}

void CheckSetSelectionManager::onCheckSetSelectionsFolderChanged(const QString& checkSetSelectionFolderPath)
{
    CheckSetSelectionFileInfoLookup& checkSetSelectionFileInfoLookup =
        m_checkSetSelectionFileInfoLookupPerFolder[checkSetSelectionFolderPath];

    // TODO: reparse for new, removed and changed files
    // assume all are removed and unlocked in the beginning
    QVector<QString> removedCheckSetSelectionIds = checkSetSelectionFileInfoLookup.keys().toVector();
    QVector<CheckSetSelection> newCheckSetSelections;
    QVector<CheckSetSelection> changedCheckSetSelections;

    QVector<QString> newUnlockedCheckSetSelectionIds = lockedCheckSetSelectionIds(checkSetSelectionFileInfoLookup);
    QVector<QString> newLockedCheckSetSelectionIds;
    // iterate all files in folder
    const QFileInfoList checkSetSelectionFileInfoList =
        QDir(checkSetSelectionFolderPath).entryInfoList(checkSetSelectionFileNameFilter(), QDir::Files);

    for (const QFileInfo& checkSetSelectionFileInfo : checkSetSelectionFileInfoList) {
        // a lock file ?
        if (checkSetSelectionFileInfo.suffix() == QLatin1String("kdevlock")) {
            const QString lockedCheckSetSelectionId = checkSetSelectionFileInfo.baseName();
            // if not in old locks, is a new lock
            if (!newUnlockedCheckSetSelectionIds.removeOne(lockedCheckSetSelectionId)) {
                newLockedCheckSetSelectionIds.append(lockedCheckSetSelectionId);
            }
            continue;
        }

        // not a checkset file ?
        if (checkSetSelectionFileInfo.suffix() != QLatin1String("kdevctcs")) {
            continue;
        }

        // all other files assumed to be checkSetSelection files
        const QString checkSetSelectionId = checkSetSelectionFileInfo.baseName();
        // load file
        const CheckSetSelection checkSetSelection = loadCheckSetSelection(checkSetSelectionFileInfo.absoluteFilePath());
        // loading failed? Treat as not existing
        if (checkSetSelection.id().isEmpty()) {
            continue;
        }

        const CheckSetSelectionFileInfoLookup::Iterator infoIt =
            checkSetSelectionFileInfoLookup.find(checkSetSelectionId);
        const bool isKnown = (infoIt != checkSetSelectionFileInfoLookup.end());
        const QDateTime fileInfoLastModified = checkSetSelectionFileInfo.lastModified();
        // is known?
        if (isKnown) {
            removedCheckSetSelectionIds.removeOne(checkSetSelectionId);

            // check timestamp
            if (fileInfoLastModified == infoIt->lastModified()) {
                continue;
            }

            // update timestamp
            infoIt->setLastModified(fileInfoLastModified);
        } else {
            CheckSetSelectionFileInfo info(fileInfoLastModified, false);
            checkSetSelectionFileInfoLookup.insert(checkSetSelectionId, info);
        }

        if (isKnown) {
            auto it = std::find_if(m_checkSetSelections.begin(), m_checkSetSelections.end(),
                                   [&checkSetSelectionId](const CheckSetSelection& existingProfile) {
                return (existingProfile.id() == checkSetSelectionId);
            });
            if (it != m_checkSetSelections.end()) {
                *it = checkSetSelection;
            }
        } else {
            newCheckSetSelections.append(checkSetSelection);
        }
        changedCheckSetSelections.append(checkSetSelection);
    }

    // remove all removed checksets
    {
        auto isProfileToRemove = [&removedCheckSetSelectionIds](const CheckSetSelection& selection) {
            return removedCheckSetSelectionIds.contains(selection.id());
        };
        m_checkSetSelections.erase(std::remove_if(m_checkSetSelections.begin(), m_checkSetSelections.end(), isProfileToRemove),
                            m_checkSetSelections.end());
    }

    for (const QString& checkSetSelectionId : std::as_const(removedCheckSetSelectionIds)) {
        checkSetSelectionFileInfoLookup.remove(checkSetSelectionId);
        if (checkSetSelectionId == m_defaultCheckSetSelectionId) {
            m_defaultCheckSetSelectionId.clear();
        }
        // TODO: how to select new one?
    }

    // add new checksets
    m_checkSetSelections.append(newCheckSetSelections);
    // if there was no default checkset before, set default to first
    const bool isDefaultCheckSetSelectionChanged = (m_defaultCheckSetSelectionId.isEmpty() && !m_checkSetSelections.isEmpty());
    if (isDefaultCheckSetSelectionChanged) {
        m_defaultCheckSetSelectionId = m_checkSetSelections.at(0).id();
    }

    // update lock info
    updateLockStatus(checkSetSelectionFileInfoLookup, newLockedCheckSetSelectionIds, newUnlockedCheckSetSelectionIds);

    // signal changes
    if (!changedCheckSetSelections.isEmpty()) {
        emit checkSetSelectionsChanged(changedCheckSetSelections);
    }
    if (!removedCheckSetSelectionIds.isEmpty()) {
        emit checkSetSelectionsRemoved(removedCheckSetSelectionIds);
    }

    if (!newUnlockedCheckSetSelectionIds.isEmpty()) {
        emit checkSetSelectionsUnlocked(newUnlockedCheckSetSelectionIds);
    }
    if (!newLockedCheckSetSelectionIds.isEmpty()) {
        emit checkSetSelectionsLocked(newLockedCheckSetSelectionIds);
    }
    if (isDefaultCheckSetSelectionChanged) {
        emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
    }
}

void CheckSetSelectionManager::onDefaultCheckSetSelectionChanged(const QString& path)
{
    QFile defaultCheckSetSelectionFile(path);
    if (!defaultCheckSetSelectionFile.open(QIODevice::ReadOnly)) {
        qCDebug(KDEV_CLANGTIDY) << "Failed to open checkset selection file " << path;
        return;
    }

    const QByteArray fileContent = defaultCheckSetSelectionFile.readAll();
    const QString checkSetSelectionId = QString::fromUtf8(fileContent);
    defaultCheckSetSelectionFile.close();

    // no id set?
    if (checkSetSelectionId.isEmpty()) {
        return;
    }

    // no change?
    if (m_defaultCheckSetSelectionId == checkSetSelectionId) {
        return;
    }

    bool isExisting = false;
    for (const CheckSetSelection& checkSetSelection : std::as_const(m_checkSetSelections)) {
        if (checkSetSelection.id() == checkSetSelectionId) {
            isExisting = true;
            break;
        }
    }

    if (isExisting) {
        m_defaultCheckSetSelectionId = checkSetSelectionId;
        emit defaultCheckSetSelectionChanged(m_defaultCheckSetSelectionId);
    }
}

}

#include "moc_checksetselectionmanager.cpp"

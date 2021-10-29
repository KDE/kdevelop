/*
    SPDX-FileCopyrightText: 2010-2012, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetselectionlock.h"

// plugin
#include <debug.h>
// Qt
#include <QLockFile>
#include <QSharedPointer>
#include <QString>

namespace Clazy {

class CheckSetSelectionLockPrivate : public QSharedData
{
public:
    CheckSetSelectionLockPrivate(const QString& fileName,
                                 const QString& checkSetSelectionId);

public:
    QSharedPointer<QLockFile> lockFile;
    QString checkSetSelectionId;
};


static QString checkSetSelectionFileLockPath(const QString& checkSetSelectionFilePath)
{
    // TODO: just ".lock" conflicts with KConfig(?) using the same
    return checkSetSelectionFilePath + QLatin1String(".kdevlock");
}

CheckSetSelectionLockPrivate::CheckSetSelectionLockPrivate(const QString& fileName,
                                                           const QString& id)
    : lockFile(new QLockFile(fileName.isEmpty() ? fileName : checkSetSelectionFileLockPath(fileName)))
    , checkSetSelectionId(id)
{
    if (!fileName.isEmpty()) {
        if (!lockFile->tryLock(1000)) {
            qCWarning(KDEV_CLAZY)
                << "Failed to acquire lock file" << fileName
                << "error =" << lockFile->error();
        }
    }
}

CheckSetSelectionLock::CheckSetSelectionLock(const QString& fileName,
                                             const QString& checkSetSelectionId)
    : d(new CheckSetSelectionLockPrivate(fileName, checkSetSelectionId))
{
}

CheckSetSelectionLock::CheckSetSelectionLock(const CheckSetSelectionLock& other) = default;

CheckSetSelectionLock::~CheckSetSelectionLock() = default;

CheckSetSelectionLock& CheckSetSelectionLock::operator=(const CheckSetSelectionLock& other) = default;

void CheckSetSelectionLock::unlock()
{
    d->lockFile->unlock();
}

bool CheckSetSelectionLock::isLocked() const
{
    return d->lockFile->isLocked();
}

QString CheckSetSelectionLock::checkSetSelectionId() const
{
    return d->checkSetSelectionId;
}

}

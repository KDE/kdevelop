/*
 * This file is part of KDevelop
 *
 * Copyright 2010-2012,2020 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "checksetselectionlock.h"

// plugin
#include <debug.h>
// Qt
#include <QLockFile>
#include <QSharedPointer>
#include <QString>

namespace ClangTidy {

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
            qCWarning(KDEV_CLANGTIDY)
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

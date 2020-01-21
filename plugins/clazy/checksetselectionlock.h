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

#ifndef KDEVCLAZY_CHECKSETSELECTIONLOCK_H
#define KDEVCLAZY_CHECKSETSELECTIONLOCK_H

// Qt
#include <QSharedDataPointer>

class QString;

namespace Clazy {

class CheckSetSelectionLockPrivate;

class CheckSetSelectionLock
{
    friend class CheckSetSelectionManager;

protected:
    CheckSetSelectionLock(const QString& fileName,
                          const QString& checkSetSelectionId);

public:
    CheckSetSelectionLock(const CheckSetSelectionLock& other);

    ~CheckSetSelectionLock();

public:
    CheckSetSelectionLock& operator=(const CheckSetSelectionLock& other);

public:
    void unlock();
    bool isLocked() const;
    QString checkSetSelectionId() const;

private:
    QSharedDataPointer<CheckSetSelectionLockPrivate> d;
};

}

#endif

/*
    SPDX-FileCopyrightText: 2010-2012, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

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

#ifndef KDEVCLAZY_CHECKSETSELECTIONMANAGER_H
#define KDEVCLAZY_CHECKSETSELECTIONMANAGER_H

// plugin
#include "checksetselection.h"
// Qt
#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QHash>

class KDirWatch;

namespace Clazy {

class CheckSetSelectionLock;

class CheckSetSelectionFileInfo
{
public:
    CheckSetSelectionFileInfo(const QDateTime& lastModified, bool locked)
        : m_lastModified(lastModified)
        , m_locked(locked)
    {}

public:
    const QDateTime& lastModified() const { return m_lastModified; }
    bool isLocked()                 const { return m_locked; }

    void setLastModified(const QDateTime& lastModified)  { m_lastModified = lastModified; }
    void setLocked(bool isLocked) { m_locked = isLocked; }

private:
    QDateTime m_lastModified;
    bool m_locked;
};


using CheckSetSelectionFileInfoLookup = QHash<QString, CheckSetSelectionFileInfo>;

class CheckSetSelectionManager : public QObject
{
    Q_OBJECT

public:
    CheckSetSelectionManager();
    ~CheckSetSelectionManager() override;

public:
    QVector<CheckSetSelection> checkSetSelections() const;
    int checkSetSelectionsCount() const;
    CheckSetSelection checkSetSelection(const QString& id) const;
    QString defaultCheckSetSelectionId() const;
    CheckSetSelection defaultCheckSetSelection() const;
    bool isCheckSetSelectionLocked(const QString& id) const;

public:
    void saveCheckSetSelections(QVector<CheckSetSelection>& checkSetSelections);
    void removeCheckSetSelections(const QVector<QString>& checkSetSelectionIds);
    void setDefaultCheckSetSelection(const QString& checkSetSelectionId);

    CheckSetSelectionLock createLock(const QString& checkSetSelectionId);

Q_SIGNALS:
    void checkSetSelectionsChanged(const QVector<CheckSetSelection>& checkSetSelections);
    void checkSetSelectionsRemoved(const QVector<QString>& checkSetSelectionIds);
    void defaultCheckSetSelectionChanged(const QString& checkSetSelectionId);
    void checkSetSelectionsLocked(const QVector<QString>& checkSetSelectionIds);
    void checkSetSelectionsUnlocked(const QVector<QString>& checkSetSelectionIds);

private:
    QString filePathOfCheckSetSelection(const QString& checkSetSelectionId) const;
    // Returns the checkSetSelection as loaded from the file with the given fileName.
    // If the loading fails the checkSetSelection has no id set.
    CheckSetSelection loadCheckSetSelection(const QString& fileName) const;
    void saveCheckSetSelection(const CheckSetSelection& checkSetSelection) const;
    void removeCheckSetSelection(const QString& checkSetSelectionId);

private Q_SLOTS:
    void onCheckSetSelectionsFolderChanged(const QString& path);
    void onDefaultCheckSetSelectionChanged(const QString& path);

private:
    QVector<CheckSetSelection> m_checkSetSelections;

    QString m_defaultCheckSetSelectionId;

    KDirWatch* m_checkSetSelectionFileWatcher;

    QHash<QString, CheckSetSelectionFileInfoLookup> m_checkSetSelectionFileInfoLookupPerFolder;
};

}

#endif

/*
    SPDX-FileCopyrightText: 2010-2012, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CHECKSETSELECTIONMANAGER_H
#define CLANGTIDY_CHECKSETSELECTIONMANAGER_H

// plugin
#include "checksetselection.h"
// Qt
#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QHash>

class KDirWatch;

namespace ClangTidy {

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

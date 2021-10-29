/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_CHECKSETSELECTIONLISTMODEL_H
#define KDEVCLAZY_CHECKSETSELECTIONLISTMODEL_H

// plugin
#include "checksetselection.h"
// Qt
#include <QAbstractItemModel>
#include <QVector>
#include <QSet>

namespace Clazy {
class CheckSetSelectionManager;

class CheckSetSelectionListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CheckSetSelectionListModel(CheckSetSelectionManager* checkSetSelectionManager,
                                        QObject* parent = nullptr);
    ~CheckSetSelectionListModel() override;

public: // QAbstractItemModel API
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;

public:
    void reload();
    void store() const;

public:
    bool hasCheckSetSelection(const QString& name) const;
    QString checkSetSelectionId(int row) const;
    QString checkSetSelectionName(int row) const;
    QString checkSetSelectionAsString(int row) const;
    int row(const QString& checkSetSelectionId) const;
    int defaultCheckSetSelectionRow() const;

    /// @return row of added selection
    int addCheckSetSelection(const QString& name);
    int cloneCheckSetSelection(const QString& name, int row);
    void removeCheckSetSelection(int row);
    void setDefaultCheckSetSelection(int row);
    void setName(int row, const QString& name);
    void setSelection(int row, const QString& selection);

Q_SIGNALS:
    void defaultCheckSetSelectionChanged(const QString& checkSetSelectionId);
    void checkSetSelectionChanged(const QString& checkSetSelectionId);

private:
    QString checkSetSelectionId(const QModelIndex& index) const;

private:
    CheckSetSelectionManager* const m_checkSetSelectionManager;

    QVector<CheckSetSelection> m_checkSetSelections;
    QString m_defaultCheckSetSelectionId;

    // tracking changed data
    mutable QVector<QString> m_added;
    mutable QSet<QString> m_edited;
    mutable QVector<QString> m_removed;
    mutable bool m_defaultChanged = false;
};

}

#endif

/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKECOMMANDSCONTENTS_H
#define CMAKECOMMANDSCONTENTS_H

#include <QVector>
#include <QMap>
#include <QAbstractItemModel>

#include "cmakedocumentation.h"

class CMakeCommandsContents : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CMakeCommandsContents(QObject* parent);

    void processOutput(int code);

    int rowCount(const QModelIndex & parent) const override;

    QVariant data(const QModelIndex & index, int role) const override;

    QString descriptionForIdentifier(const QString& id, CMakeDocumentation::Type t) const;
    QVector<QString> names(CMakeDocumentation::Type t) const;
    CMakeDocumentation::Type typeFor(const QString &id) const;
    void showItemAt(const QModelIndex &idx) const;

    QModelIndex parent(const QModelIndex & child) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;

private:
    QMap<QString, CMakeDocumentation::Type> m_typeForName;
    QVector<QVector<QString>> m_namesForType;
};
#endif // CMAKECOMMANDSCONTENTS_H

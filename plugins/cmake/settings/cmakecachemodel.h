/*
    SPDX-FileCopyrightText: 2007-2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKECACHEMODEL_H
#define CMAKECACHEMODEL_H

#include <cmakecommonexport.h>
#include <QStandardItemModel>
#include <QVariant>
#include <QSet>

#include <util/path.h>

/**
* This class parses the CMakeCache.txt files and put it to a Model
* @author Aleix Pol
*/
class CMakeCacheModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        CMakeCacheModel(QObject* parent, const KDevelop::Path &path);
        ~CMakeCacheModel() override {}

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        int internal() const { return m_internalBegin; }
        
        QString value(const QString& name) const;
        bool isAdvanced(int i) const;
        bool isInternal(int i) const;
        
        QList<QModelIndex> persistentIndices() const;
        KDevelop::Path filePath() const;
        void read();
        QVariantMap changedValues() const;

    Q_SIGNALS:
        void valueChanged(const QString& name, const QString& value) const;

    public Q_SLOTS:
        void reset();
        
    private:
        KDevelop::Path m_filePath;
        int m_internalBegin;
        QSet<QString> m_internal;
        QSet<int> m_modifiedRows;
};

#endif

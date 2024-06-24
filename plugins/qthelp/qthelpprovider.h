/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPROVIDER_H
#define QTHELPPROVIDER_H

#include "qthelpproviderabstract.h"

class QtHelpProvider : public QtHelpProviderAbstract
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IDocumentationProvider)
public:
    /**
     * @param collectionFilePath the path to the .qhc cache file
     * @param qchInputFilePath the path to the .qch file that we are parsing
     * @param nameSpace the namespace name for @p qchInputFilePath
     */
    QtHelpProvider(QObject* parent, const QString& collectionFilePath, const QString& qchInputFilePath,
                   const QString& nameSpace, const QString& name, const QString& iconName);

    QIcon icon() const override;
    QString name() const override;
    void setName(const QString& name);
    QString qchInputFilePath() const;
    QString nameSpace() const;
    QString iconName() const;
    void setIconName(const QString& iconName);
private:
    QString m_qchInputFilePath;
    QString m_nameSpace;
    QString m_name;
    QString m_iconName;
};

#endif // QTHELPPROVIDER_H

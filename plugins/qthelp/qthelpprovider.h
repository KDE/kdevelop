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
    QtHelpProvider(QObject* parent, const QString& fileName, const QString& name, const QString& iconName);

    QIcon icon() const override;
    QString name() const override;
    void setName(const QString& name);
    QString fileName() const;
    QString iconName() const;
    void setIconName(const QString& iconName);
private:
    QString m_fileName;
    QString m_name;
    QString m_iconName;
};

#endif // QTHELPPROVIDER_H
